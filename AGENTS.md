````markdown
# AGENTS.md

## Goal
Enable multiple agents to work in parallel on the same repo with low merge risk and predictable review.

## Canonical spec note
Technical_Specification_Sheet_V1.0.md is the same document as `Docs/TECHSPEC.md`. Treat `Docs/TECHSPEC.md` as the canonical path.

## Source of truth for process
1. Always follow `Docs/TECHSPEC.md`
2. Reference `Docs/TODO.md` for workflow overview

If any guidance conflicts, follow this priority order
1. `Docs/TECHSPEC.md`
2. `Docs/TODO.md`

## Required lines
READ `Docs/TECHSPEC.md` before conducting your work.
Each work should reference `Docs/TODO.md` for overview of the workflow.
Please ALWAYS consult with `Docs/TECHSPEC.md`

## Role and methodology
You are a senior software engineer who follows Kent Beck TDD and Tidy First.

You must follow this cycle every time
1. Red
2. Green
3. Refactor

## Core development principles
- Always follow the TDD cycle: Red -> Green -> Refactor
- Write the simplest failing test first
- Implement the minimum code needed to make tests pass
- Refactor only after tests are passing
- Follow Tidy First by separating structural changes from behavioral changes
- Maintain high code quality throughout development

## TDD methodology guidance
- Start by writing a failing test that defines a small increment of functionality
- Use meaningful test names that describe behavior, for example `shouldSumTwoPositiveNumbers`
- Make test failures clear and informative
- Write just enough code to make the test pass
- Once tests pass, consider if refactoring is needed
- Repeat the cycle for new functionality
- When fixing a defect, first write an API level failing test then write the smallest possible reproducer test then get both tests to pass

## Tidy First approach
Separate all changes into two distinct types
1. Structural changes: renaming, extracting methods, moving code, rearranging without changing behavior
2. Behavioral changes: adding or modifying actual functionality

Rules
- Never mix structural and behavioral changes in the same commit
- Always make structural changes first when both are needed
- Validate structural changes do not alter behavior by running tests before and after

## Commit discipline
Only commit when all are true
1. All tests are passing
2. All compiler and linter warnings have been resolved
3. The change represents a single logical unit of work
4. Commit messages clearly state whether the commit contains structural or behavioral changes

Use small, frequent commits.

Commit message format
- `structural: <short description>`
- `behavioral: <short description>`

## Task selection and user trigger
You may only work on one item at a time.

When the user says `go`
1. Find the next unchecked item in `Docs/TODO.md` on `origin/main` that is not claimed by another agent (do not rely on a potentially stale working tree copy)
2. Claim it (and merge that claim to `main` so other agents can see it)
3. Implement the test for that item
4. Implement only enough code to make that test pass

Rules
- Do not combine items
- If blocked, leave it unchecked and add a short blocker note under it, then move to the next unchecked item

## Task claiming so agents can work in parallel
Before starting work, an agent must claim exactly one TODO item so other agents can move on.

Claim format in `Docs/TODO.md`
- Append a claim tag to the end of the TODO line:
  - `[IN-PROGRESS: agent_id]`

Example
- `- [ ] Add droplet ROI validator [IN-PROGRESS: agent02]`

Claim rules
1. Claim only one item at a time
2. Do not claim an item already claimed
3. If you stop working, remove your claim tag
4. When the work is complete and the user confirms, replace the checkbox with checked and remove the claim tag

Claim visibility rule
- A claim is not considered active until it has been merged into `main` (so it is visible to everyone on GitHub from the `main` branch view of `Docs/TODO.md`).
- Do not start implementation work until the claim-only PR has been merged into `main`.

Completion example
- `- [x] Add droplet ROI validator`
  - `Done by agent02, commit abc1234, PR 57`

## Parallel agent policy
1. Each agent must use a unique agent id
   Examples: agent01, agent02, agent03
2. Each agent must work on only one selected item
3. Each agent must create its own PR from main before doing work
4. Do not touch another agent's branch

Branch naming
- `agent_id/task_slug`
Example
- `agent02/todo_droplet_tracker_autotune`

## Start of work procedure
1. Sync your local view of `origin/main` and the TODO list (worktree-safe)
   - This repo may have `main` checked out in another worktree. Prefer syncing via fetch and branching from `origin/main` (not local `main`).
   - `git fetch origin --prune`
   - Review the up-to-date TODO directly from `origin/main`:
     - `git show origin/main:Docs/TODO.md | Out-Host -Paging`
   - PowerShell footgun fixes: backtick is an escape character, so prefer single-quoted patterns (or omit Markdown backticks) and use `Get-Content <path> | ForEach-Object -Begin { $i=1 } -Process { "{0,5}: {1}" -f $i++, $_ }` instead of `nl`.
   - Optional: update local `main` only if it is available in your worktree:
     - `git switch main`
     - `git pull --ff-only origin main`

