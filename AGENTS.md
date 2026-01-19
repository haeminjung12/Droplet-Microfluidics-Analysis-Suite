# AGENTS

This repo supports many parallel agents.
This file is the minimum rules and an index to detailed instructions.

## Keep context small
- Specs are split in Docs\TECHSPEC_SPLIT.
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

## Minimum reads (after preflight)
1. Docs\TODO.md
2. Docs\TECHSPEC_SPLIT\TECHSPEC_INDEX.md
3. Docs\AGENTS\01_workflow.md

After the minimum reads, do not do any task-specific investigation (repo search, opening code/spec files, extra git commands) until you have claimed the task in Docs\TODO.md using the sign marker rules.

## Choose the next available task
Use this exact rule unless the user specifies otherwise:
1. Scan Docs\TODO.md from top to bottom.
2. The next available task is the first unchecked entry "[ ]" that does NOT have a SIGN ... ACTIVE marker directly under it.
3. If you find none, stop and ask the user.
4. Do not run searches (rg/grep) or additional commands to "find" a task. Only the top-to-bottom scan is allowed.

## Claim visibility is mandatory before work
The task claim must be visible in the main branch before any task work begins.

Required sequence:
1. Create a branch from main.
   - Pattern: vk/<id>-task-<NN>-<short-topic>
2. Add the SIGN ... ACTIVE marker under the chosen task in Docs\TODO.md.
   - SIGN <branch-name> ACTIVE task NN
3. Commit and push the claim branch.
4. Open a PR and merge it into main immediately (claim-only change).
5. Confirm the claim is visible on GitHub at:
   - https://github.com/haeminjung12/Droplet-Microfluidics-Analysis-Suite/blob/main/Docs/TODO.md
6. Only after the main branch shows the claim may you start any task work.

If you cannot merge or cannot verify the main branch shows the claim, stop and ask the user. Do not proceed.

## During work
- Keep changes scoped to the chosen task.
- One task per PR. No unrelated refactors.
- If you touched public headers, update or add unit tests.
- Do not push code changes until the task is complete (the only exception is the initial claim push above).
- Add a short comment near implementations that points to the relevant split spec file.

## End of a task
1. Run the required tests for your task.
2. Push final changes.
3. Mark the draft PR ready and update the description.
4. In the PR description include: what changed, how to test, and evidence (logs/timings).
5. Update Docs\TODO.md for completion:
   - Change the task checkbox from [ ] to [x].
   - Replace the ACTIVE marker with this line directly under the task entry (before the verification bullet):
     - SIGN <branch-name> DONE task NN
6. Merge the PR and confirm the DONE marker and [x] are visible on main at:
   - https://github.com/haeminjung12/Droplet-Microfluidics-Analysis-Suite/blob/main/Docs/TODO.md

If you cannot merge or cannot verify the main branch shows the DONE marker, stop and ask the user. Do not proceed.

## Build and tests
- Build: Visual Studio MSBuild only.
  - Use x64 (some projects are not configured for Win32).
  - If you hit `MSB8020` about toolset `v180`, retarget the project(s) to the installed toolset (currently `v145`).
  - & "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" "build\DropletAnalyzer.sln" /m /p:Configuration=Release /p:Platform=x64
- Tests: prefer headless tests (unit tests, CLI E2E).
- GUI testing: only ask when GUI-facing; give exact steps and expected results; ask for pass/fail and screenshot on failure.
- Evidence for PR: unit test command + pass result; performance claims include dataset, timings, and machine notes.

## Spec read map
Use only the files needed for your task. See:
- Docs\AGENTS\00_spec_read_map.md

When requirements are missing:
- Add or refine requirements in Docs\TECHSPEC_SPLIT\13_open_questions.md
- Link the new note from Docs\TODO.md

## Troubleshooting
If you are confused, blocked, or see conflicting instructions, stop and follow:
- 04_troubleshooting.md

## Detailed instructions
- Docs\AGENTS\00_spec_read_map.md
- Docs\AGENTS\01_workflow.md
- Docs\AGENTS\02_build_and_tests.md
- Docs\AGENTS\03_task_execution_rules.md
- Docs\AGENTS\04_troubleshooting.md
