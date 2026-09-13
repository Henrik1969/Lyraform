# Lyraform tester/critic report

## Summary

<!-- One sentence describing the failure, surprising behavior, or criticism. -->

## Classification

- [ ] build/configuration
- [ ] test failure
- [ ] runtime/native execution
- [ ] diagnostics or refusal behavior
- [ ] documentation mismatch
- [ ] architecture/semantics criticism
- [ ] security or isolation concern
- [ ] reproducibility/determinism

## Exact revision

```text
git rev-parse HEAD:
git status --short --branch:
repository:
```

## Environment

```text
OS/distribution:
architecture:
kernel:
cmake:
ninja:
cc/c++:
clang:
python3:
jq:
libncursesw:
valgrind (if used):
```

## Commands run

```text
./tools/check-onboarding-prerequisites.sh
./igor doctor
./igor --build-dir /tmp/lyraform-onboarding-build test
```

Add any focused command or environment variable here.

## Expected result

<!-- State the documented or contractually expected behavior. -->

## Actual result

<!-- Include the exact structured diagnostic, exit status, and relevant output. -->

```text

```

## Reproduction

<!-- Include the smallest source, artifact, policy, or command sequence that
     reproduces the finding. Remove secrets and unrelated local paths. -->

## Evidence

- failing CTest name:
- captured artifact/log path:
- native executable or exit status:
- sanitizer/Valgrind result:
- screenshot or image, if relevant:

## Impact and suggested direction

<!-- Explain who is affected and why this matters. A proposed fix is welcome,
     but keep observed facts separate from interpretation. -->
