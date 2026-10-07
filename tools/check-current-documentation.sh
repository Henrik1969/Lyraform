#!/bin/sh
set -eu

root=${FLOWCORE_ROOT:?}
status=$root/docs/current-status.md
presentation=$root/docs/presentation/current-status-v1.json

fail() {
    printf 'current-documentation drift: %s\n' "$1" >&2
    exit 1
}

require_text() {
    file=$1
    expected=$2
    grep -Fq "$expected" "$file" || fail "$file lacks required current truth: $expected"
}

require_text "$root/README.md" '# Lyraform'
require_text "$root/README.md" 'docs/current-status.md'
require_text "$root/docs/index.md" 'current-status.md'
require_text "$root/Lyraform/README.md" 'docs/current-status.md'
require_text "$root/docs/presentation/README.md" '# Lyraform presentation authority'
require_text "$status" 'authority:     main'
require_text "$status" 'toolchain:     Igor'
require_text "$status" 'experimental / unstable / not production-ready'
require_text "$status" 'certification: none claimed'
require_text "$status" 'checkpoints/2026-09-26-current-truth-consolidation.md'
require_text "$status" 'checkpoints/2026-09-26-bounded-unique-transfer-forwarding-chain.md'
require_text "$status" 'checkpoints/2026-09-26-canonical-authority-convergence-gate-0.md'
require_text "$status" 'checkpoints/2026-09-26-canonical-source-operation-coverage.md'
require_text "$status" 'checkpoints/2026-09-26-canonical-scalar-operation-authority.md'
require_text "$status" 'checkpoints/2026-09-26-canonical-target-producer-audit.md'
require_text "$status" 'checkpoints/2026-09-26-canonical-guard-disposition-authority.md'
require_text "$status" 'checkpoints/2026-09-27-canonical-text-outcome-ownership-authority.md'
require_text "$status" 'checkpoints/2026-09-27-canonical-provider-effect-authority.md'
require_text "$status" 'checkpoints/2026-09-27-canonical-operation-connectivity.md'
require_text "$status" 'checkpoints/2026-09-27-canonical-parallel-candidate-connectivity.md'
require_text "$status" 'checkpoints/2026-09-27-canonical-authority-preservation.md'
require_text "$status" 'checkpoints/2026-09-27-canonical-authority-envelope.md'
require_text "$status" 'checkpoints/2026-09-27-whole-language-authority-audit.md'
require_text "$status" 'checkpoints/2026-09-27-canonical-failure-consumer-authority.md'
require_text "$status" 'checkpoints/2026-09-27-failure-response-disposition-decision-brief.md'
require_text "$status" 'architecture/decisions/0063-bounded-disposition-evidence-epochs.md'
require_text "$status" 'checkpoints/2026-10-07-convergence-integration.md'
require_text "$status" 'architecture/canonical-source-form-audit-v1.json'
require_text "$status" 'checkpoints/2026-09-26-legacy-oracle-deprecation.md'
require_text "$status" 'tasks/canonical-language-authority-convergence.md'
require_text "$status" 'deprecated legacy'
require_text "$root/docs/architecture/decisions/0061-legacy-is-deprecated-oracle-only.md" 'Status: accepted.'
require_text "$root/docs/architecture/text-outcome-v0.1.md" 'Those observations do not create owners.'

evergreen_files="
$root/README.md
$root/Lyraform/CURRENT.md
$root/Lyraform/README.md
$root/Flowanalyst/CURRENT.md
$root/Flowcontracts/README.md
$root/docs/current-status.md
$root/docs/index.md
$root/docs/onboarding/README.md
$root/ALPHA-TESTING.md
"

if grep -En '([0-9]+/[0-9]+[[:space:]]+PASS|out of [0-9]+|registered tests:[[:space:]]*[0-9]+|registers [0-9]+ CTest|complete [0-9]+-test)' $evergreen_files; then
    fail 'an evergreen document contains a floating exact test total'
fi

if grep -Fq 'delivered to multiple consumers' "$root/docs/architecture/text-outcome-v0.1.md"; then
    fail 'TextOutcome still claims implicit owned fan-out'
