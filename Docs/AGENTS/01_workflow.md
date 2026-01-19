# Agent workflow

## Preflight (must do before opening Docs\TODO.md)

1. Ensure your repo is clean.
   - No uncommitted changes.
2. Sync with origin and fast forward main.
   - git fetch origin
   - git switch main
   - git pull --ff-only origin main
3. Verify Docs\TODO.md is up to date locally.
   - git diff --name-only origin/main -- Docs\TODO.md
   - This must output nothing.
   - If it outputs Docs\TODO.md, repeat the pull step and recheck.

## Start of a task

1. Pick one task from Docs\TODO.md.
2. Create a branch from main.
   - Branch name pattern
     - agentXX_taskNN_short_topic
3. In your first message in the PR description add a task marker.
   - Use this exact marker
     - SIGN agentXX ACTIVE task NN
4. Update Docs\TODO.md under that task with the same marker line so other agents do not collide.

## During work

1. Keep changes scoped to the chosen task.
2. Commit locally as needed.
3. Do not push until the task is complete.

## End of a task

1. Run the required tests for your task.
2. Push once.
3. Open a PR.
4. In the PR description include
   - What you changed
   - How to test it
   - Evidence like log snippets and timings when relevant
5. Remove the ACTIVE marker in Docs\TODO.md and replace it with
   - SIGN agentXX DONE task NN

## Merge rules

1. One task per PR.
2. No unrelated refactors.
3. If you touched public headers then update or add unit tests.
