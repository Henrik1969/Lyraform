#!/usr/bin/env python3
"""ADR 0068 shape checks on actual frontend output; no execution claim."""
import json
import pathlib
import subprocess
import sys
import tempfile

build = pathlib.Path(sys.argv[1])
cases = [
    ('complete', '{left: 7, right: 30}', set()),
    ('reordered', '{right: 30, left: 7}', set()),
    ('missing', '{left: 7}', {'FLOWANALYST_RECORD_MISSING_FIELD'}),
    ('extra', '{left: 7, right: 30, spare: 1}', {'FLOWANALYST_RECORD_UNKNOWN_FIELD'}),
    ('duplicate', '{left: 7, right: 30, left: 8}', {'FLOWANALYST_RECORD_DUPLICATE_FIELD'}),
    ('duplicate-missing', '{left: 7, left: 8}',
     {'FLOWANALYST_RECORD_DUPLICATE_FIELD', 'FLOWANALYST_RECORD_MISSING_FIELD'}),
]
with tempfile.TemporaryDirectory(prefix='lyraform-record-shape-') as directory:
    source = pathlib.Path(directory) / 'input.flow'
    count = 0
    for nested in (False, True):
        for name, literal, expected in cases:
            declarations = 'record Sample { left : int\nright : int\n}\n'
            type_name = 'Sample'
            if nested:
                declarations += 'record Container { inner : Sample\n}\n'
                type_name = 'Container'
                literal = '{inner: ' + literal + '}'
            source.write_text('program construction_shapes\n' + declarations +
                              'main {\n item : ' + type_name + '(' + literal + ')\n 0 -> return\n}\n')
            front = subprocess.run([build / 'flowmini/flowmini', '--dump-frontend-bundle', source],
                                   capture_output=True, text=True, check=True).stdout
            parsed = json.loads(front)
            record_names = [d['name'] for d in parsed['ast']['declaration_pool'] if d['kind'] == 'record']
            assert record_names == (['Sample', 'Container'] if nested else ['Sample']), record_names
            for version in (1, 2):
                result = subprocess.run([build / 'flowanalyst/flowanalyst', '--lowering-plan-version', str(version)],
                                        input=front, capture_output=True, text=True)
                report = json.loads(result.stdout)
                issues = [d for d in report['diagnostics'] if d['code'].startswith('FLOWANALYST_RECORD_')]
                assert {d['code'] for d in issues} == expected, (name, nested, version, report)
                assert all(d['provenance']['line'] > 0 for d in issues), issues
                assert result.returncode == (2 if expected or version == 2 else 0), report
                if version == 2:
                    assert report['lowering_plan']['status'] == 'blocked'
                count += 1
print(f'PASS: {count} source record shape checks; execution remains refused')

# Type checks reuse the existing int/Bool rules, including operator result types
# and declaration-before-use. These are not backend execution tests.
typed_cases = [
    ('int-literal', 'int', '7', '', False),
    ('int-arithmetic', 'int', '2 + 5', '', False),
    ('bool-comparison', 'Bool', '2 < 5', '', False),
    ('bool-literal', 'Bool', 'true', '', False),
    ('int-as-bool', 'Bool', '7', '', True),
    ('bool-as-int', 'int', 'true', '', True),
    ('text-as-int', 'int', '"bad"', '', True),
    ('float-as-int', 'int', '1.5', '', True),
    ('record-as-int', 'int', '{x: 1}', '', True),
    ('comparison-as-int', 'int', '2 < 5', '', True),
    ('arithmetic-as-bool', 'Bool', '2 + 5', '', True),
    ('identifier', 'int', 'value', 'value : int(7)\n', False),
    ('wrong-identifier', 'Bool', 'value', 'value : int(7)\n', True),
    ('invalid-operator', 'int', 'true + false', '', True),
    ('scalar-as-record', 'Leaf', '7', '', True),
    ('literal-record', 'Leaf', '{leaf: 3}', '', False),
]
with tempfile.TemporaryDirectory(prefix='lyraform-record-types-') as directory:
    source = pathlib.Path(directory) / 'input.flow'
    count = 0
    for nested in (False, True):
        for name, field_type, initializer, prefix, invalid in typed_cases:
            declarations = 'record Leaf { leaf : int\n}\n' + f'record Sample {{ field_value : {field_type}\n}}\n'
            type_name = 'Sample'
            literal = '{field_value: ' + initializer + '}'
            if nested:
                declarations += 'record Container { inner : Sample\n}\n'
                type_name = 'Container'
                literal = '{inner: ' + literal + '}'
            source.write_text('program construction_types\n' + declarations +
                              'main {\n' + prefix + 'item : ' + type_name + '(' + literal + ')\n0 -> return\n}\n')
            front = subprocess.run([build / 'flowmini/flowmini', '--dump-frontend-bundle', source],
                                   capture_output=True, text=True, check=True).stdout
            for version in (1, 2):
                result = subprocess.run([build / 'flowanalyst/flowanalyst', '--lowering-plan-version', str(version)],
                                        input=front, capture_output=True, text=True)
                report = json.loads(result.stdout)
                issues = [d for d in report['diagnostics'] if d['code'] == 'FLOWANALYST_RECORD_FIELD_TYPE']
                assert bool(issues) == invalid, (name, nested, version, report)
                assert all(d['provenance']['line'] > 0 for d in issues), issues
                assert result.returncode == (2 if invalid or version == 2 else 0), (name, report)
                count += 1
