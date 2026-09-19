#!/bin/sh
set -eu
build=$1
root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
tmpdir=$(mktemp -d /tmp/lyraform-target-facts.XXXXXX)
trap 'rm -rf "$tmpdir"' EXIT
compiler="$build/flowmini/flowmini"
analyst="$build/flowanalyst/flowanalyst"
parallel="$build/flowtools/flowparallel/flowparallel"
"$compiler" --dump-frontend-bundle "$root/target-preservation.flow" > "$tmpdir/frontend.json"
for version in 1 2; do
    "$analyst" --lowering-plan-version "$version" "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
    "$analyst" --lowering-plan-version "$version" "$tmpdir/frontend.json" > "$tmpdir/repeated.json"
    cmp "$tmpdir/semantic.json" "$tmpdir/repeated.json"
    jq -S '[.lowering_plan.operations[] | .target_fact // empty]' "$tmpdir/semantic.json" > "$tmpdir/facts.json"
    jq -e 'length==5 and .[0].version==2 and .[0].assignability=="unresolved" and
        all(.[1:3][]; .version==3 and .assignability.state=="assignable" and .assignability.update=="reconstruct_and_rebind") and
        all(.[3:][]; .version==2 and .assignability=="unresolved") and
        ([.[].kind]==["identifier","field_path","field_path","indexed","indexed"]) and
        .[0].destination_type=="int" and .[0].resolution=="resolved" and
        all(.[1:3][]; .base_symbol_id>=0 and .base_type_symbol_id>=0 and .destination_type=="int" and
            .resolution=="resolved" and .execution=="unsupported") and
        ([.[2].members[].name]==["a","b"]) and all(.[2].members[]; .member_symbol_id>=0 and .member_type!=null) and
        .[4].index_count==2 and .[4].indices[0].expression_id != .[4].indices[1].expression_id and
        all(.[3:][]; .base_symbol_id>=0 and .destination_type==null and .resolution=="partially_resolved" and .execution=="unsupported")' "$tmpdir/facts.json" >/dev/null
    jq -S '[.ast.statement_pool[] | select(.kind=="placement" and .payload.target.kind=="indexed") | .payload.target.indexes]' "$tmpdir/frontend.json" > "$tmpdir/parsed-indices.json"
    jq -S '[.[] | select(.kind=="indexed") | [.indices[].expression_id]]' "$tmpdir/facts.json" > "$tmpdir/preserved-indices.json"
    cmp "$tmpdir/parsed-indices.json" "$tmpdir/preserved-indices.json"
    "$build/flowbind/flowbind" "$tmpdir/semantic.json" > "$tmpdir/binding.json"
    jq -S '.target_facts' "$tmpdir/binding.json" > "$tmpdir/bound-facts.json"
    cmp "$tmpdir/facts.json" "$tmpdir/bound-facts.json"
    "$parallel" "$tmpdir/semantic.json" > "$tmpdir/execution.json"
    "$build/flowoptimize/flowoptimize" "$tmpdir/execution.json" > "$tmpdir/optimization.json"
    "$build/flowlower/flowprepare" "$tmpdir/optimization.json" > "$tmpdir/backend.json"
    for stage in execution optimization backend; do
        jq -S '[.lowering_plan.operations[] | .target_fact // empty]' "$tmpdir/$stage.json" > "$tmpdir/stage-facts.json"
        cmp "$tmpdir/facts.json" "$tmpdir/stage-facts.json"
        "$build/flowcontracts/flowvalidate" "$tmpdir/$stage.json" > /dev/null
    done
    if "$build/flowlower/flowlower" --emit-llvm "$tmpdir/forbidden.ll" "$tmpdir/backend.json" > "$tmpdir/llvm.out" 2> "$tmpdir/llvm.err"; then exit 1; fi
    grep -q 'unsupported target kind' "$tmpdir/llvm.err" "$tmpdir/llvm.out"
    if "$build/tinyvm/flowtinylower" "$tmpdir/backend.json" "$tmpdir/forbidden.tvm" > "$tmpdir/tiny.out" 2> "$tmpdir/tiny.err"; then exit 1; fi
    grep -q 'unsupported target kind' "$tmpdir/tiny.err" "$tmpdir/tiny.out"
    test ! -e "$tmpdir/forbidden.ll"
    test ! -e "$tmpdir/forbidden.tvm"
done
for mutation in \
 '.kind="invented"' \
 'del(.base_symbol_id)' \
 '.members[1].ordinal=0' \
 '.indices=[{"ordinal":0}]' \
 '.index_count=99' \
 '.destination_type="Bool"' \
 '.provenance.line=-1' \
 '.kind="identifier"' \
 '.version=99' \
 '.operation_id=999'; do
    case "$mutation" in
        .indices*|.index_count*) selection='.target_fact.kind=="indexed"' ;;
        *) selection='.target_fact.kind=="field_path" and (.target_fact.members|length)==2' ;;
    esac
    jq "(.lowering_plan.operations[] | select($selection) | .target_fact) |= ($mutation)" "$tmpdir/backend.json" > "$tmpdir/hostile.json"
    if "$build/flowcontracts/flowvalidate" "$tmpdir/hostile.json" >/dev/null 2>&1; then exit 1; fi
    if "$build/flowlower/flowlower" --emit-llvm "$tmpdir/forbidden.ll" "$tmpdir/hostile.json" >/dev/null 2>&1; then exit 1; fi
    if "$build/tinyvm/flowtinylower" "$tmpdir/hostile.json" "$tmpdir/forbidden.tvm" >/dev/null 2>&1; then exit 1; fi
done
echo 'Target preservation: PASS (5 target shapes, 2 plan versions, 5 stage boundaries, backend refusal, 10 hostile mutations)'
