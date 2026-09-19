#!/bin/sh
set -eu
build=$1
compiler="$build/flowmini/flowmini"
analyst="$build/flowanalyst/flowanalyst"
tmpdir=$(mktemp -d /tmp/lyraform-field-resolution.XXXXXX)
trap 'rm -rf "$tmpdir"' EXIT
write_source() {
    printf 'program field_resolution\ntype Inner {\n b : int\n}\ntype Point {\n x : int\n y : Bool\n a : Inner\n}\nmain {\n%s\n}\n' "$1" > "$tmpdir/input.flow"
}
analyze() {
    "$compiler" --dump-frontend-bundle "$tmpdir/input.flow" > "$tmpdir/frontend.json"
    "$analyst" "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
    "$analyst" "$tmpdir/frontend.json" > "$tmpdir/repeated.json"
    cmp "$tmpdir/semantic.json" "$tmpdir/repeated.json"
}
write_source 'p : Point({x:0,y:false,a:{b:0}})
20 -> p.x
true -> p.y
20 -> p.a.b'
analyze
jq -e '.status=="ok" and ([.lowering_plan.operations[]|.target_fact//empty|select(.kind=="field_path")] | length)==3' "$tmpdir/semantic.json" >/dev/null
jq -S '[.lowering_plan.operations[]|.target_fact//empty|select(.kind=="field_path")]' "$tmpdir/semantic.json" > "$tmpdir/facts.json"
jq -e '([.[].destination_type]==["int","Bool","int"]) and
    all(.[]; .version==3 and .resolution=="resolved" and .assignability.state=="assignable" and
        .assignability.authority=="lyraform.aggregate_reconstruction/v1" and
        .assignability.root_authority=="established_local_rebinding/v1" and
        .assignability.update=="reconstruct_and_rebind" and
        .assignability.source_expression_id>=0 and .assignability.root_declaration_statement_id>=0 and
        .assignability.source_type==.assignability.destination_type and .execution=="unsupported" and .failure==null) and
    ([.[2].members[].name]==["a","b"]) and
    .[2].members[0].owner_type=="Point" and .[2].members[0].member_type=="Inner" and
    .[2].members[1].owner_type=="Inner" and .[2].members[1].member_type=="int" and
    all(.[2].members[]; .owner_type_symbol_id>=0 and .member_symbol_id>=0 and
        (.member_declaration_path|startswith("/declaration_pool/")) and
        .member_declaration_location.line>0 and .member_declaration_location.column>0)' "$tmpdir/facts.json" >/dev/null
"$build/flowbind/flowbind" "$tmpdir/semantic.json" > "$tmpdir/binding.json"
"$build/flowtools/flowparallel/flowparallel" "$tmpdir/semantic.json" > "$tmpdir/execution.json"
"$build/flowoptimize/flowoptimize" "$tmpdir/execution.json" > "$tmpdir/optimization.json"
"$build/flowlower/flowprepare" "$tmpdir/optimization.json" > "$tmpdir/backend.json"
for stage in binding execution optimization backend; do
    if test "$stage" = binding; then jq -S '[.target_facts[]|select(.kind=="field_path")]' "$tmpdir/$stage.json" > "$tmpdir/stage.json"
    else jq -S '[.lowering_plan.operations[]|.target_fact//empty|select(.kind=="field_path")]' "$tmpdir/$stage.json" > "$tmpdir/stage.json"; fi
    cmp "$tmpdir/facts.json" "$tmpdir/stage.json"
