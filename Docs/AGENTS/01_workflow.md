# Agent workflow

## Preflight (must do before opening Docs\TODO.md)

1. Ensure your repo is clean.
   - No uncommitted changes.
2. Sync with origin and fast forward main.
   - git fetch origin
   - git switch main
   - git pull --ff-only origin main
   - If `git switch main` fails because `main` is checked out in another worktree, run the pull in that worktree instead, then return here and re-run `git fetch origin`.
3. Verify Docs\TODO.md is up to date locally.
   - git diff --name-only origin/main -- Docs\TODO.md
   - This must output nothing.
   - If it outputs Docs\TODO.md, repeat the pull step and recheck.

## Start of a task

1. Pick one task from Docs\TODO.md.
2. Create a branch from main.
   - Branch name pattern
     - agentXX_taskNN_short_topic
3. Claim the task in Docs\TODO.md so other agents do not collide.
   - Add this exact marker under the task entry
     - SIGN agentXX ACTIVE task NN
4. Make the claim visible to other agents.
   - Push the branch and open a draft PR immediately after adding the ACTIVE marker.
   - Put the same marker line at the top of the PR description.
   - After this point, keep pushing restricted to completion (avoid frequent intermediate pushes).

## During work

1. Keep changes scoped to the chosen task.
2. Commit locally as needed.
3. Do not push code changes until the task is complete (the only exception is the initial claim push above).

## End of a task

1. Run the required tests for your task.
2. Push final changes.
3. Mark the draft PR ready and update the description.
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