2. Select the next unclaimed unchecked item from `origin/main:Docs/TODO.md`
   - If your working tree `Docs/TODO.md` differs from `origin/main:Docs/TODO.md`, treat the `origin/main` version as canonical for claiming.

3. Create a short-lived claim branch from `origin/main`
   - `git checkout -b agent_id/claim_task_slug origin/main`

4. Claim the item in `Docs/TODO.md` and commit immediately (structural)
   - Append `[IN-PROGRESS: agent_id]` to the TODO line
   - `git add Docs/TODO.md`
   - `git commit -m "structural: claim TODO item <task_slug>"`

5. Merge the claim into `main` so other agents can see it immediately
   - `git push -u origin HEAD`
   - `gh pr create --title "structural: claim TODO <id> <task_slug>" --body "Agent: agent_id"`
   - `gh pr merge <pr_number> --merge`
   - **Worktree note:** Avoid `gh pr merge --delete-branch` here; in multi-worktree setups it can fail with `fatal: 'main' is already used by worktree ...` because it tries to manipulate local branches during deletion.
   - If you want to delete the claim branch after merge, do it manually:
     - `git push origin --delete agent_id/claim_task_slug`
     - `git checkout --detach origin/main` then `git branch -D agent_id/claim_task_slug`

6. Create the work branch from updated `origin/main` and open a draft PR
   - `git fetch origin`
   - `git checkout -b agent_id/task_slug origin/main`
   - `git push -u origin HEAD`
   - `gh pr create --draft --title "TODO <id>: <short title>" --body "Agent: agent_id"`

7. Read `Docs/TECHSPEC.md` again
   Confirm the selected item matches the intended architecture

## Local workflow during work
- Make changes locally
- Commit locally as needed to save progress
- Avoid frequent pushes; push only when you need to update the PR (or when the user says `push`)

## Running tests
- Always write one test at a time
- Run all tests after each Red Green cycle
- Skip long running tests only if `Docs/TECHSPEC.md` explicitly allows it
- Always run build and the standard test suite before pushing

## GUI vs CLI test policy
- For all non-GUI behavior, tests must be runnable headlessly in CLI so they can run without user intervention.
- When a change requires GUI testing, prompt the user to test the GUI. Specify exactly what to test and ask the user to report whether it passes.

## Troubleshooting Claim Visibility
If you have followed the "Start of work procedure" to claim a task, but your claim is not visible on github.com, follow these steps to troubleshoot the issue:

1. **Verify Local Claim:**
   - Run `git log --oneline --graph --decorate --all` to check if your local branch contains the claim commit. The commit message should be `structural: claim TODO item <task_slug>`.

2. **Check for Unpushed Commits:**
   - Run `git status` to see if your local branch is ahead of the remote branch. If it is, you may have unpushed commits.
   - Run `git push` to push your changes to the remote branch.

3. **Verify Pull Request Status:**
   - Go to the GitHub repository and check if a pull request for your claim branch exists.
   - If the pull request exists, check if it has been merged.
   - If the pull request has not been created, you can create it manually using the `gh pr create` command as described in the "Start of work procedure".

4. **Check for Merge Conflicts:**
   - If the pull request has merge conflicts, you will need to resolve them before it can be merged.
   - Follow the instructions on GitHub to resolve the conflicts.

5. **Verify `main` Branch:**
   - Once the pull request is merged, confirm `origin/main` contains the claim.
   - **Worktree note:** Do not `git checkout main` in this worktree; `main` may be checked out in another worktree and Git will block it.
   - Use:
     - `git fetch origin`
     - `git show origin/main:Docs/TODO.md` (verify your `[IN-PROGRESS: agent_id]` tag is visible)

If you have followed all these steps and the claim is still not visible, ask for help.

# Build

Use **Visual Studio MSBuild only**. Do **not** search for other compilers or use non-VS toolchains.

## Toolchain (no searching)
Some environments do not have `cmake` on `PATH`. Use these fixed Visual Studio tool paths (do not recursively search for compilers/tools):
- MSBuild: `C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe`
- CMake: `C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`
- CTest: `C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe`

## Make the compiler visible (recommended)
Run in a Visual Studio Developer PowerShell session, or run this once per shell to put the MSVC compiler on `PATH` (no filesystem searching required):
- `Import-Module "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"`
- `Enter-VsDevShell -VsInstallPath "C:\Program Files\Microsoft Visual Studio\18\Community" -SkipAutomaticLocation -DevCmdArguments "-arch=x64 -host_arch=x64"`

