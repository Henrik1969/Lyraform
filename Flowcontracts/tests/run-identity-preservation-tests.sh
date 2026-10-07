#!/bin/sh
set -eu

flowmini=${FLOWMINI_BIN:?}
flowanalyst=${FLOWANALYST_BIN:?}
flowbind=${FLOWBIND_BIN:?}
flowparallel=${FLOWPARALLEL_BIN:?}
flowoptimize=${FLOWOPTIMIZE_BIN:?}
flowlower=${FLOWLOWER_BIN:?}
fixture=${FLOW_IDENTITY_FIXTURE:?}

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
policy="$tmpdir/policy"
printf '%s\n' \
  'allow libc.so.6 open c io c_string,c_int c_int' \
  'allow libc.so.6 sendfile c io c_int,c_int,c_pointer,c_size_t c_long' \
  'allow libc.so.6 close c io c_int c_int' > "$policy"

"$flowmini" --dump-frontend-bundle "$fixture" > "$tmpdir/frontend.json"
"$flowanalyst" < "$tmpdir/frontend.json" > "$tmpdir/semantic.json"
"$flowbind" --policy "$policy" < "$tmpdir/semantic.json" > "$tmpdir/binding.json"
"$flowparallel" < "$tmpdir/semantic.json" > "$tmpdir/execution.json"
"$flowoptimize" < "$tmpdir/execution.json" > "$tmpdir/optimization.json"
"$flowlower" --binding-report "$tmpdir/binding.json" --emit-llvm "$tmpdir/flowcat.ll" < "$tmpdir/optimization.json" > "$tmpdir/lowering.json"

jq -e '
  .status == "ok" and
  any(.lowering_plan.operations[]; .kind == "loop" and .body_block_id >= 0) and
  any(.lowering_plan.operations[]; .kind == "branch" and .then_block_id >= 0) and
  any(.lowering_plan.operations[]; .kind == "external_call" and .provider.symbol == "sendfile" and .effect_contract.external == "io" and (.argument_resources | length) == 4) and
  any(.lowering_plan.operations[]; .kind == "return_value")
' "$tmpdir/semantic.json" >/dev/null
jq -e '.status == "ready" and all(.capabilities[]; .status == "authorized")' "$tmpdir/binding.json" >/dev/null
jq -e '.status == "ready" and .artifact.backend == "llvm"' "$tmpdir/lowering.json" >/dev/null

for stage in execution optimization; do
  jq -S '.source,.targets,.abi_type_contracts,.lowering_plan' "$tmpdir/semantic.json" > "$tmpdir/semantic.authority"
  jq -S '.source,.targets,.abi_type_contracts,.lowering_plan' "$tmpdir/$stage.json" > "$tmpdir/$stage.authority"
  cmp -s "$tmpdir/semantic.authority" "$tmpdir/$stage.authority"
done

reject_parallel() {
  mutation=$1
  jq "$mutation" "$tmpdir/semantic.json" > "$tmpdir/mutated.json"
  if "$flowparallel" < "$tmpdir/mutated.json" >/dev/null 2>&1; then
    echo "Flowparallel accepted identity mutation: $mutation" >&2
    exit 1
  fi
}

reject_parallel '.source.path = 7'
reject_parallel '.targets = [{"symbol_id":1,"name":7,"main_count":1}]'
reject_parallel '.lowering_plan.operations[1].id = .lowering_plan.operations[0].id'
reject_parallel '(.lowering_plan.operations[] | select(.kind == "loop") | .block_id) = -1'
reject_parallel '(.lowering_plan.operations[] | select(.kind == "loop") | .body_block_id) = -2'
reject_parallel '(.lowering_plan.operations[] | select(.kind == "external_call") | .provider.symbol) = 7'
reject_parallel '.abi_type_contracts[0].name = 7'
reject_parallel '(.lowering_plan.operations[] | select(.kind == "external_call") | .effect_contract.external) = 7'
reject_parallel '(.lowering_plan.operations[] | select(.kind == "external_call") | .argument_resources[0].memory_effect) = 7'
reject_parallel '.binding_requirements += [.binding_requirements[0]]'
reject_parallel '.binding_requirements += [{"contract":"unused","library":"libc.so.6","symbol":"abs","convention":"c","effect":"pure","parameter_types":"c_int","return_type":"c_int","evidence":""}]'

jq '(.capabilities[] | .status) = "denied"' "$tmpdir/binding.json" > "$tmpdir/denied-binding.json"
if "$flowlower" --binding-report "$tmpdir/denied-binding.json" --emit-llvm "$tmpdir/denied.ll" < "$tmpdir/optimization.json" >/dev/null 2>&1; then
  echo 'Flowlower accepted mutated authorization evidence' >&2
  exit 1
fi

echo 'end-to-end identity preservation: PASS'
