# Troubleshooting

Use this when you are confused, blocked, or see conflicting instructions.

## Immediate rule
Stop and ask the user if you cannot resolve the issue from this file.
Do not try multiple alternative approaches.

## Conflicting instructions
- Follow the stricter rule.
- If it is still unclear, stop and ask the user.

## Preflight and task claim
- If you cannot complete preflight, stop and ask.
- The claim must be merged into main and visible on GitHub before any task work starts.
- If the claim is not visible, stop and ask.

## Task selection
- Only use the top-to-bottom scan rule in Docs\TODO.md.
- Do not run searches or extra commands to find tasks.

## Build and tests
- Use MSBuild only.
- Do not run CMake. If `build\DropletAnalyzer.sln` is missing, stop and ask.
- Run only the tests required by the task.

## Test artifacts and cleanup
- Prefer tests that write to a temp path and delete files in teardown.
- Do not both modify tests and add .gitignore entries for the same artifact.
- Do not delete files unless instructed.

## Missing tools or paths
- If a required tool or path is missing, stop and ask before searching or installing.
