#!/usr/bin/env python3
"""Preserve private ordinary-record types and reject altered type contracts."""
import copy
import json
import pathlib
import subprocess
import sys
import tempfile

build = pathlib.Path(sys.argv[1])
def run(tool, data=None, args=(), okay=True):
    result = subprocess.run([build / tool, *map(str, args)], input=data, text=True, capture_output=True)
    assert (result.returncode == 0) == okay, (tool, result.returncode, result.stdout, result.stderr)
    return result.stdout

with tempfile.TemporaryDirectory(prefix='lyraform-record-catalog-') as directory:
    out = pathlib.Path(directory)
    source = out / 'input.flow'
    source.write_text('''program typed_record_declarations
record Leaf { enabled : Bool
count : int
}
record Pair { left : Leaf
right : Leaf
}
record Recursive { next : Recursive
}
record Resource { value : Text
}
main { 0 -> return
}
''')
    front = run('flowmini/flowmini', args=['--dump-frontend-bundle', source])
    sem = run('flowanalyst/flowanalyst', front, ['--lowering-plan-version', '2'])
    catalog = json.loads(sem)['lowering_plan']['ordinary_record_types']
    types = {t['name']: t for t in catalog['types']}
    assert set(types) == {'Leaf', 'Pair', 'Resource'}, types
    assert types['Leaf']['fields'] == [
        {'name': 'enabled', 'type': {'kind': 'scalar', 'name': 'Bool'}},
        {'name': 'count', 'type': {'kind': 'scalar', 'name': 'int'}}]
    assert [f['type'] for f in types['Pair']['fields']] == [
        {'kind': 'record', 'symbol_id': types['Leaf']['symbol_id']}] * 2
    assert types['Resource']['fields'] == [
        {'name': 'value', 'type': {'kind': 'scalar', 'name': 'Text'}}]
    parallel = run('flowtools/flowparallel/flowparallel', sem)
    optimized = run('flowoptimize/flowoptimize', parallel)
    prepared = run('flowlower/flowprepare', optimized)
    stages = [('semantic', sem, 'flowtools/flowparallel/flowparallel'),
              ('parallel', parallel, 'flowoptimize/flowoptimize'),
              ('optimized', optimized, 'flowlower/flowprepare'),
              ('prepared', prepared, None)]
    count = 0
    for stage, raw, consumer in stages:
        doc = json.loads(raw)
        assert doc['lowering_plan']['ordinary_record_types'] == catalog, stage
        run('flowcontracts/flowvalidate', raw)
        for mutation in ('missing-ref', 'cycle', 'duplicate-id', 'duplicate-field', 'scalar',
                         'layout', 'offset', 'semantics', 'ready', 'version'):
            bad = copy.deepcopy(doc)
            c = bad['lowering_plan']['ordinary_record_types']
            t = {entry['name']: entry for entry in c['types']}
            if mutation == 'missing-ref': t['Pair']['fields'][0]['type']['symbol_id'] = 99999
            elif mutation == 'cycle': t['Pair']['fields'][0]['type']['symbol_id'] = t['Pair']['symbol_id']
            elif mutation == 'duplicate-id': t['Pair']['symbol_id'] = t['Leaf']['symbol_id']
            elif mutation == 'duplicate-field': t['Leaf']['fields'][1]['name'] = 'enabled'
            elif mutation == 'scalar': t['Leaf']['fields'][0]['type']['name'] = 'c_pointer'
            elif mutation == 'layout': c['layout'] = 'c_abi'
            elif mutation == 'offset': t['Leaf']['fields'][0]['offset'] = 0
            elif mutation == 'semantics': c['semantics'] = 'shared_reference'
            elif mutation == 'ready': c['status'] = 'ready'
            elif mutation == 'version': c['version'] = 2
            encoded = json.dumps(bad)
            run('flowcontracts/flowvalidate', encoded, okay=False)
            if consumer: run(consumer, encoded, okay=False)
            count += 1
print(f'PASS: record catalog preserved across 4 stages; {count} hostile contracts rejected')