done
refuse_assignment() {
    write_source "$1"
    "$compiler" --dump-frontend-bundle "$tmpdir/input.flow" > "$tmpdir/frontend.json"
    set +e
    "$analyst" "$tmpdir/frontend.json" > "$tmpdir/refused.json"
    rc=$?
    set -e
    test "$rc" -eq 2
    jq -e --arg source "$2" --arg destination "$3" '.status=="error" and .lowering_plan.status=="blocked" and
        any(.diagnostics[]; .code=="FLOWANALYST_MEMBER_ASSIGNABILITY_REFUSED") and
        any(.lowering_plan.operations[]; .target_fact.assignability.state?=="refused" and
            .target_fact.resolution=="resolved" and .target_fact.destination_type==$destination and
            .target_fact.assignability.source_type==$source and .target_fact.assignability.destination_type==$destination and
            .target_fact.failure.code=="FLOWANALYST_MEMBER_ASSIGNABILITY_REFUSED")' "$tmpdir/refused.json" >/dev/null
    jq '.status="ok" | .lowering_plan.status="ready"' "$tmpdir/refused.json" > "$tmpdir/forged.json"
    if "$build/flowcontracts/flowvalidate" "$tmpdir/forged.json" >/dev/null 2>&1; then exit 1; fi
    if "$build/flowlower/flowlower" --emit-llvm "$tmpdir/forbidden.ll" "$tmpdir/forged.json" >/dev/null 2>&1; then exit 1; fi
    if "$build/tinyvm/flowtinylower" "$tmpdir/forged.json" "$tmpdir/forbidden.tvm" >/dev/null 2>&1; then exit 1; fi
}
refuse_assignment 'p : Point({x:0,y:false,a:{b:0}})
true -> p.x' Bool int
refuse_assignment 'p : Point({x:0,y:false,a:{b:0}})
20 -> p.y' int Bool
# An aggregate without an explicit initializer has no established local
# rebinding authority in this bounded stage.
write_source 'p : Point
20 -> p.x'
"$compiler" --dump-frontend-bundle "$tmpdir/input.flow" > "$tmpdir/frontend.json"
"$analyst" "$tmpdir/frontend.json" > "$tmpdir/unresolved.json"
jq -e '.status=="ok" and any(.lowering_plan.operations[];
    .target_fact.version?==3 and .target_fact.resolution=="resolved" and
    .target_fact.assignability.state=="unresolved" and .target_fact.assignability.authority==null)' "$tmpdir/unresolved.json" >/dev/null
# A later initializer cannot retroactively authorize an earlier placement.
write_source '20 -> p.x
p : Point({x:0,y:false,a:{b:0}})'
"$compiler" --dump-frontend-bundle "$tmpdir/input.flow" > "$tmpdir/frontend.json"
"$analyst" "$tmpdir/frontend.json" > "$tmpdir/forward-root.json"
jq -e '.status=="ok" and any(.lowering_plan.operations[];
    .target_fact.version?==3 and .target_fact.resolution=="resolved" and
    .target_fact.assignability.state=="unresolved")' "$tmpdir/forward-root.json" >/dev/null
# Parameters are not local rebinding destinations under this bounded authority.
printf 'program parameter_root\ntype Point {\n x : int\n}\nfn update(p : Point): int {\n20 -> p.x\n20 -> return\n}\nmain {\n}\n' > "$tmpdir/input.flow"
"$compiler" --dump-frontend-bundle "$tmpdir/input.flow" > "$tmpdir/frontend.json"
"$analyst" "$tmpdir/frontend.json" > "$tmpdir/parameter.json"
jq -e '.status=="ok" and any(.lowering_plan.operations[];
    .target_fact.version?==3 and .target_fact.resolution=="resolved" and
    .target_fact.assignability.state=="unresolved" and .target_fact.assignability.root_authority==null)' "$tmpdir/parameter.json" >/dev/null