## Build commands
Run from the repo root:

1. Configure (generates `build/DropletAnalyzer.sln`):
   - `& "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -S . -B build -G "Visual Studio 17 2022" -A x64 -DVCPKG_ROOT="C:/vcpkg"`
2. Build:
   - `& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" "build\\DropletAnalyzer.sln" /m /p:Configuration=Release`
3. Tests (if enabled/configured):
   - `& "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe" -C Release --output-on-failure`


# Dependencies (verify these directories exist)

* OpenCV (via vcpkg):

  * `C:\vcpkg\installed\x64-windows\include\opencv4`
  * `C:\vcpkg\installed\x64-windows\bin`

* Hamamatsu DCAM SDK:

  * `C:\Users\goals\Codex\CNN for Droplet Sorting\python_pipeline\Hamamatsu_DCAMSDK4_v25056964\dcamsdk4\inc`
  * `C:\Users\goals\Codex\CNN for Droplet Sorting\python_pipeline\Hamamatsu_DCAMSDK4_v25056964\dcamsdk4\lib\win64`

If any of these differ on your machine, update the CMake cache (e.g. `VCPKG_ROOT`) and reconfigure, but still build with MSBuild.

## Finish procedure before pushing

1. Make sure your branch is up to date with main
   `git fetch origin`
   `git merge origin/main`
2. Resolve conflicts if needed
3. Run build and tests locally

## Push procedure

- First push (publish branch): `git push -u origin HEAD`
- Subsequent pushes (update PR): `git push` (only when needed / user requested)

## Branch Management

### Tracking Remote Branches

It is crucial that your local branches track their remote counterparts on `origin`. This ensures that `git status` provides accurate information about whether your branch is ahead of, behind, or has diverged from the remote branch. Without a tracking relationship, your local branch is "invisible" to `git` in terms of its relationship with the remote repository.

### Setting an Upstream Branch

When you create a new local branch, it does not automatically track a remote branch. You must explicitly set the upstream branch when you first push it. The `Start of work procedure` already includes this step.

The command to push a new branch and set its upstream is:

`git push --set-upstream origin <branch-name>`

You can also use the shorthand `-u` for `--set-upstream`:

`git push -u origin <branch-name>`

### Verifying Branch Tracking

To check if your local branches are tracking remote branches, use the `-vv` flag with `git branch`:

`git branch -vv`

The output will show the tracking information in square brackets `[]` for each branch.

**Example of a tracking branch:**
`* my-feature-branch  a1b2c3d [origin/my-feature-branch] Add new feature`

**Example of a non-tracking branch:**
`* my-other-branch    e4f5g6h Another commit message`

### Fixing a Non-Tracking Branch

If you find a branch that is not tracking a remote counterpart (and it should be), you can fix it by pushing it and setting the upstream:

`git push --set-upstream origin <branch-name>`

This will create the branch on the remote `origin` if it doesn't exist and establish the tracking relationship.

## Merge procedure with user gate

1. Ask the user
2. Mark PR ready (if draft): `gh pr ready <pr_number>`
3. Merge: `gh pr merge <pr_number> --merge`
   - If branch deletion fails due to a local worktree constraint, delete manually:
     - `git push origin --delete <branch>`
     - `git checkout --detach origin/main` then `git branch -D <branch>`

## Cleanup after merge

* Delete the branch after the PR is merged

## Updating TODO.md after completion

Only after the user confirms the work is complete

This is a required workflow step so `Docs/TODO.md` on `main` remains the single source of truth for what is claimed vs done.

1. Create a short-lived completion branch from `origin/main`
   - `git fetch origin`
   - `git checkout -b agent_id/complete_<task_slug> origin/main`
2. Update `Docs/TODO.md` for the exact item you worked on
   - Check the box (`[x]`)
   - Remove the `[IN-PROGRESS: agent_id]` claim tag
   - Log completion under that line with:
     - agent id
     - commit hash
     - PR number
3. Commit the completion update (structural) and merge it into `main`
   - `git add Docs/TODO.md`
   - `git commit -m "structural: complete TODO item <task_slug>"`
   - `git push -u origin HEAD`
   - `gh pr create --title "structural: complete TODO <id> <task_slug>" --body "Agent: agent_id"`
   - `gh pr merge <pr_number> --merge`
   - (Optional cleanup) If branch deletion fails due to a worktree constraint, delete manually:
     - `git push origin --delete agent_id/complete_<task_slug>`
     - `git checkout --detach origin/main` then `git branch -D agent_id/complete_<task_slug>`

Example log line

* Done by agent01, commit abc1234, PR 57

```
```
