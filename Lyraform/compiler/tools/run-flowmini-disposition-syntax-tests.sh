#!/usr/bin/env bash
set -euo pipefail

root="${FLOWMINI_ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
compiler="${FLOWMINI_BIN:-$root/cmake-build-debug/flowmini}"
tmpdir="$(mktemp -d)"
cleanup() { /usr/bin/rm -rf -- "$tmpdir"; }
trap cleanup EXIT

positive="$root/examples/disposition/text_outcome_consumer.flow"
"$compiler" --dump-frontend-bundle "$positive" > "$tmpdir/positive.json"
"$compiler" --dump-frontend-bundle "$positive" > "$tmpdir/repeated.json"
cmp "$tmpdir/positive.json" "$tmpdir/repeated.json"

jq -e '
  .parse_validity.state=="outside_scope" and
  .parse_validity.scope=="canonical_disposition_structure" and
  .parse_validity.coverage=="complete" and
  .parse_validity.recovery_used==false and
  .disposition_syntax.format=="flowmini.disposition_syntax" and
  .disposition_syntax.version==1 and
  .disposition_syntax.status=="structural" and
  .disposition_syntax.execution=="unsupported" and
  (.graph_syntax.nodes|length)==0 and (.graph_syntax.wires|length)==0 and
  (.disposition_syntax.functions|length)==2 and
  (.disposition_syntax.functions[]|select(.name=="compose_text")|
    .result_form=="ordinary" and .result_type=="Text" and
    .failures.present==true and [.failures.types[].spelling]==["TextFailure"] and
    .faults.present==true and [.faults.types[].spelling]==["TextIntegrityFault"]) and
  (.disposition_syntax.functions[]|select(.name=="use_fallback")|
    .result_form=="recover" and .result_type=="Text" and
    .parameters[0].type_form=="failure_envelope" and
    .parameters[0].payload_type=="TextFailure") and
  (.disposition_syntax.consumers[0]|
    .name=="text_failures" and [.members[].function_name]==["use_fallback"]) and
  [.disposition_syntax.nodes[].structural_id]==[0,1,2,3] and
  [.disposition_syntax.nodes[].implementation_kind]==
    ["source_function","consumer_instance","source_function","activation_containment"] and
  [.disposition_syntax.wires[].structural_id]==[0,1,2,3] and
  [.disposition_syntax.wires[].wire_id]==
    ["disposition-wire:0","disposition-wire:1","disposition-wire:2","disposition-wire:3"] and
  [.disposition_syntax.wires[].from.port_id]==["out","failure","fault","out"] and
  [.disposition_syntax.wires[].to.port_id]==["success","failure","fault","in"]
' "$tmpdir/positive.json" >/dev/null

imported="$root/examples/disposition/imported_disposition.flow"
"$compiler" --dump-frontend-bundle "$imported" > "$tmpdir/imported.json"
jq -e '
  (.disposition_syntax.functions[]|select(.name=="imported_transform")|
    .result_form=="transform" and .result_type=="OtherFailure" and
    .parameters[0].type_form=="failure_envelope" and
    (.parameters[0].provenance.source|endswith("examples/disposition/imported_response.flow"))) and
  (.disposition_syntax.consumers[]|select(.name=="imported_failures")|
    (.provenance.source|endswith("examples/disposition/imported_response.flow")) and
    (.members[0].provenance.source|endswith("examples/disposition/imported_response.flow")))
' "$tmpdir/imported.json" >/dev/null

reject() {
  local name="$1"
  local source="$2"
  printf '%s\n' "$source" > "$tmpdir/$name.flow"
  if "$compiler" --dump-frontend-bundle "$tmpdir/$name.flow" > "$tmpdir/$name.json" 2> "$tmpdir/$name.err"; then
    echo "accepted malformed disposition syntax: $name" >&2
    exit 1
  fi
  test -s "$tmpdir/$name.err"
}

reject incomplete_set 'program bad
fn produce(): Text fails { TextFailure
main { return 0 }'
reject duplicate_clause 'program bad
fn produce(): Text fails { TextFailure } fails { OtherFailure } { return 0 }
main { return 0 }'
reject duplicate_type 'program bad
fn produce(): Text fails { TextFailure, TextFailure } { return 0 }
main { return 0 }'
reject malformed_envelope 'program bad
fn response(problem : failure): recover Text { return 0 }
main { return 0 }'
reject malformed_result 'program bad
fn response(problem : failure TextFailure): recover { return 0 }
main { return 0 }'
reject duplicate_member 'program bad
fn response(problem : failure TextFailure): recover Text { return 0 }
consumer failures {
  response
  response
}
main { return 0 }'
reject wrong_position 'program bad
main {
  consumer failures { response }
}'
reject missing_wire_arrow 'program bad
node producer_node : fn produce
node handled : consumer failures
wire producer_node.failure handled.failure
main { return 0 }'
reject malformed_endpoint 'program bad
node producer_node : fn produce
node handled : consumer failures
wire producer_node.failure => handled
main { return 0 }'
reject malformed_containment 'program bad
node quarantine : containment process
main { return 0 }'

echo 'Disposition syntax: PASS (2 deterministic positive projections, imported spans, 10 malformed forms refused)'