print(f'PASS: {count} source record scalar-type checks; execution remains refused')

bound_cases = [
    ('self-recursion', 'record Loop { next : Loop\n}\n', 'Loop', '{next: 7}', '',
     {'FLOWANALYST_RECORD_TYPE_NOT_FINITE'}),
    ('mutual-recursion', 'record First { next : Second\n}\nrecord Second { next : First\n}\n',
     'First', '{next: 7}', '', {'FLOWANALYST_RECORD_TYPE_NOT_FINITE'}),
    ('shared-finite', 'record Leaf { value : int\n}\nrecord Pair { left : Leaf\nright : Leaf\n}\n',
     'Pair', '{left: {value: 2}, right: {value: 3}}', '', set()),
    ('pure-call', 'record Sample { value : int\n}\n', 'Sample', '{value: calculate()}',
     'fn calculate(): int {\n 7 -> return\n}\n', set()),
    ('unproved-call', 'record Sample { value : int\n}\n', 'Sample', '{value: calculate()}',
     'fn calculate(): int {\n value : int(7)\n value -> return\n}\n',
     {'FLOWANALYST_RECORD_INITIALIZER_PURITY_UNPROVEN'}),
    ('nested-unproved-call', 'record Leaf { value : int\n}\nrecord Outer { leaf : Leaf\n}\n',
     'Outer', '{leaf: {value: calculate()}}',
     'fn calculate(): int {\n value : int(7)\n value -> return\n}\n',
     {'FLOWANALYST_RECORD_INITIALIZER_PURITY_UNPROVEN'}),
]
with tempfile.TemporaryDirectory(prefix='lyraform-record-bounds-') as directory:
    source = pathlib.Path(directory) / 'input.flow'
    count = 0
    codes = {'FLOWANALYST_RECORD_TYPE_NOT_FINITE', 'FLOWANALYST_RECORD_INITIALIZER_PURITY_UNPROVEN'}
    for name, declarations, type_name, initializer, functions, expected in bound_cases:
        source.write_text('program construction_bounds\n' + declarations + functions +
                          'main {\nitem : ' + type_name + '(' + initializer + ')\n0 -> return\n}\n')
        front = subprocess.run([build / 'flowmini/flowmini', '--dump-frontend-bundle', source],
                               capture_output=True, text=True, check=True).stdout
        for version in (1, 2):
            result = subprocess.run([build / 'flowanalyst/flowanalyst', '--lowering-plan-version', str(version)],
                                    input=front, capture_output=True, text=True)
            report = json.loads(result.stdout)
            observed = {d['code'] for d in report['diagnostics']} & codes
            assert observed == (expected if version == 2 else set()), (name, version, report)
            if name == 'pure-call':
                assert any(f['effect'] == 'pure' for f in report['effect_facts']), report
            if version == 2:
                assert report['lowering_plan']['status'] == 'blocked'
                assert result.returncode == 2
            count += 1
print(f'PASS: {count} source record finiteness/purity checks; execution remains refused')