refuse() {
    write_source "$1"
    "$compiler" --dump-frontend-bundle "$tmpdir/input.flow" > "$tmpdir/frontend.json"
    set +e
    "$analyst" "$tmpdir/frontend.json" > "$tmpdir/refused.json"
    rc=$?
    set -e
    test "$rc" -eq 2
    jq -e --arg code "$2" '.status=="error" and .lowering_plan.status=="blocked" and
        any(.diagnostics[]; .code==$code) and
        any(.lowering_plan.operations[]; (.target_fact.resolution?=="refused") and .target_fact.assignability.state=="unresolved" and .target_fact.destination_type==null)' "$tmpdir/refused.json" >/dev/null
    jq '.status="ok" | .lowering_plan.status="ready"' "$tmpdir/refused.json" > "$tmpdir/forged.json"
    if "$build/flowtools/flowparallel/flowparallel" "$tmpdir/forged.json" >/dev/null 2>&1; then exit 1; fi
}
refuse 'p : Point({x:0,y:false,a:{b:0}})
20 -> p.z' FLOWANALYST_FIELD_NOT_FOUND
refuse 'p : Point({x:0,y:false,a:{b:0}})
20 -> p.x.foo' FLOWANALYST_FIELD_OWNER_NOT_AGGREGATE
refuse '20 -> missing.x' FLOWANALYST_TARGET_BASE_UNRESOLVED
refuse 'x : int(0)
20 -> x.foo' FLOWANALYST_FIELD_OWNER_NOT_AGGREGATE
printf 'program duplicate_field\ntype Point {\n x : int\n x : Bool\n}\nmain {\np : Point({x:0})\n20 -> p.x\n}\n' > "$tmpdir/input.flow"
"$compiler" --dump-frontend-bundle "$tmpdir/input.flow" > "$tmpdir/frontend.json"
set +e
"$analyst" "$tmpdir/frontend.json" > "$tmpdir/refused.json"
rc=$?
set -e
test "$rc" -eq 2
jq -e 'any(.diagnostics[]; .code=="FLOWANALYST_FIELD_AMBIGUOUS") and .lowering_plan.status=="blocked"' "$tmpdir/refused.json" >/dev/null
# Serialized contradictions are rejected independently of backend support.
for mutation in \
 '.members[0].owner_type_symbol_id=999' \
 '.members[0].name="other"' \
 '.members[0].member_type="Bool"' \
 '.members[1].ordinal=0' \
 '.members[1].owner_type="Point"' \
 '.destination_type="Bool"' \
 '.members[0].member_symbol_id=null' \
 '.kind="indexed"'; do
    jq "(.lowering_plan.operations[]|select(.target_fact.members|length==2)|.target_fact)|=($mutation)" "$tmpdir/backend.json" > "$tmpdir/hostile.json"
    if "$build/flowcontracts/flowvalidate" "$tmpdir/hostile.json" >/dev/null 2>&1; then exit 1; fi
    if "$build/flowlower/flowlower" --emit-llvm "$tmpdir/forbidden.ll" "$tmpdir/hostile.json" >/dev/null 2>&1; then exit 1; fi
    if "$build/tinyvm/flowtinylower" "$tmpdir/hostile.json" "$tmpdir/forbidden.tvm" >/dev/null 2>&1; then exit 1; fi
done
# Assignability authority is a compatibility contract, not advisory metadata.
for mutation in \
 '.assignability.state="unresolved"' \
 '.assignability.authority="invented"' \
 '.assignability.root_authority="invented"' \
 '.assignability.update="mutate_in_place"' \
 '.assignability.source_expression_id=999' \
 '.assignability.source_type="Bool"' \
 '.assignability.destination_type="Bool"' \
 '.assignability.root_declaration_statement_id=-1'; do
    jq "(.lowering_plan.operations[]|select(.target_fact.assignability.state?==\"assignable\")|.target_fact)|=($mutation)" "$tmpdir/backend.json" > "$tmpdir/hostile.json"
    if "$build/flowcontracts/flowvalidate" "$tmpdir/hostile.json" >/dev/null 2>&1; then exit 1; fi
    if "$build/flowlower/flowlower" --emit-llvm "$tmpdir/forbidden.ll" "$tmpdir/hostile.json" >/dev/null 2>&1; then exit 1; fi
    if "$build/tinyvm/flowtinylower" "$tmpdir/hostile.json" "$tmpdir/forbidden.tvm" >/dev/null 2>&1; then exit 1; fi
done
# Mission 06 mixed forms remain parser refusals.
for target in 'p.x[i]' 'items[i].field' 'a.b[i].c'; do
    write_source "p : Point({x:0,y:false,a:{b:0}})
20 -> $target"
    "$compiler" --dump-frontend-bundle "$tmpdir/input.flow" > "$tmpdir/mixed.json"
    jq -e '.parse_validity.state=="recovered" and any(.ast.statement_pool[]; .kind=="unknown")' "$tmpdir/mixed.json" >/dev/null
done
echo 'Field resolution and bounded member assignability: PASS (3 admitted paths, 2 type refusals, 3 unresolved roots, 5 target refusals, 16 hostile facts, 3 mixed parser refusals)'
