# AGENTS_V2

This repo supports many parallel agents.

## Keep context small

- Specs are split into Docs\TECHSPEC_SPLIT.
- Do not open TECHSPEC.md unless a split file is missing content.
- Read only the spec files needed for the task.

## Preflight (must do before opening Docs\TODO.md)

1. Ensure your repo is clean (no uncommitted changes).
2. Sync with origin and fast-forward main.
   - git fetch origin
   - git switch main
   - git pull --ff-only origin main
   - If `git switch main` fails because `main` is checked out in another worktree, run the pull in that worktree instead, then return here and re-run `git fetch origin`.
3. Verify Docs\TODO.md is up to date locally.
   - git diff --name-only origin/main -- Docs\TODO.md
   - This must output nothing.

If you cannot complete preflight (for example, no network access), stop and ask the user before proceeding.

## Minimum reads before starting any task

After preflight, read:

1. Docs\TODO.md
2. Docs\TECHSPEC_SPLIT\TECHSPEC_INDEX.md
3. Docs\AGENTS\01_workflow.md

After the minimum reads, do not do any task-specific investigation (repo search, opening code/spec files, extra git commands) until you have claimed the task in Docs\TODO.md using the sign marker rules. Follow the sequence exactly; do not explore alternative steps.

## Pick task specific spec files

Use the spec map to pick the 1 or 2 spec files relevant to the task

- Docs\AGENTS\00_spec_read_map.md

## Build and tests

Build and testing rules live here

- Docs\AGENTS\02_build_and_tests.md

## Claim and close tasks

Every agent must claim a task in Docs\TODO.md and in the PR description using the sign marker rules

- Docs\AGENTS\03_task_execution_rules.md

## Agent instruction files

1. Docs\AGENTS\00_spec_read_map.md
2. Docs\AGENTS\01_workflow.md
3. Docs\AGENTS\02_build_and_tests.md
4. Docs\AGENTS\03_task_execution_rules.md
