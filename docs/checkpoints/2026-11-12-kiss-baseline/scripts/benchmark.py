import argparse
from pathlib import Path
parser = argparse.ArgumentParser(description="Portable replay adapted from the KISS audit; writes only to --output.")
parser.add_argument("--repo", type=Path, required=True)
parser.add_argument("--build", type=Path, required=True)
parser.add_argument("--probes", type=Path, required=True, help="existing replay output directory")
parser.add_argument("--output", type=Path, required=True)
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
import pathlib,subprocess,time,json,statistics,os
b=args.build;r=(args.probes/'B_box');repo=args.repo
cases={
'frontend':([b/'flowmini/flowmini','--dump-frontend-bundle',(Path(__file__).resolve().parents[1]/'sources/slice2/B_box.flow')],None),
'analyst':([b/'flowanalyst/flowanalyst','--lowering-plan-version','2'],(r/'front.out').read_text()),
'bind':([b/'flowbind/flowbind'],(r/'semantic.out').read_text()),
'parallel':([b/'flowtools/flowparallel/flowparallel'],(r/'semantic.out').read_text()),
'optimize':([b/'flowoptimize/flowoptimize'],(r/'parallel.out').read_text()),
'prepare':([b/'flowlower/flowprepare','--target','Demo',r/'optimized.json'],None),
'validate_optimized':([b/'flowcontracts/flowvalidate'],(r/'optimize.out').read_text()),
'validate_prepared':([b/'flowcontracts/flowvalidate'],(r/'prepare.out').read_text())}
result={}
for name,(cmd,data) in cases.items():
 samples=[]
 for i in range(12):
  t=time.perf_counter();p=subprocess.run(list(map(str,cmd)),input=data,text=True,capture_output=True);elapsed=(time.perf_counter()-t)*1000
  assert p.returncode==0,(name,p.stderr)
  if i:samples.append(elapsed)
 result[name]={'median_ms':round(statistics.median(samples),3),'min_ms':round(min(samples),3),'max_ms':round(max(samples),3),'samples':11}
print(json.dumps(result,indent=2))
t=time.perf_counter();p=subprocess.run(['python3',str(repo/'tools/test-refinement-construction.py'),'--build',str(b)],text=True,capture_output=True)
result['focused_existing_refinement_test']={'seconds':round(time.perf_counter()-t,3),'exit':p.returncode,'stdout':p.stdout,'stderr':p.stderr};print(result['focused_existing_refinement_test'])
(args.output/'timings.json').write_text(json.dumps(result,indent=2))
