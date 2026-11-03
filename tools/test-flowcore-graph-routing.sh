#!/bin/sh
set -eu

flowmini=${FLOWMINI_BIN:?FLOWMINI_BIN is required}
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

cat > "$tmpdir/ports.flow" <<'EOF'
program ports

producer start : start.record
node probe : record.port_probe
sink halt : halt.record

main {
    one : int32(1)
}

wire start.out => probe.left
wire probe.out => halt.in
EOF

"$flowmini" --trace true "$tmpdir/ports.flow" > "$tmpdir/stdout" 2> "$tmpdir/stderr"
grep -Eq 'route start\.out => probe\.left \[wire:[0-9]+\] \[signal:[0-9]+\]' "$tmpdir/stderr"
grep -Eq 'route probe\.out => halt\.in \[wire:[0-9]+\] \[signal:[0-9]+\]' "$tmpdir/stderr"

cat > "$tmpdir/fanout.flow" <<'EOF'
program fanout

producer start : start.record
node left : record.port_probe
node right : record.port_probe
sink left_halt : halt.record
sink right_halt : halt.record

wire start.out => left.left
wire start.out => right.right
wire left.out => left_halt.in
wire right.out => right_halt.in
main { marker : int32(1) }
EOF

"$flowmini" --trace true "$tmpdir/fanout.flow" > "$tmpdir/fanout.stdout" 2> "$tmpdir/fanout.stderr"
left_route=$(grep -E 'route start\.out => left\.left \[wire:0\] \[signal:[0-9]+\]' "$tmpdir/fanout.stderr")
right_route=$(grep -E 'route start\.out => right\.right \[wire:1\] \[signal:[0-9]+\]' "$tmpdir/fanout.stderr")
left_signal=$(printf '%s\n' "$left_route" | sed -E 's/.*\[(signal:[0-9]+)\].*/\1/')
right_signal=$(printf '%s\n' "$right_route" | sed -E 's/.*\[(signal:[0-9]+)\].*/\1/')
test "$left_signal" = "$right_signal"
left_delivery=$(printf '%s\n' "$left_route" | sed -E 's/.*\[(delivery:[0-9]+)\].*/\1/')
right_delivery=$(printf '%s\n' "$right_route" | sed -E 's/.*\[(delivery:[0-9]+)\].*/\1/')
test "$left_delivery" = 'delivery:0'
test "$right_delivery" = 'delivery:1'
# Fan-out schedules two deliveries, without evaluating the producer twice.
test "$(grep -c 'enter start with' "$tmpdir/fanout.stderr")" -eq 1
test "$(grep -c 'enter left with' "$tmpdir/fanout.stderr")" -eq 1
test "$(grep -c 'enter right with' "$tmpdir/fanout.stderr")" -eq 1

cat > "$tmpdir/unconnected.flow" <<'EOF'
program unconnected
producer start : start.record
main { marker : int32(1) }
EOF
"$flowmini" --trace true "$tmpdir/unconnected.flow" > "$tmpdir/unconnected.stdout" 2> "$tmpdir/unconnected.stderr"
grep -Fq 'no wire connected from start.out; dropping envelope' "$tmpdir/unconnected.stderr"

cat > "$tmpdir/bad-port.flow" <<'EOF'
program bad_port
producer start : start.record
sink halt : halt.record
wire start.missing => halt.in
main { marker : int32(1) }
EOF
if "$flowmini" "$tmpdir/bad-port.flow" >/dev/null 2> "$tmpdir/bad-port.stderr"; then
    echo 'invalid source port unexpectedly accepted' >&2
    exit 1
fi
grep -Fq 'source endpoint start.missing is not an output port' "$tmpdir/bad-port.stderr"

cat > "$tmpdir/missing-required.flow" <<'EOF'
program missing_required
producer start : start.record
sink halt : halt.record
main { marker : int32(1) }
EOF
if "$flowmini" "$tmpdir/missing-required.flow" >/dev/null 2> "$tmpdir/missing-required.stderr"; then
    echo 'unconnected required input unexpectedly accepted' >&2
    exit 1
fi
grep -Fq 'required input is not connected: halt.in' "$tmpdir/missing-required.stderr"

# The two probe inputs are alternative activation ports. Either may be absent,
# while the terminal sink's required input remains connected.
cat > "$tmpdir/optional-input.flow" <<'EOF'
program optional_input
producer start : start.record
node probe : record.port_probe
sink halt : halt.record
wire start.out => probe.right
wire probe.out => halt.in
main { marker : int32(1) }
EOF
"$flowmini" --trace true "$tmpdir/optional-input.flow" >/dev/null 2> "$tmpdir/optional-input.stderr"
grep -Fq 'route start.out => probe.right' "$tmpdir/optional-input.stderr"
grep -Fq 'route probe.out => halt.in' "$tmpdir/optional-input.stderr"

cat > "$tmpdir/failure.flow" <<'EOF'
program failure_route
producer input : stdin.text
node parse : parse.int.to_record
sink halt : halt.record
wire input.out => parse.in
wire parse.out => halt.in
main { marker : int32(1) }
EOF
if printf 'not-an-integer\n' | "$flowmini" --trace true "$tmpdir/failure.flow" >/dev/null 2> "$tmpdir/failure.stderr"; then
    echo 'failing graph node unexpectedly succeeded' >&2
    exit 1
fi
grep -Eq 'failure at parse\.in via wire:0 signal:[0-9]+: expected integer input' "$tmpdir/failure.stderr"
grep -Fq 'fatal in parse.int: expected integer input' "$tmpdir/failure.stderr"
grep -Eq 'failure at parse\.in via wire:0 signal:[0-9]+: .*\[delivery:[0-9]+\]' "$tmpdir/failure.stderr"
if grep -Fq 'route parse.out =>' "$tmpdir/failure.stderr"; then
    echo 'failed activation emitted a normal output' >&2; exit 1
fi

echo 'Flowcore graph routing: PASS'
