import pathlib,subprocess,json,sys
import argparse
parser=argparse.ArgumentParser()
parser.add_argument('--build',type=pathlib.Path,required=True)
parser.add_argument('--sources',type=pathlib.Path,required=True)
parser.add_argument('--output',type=pathlib.Path,required=True)
parser.add_argument('--pattern',default='*.flow')
args=parser.parse_args()
R=args.sources.resolve(); B=args.build.resolve(); O=args.output.resolve(); O.mkdir(parents=True,exist_ok=True)
T={k:B/v for k,v in dict(front='flowmini/flowmini',semantic='flowanalyst/flowanalyst',bind='flowbind/flowbind',parallel='flowtools/flowparallel/flowparallel',optimize='flowoptimize/flowoptimize',prepare='flowlower/flowprepare',validate='flowcontracts/flowvalidate',lower='flowlower/flowlower',tiny='tinyvm/flowtinylower',run='tinyvm/flowtinyrun').items()}
def probe(p):
 d=O/p.stem; d.mkdir(exist_ok=True); res={}; data=None
 def step(name,cmd,inp=None):
  q=subprocess.run(list(map(str,cmd)),input=inp,text=True,capture_output=True,timeout=30)
  (d/(name+'.out')).write_text(q.stdout);(d/(name+'.err')).write_text(q.stderr)
  res[name]=q.returncode
  if q.returncode: raise RuntimeError(name)
  return q.stdout
 try:
  f=step('front',[T['front'],'--dump-frontend-bundle',p]);s=step('semantic',[T['semantic'],'--lowering-plan-version','2'],f)
  step('bind',[T['bind']],s)
  a=step('parallel',[T['parallel']],s);o=step('optimize',[T['optimize']],a)
  step('validate',[T['validate']],o);(d/'optimized.json').write_text(o)
  target=['--target','Demo'] if 'target Demo' in p.read_text() else []
  prep=step('prepare',[T['prepare'],*target,d/'optimized.json']);step('validate_backend',[T['validate']],prep)
  # Backends checked independently.
  for backend in ('native','tiny'):
   try:
    if backend=='native':
     step('lower',[T['lower'],*target,'--emit-llvm',d/'program.ll'],o)
     step('link',['clang-18',d/'program.ll','-o',d/'program'])
     q=subprocess.run([str(d/'program')],capture_output=True,text=True,timeout=10);res['native_result']=q.returncode
    else:
     step('tiny',[T['tiny'],'-',d/'program.tvm'],prep)
     out=step('tiny_run',[T['run'],d/'program.tvm']);res['tiny_result']=json.loads(out)
   except RuntimeError: pass
 except RuntimeError: pass
 except Exception as e: res['harness_error']=str(e)
 if (d/'semantic.out').exists():
  try:
   j=json.loads((d/'semantic.out').read_text());res['diagnostics']=[{k:x.get(k) for k in ('code','message','provenance')} for x in j.get('diagnostics',[])];res['plan']=j.get('lowering_plan',{}).get('status')
  except Exception: pass
 (d/'result.json').write_text(json.dumps(res,indent=2));print(p.stem,json.dumps(res))
for p in sorted(R.glob(args.pattern)):probe(p)
