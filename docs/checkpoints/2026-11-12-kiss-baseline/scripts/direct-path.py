import argparse
from pathlib import Path
parser = argparse.ArgumentParser(description="Portable replay adapted from the KISS audit; writes only to --output.")
parser.add_argument("--repo", type=Path, required=True)
parser.add_argument("--build", type=Path, required=True)
parser.add_argument("--probes", type=Path, required=True, help="existing replay output directory")
parser.add_argument("--output", type=Path, required=True)
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
import pathlib,subprocess,json,hashlib
b=args.build;r=(args.probes/'B_box');out=args.output
def run(n,cmd,data=None):
 p=subprocess.run(list(map(str,cmd)),input=data,text=True,capture_output=True);(out/(n+'.out')).write_text(p.stdout);(out/(n+'.err')).write_text(p.stderr);assert p.returncode==0,(n,p.stderr);return p.stdout
opt=run('direct-optimize',[b/'flowoptimize/flowoptimize'],(r/'semantic.out').read_text());(out/'direct-optimized.json').write_text(opt)
run('direct-validate',[b/'flowcontracts/flowvalidate'],opt)
run('direct-lower',[b/'flowlower/flowlower','--target','Demo','--emit-llvm',out/'direct.ll'],opt)
prep=run('direct-prepare',[b/'flowlower/flowprepare','--target','Demo',out/'direct-optimized.json'])
run('direct-tiny',[b/'tinyvm/flowtinylower','-',out/'direct.tvm'],prep)
tiny=json.loads(run('direct-execute',[b/'tinyvm/flowtinyrun',out/'direct.tvm']))
old=json.loads((r/'optimize.out').read_text());new=json.loads(opt)
print(json.dumps({'plan_equal':old['lowering_plan']==new['lowering_plan'],'llvm_byte_equal':(r/'program.ll').read_bytes()==(out/'direct.ll').read_bytes(),'tiny_result':tiny['result'],'tiny_trap':tiny['trap']}))
