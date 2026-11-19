import argparse
from pathlib import Path
parser = argparse.ArgumentParser(description="Portable replay adapted from the KISS audit; writes only to --output.")
parser.add_argument("--repo", type=Path, required=True)
parser.add_argument("--build", type=Path, required=True)
parser.add_argument("--probes", type=Path, required=True, help="existing replay output directory")
parser.add_argument("--output", type=Path, required=True)
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
import pathlib,subprocess,json,re,collections,statistics,time
repo=args.repo;paths=subprocess.check_output(['git','ls-files'],cwd=repo,text=True).splitlines()
rows={}
for comp in ['Flowanalyst','Flowcontracts','Flowbind','Flowparallel','Flowoptimize','Flowlower','Lyraform/compiler','subprojects/TinyVM']:
 files=[p for p in paths if any(p.startswith(comp+'/'+s+'/') for s in ['src','include','tools']) and pathlib.Path(p).suffix in ['.h','.hpp','.c','.cpp']]
 # tools are included explicitly for TinyVM executables and component tooling.
 lines=sum(len((repo/p).read_bytes().splitlines()) for p in files)
 tests=[p for p in paths if p.startswith(comp+'/tests/')]
 includes=collections.Counter()
 for p in files:
  for name in re.findall(r'#include\s*[<"]([^>"\n]+)',(repo/p).read_text(errors='replace')):
   if name.startswith('flowcontracts/'): includes[name]+=1
 rows[comp]={'implementation_files':len(files),'physical_lines':lines,'test_files':len(tests),'direct_flowcontracts_include_sites':sum(includes.values()),'shared_headers':dict(includes)}
b=args.build
log=[]
for line in (b/'.ninja_log').read_text().splitlines():
 if line.startswith('#'):continue
 a=line.split('\t');log.append({'start_ms':int(a[0]),'end_ms':int(a[1]),'output':a[3]})
print('SIZE',json.dumps(rows,indent=2))
print('BUILD',len(log),'elapsed_span_s',(max(x['end_ms'] for x in log)-min(x['start_ms'] for x in log))/1000,'sum_action_wall_s',sum(x['end_ms']-x['start_ms'] for x in log)/1000)
print('LONGEST',sorted(log,key=lambda x:x['end_ms']-x['start_ms'],reverse=True)[:5])
art={};formats=set()
for case in ['A_float_calls','B_box','D_combined']:
 base=args.probes/case; sizes={};plans=[]
 for stage in ['front','semantic','bind','parallel','optimize','prepare']:
  p=base/(stage+'.out');j=json.loads(p.read_text());sizes[stage]=p.stat().st_size
  def walk(v):
   if isinstance(v,dict):
    if isinstance(v.get('format'),str):formats.add((v['format'],str(v.get('version',''))))
    for c in v.values():walk(c)
   elif isinstance(v,list):
    for c in v:walk(c)
  walk(j)
  if 'lowering_plan' in j:plans.append((stage,j['lowering_plan']))
 art[case]={'bytes':sizes,'plan_equal_to_semantic':{n:p==plans[0][1] for n,p in plans},'compact_plan_bytes':len(json.dumps(plans[0][1],separators=(',',':')).encode())}
print('ARTIFACTS',json.dumps(art));print('FORMATS',len(formats),sorted(formats))
result={'size':rows,'build_actions':log,'artifacts':art,'observed_formats':sorted(formats)}
(args.output/'inventory.json').write_text(json.dumps(result,indent=2))
