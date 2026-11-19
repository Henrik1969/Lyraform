#!/usr/bin/env python3
"""Native record emitter units using unchanged blocked source lowering plans."""
import copy
import hashlib
import json
import pathlib
import re
import subprocess
import sys
import tempfile

build, root = map(pathlib.Path, sys.argv[1:3])
unit = pathlib.Path(sys.argv[3])
sanitize = '--sanitize' in sys.argv[4:]
memcheck = '--valgrind' in sys.argv[4:]
assert not (sanitize and memcheck)
if '--require-tinyvm' in sys.argv[4:]:
    assert (build / 'tinyvm/flowtiny_record_unit').exists(), 'TinyVM record unit target is required'
def run(command, data=None, expected=0):
    result = subprocess.run(list(map(str, command)), input=data, text=True, capture_output=True)
    assert result.returncode == expected, (command, result.returncode, result.stdout, result.stderr)
    return result

source_text = '''program native_record_values
record Leaf { count : int32
enabled : Bool
}
record Pair { left : Leaf
right : Leaf
}
main {
 left_value : int32(LEFT_VALUE)
 right_value : int32(7)
 nested_enabled : Bool(true)
 item : Pair({right: {enabled: RIGHT_FLAG, count: right_value}, left: {count: left_value, enabled: nested_enabled}})
 copied : Pair(item)
 selected : Leaf(copied.left)
 number : int32(selected.count)
 other : int32(copied.right.count)
 enabled : Bool(item.right.enabled)
 total : c_int(LEFT_VALUE + 7)
 if enabled {
  total -> return
 } else {
  73 -> return
 }
}
'''
with tempfile.TemporaryDirectory(prefix='lyraform-native-record-unit-') as directory:
    out = pathlib.Path(directory)
    count = 0
    for variant, left, flag, expected in [('nested-copy', '2 + 3', 'true', 12),
                                         ('renamed-negative', '-4', 'true', 3),
                                         ('false-field', '2 + 3', 'false', 73),
                                         ('not-false', '2 + 3', 'not false', 12),
                                         ('not-true', '2 + 3', 'not true', 73),
                                         ('nested-not', '2 + 3', 'not (not true)', 12)]:
        source = source_text.replace('LEFT_VALUE', left).replace('RIGHT_FLAG', flag)
        if variant == 'renamed-negative':
            for before, after in [('Leaf', 'Cell'), ('Pair', 'Bundle'), ('count', 'amount'),
                                  ('copied', 'duplicate'), ('native_record_values', 'different_source')]:
                source = source.replace(before, after)
        path = out / (variant + '.flow'); path.write_text(source)
        front = run([build / 'flowmini/flowmini', '--dump-frontend-bundle', path]).stdout
        semantic = run([build / 'flowanalyst/flowanalyst', '--lowering-plan-version', '2'], front, 2).stdout
        plan = json.loads(semantic)['lowering_plan']
        assert plan['status'] == 'blocked'
        # The public pipeline still refuses admission of this source program.
        public = subprocess.run([build / 'flowtools/flowparallel/flowparallel'], input=semantic,
                                text=True, capture_output=True)
        assert public.returncode != 0
        report = out / (variant + '.json'); report.write_text(semantic)
        tiny_unit = build / 'tinyvm/flowtiny_record_unit'
        if 'not' in variant:
            malformed = copy.deepcopy(json.loads(semantic))
            def corrupt_not(value):
                if isinstance(value, dict):
                    if value.get('kind') == 'unary' and value.get('operator') == 'not':
                        value['operand'] = {'kind': 'integer_literal', 'type': 'int32',
                                            'expression_id': value['operand']['expression_id'],
                                            'value': '1'}
                        return True
                    return any(corrupt_not(child) for child in value.values())
                if isinstance(value, list):
                    return any(corrupt_not(child) for child in value)
                return False
            assert corrupt_not(malformed['lowering_plan'])
            hostile = out / (variant + '-invalid.json')
            hostile.write_text(json.dumps(malformed))
            for command in ([unit, '--record-plan', hostile], [tiny_unit, hostile]):
                if not pathlib.Path(command[0]).exists(): continue
                refused = subprocess.run(list(map(str, command)), text=True, capture_output=True)
                assert refused.returncode != 0, (command, refused.stdout)
                assert 'record_operand' in refused.stderr, refused.stderr
        if tiny_unit.exists():
            vm_command = ['valgrind', '--error-exitcode=99', '--leak-check=full',
                          '--errors-for-leak-kinds=all', tiny_unit, report] if memcheck else [tiny_unit, report]
            vm = run(vm_command)
            observation = json.loads(vm.stdout)
            assert observation == {'result': expected, 'trap': 0, 'engines': 2, 'independent_fields': 4}, observation
            print(json.dumps({'variant': variant, 'tinyvm': observation}))
            if memcheck: print(vm.stderr)

        ll = out / (variant + '.ll')
        emitted = run([unit, '--record-plan', report]).stdout
        instrumented = re.sub(r'^(define .*\)) \{$', r'\1 sanitize_address {', emitted,
                              flags=re.MULTILINE) if sanitize else emitted
        if sanitize: assert 'sanitize_address {' in instrumented
        ll.write_text(instrumented)
        assert 'insertvalue %flow.record.' in emitted and 'extractvalue %flow.record.' in emitted
        assert 'load %flow.record.' in emitted and emitted.count('store %flow.record.') >= 3
        assert emitted.count('alloca %flow.record.') >= 3
        assert 'malloc' not in emitted and 'memcpy' not in emitted
        for optimization in ('-O0', '-O2'):
            binary = out / (variant + optimization)
            flags = ['-fsanitize=address', '-fno-omit-frame-pointer', '-mllvm', '-asan-opt=false',
                     '-mllvm', '-asan-skip-promotable-allocas=false',
                     '-mllvm', '-asan-use-stack-safety=false'] if sanitize else []
            if sanitize:
                checked_ir = out / (variant + optimization + '.asan.ll')
                run(['clang-18', optimization, *flags, '-S', '-emit-llvm', ll, '-o', checked_ir])
                # At O2 the pure constant program can disappear; O0 must contain
                # actual checks, not merely a linked sanitizer runtime.
                if optimization == '-O0':
                    checked = checked_ir.read_text()
                    assert re.search(r'call void @__asan_report_(load|store)', checked), checked
            run(['clang-18', optimization, *flags, ll, '-o', binary])
            command = ['valgrind', '--error-exitcode=99', '--leak-check=full',
                       '--errors-for-leak-kinds=all', binary] if memcheck else [binary]
            result = run(command, expected=expected)
            if memcheck: print(result.stderr)
            print(json.dumps({'variant': variant, 'optimization': optimization, 'exit': expected,
                              'source_sha256': hashlib.sha256(source.encode()).hexdigest(),
                              'llvm_sha256': hashlib.sha256(emitted.encode()).hexdigest(),
                              'binary_sha256': hashlib.sha256(binary.read_bytes()).hexdigest()}))
            count += 1
print(f'PASS: {count} native record unit binaries; public source admission remains blocked')
