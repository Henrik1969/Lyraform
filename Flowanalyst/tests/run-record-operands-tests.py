#!/usr/bin/env python3
"""Typed literal construction evidence, not executable record acceptance."""
import copy
import json
import pathlib
import subprocess
import sys
import tempfile

build = pathlib.Path(sys.argv[1])
with tempfile.TemporaryDirectory(prefix='lyraform-record-operands-') as directory:
    source = pathlib.Path(directory) / 'input.flow'
    source.write_text('''program construction_operands
record Leaf { count : int
enabled : Bool
}
record Pair { left : Leaf
right : Leaf
}
main {
 item : Pair({right: {enabled: true, count: 7}, left: {count: 2 + 3, enabled: 2 < 4}})
 0 -> return
}
''')
    front = subprocess.run([build / 'flowmini/flowmini', '--dump-frontend-bundle', source],
                           capture_output=True, text=True, check=True).stdout
    analyzed = subprocess.run([build / 'flowanalyst/flowanalyst', '--lowering-plan-version', '2'],
                              input=front, capture_output=True, text=True)
    assert analyzed.returncode == 2
    plan = json.loads(analyzed.stdout)['lowering_plan']
    operand = plan['operations'][0]['operands'][0]
    assert operand['kind'] == 'record_construct'
    assert [f['name'] for f in operand['fields']] == ['left', 'right']
    left = operand['fields'][0]['operand']
    right = operand['fields'][1]['operand']
    assert [f['name'] for f in left['fields']] == ['count', 'enabled']
    assert [f['operand']['type'] for f in left['fields']] == ['int', 'Bool']
    assert right['fields'][0]['operand']['value'] == '7'
    assert right['fields'][1]['operand']['value'] == 'true'
    assert plan['status'] == 'blocked'
    def validate(value):
        result = subprocess.run([build / 'flowcontracts/flowvalidate'], input=json.dumps(value),
                                capture_output=True, text=True)
        return json.loads(result.stdout)
    good = validate(plan)
    assert good['classification'] in ('valid', 'blocked'), good
    for mutation in ('catalog', 'unknown-type', 'missing-field', 'duplicate-field', 'field-type',
                     'scalar-value', 'operator', 'expression', 'offset', 'ready', 'version'):
        bad = copy.deepcopy(plan)
        op = bad['operations'][0]['operands'][0]
        nested = op['fields'][0]['operand']
        if mutation == 'catalog': del bad['ordinary_record_types']
        elif mutation == 'unknown-type': op['record_symbol_id'] = 99999
        elif mutation == 'missing-field': op['fields'].pop()
        elif mutation == 'duplicate-field': op['fields'][1]['name'] = 'left'
        elif mutation == 'field-type': nested['fields'][0]['operand']['type'] = 'Bool'
        elif mutation == 'scalar-value': op['fields'][1]['operand']['fields'][1]['operand']['value'] = 'maybe'
        elif mutation == 'operator': nested['fields'][0]['operand']['operator'] = '<'
        elif mutation == 'expression': op['expression_id'] = -1
        elif mutation == 'offset': op['fields'][0]['offset'] = 0
        elif mutation == 'ready': bad['status'] = 'ready'
        elif mutation == 'version': bad['version'] = 1
        result = validate(bad)
        assert result['classification'] == 'invalid', (mutation, result)
        assert 'record_operand' in result['path'], (mutation, result)
print('PASS: typed nested construction and scalar operators; 11 hostile operand changes rejected; execution blocked')
