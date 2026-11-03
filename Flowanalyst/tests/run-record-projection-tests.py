#!/usr/bin/env python3
"""Typed local record reads/copy operands and projection; no runtime claim."""
import copy
import json
import pathlib
import subprocess
import sys
import tempfile

build = pathlib.Path(sys.argv[1])
with tempfile.TemporaryDirectory(prefix='lyraform-record-projection-') as directory:
    source = pathlib.Path(directory) / 'input.flow'
    source.write_text('''program record_projection
record Leaf { count : int32
enabled : Bool
}
record Pair { left : Leaf
right : Leaf
}
main {
 left_count : int32(2 + 3)
 right_count : int32(7)
 nested_enabled : Bool(true)
 item : Pair({right: {enabled: true, count: right_count}, left: {count: left_count, enabled: nested_enabled}})
 copied : Pair(item)
 selected : Leaf(copied.left)
 number : int32(selected.count)
 enabled : Bool(item.right.enabled)
 result : c_int(number)
 result -> return
}
''')
    front = subprocess.run([build / 'flowmini/flowmini', '--dump-frontend-bundle', source],
                           capture_output=True, text=True, check=True).stdout
    analyzed = subprocess.run([build / 'flowanalyst/flowanalyst', '--lowering-plan-version', '2'],
                              input=front, capture_output=True, text=True)
    assert analyzed.returncode == 2
    plan = json.loads(analyzed.stdout)['lowering_plan']
    defs = [o for o in plan['operations'] if o['kind'] == 'value_definition']
    record_start = next(index for index, definition in enumerate(defs)
                        if definition['operands'][0]['kind'] == 'record_construct')
    record_defs = defs[record_start:record_start + 5]
    assert [d['operands'][0]['kind'] for d in record_defs] == [
        'record_construct', 'record_value', 'record_project', 'record_project', 'record_project']
    assert record_defs[1]['operands'][0]['symbol_id'] == record_defs[0]['result_symbol_id']
    assert record_defs[2]['operands'][0]['base']['symbol_id'] == record_defs[1]['result_symbol_id']
    assert record_defs[3]['operands'][0]['type'] == 'int32'
    assert record_defs[4]['operands'][0]['type'] == 'Bool'
    assert record_defs[4]['operands'][0]['base']['field'] == 'right'
    def validate(value):
        result = subprocess.run([build / 'flowcontracts/flowvalidate'], input=json.dumps(value),
                                capture_output=True, text=True)
        return json.loads(result.stdout)
    assert validate(plan)['classification'] == 'blocked'
    for mutation in ('unknown-local', 'wrong-record', 'missing-field', 'result-type', 'category',
                     'cross-function', 'cycle', 'duplicate-definition', 'catalog', 'ready'):
        bad = copy.deepcopy(plan)
        d = [o for o in bad['operations'] if o['kind'] == 'value_definition']
        if mutation == 'unknown-local': d[1]['operands'][0]['symbol_id'] = 99999
        elif mutation == 'wrong-record': d[1]['operands'][0]['record_symbol_id'] = d[2]['operands'][0]['result_type']['symbol_id']
        elif mutation == 'missing-field': d[2]['operands'][0]['field'] = 'absent'
        elif mutation == 'result-type': d[3]['operands'][0]['result_type']['name'] = 'Bool'
        elif mutation == 'category': d[3]['operands'][0]['type'] = 'Bool'
        elif mutation == 'cross-function': d[1]['function_symbol_id'] = 99999
        elif mutation == 'cycle': d[1]['operands'][0]['symbol_id'] = d[1]['result_symbol_id']
        elif mutation == 'duplicate-definition': d[1]['result_symbol_id'] = d[0]['result_symbol_id']
        elif mutation == 'catalog': del bad['ordinary_record_types']
        elif mutation == 'ready': bad['status'] = 'ready'
        result = validate(bad)
        assert result['classification'] == 'invalid', (mutation, result)
        assert 'record_operand' in result['path'], (mutation, result)
print('PASS: typed copies and nested projections; 10 hostile identity/type changes refused; no execution claim')
