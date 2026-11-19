#!/usr/bin/env python3
"""Compile Lyraform application logic and compare with a separate ConfigResolve oracle."""
import argparse, hashlib, json, pathlib, random, re, subprocess
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--build',type=pathlib.Path,required=True)
p.add_argument('--reference-build',type=pathlib.Path,required=True)
p.add_argument('--output',type=pathlib.Path,required=True)
a=p.parse_args(); root=pathlib.Path(__file__).resolve().parents[2]; here=root/'experiments/configresolve'
build=a.build.resolve(); out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
reference=root.parent/'ConfigResolve'
manifest=json.loads((here/'reference-manifest.json').read_text())
for name,h in manifest['files'].items():
    assert hashlib.sha256((reference/name).read_bytes()).hexdigest()==h, f'reference changed: {name}'
commands=[]
def run(args, *, input=None, accepted=(0,)):
    args=list(map(str,args)); commands.append(args)
    r=subprocess.run(args,input=input,text=True,capture_output=True)
    if r.returncode not in accepted: raise RuntimeError(f'{args}: exit {r.returncode}\n{r.stdout}\n{r.stderr}')
    return r
run(['c++','-std=c++20','-I',reference/'include',here/'reference-pair.cpp',a.reference_build.resolve()/'libconfigresolve.a','-o',out/'reference-pair'])
# Exhaustive small cross-product, followed by deterministic held-out values.
cases=[(x,xp,y,yp,m) for x,y in [(7,19),(19,7),(7,7),(-19,7)] for xp,yp in [(10,30),(30,10),(30,30),(-30,-10)] for m in range(4)]
rng=random.Random(93719)
for _ in range(32):cases.append((rng.randint(-100000,100000),rng.choice([1,3,19]),rng.randint(-100000,100000),rng.choice([1,3,19]),rng.randrange(4)))
oracle=run([out/'reference-pair'],input=''.join(' '.join(map(str,c))+'\n' for c in cases))
expected=list(map(int,oracle.stdout.splitlines()));assert len(expected)==len(cases)
(out/'cases.json').write_text(json.dumps([{'args':c,'expected':e} for c,e in zip(cases,expected)],indent=2)+'\n')
source=(here/'scalar-resolution.flow').read_text(); base=source[:source.index('\nmain {')]
# The assertions are test-only source; the application function is copied unchanged.
def testcase_source(name,answers,rename=False):
    body=base
    if rename:body=body.replace('configresolve_scalar_probe','unrelated_two_candidate_probe').replace('resolve_pair','choose_integer_candidate')
    fun='choose_integer_candidate' if rename else 'resolve_pair'
    lines=[body,'main {','    mismatch : int32(1)','    passed : int32(0)']
    for i,(case,answer) in enumerate(zip(cases,answers)):
        for k,v in zip(['a','ap','b','bp','mode'],case):lines.append(f'    {k}{i} : int32({v})')
        lines += [f'    got{i} : int32(0)',f'    want{i} : int32({answer})',f'    {fun}(a{i}, ap{i}, b{i}, bp{i}, mode{i}) -> got{i}',f'    if got{i} != want{i} {{','        mismatch -> return','    }']
    lines+=['    passed -> return','}']; target=out/(name+'.flow');target.write_text('\n'.join(lines)+'\n');return target

def compile_source(source,name):
    directory=out/name;directory.mkdir(exist_ok=True)
    data=run([build/'flowmini/flowmini','--dump-frontend-bundle',source]).stdout;(directory/'frontend.json').write_text(data)
    stages=[('semantic',build/'flowanalyst/flowanalyst',['--lowering-plan-version','2']),('parallel',build/'flowtools/flowparallel/flowparallel',[]),('optimized',build/'flowoptimize/flowoptimize',[])]
    for label,tool,args in stages:
        data=run([tool,*args],input=data).stdout;(directory/(label+'.json')).write_text(data)
        if label=='semantic':
            sem=json.loads(data);coverage=sem['lowering_plan']['source_operation_coverage']
            assert sem['status']=='ok' and sem['lowering_plan']['status']=='ready' and coverage['status']=='complete' and coverage['refused_count']==0
    report=run([build/'flowlower/flowlower','--emit-llvm',directory/'program.ll'],input=data).stdout;(directory/'lowering.json').write_text(report)
    assert json.loads(report)['artifact']['status']=='emitted'
    run(['clang',directory/'program.ll','-o',directory/'program'])
    symbols=run(['nm','-u',directory/'program']).stdout;(directory/'undefined-symbols.txt').write_text(symbols)
    assert not re.search(r'configresolve|resolve_pair|choose_integer',symbols,re.I)
    return directory
results={}
for name,answers,rename,wanted in [('comparison',expected,False,0),('renamed',expected,True,0),('hostile-oracle',[99]+expected[1:],False,1)]:
    directory=compile_source(testcase_source(name,answers,rename),name)
    result=run([directory/'program'],accepted=(wanted,));results[name]={'exit':result.returncode,'sha256':hashlib.sha256((directory/'program').read_bytes()).hexdigest()}
# Retain the standalone source's own native application probe, independent of assertions.
directory=compile_source(here/'scalar-resolution.flow','standalone');run([directory/'program'],accepted=(2,));results['standalone']={'exit':2,'sha256':hashlib.sha256((directory/'program').read_bytes()).hexdigest()}
# TinyVM consumes the same backend-neutral artifact; unsupported must remain explicit.
prepared=run([build/'flowlower/flowprepare',out/'comparison/optimized.json']).stdout
(out/'comparison/prepared.json').write_text(prepared)
tiny=run([build/'tinyvm/flowtinylower',out/'comparison/prepared.json',out/'comparison/program.tvm'],accepted=(0,2))
(out/'tinyvm-lowering.json').write_text(tiny.stdout)
results['tinyvm']={'lower_exit':tiny.returncode,'report':tiny.stdout}
if tiny.returncode==0:
    vm=run([build/'tinyvm/flowtinyrun',out/'comparison/program.tvm']);results['tinyvm']['execution']=vm.stdout
    assert '"result":0' in vm.stdout.replace(' ','')
baseline=json.loads((here/'evidence/compiler-baseline.json').read_text())
protected=[name for name in baseline['files'] if name.startswith(('Flow','Lyraform/compiler/','subprojects/','Capabilities/','tools/')) or name in ['igor','CMakeLists.txt']]
review=json.loads((here/'generic-compiler-changes.json').read_text())
reviewed={entry['path']:entry for entry in review['changes']}
changed=[]
for name in protected:
    current=hashlib.sha256((root/name).read_bytes()).hexdigest()
    if current != baseline['files'][name]:
        assert name in reviewed and current==reviewed[name]['sha256'], f'unreviewed base change: {name}'
        assert (root/reviewed[name]['mission']).is_file()
        changed.append(name)
for name,entry in reviewed.items():
    assert hashlib.sha256((root/name).read_bytes()).hexdigest()==entry['sha256'], f'review drift: {name}'
report={'protected_files_verified':len(protected),'cases':len(cases),'results':results,'compiler_modified':bool(changed),'reviewed_generic_changes':changed,'scope':'Two integer facts only; not the complete port','commands':commands}
(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:v for k,v in report.items() if k!='commands'},indent=2))
