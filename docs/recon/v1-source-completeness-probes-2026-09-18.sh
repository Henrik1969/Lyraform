#!/bin/sh
# Observational Phase 4 probes, not an admission test or a new language policy.
set -eu
build=${1:-/tmp/lyraform-phase2-build}
out=$(mktemp -d /tmp/lyraform-phase4-recon.XXXXXX)
compiler="$build/flowmini/flowmini"
analyst="$build/flowanalyst/flowanalyst"
printf 'evidence_directory=%s\n' "$out"
printf 'probe\tfrontend_rc\tanalyst_rc\tstatus\tplan\tstatements\texpressions\tdeclarations\n' > "$out/results.tsv"
probe() {
    name=$1
    # .flowir bypasses import expansion for raw structural-parser observation.
    # --dump-frontend-bundle still invokes the structural parser, not the IR parser.
    printf '%s\n' "$2" > "$out/$name.flowir"
    set +e
    timeout 5 "$compiler" --dump-frontend-bundle "$out/$name.flowir" > "$out/$name.frontend.json" 2> "$out/$name.frontend.err"
    front=$?
    analysis=NA
    if test "$front" -eq 0; then
        timeout 5 "$analyst" "$out/$name.frontend.json" > "$out/$name.analysis.json" 2> "$out/$name.analysis.err"
        analysis=$?
    fi
    set -e
    if test "$front" -eq 0; then
        jq -S '{ast:.ast,graph:.graph_syntax} | walk(if type=="object" then del(.location,.provenance) else . end)' "$out/$name.frontend.json" > "$out/$name.normalized.json"
        summary=$(jq -r '[.status // "none",.lowering_plan.status // "none"] | @tsv' "$out/$name.analysis.json")
        counts=$(jq -r '[.ast.statement_pool|length] + [.ast.expression_pool|length] + [.ast.declaration_pool|length] | @tsv' "$out/$name.frontend.json")
        printf '%s\t%s\t%s\t%s\t%s\n' "$name" "$front" "$analysis" "$summary" "$counts" >> "$out/results.tsv"
    else
        printf '%s\t%s\tNA\tNA\tNA\tNA\tNA\tNA\n' "$name" "$front" >> "$out/results.tsv"
    fi
}
body() { probe "$1" "program probe
main {
$2
}"; }
body scalar 'x : int(20)
print x'
body decl_tail 'x : int(20) junk 99
print x'
body literal_tail 'x : int(20 99)
print x'
body uninitialized 'x : int'
body place 'x : int(0)
20 -> x
x -> return'
body place_type 'x : int(0)
20 -> x : Bool
x -> return'
body place_print 'x : int(0)
20 -> x
print x'
body place_type_print 'x : int(0)
20 -> x : Bool
print x'
body place_junk 'x : int(0)
20 -> x junk 99
x -> return'
body return_keyword 'return 20'
body return_tail 'return 20 99'
body return_arrow '20 -> return'
body unary 'x : int(-20)
flag : Bool(not false)'
body binary 'x : int(2 + 3 * 4)'
body parenthesized 'x : int((2 + 3) * 4)'
body missing_paren 'x : int((20)'
body extra_paren 'x : int(20))'
body unknown 'x : int(20)
mystery 99
print x'
body nested_block 'x : int(20)
{ true -> x }
print x'
body member '20 -> p.x'
body member_deep '20 -> p.a.b'
body member_dot '20 -> p.x.'
body member_double_dot '20 -> p..x'
body member_index '20 -> p.x[i]'
body index '20 -> items[i]'
body index_two '20 -> matrix[r,c]'
body index_missing '20 -> items[i'
body index_empty '20 -> items[]'
body index_gap '20 -> matrix[r,,c]'
body index_field '20 -> items[i].field'
body index_again '20 -> items[i][j]'
body access 'x : int(p.a.b)
y : int(items[i].field)'
body list 'x : list<int>([1,2,3])'
body list_gap 'x : list<int>([1,,2,3])'
body array 'x : array<int>[2,3]([1,2,3,4,5,6])'
body array_missing 'x : array<int>[2,3([1,2,3])'
body record_literal 'p : Point({x:1,y:2})'
body record_literal_bad 'p : Point({x:1 junk,y:2})'
body if_ok 'x : int(0)
if true { 20 -> x }
x -> return'
body if_tail 'x : int(0)
if true junk 99 { 20 -> x }
x -> return'
body if_missing 'if true'
body else_chain 'if true { break }
else if false { continue }
else { return 1 }'
body else_dangling 'x : int(0)
else { 99 -> x }
x -> return'
body while_ok 'while false { break }'
body while_tail 'while false junk 99 { break }'
body break_tail 'break junk 99
continue junk 42'
probe type_bare 'program probe
type Point {
 x : int
 y : int
}
main { return 0 }'
probe type_field 'program probe
type Point {
 field x : int
 field y : int
}
main { return 0 }'
probe type_tail 'program probe
type Point {
 x : int junk 99
 y : int
}
main { return 0 }'
probe type_missing 'program probe
type Point {
 x : int'
probe function 'program probe
fn f(a : int, b : int) : int { a -> return }
main { x : int(f(1,2))
x -> return }'
probe parameter_gap 'program probe
fn f(a : int,, b : int) : int { a -> return }
main { x : int(f(1,2))
x -> return }'
probe call_gap 'program probe
fn f(a : int, b : int) : int { a -> return }
main { x : int(f(1,,2))
x -> return }'
probe function_header_tail 'program probe
fn f(a : int) : int junk { a -> return }
main { return 0 }'
probe import_ok 'program probe
import "missing.flow" as lib
main { return 0 }'
probe import_tail 'program probe
import "missing.flow" as lib junk 99
main { return 0 }'
probe target 'program probe
target native {
main { return 0 }
}'
probe target_extra 'program probe
target native {
port x : int
main { return 0 }
}'
probe node 'program probe
node a : int.add'
probe node_tail 'program probe
node a : int.add junk'
probe wire 'program probe
wire a.out => b.in'
probe wire_bad 'program probe
wire a.out.extra => b.in'
probe wire_tail 'program probe
wire a.out => b.in junk'
probe wire_bare 'program probe
a.out => b.in'
probe provider_unknown 'program probe
provider p : Whatever
contract C { port x : int }
main { return 0 }'
probe abi 'program probe
abi C {
library "libexample.so"
convention "C"
extern fn f(a : c_int) : c_int
}
main { return 0 }'
probe abi_unknown 'program probe
abi C {
library "libexample.so"
convention "C"
unknown obligation
extern fn f(a : c_int) : c_int
}
main { return 0 }'
probe legacy_module 'module probe
producer a : start.record
sink b : halt.record
wire a.out => b.in'
printf 'pair\tnormalized_ast_and_graph_equal\n' > "$out/pairs.tsv"
for pair in 'scalar decl_tail' 'scalar literal_tail' 'scalar unknown' 'scalar nested_block' 'place place_type' 'place place_junk' 'return_keyword return_tail' 'member member_dot' 'member member_index' 'index index_missing' 'index index_field' 'index index_again' 'type_bare type_field' 'type_bare type_tail' 'if_ok if_tail' 'while_ok while_tail' 'function parameter_gap' 'function call_gap' 'list list_gap' 'record_literal record_literal_bad' 'import_ok import_tail' 'target target_extra' 'abi abi_unknown'; do
    set -- $pair
    if cmp -s "$out/$1.normalized.json" "$out/$2.normalized.json"; then equal=YES; else equal=NO; fi
    printf '%s\t%s\n' "$pair" "$equal" >> "$out/pairs.tsv"
done
cat "$out/results.tsv"
cat "$out/pairs.tsv"
printf 'probe\tdirect_rc\n' > "$out/direct.tsv"
for name in scalar decl_tail literal_tail missing_paren extra_paren unknown nested_block place_print place_type_print; do
    set +e
    "$build/flowmini/flowmini_scalar_path_test" --test-forbid-legacy --trace true "$out/$name.flowir" > "$out/$name.direct.out" 2> "$out/$name.direct.err"
    status=$?
    set -e
    printf '%s\t%s\n' "$name" "$status" >> "$out/direct.tsv"
done
cat "$out/direct.tsv"
if test "${LYRAFORM_RECON_FIXTURES_ONLY:-0}" = 1; then exit 0; fi
# Confirm the staged loss on a normal .flow input, not only raw inspection.
cp "$out/place_type.flowir" "$out/staged-loss.flow"
"$compiler" --dump-frontend-bundle "$out/staged-loss.flow" > "$out/staged-loss.frontend.json"
"$analyst" "$out/staged-loss.frontend.json" > "$out/staged-loss.semantic.json"
"$build/flowtools/flowparallel/flowparallel" "$out/staged-loss.semantic.json" > "$out/staged-loss.execution.json"
"$build/flowoptimize/flowoptimize" "$out/staged-loss.execution.json" > "$out/staged-loss.optimization.json"
"$build/flowlower/flowprepare" "$out/staged-loss.optimization.json" > "$out/staged-loss.lowering.json"
"$build/flowcontracts/flowvalidate" "$out/staged-loss.lowering.json" > "$out/staged-loss.validation.json"
"$build/flowlower/flowlower" --emit-llvm "$out/staged-loss.ll" "$out/staged-loss.lowering.json" > "$out/staged-loss.llvm.json"
clang "$out/staged-loss.ll" -o "$out/staged-loss"
set +e
"$out/staged-loss"
status=$?
set -e
printf 'staged-loss LLVM exit=%s\n' "$status"
test "$status" -eq 20
"$build/tinyvm/flowtinylower" "$out/staged-loss.lowering.json" "$out/staged-loss.tvm" > "$out/staged-loss.tiny.json"
"$build/tinyvm/flowtinyrun" "$out/staged-loss.tvm" > "$out/staged-loss.result.json"
jq -e '.result == 20' "$out/staged-loss.result.json"