fi

adr_0059=$(grep -n 'ADR-0059:' "$root/docs/architecture/README.md" | cut -d: -f1)
adr_0060=$(grep -n 'ADR-0060:' "$root/docs/architecture/README.md" | cut -d: -f1)
adr_0061=$(grep -n 'ADR-0061:' "$root/docs/architecture/README.md" | cut -d: -f1)
adr_0062=$(grep -n 'ADR-0062:' "$root/docs/architecture/README.md" | cut -d: -f1)
adr_0063=$(grep -n 'ADR-0063:' "$root/docs/architecture/README.md" | cut -d: -f1)
test -n "$adr_0059" && test -n "$adr_0060" && test -n "$adr_0061" && test -n "$adr_0062" && test -n "$adr_0063" || fail 'ADR 0059 through 0063 index entries are missing'
test "$adr_0059" -lt "$adr_0060" || fail 'architecture decision index is out of numerical order'
test "$adr_0060" -lt "$adr_0061" || fail 'architecture decision index is out of numerical order'
test "$adr_0061" -lt "$adr_0062" || fail 'architecture decision index is out of numerical order'
test "$adr_0062" -lt "$adr_0063" || fail 'architecture decision index is out of numerical order'

jq -e '
  .schema == "flowcore.presentation-status/v1" and
  .project.name == "Lyraform" and
  .project.repository == "Henrik1969/Lyraform" and
  .project.production_ready == false and
  .source_state.branch == "main" and
  (.source_state.commit | type == "string" and length == 40) and
  .verification.canonical_ctest.passed == .verification.canonical_ctest.total and
  .verification.canonical_ctest.total > 0 and
  (.authority | index("docs/checkpoints/2026-09-26-current-truth-consolidation.md")) != null and
  (.authority | index("docs/checkpoints/2026-09-26-bounded-unique-transfer-forwarding-chain.md")) != null and
  (.authority | index("docs/checkpoints/2026-10-07-convergence-integration.md")) != null and
  (.authority | index("docs/checkpoints/2026-09-27-canonical-failure-consumer-authority.md")) != null and
  (.authority | index("docs/checkpoints/2026-09-26-uniform-owned-transfer.md")) != null
' "$presentation" >/dev/null || fail 'presentation status snapshot is invalid or contradicts current authority'

snapshot_commit=$(jq -r '.source_state.commit' "$presentation")
git -C "$root" cat-file -e "$snapshot_commit^{commit}" 2>/dev/null ||
    fail 'presentation snapshot commit is not present in repository history'

jq -r '.authority[]' "$presentation" | while IFS= read -r authority; do
    test -f "$root/$authority" || fail "presentation authority is missing: $authority"
done

python3 - "$root" \
    README.md \
    Lyraform/CURRENT.md \
    Lyraform/README.md \
    Flowanalyst/CURRENT.md \
    Flowcontracts/README.md \
    docs/current-status.md \
    docs/index.md \
    docs/flowmini/testing.md \
    docs/architecture/README.md \
    docs/architecture/text-outcome-v0.1.md \
    docs/onboarding/README.md \
    docs/presentation/README.md \
    ALPHA-TESTING.md <<'PY'
import pathlib
import re
import sys

root = pathlib.Path(sys.argv[1]).resolve()
failed = []
pattern = re.compile(r"(?<!!)\[[^]]*\]\(([^)]+)\)")
for relative in sys.argv[2:]:
    source = root / relative
    text = source.read_text(encoding="utf-8")
    for raw in pattern.findall(text):
        target = raw.strip().split("#", 1)[0]
        if not target or "://" in target or target.startswith("mailto:"):
            continue
        target = target.strip("<>")
        resolved = (source.parent / target).resolve()
        if not resolved.exists():
            failed.append(f"{relative}: {raw}")
if failed:
    print("current-documentation drift: broken local links", file=sys.stderr)
    for item in failed:
        print(f"  {item}", file=sys.stderr)
    raise SystemExit(1)
PY

printf '%s\n' 'Current Lyraform documentation: PASS'
