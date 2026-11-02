#!/usr/bin/env python3
"""Explicit integer conversion intent, exact values and hostile artifacts."""
import argparse
import json
import pathlib
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument('--build', type=pathlib.Path, required=True)
build = parser.parse_args().build.resolve()
tools = dict(frontend='flowmini/flowmini', analyze='flowanalyst/flowanalyst',
             validate='flowcontracts/flowvalidate', bind='flowbind/flowbind',
             parallel='flowtools/flowparallel/flowparallel', optimize='flowoptimize/flowoptimize',
             prepare='flowlower/flowprepare', lower='flowlower/flowlower',
             tiny_lower='tinyvm/flowtinylower', tiny_run='tinyvm/flowtinyrun')

def run(stage, data=None, *args):
    return subprocess.run([str(build / tools[stage]), *map(str, args)], input=data,
                          capture_output=True, text=True, timeout=30)

def good(stage, data=None, *args):
    result = run(stage, data, *args)
    assert result.returncode == 0, (stage, result.stdout, result.stderr)
    return result.stdout

positives = [
    ('guarded_size', 'c_size_t', 'y : c_size_t(guarded(17))', 17),
    ('contextual_literal', 'int64', 'y : int64(wider(19))', 19),
    ('call_widen', 'int64', 'y : int64(small())', 7),
    ('dynamic_widen', 'int64', 'x : int32(17)\ny : int64(wider(x))', 17),
    ('negative_widen', 'int64', 'x : int32(-42)\ny : int64(x)', -42),
    ('widen', 'int64', 'x : int32(2147483647)\ny : int64(x)', 2147483647),
    ('wide', 'int64', 'x : int64(4294967297)\ny : int64(x)', 4294967297),
    ('narrow', 'int32', 'x : int64(2147483647)\ny : int32(x)', 2147483647),
    ('negative_narrow', 'int32', 'x : int64(-42)\ny : int32(x)', -42),
    ('abi_explicit', 'c_long', 'x : c_int(42)\ny : c_long(x)', 42),
]
source_root = pathlib.Path(__file__).resolve().parents[1]
negatives = [
    ('external_result_placement', f'import "{source_root}/Lyraform/compiler/std/abi/libc.flow"\nmain {{input : c_long(-42)\noutput : c_int(0)\nlabs(input) -> output\n0 -> return\n}}'),
    ('unguarded_signedness', 'fn f(n : int64): c_size_t {y : c_size_t(n)\ny -> return\n}\nmain {0 -> return\n}'),
    ('nested_changed_guard_source', 'fn f(n : int64): c_size_t {if n > 0 {if n == 7 {-1 -> n\n}\ny : c_size_t(n)\ny -> return\n} else {0 -> return\n}\n}\nmain {0 -> return\n}'),
    ('changed_guard_source', 'fn f(n : int64): c_size_t {if n > 0 {-1 -> n\ny : c_size_t(n)\ny -> return\n} else {0 -> return\n}\n}\nmain {0 -> return\n}'),
    ('wrong_guard_polarity', 'fn f(n : int64): c_size_t {if n < 0 {y : c_size_t(n)\ny -> return\n} else {0 -> return\n}\n}\nmain {0 -> return\n}'),
    ('call_result_placement', 'fn f(): int64 {7 -> return\n}\nmain {x : int32(0)\nf() -> x\n0 -> return\n}'),
    ('index_narrowing', 'main(args : list<string>) {i : int64(1)\nx : c_string(args[i])\n0 -> return\n}'),
    ('invalid_source_range', 'main {x : c_int(2147483648)\ny : c_long(x)\n0 -> return\n}'),
    ('uninitialized_target', 'main {x : int32(7)\ny : int64\nx -> y\n0 -> return\n}'),
    ('assignment_widen', 'main { x : int32(7)\ny : int64(0)\nx -> y\n0 -> return\n}'),
    ('assignment_narrow', 'main { x : int64(7)\ny : int32(0)\nx -> y\n0 -> return\n}'),
    ('return_widen', 'fn f(): int64 {x : int32(7)\nx -> return\n}\nmain {0 -> return\n}'),
    ('return_narrow', 'fn f(): int32 {x : int64(7)\nx -> return\n}\nmain {0 -> return\n}'),
    ('argument', 'fn f(n : int64): c_int {0 -> return\n}\nmain {x : int32(7)\nf(x) -> return\n}'),
    ('mixed', 'main {x : int32(7)\ny : int64(9)\nx + y -> return\n}'),
    ('truncate', 'main {x : int64(2147483648)\ny : int32(x)\n0 -> return\n}'),
    ('negative_truncate', 'main {x : int64(-2147483649)\ny : int32(x)\n0 -> return\n}'),
    ('dynamic_narrow', 'fn f(x : int64): int32 {y : int32(x)\ny -> return\n}\nmain {0 -> return\n}'),
    ('same_width_identity', 'main {x : int32(7)\ny : c_int(0)\nx -> y\n0 -> return\n}'),
]
with tempfile.TemporaryDirectory(prefix='lyraform-conversions-') as directory:
    root = pathlib.Path(directory)
    def analyze(name, body):
        source = root / (name + '.flow')
        lines = body.splitlines()
        imports = []
        while lines and lines[0].startswith('import '):
            imports.append(lines.pop(0))
        source.write_text('\n'.join(imports) + '\nprogram conversion_check\n' + '\n'.join(lines) + '\n')
        frontend = good('frontend', None, '--dump-frontend-bundle', source)
        return run('analyze', frontend, '--lowering-plan-version', '2')
    for name, carrier, body, expected in positives:
        observation = 'if y == ' + str(expected) + ' {0 -> return\n} else {1 -> return\n}'
        if expected > 2147483647:
            # An independent in-range threshold catches systematic truncation
            # of both the computed value and its equality literal on TinyVM.
            observation = 'if y > 2147483647 {\n' + observation + '\n} else {1 -> return\n}'
        source = ('fn guarded(n : int64): c_size_t {if n > 0 {size : c_size_t(n)\nsize -> return\n} else {0 -> return\n}\n}\nfn small(): int32 {7 -> return\n}\nfn wider(n : int32): int64 {result : int64(n)\nresult -> return\n}\nfn specimen(): ' + carrier + ' {\n' + body + '\ny -> return\n}\n'
                  'main {\ny : ' + carrier + '(specimen())\n' + observation + '\n}')
        analysis = analyze(name, source)
        assert analysis.returncode == 0, (name, analysis.stdout, analysis.stderr)
        report = json.loads(analysis.stdout)
        if name == 'widen': captured = report
        parallel = good('parallel', analysis.stdout)
        optimized = good('optimize', parallel)
        prepared = good('prepare', optimized)
        llvm = root / (name + '.ll')
        good('lower', prepared, '--emit-llvm', llvm)
        function = next(f for f in report['lowering_plan']['functions'] if f['name'] == 'specimen')
        # Observe the complete returned language value outside process status.
        text = llvm.read_text().replace('@main(', '@original_main(')
        llvm.write_text(text)
        ctype = 'int32_t' if carrier == 'int32' else 'int64_t'
        driver = root / 'driver.c'
        driver.write_text('#include <stdint.h>\n#include <stdio.h>\n'
                          f'extern {ctype} specimen(void) __asm__("flow.function.{function["symbol_id"]}");\n'
                          'int main(void) { printf("%lld\\n", (long long)specimen()); return 0; }\n')
        for optimization in ('-O0', '-O2'):
            binary = root / (name + optimization)
            result = subprocess.run(['clang-18', optimization, str(llvm), str(driver), '-o', str(binary)],
                                    capture_output=True, text=True, timeout=30)
            assert result.returncode == 0, result.stderr
            result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
            assert result.returncode == 0 and int(result.stdout) == expected, (name, result.stdout)
        tiny = root / (name + '.tvm')
        good('tiny_lower', prepared, '-', tiny)
        result = json.loads(good('tiny_run', None, tiny))
        assert result['status'] == 'completed' and result['result'] == 0, (name, result)
    for name, source in negatives:
        result = analyze(name, source)
        assert result.returncode == 2, (name, result.stdout, result.stderr)
        report = json.loads(result.stdout)
        assert report['lowering_plan']['status'] == 'blocked', name
        assert 'FLOWANALYST_INTEGER_ADMISSION_REFUSED' in result.stdout, (name, result.stdout)
    attacks = 0
    parallel_capture = good('parallel', json.dumps(captured))
    optimized_capture = good('optimize', parallel_capture)
    prepared_capture = good('prepare', optimized_capture)
    for stage, artifact, args in (
        ('optimize', parallel_capture, []),
        ('prepare', optimized_capture, []),
        ('lower', optimized_capture, ['--emit-llvm', root / 'bad.ll']),
        ('lower', prepared_capture, ['--emit-llvm', root / 'bad-prepared.ll']),
        ('tiny_lower', prepared_capture, ['-', root / 'bad.tvm']),
    ):
        report = json.loads(artifact)
        conversion = next(o['operands'][0] for o in report['lowering_plan']['operations']
                          if o['operands'] and o['operands'][0].get('kind') == 'conversion')
        conversion.pop('conversion')
        result = run(stage, json.dumps(report), *args)
        assert result.returncode != 0, (stage, result.stdout)
        attacks += 1
    assert not list(root.glob('bad*'))
    for mutation in ('marker', 'placement', 'source_type', 'range', 'unary', 'missing_result_catalog'):
        for version in (1, 2):
            report = json.loads(json.dumps(captured))
            plan = report['lowering_plan']
            plan['version'] = version
            operation = next(o for o in plan['operations'] if o['operands'] and o['operands'][0].get('kind') == 'conversion' and o['function_symbol_id'] == next(f['symbol_id'] for f in plan['functions'] if f['name'] == 'specimen'))
            operand = operation['operands'][0]
            if mutation == 'unary':
                operand['kind'] = 'unary'
                operand['operator'] = '+'
                operand.pop('conversion')
                operand.pop('from_type')
            elif mutation == 'missing_result_catalog': plan.pop('functions')
            elif mutation == 'marker': operand.pop('conversion')
            elif mutation == 'placement': operation['kind'] = 'assignment'
            elif mutation == 'source_type': operand['from_type'] = 'int64'
            elif mutation == 'range':
                operand['type'] = 'int32'
                operand['from_type'] = 'int64'
                operand['operand']['type'] = 'int64'
                for op in plan['operations']:
                    if op.get('result_symbol_id') == operand['operand']['symbol_id']:
                        op['operands'][0].update(type='int64', value='2147483648')
            for stage in ('validate', 'bind', 'parallel'):
                result = run(stage, json.dumps(report))
                assert result.returncode != 0, (mutation, version, stage)
                attacks += 1
    print(json.dumps(dict(positive_sources=len(positives), negative_sources=len(negatives),
                          hostile_boundaries=attacks, native_full_results='O0/O2', tinyvm_full_value_comparisons=True)))
