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
- Always follow the TDD cycle: Red → Green → Refactor
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
1. Find the next unchecked item in `Docs/TODO.md` that is not claimed by another agent
2. Claim it
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

Completion example
- `- [x] Add droplet ROI validator`
  - `Done by agent02, commit abc1234, PR 57`

## Parallel agent policy
1. Each agent must use a unique agent id
   Examples: agent01, agent02, agent03
2. Each agent must work on only one selected item
3. Each agent must create its own PR from main before doing work
4. Do not touch another agent’s branch

Branch naming
- `agent_id/task_slug`
Example
- `agent02/todo_droplet_tracker_autotune`

## Start of work procedure
1. Sync with `origin/main` (worktree-safe)
   - This repo may have `main` checked out in another worktree. Prefer syncing via fetch and branching from `origin/main`.
   - `git fetch origin`

2. Create a new branch from `origin/main`
   - `git checkout -b agent_id/task_slug origin/main`

3. Select the next unclaimed unchecked item in `Docs/TODO.md`

4. Claim it in `Docs/TODO.md` using the claim format above and commit that claim immediately
   Commit message must be structural
   - `structural: claim TODO item <task_slug>`

5. Push your branch and open a draft PR immediately (so others can see your claim)
   - `git push -u origin HEAD`
   - `gh pr create --draft --title "TODO <id>: <short title>" --body "Agent: agent_id"`

6. Read `Docs/TECHSPEC.md` again
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

# Build

Use **Visual Studio MSBuild only**. Do **not** search for other compilers or use non-VS toolchains.

Run from the repo root:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" "build\droplet_pipeline.sln" /m /p:Configuration=Release
````

# Dependencies (verify these directories exist)

* OpenCV (via vcpkg):

  * `C:\vcpkg\installed\x64-windows\include\opencv4`
  * `C:\vcpkg\installed\x64-windows\bin`
* ONNX Runtime (GPU build):

  * `C:\onnxruntime-gpu\include`
  * `C:\onnxruntime-gpu\lib`
* Hamamatsu DCAM SDK:

  * `C:\Users\goals\Codex\CNN for Droplet Sorting\python_pipeline\Hamamatsu_DCAMSDK4_v25056964\dcamsdk4\inc`
  * `C:\Users\goals\Codex\CNN for Droplet Sorting\python_pipeline\Hamamatsu_DCAMSDK4_v25056964\dcamsdk4\lib\win64`

If any of these differ on your machine, update the CMake cache (e.g. `ONNXRUNTIME_DIR` or `VCPKG_ROOT`) and reconfigure, but still build with MSBuild.

## Finish procedure before pushing

1. Make sure your branch is up to date with main
   `git fetch origin`
   `git merge origin/main`
2. Resolve conflicts if needed
3. Run build and tests locally

## Push procedure

- First push (publish branch): `git push -u origin HEAD`
- Subsequent pushes (update PR): `git push` (only when needed / user requested)

## Merge procedure with user gate

1. Ask the user
2. Only merge after the user types `go`
3. Mark PR ready (if draft): `gh pr ready <pr_number>`
4. Merge: `gh pr merge <pr_number> --merge`
   - If branch deletion fails due to a local worktree constraint, delete manually:
     - `git push origin --delete <branch>`
     - `git checkout --detach origin/main` then `git branch -D <branch>`

## Cleanup after merge

* Delete the branch after the PR is merged

## Updating TODO.md after completion

Only after the user confirms the work is complete

1. Check the box for the exact `Docs/TODO.md` line that was worked on and remove the claim tag
2. Log completion under that line with

   * agent id
   * commit hash
   * PR number

Example log line

* Done by agent01, commit abc1234, PR 57

```
```
