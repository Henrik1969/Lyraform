# Lyraform repository rulesets

`protect-main.json` is the reviewable policy artifact for Lyraform's
canonical `main` branch.

## Invariant

Normal development remains direct and uncomplicated, while the canonical
branch cannot be deleted or rewritten destructively.

## Policy

The active ruleset targets only `refs/heads/main` and blocks:

- branch deletion;
- non-fast-forward updates, including force pushes.

It permits ordinary fast-forward direct pushes. It does not require pull
requests, approvals, signed commits, linear history, deployments, or status
checks. It declares no bypass actors.

## Import

In the Lyraform repository on GitHub, open **Settings → Rules → Rulesets**,
choose **New ruleset → Import a ruleset**, select
`.github/rulesets/protect-main.json`, review the preview, and create it.

GitHub documents repository-ruleset imports in
[Managing rulesets for a repository](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-rulesets/managing-rulesets-for-a-repository).

## Verify

After importing, verify in **Settings → Rules → Rulesets** that:

- `protect-main` is active;
- its target is branch `main` only;
- deletion and non-fast-forward rules are present;
- no pull-request, review, signature, deployment, or status-check rules are present;
- no bypass actors are configured.

The ruleset API can also be used to inspect the live configuration:

```bash
gh api repos/Henrik1969/Lyraform/rulesets
gh api repos/Henrik1969/Lyraform/rulesets/<RULESET_ID>
gh api repos/Henrik1969/Lyraform/rules/branches/main
```

The first two responses expose the ruleset definition. The branch-rules
endpoint exposes the effective rules applying to `main`.

## Export and compare

Use GitHub's ruleset page to download the live ruleset JSON later. Save it
outside the repository first if it contains GitHub-managed metadata, then
compare the policy fields with the repository copy:

```bash
jq '{name,target,enforcement,conditions,rules,bypass_actors}' \
  .github/rulesets/protect-main.json > /tmp/protect-main-repository.json
jq '{name,target,enforcement,conditions,rules,bypass_actors}' \
  /path/to/protect-main-live.json > /tmp/protect-main-live.json
diff -u /tmp/protect-main-repository.json /tmp/protect-main-live.json
```

The exported live file may contain IDs, source metadata, timestamps, links,
or other server-managed fields. Compare the policy fields rather than those
generated fields.
