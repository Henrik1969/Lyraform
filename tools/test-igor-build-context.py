from pathlib import Path
import tempfile,os,subprocess
igor=(Path(__file__).resolve().parents[1] / 'igor').read_text()
with tempfile.TemporaryDirectory(prefix='igor-context-') as temp:
 root=Path(temp)/'source space';root.mkdir();driver=root/'igor';driver.write_text(igor);driver.chmod(0o755);bin=Path(temp)/'bin';bin.mkdir();log=Path(temp)/'calls';fake=bin/'cmake';fake.write_text('#!/bin/sh\nprintf "%s\\n" "$*" >> "$IGOR_TEST_LOG"\n');fake.chmod(0o755)
 env={**os.environ,'PATH':str(bin)+':'+os.environ['PATH'],'IGOR_TEST_LOG':str(log)}
 def run(build,ok,marker=None):
  log.write_text('');p=subprocess.run([str(driver),'--build-dir',str(build),'build'],env=env,capture_output=True,text=True)
  assert (p.returncode==0)==ok,p.stderr
  if marker:assert marker in p.stderr,p.stderr
  assert bool(log.read_text())==ok,(p.returncode,log.read_text())
 cache=Path(temp)/'build space';cache.mkdir()
 (cache/'CMakeCache.txt').write_text('CMAKE_HOME_DIRECTORY:INTERNAL=/retired/source\n');run(cache,False,'build cache belongs to');assert '/retired/source' in (cache/'CMakeCache.txt').read_text()
 (cache/'CMakeCache.txt').write_text('CMAKE_HOME_DIRECTORY:INTERNAL='+str(root)+'\n');run(cache,True)
 alias=Path(temp)/'alias';alias.symlink_to(root);(cache/'CMakeCache.txt').write_text('CMAKE_HOME_DIRECTORY:INTERNAL='+str(alias)+'\n');run(cache,True)
 (cache/'CMakeCache.txt').write_text('OTHER:STRING=value\n');run(cache,False,'no source identity')
 run(Path(temp)/'fresh',True)
 print('Igor build-context: 5 cases passed (foreign, matching, alias, missing identity, fresh).')
