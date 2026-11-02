import ast, hashlib, pathlib, subprocess, tempfile
root=pathlib.Path('/home/henrik/Projekter/Udvikling/LyraForm')
build=pathlib.Path('/tmp/lyraform-capabilities-health-20261017')
module=ast.parse((root/'Flowlower/tests/run-native-record-unit.py').read_text())
template=next(ast.literal_eval(n.value) for n in module.body if isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='source_text' for t in n.targets))
variants=[('add','2 + 3','true'),('negative','-4','true'),('false','2 + 3','false'),('not-false','2 + 3','not false'),('not-true','2 + 3','not true'),('nested-not','2 + 3','not (not (2 < 4))')]
with tempfile.TemporaryDirectory() as d:
    for name,left,flag in variants:
        source=pathlib.Path(d)/(name+'.flow');source.write_text(template.replace('LEFT_VALUE',left).replace('RIGHT_FLAG',flag))
        frontend=subprocess.run([build/'flowmini/flowmini','--dump-frontend-bundle',source],capture_output=True,check=True).stdout
        for version in ['1','2']:
            reports=[subprocess.run([exe,'--lowering-plan-version',version],input=frontend,capture_output=True) for exe in ['/tmp/lyraform-record-facts-baseline',build/'flowanalyst/flowanalyst']]
            assert (reports[0].returncode,reports[0].stdout,reports[0].stderr)==(reports[1].returncode,reports[1].stdout,reports[1].stderr),(name,version)
            print(name,version,hashlib.sha256(reports[1].stdout).hexdigest(),'identical')
print('PASS: 12 byte-identical reports and matching exit/stderr observations')
