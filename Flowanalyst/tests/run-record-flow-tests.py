#!/usr/bin/env python3
"""Record local availability across structured branches, not execution admission."""
import copy
import json
import pathlib
import subprocess
import sys
import tempfile

build = pathlib.Path(sys.argv[1])
with tempfile.TemporaryDirectory(prefix='lyraform-record-flow-') as directory:
    source = pathlib.Path(directory) / 'input.flow'
    source.write_text('''program record_control
record Cell { count : int32
}
main {
 count_value : int32(7)
 item : Cell({count: count_value})
 root_copy : Cell(item)
 if true {
  branch_copy : Cell(item)
  first : int32(branch_copy.count)
 } else {
  sibling_copy : Cell(item)
  second : int32(sibling_copy.count)
 }
 after : int32(item.count)
 result : c_int(after)
 result -> return
}
''')
    front = subprocess.run([build / 'flowmini/flowmini', '--dump-frontend-bundle', source],
                           capture_output=True, text=True, check=True).stdout
    analysis = subprocess.run([build / 'flowanalyst/flowanalyst', '--lowering-plan-version', '2'],
                              input=front, capture_output=True, text=True)
    assert analysis.returncode == 2
    plan = json.loads(analysis.stdout)['lowering_plan']
    def validate(value):
        result = subprocess.run([build / 'flowcontracts/flowvalidate'], input=json.dumps(value),
                                capture_output=True, text=True)
        return json.loads(result.stdout)
    assert validate(plan)['classification'] == 'blocked', validate(plan)
    def parts(value):
        ops = value['operations']
        copies = [o for o in ops if o.get('operands') and o['operands'][0]['kind'] == 'record_value']
        branch = next(o for o in ops if o['kind'] == 'branch')
        root_copy = next(o for o in copies if o['block_id'] == branch['block_id'])
        then_copy = next(o for o in copies if o['block_id'] == branch['then_block_id'])
        else_copy = next(o for o in copies if o['block_id'] == branch['else_block_id'])
        after = next(o for o in ops if o['block_id'] == branch['block_id'] and
                     o.get('operands') and o['operands'][0]['kind'] == 'record_project')
        original = next(o for o in ops if o.get('operands') and o['operands'][0]['kind'] == 'record_construct')
        return branch, root_copy, then_copy, else_copy, after, original
    for mutation in ('forward', 'sibling', 'escape', 'orphan', 'overwrite', 'cycle', 'shared-block'):
        bad = copy.deepcopy(plan)
        branch, root_copy, then_copy, else_copy, after, original = parts(bad)
        if mutation == 'forward': root_copy['operands'][0]['symbol_id'] = then_copy['result_symbol_id']
        elif mutation == 'sibling': else_copy['operands'][0]['symbol_id'] = then_copy['result_symbol_id']
        elif mutation == 'escape': after['operands'][0]['base']['symbol_id'] = then_copy['result_symbol_id']
        elif mutation == 'orphan': root_copy['block_id'] = 99999
        elif mutation == 'overwrite':
            root_copy['kind'] = 'assignment'; root_copy['result_symbol_id'] = original['result_symbol_id']
        elif mutation == 'cycle': branch['then_block_id'] = branch['block_id']
        elif mutation == 'shared-block': branch['else_block_id'] = branch['then_block_id']
        result = validate(bad)
        assert result['classification'] == 'invalid', (mutation, result)
        assert 'record_operand' in result['path'], (mutation, result)
        expected = {'forward': 'not dominated', 'sibling': 'not dominated', 'escape': 'not dominated',
                    'orphan': 'outside its function', 'overwrite': 'overwrite',
                    'cycle': 'multiple owners', 'shared-block': 'multiple owners'}[mutation]
        assert expected in result['reason'], (mutation, result)
print('PASS: structured record availability; 7 hostile control/definition changes rejected')

with tempfile.TemporaryDirectory(prefix='lyraform-record-loop-flow-') as directory:
    source = pathlib.Path(directory) / 'input.flow'
    source.write_text('''program record_loop_control
record Cell { count : int32
}
main {
 count_value : int32(7)
 item : Cell({count: count_value})
 while false {
  body_copy : Cell(item)
  inside : int32(body_copy.count)
 }
 after : int32(item.count)
 result : c_int(after)
 result -> return
}
''')
    front = subprocess.run([build / 'flowmini/flowmini', '--dump-frontend-bundle', source],
                           capture_output=True, text=True, check=True).stdout
    analysis = subprocess.run([build / 'flowanalyst/flowanalyst', '--lowering-plan-version', '2'],
                              input=front, capture_output=True, text=True)
    assert analysis.returncode == 2
    plan = json.loads(analysis.stdout)['lowering_plan']
    good = validate(plan)
    assert good['classification'] == 'blocked', good
    loop = next(o for o in plan['operations'] if o['kind'] == 'loop')
    copied = next(o for o in plan['operations'] if o['block_id'] == loop['body_block_id'] and
                  o.get('operands') and o['operands'][0]['kind'] == 'record_value')
    after = next(o for o in plan['operations'] if o['block_id'] == loop['block_id'] and
                 o.get('operands') and o['operands'][0]['kind'] == 'record_project')
    after['operands'][0]['base']['symbol_id'] = copied['result_symbol_id']
    bad = validate(plan)
    assert bad['classification'] == 'invalid' and 'not dominated' in bad['reason'], bad
print('PASS: loop-local definition cannot escape a possibly zero-iteration loop')
