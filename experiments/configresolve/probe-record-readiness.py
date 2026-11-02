#!/usr/bin/env python3
"""Capture record admission evidence without modifying the compiler."""
import argparse,json,pathlib,subprocess
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',required=True,type=pathlib.Path);p.add_argument('--output',required=True,type=pathlib.Path);a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
base=(pathlib.Path(__file__).parent/'readiness/ordinary-record.flow').read_text()
cases={'valid':base,'reordered':base.replace('{value: 7, rank: 30}','{rank: 30, value: 7}'),'missing':base.replace('{value: 7, rank: 30}','{value: 7}'),'extra':base.replace('{value: 7, rank: 30}','{value: 7, rank: 30, spare: 9}'),'wrong-type':base.replace('value: 7','value: "wrong"'),'unknown-record':base.replace('candidate : Candidate','candidate : Unknown'),'unknown-member':base.replace('candidate.value','candidate.absent'),'copy':base.replace('    observed : int(candidate.value)','    copied : Candidate(candidate)\n    observed : int(copied.value)'),'renamed':base.replace('Candidate','Measurement').replace('candidate','sample')}
results=[]
for name,source in cases.items():
 d=out/name;d.mkdir(exist_ok=True);f=d/'source.flow';f.write_text(source)
 def run(tool,args,data=None):
  r=subprocess.run([str(a.build/tool),*map(str,args)],input=data,text=True,capture_output=True);(d/(tool.name+'.stdout')).write_text(r.stdout);(d/(tool.name+'.stderr')).write_text(r.stderr);return r
 front=run(pathlib.Path('flowmini/flowmini'),['--dump-frontend-bundle',f]);row={'case':name,'frontend_exit':front.returncode}
 if front.returncode==0:
  sem=run(pathlib.Path('flowanalyst/flowanalyst'),['--lowering-plan-version','2'],front.stdout);x=json.loads(sem.stdout);row.update(analyst_exit=sem.returncode,status=x.get('status'),plan_status=x.get('lowering_plan',{}).get('status'),diagnostics=x.get('diagnostics',[]))
  if sem.returncode==0:
   parallel=run(pathlib.Path('flowtools/flowparallel/flowparallel'),[],sem.stdout)
   if parallel.returncode==0:
    opt=run(pathlib.Path('flowoptimize/flowoptimize'),[],parallel.stdout)
    if opt.returncode==0:
     low=run(pathlib.Path('flowlower/flowlower'),['--emit-llvm',d/'program.ll'],opt.stdout);row.update(lower_exit=low.returncode,lower_stderr=low.stderr,artifact_exists=(d/'program.ll').exists())
 results.append(row)
(out/'report.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results,indent=2))
