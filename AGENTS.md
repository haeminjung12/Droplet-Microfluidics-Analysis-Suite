```markdown
# AGENTS.md

## Purpose
Enable multiple agents to work in parallel on the same repo with low merge risk and predictable integration into main.

## Canonical documentation
- `Docs/TECHSPEC.md` is the canonical technical spec
- `Docs/TODO.md` is the canonical task list

If guidance conflicts follow this priority order
1. `Docs/TECHSPEC.md`
2. `Docs/TODO.md`
3. `AGENTS.md`

Required
Read `Docs/TECHSPEC.md` before doing any work.

## Canonical spec note
`Technical_Specification_Sheet_V1.0.md` is the same document as `Docs/TECHSPEC.md`.
Treat `Docs/TECHSPEC.md` as the canonical path.

## Engineering approach
You are a senior engineer who follows TDD and Tidy First.

TDD cycle
1. Red
2. Green
3. Refactor

Tidy First rule
Keep structural changes separate from behavioral changes.

Definitions
1. Structural changes
   Renames, moves, extraction, reformatting, no behavior change
2. Behavioral changes
   New logic, changed behavior, bug fixes

## Commit discipline
Only commit when all are true
1. Tests pass
2. Build passes
3. No compiler warnings and no linter warnings
4. One logical unit of work per commit

Use small frequent local commits.

Commit message format
- `structural: <short description>`
- `behavioral: <short description>`

Do not mix structural and behavioral changes in the same commit.

## Parallel agent policy
1. Each agent uses a unique agent id
   Examples `agent01` `agent02` `agent03`
2. Each agent works on exactly one TODO item at a time
3. Each agent uses its own branch and PR
4. Do not touch another agent branch

Branch naming
- Claim branch `agent_id/claim_task_slug`
- Work branch `agent_id/task_slug`

## Task ownership and claiming
Goal
Prevent two agents from starting the same task by using `Docs/TODO.md` on `main` as the lock.

Claim format in `Docs/TODO.md`
Append a claim tag at the end of the TODO line
`[IN-PROGRESS: agent_id]`

Claim rules
1. Claim only one item at a time
2. Do not claim an item already claimed
3. If you stop working remove your claim tag
4. A claim is active only after the claim PR is merged into `main`
5. Do not start implementation until the claim PR is merged

Completion logging format in `Docs/TODO.md`
- Check the box and remove the claim tag
- Add a log line under the item
  - `Done by agent_id, PR pr_number`

Example
- [x] Add droplet ROI validator
  - Done by agent02, PR 57

## User gating
You may only work on one item at a time.

When the user says `go`
1. Confirm everything is merged correctly
2. End the session

Never move to the next task without explicit user instruction.

If blocked by conflicts, task ownership issues, or process ambiguity
Stop and ask the user.

## Start of work procedure
Worktree safe sync
1. Sync origin
   - `git fetch origin --prune`
2. Review the canonical TODO from `origin/main`
   - `git show origin/main:Docs/TODO.md | Out-Host -Paging`

Select task
- Pick the next unchecked unclaimed item from `origin/main:Docs/TODO.md`

Create claim branch
- `git checkout -b agent_id/claim_task_slug origin/main`

Apply claim
1. Update `Docs/TODO.md` by appending the claim tag
2. Commit immediately structural
   - `git add Docs/TODO.md`
   - `git commit -m "structural: claim TODO item task_slug"`

Publish claim and merge
1. Push claim branch
   - `git push -u origin HEAD`
2. Create PR
   - `gh pr create --title "structural: claim TODO id task_slug" --body "Agent: agent_id"`
3. Merge claim PR into `main`
   - `gh pr merge pr_number --merge`
4. Optional branch cleanup if needed and if safe in your worktree
   - `git push origin --delete agent_id/claim_task_slug`
   - `git checkout --detach origin/main`
   - `git branch -D agent_id/claim_task_slug`

Create work branch
1. Refresh `origin/main` after claim merge
   - `git fetch origin`
2. Create work branch from `origin/main`
   - `git checkout -b agent_id/task_slug origin/main`
3. Publish branch and open a draft PR
   - `git push -u origin HEAD`
   - `gh pr create --draft --title "TODO id short title" --body "Agent: agent_id"`

Re read the spec
Read `Docs/TECHSPEC.md` again and confirm the task matches the intended architecture.

## Local workflow during work
1. Make changes locally
2. Commit locally as needed to save progress
3. Avoid frequent pushes
4. Push only when you need to update the PR or when the user says `push`

## Tests and validation
TDD rules
1. Write one test at a time
2. Run tests after each Red Green cycle
3. Run the standard test suite before any push that updates the PR

GUI vs CLI test policy
- For non GUI behavior, tests must run headlessly from CLI
- If GUI testing is required, prompt the user
  - Specify exactly what to test
  - Ask the user to report pass or fail

## Finish procedure before requesting merge
1. Sync with `origin/main`
   - `git fetch origin`
   - `git merge origin/main`
2. Resolve conflicts if needed
3. Run build and tests locally

## Merge procedure with user gate
1. Ask the user for approval to merge
2. If the user confirms completion, update `Docs/TODO.md` inside the same work PR
   1. Check the box for the item
   2. Remove the claim tag
   3. Add the completion log line
   4. Commit structural
      - `git add Docs/TODO.md`
      - `git commit -m "structural: complete TODO item task_slug"`
   5. Push the update
      - `git push`
3. Mark PR ready if it is draft
   - `gh pr ready pr_number`
4. Merge the work PR using squash
   - `gh pr merge pr_number --squash`

## Cleanup after merge
Branch cleanup is recommended but not blocking.
If deletion fails due to worktree constraints, delete manually.
1. Delete remote branch
   - `git push origin --delete branch`
2. Detach and delete local branch
   - `git checkout --detach origin/main`
   - `git branch -D branch`

## Troubleshooting claim visibility
If your claim is not visible on GitHub
1. Confirm the claim commit exists locally
   - `git log --oneline --decorate --graph --all`
2. Confirm the claim branch was pushed
   - `git status`
3. Confirm the claim PR exists and is merged
4. Confirm `origin/main` contains the claim
   - `git fetch origin`
   - `git show origin/main:Docs/TODO.md`

If still not visible, ask the user for help.

## Environment and PowerShell gotchas log
Goal
Make builds reproducible on Windows using Visual Studio tooling only.

Always use a Developer PowerShell
Run this once per new shell
- `Import-Module "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"`
- `Enter-VsDevShell -VsInstallPath "C:\Program Files\Microsoft Visual Studio\18\Community" -SkipAutomaticLocation -DevCmdArguments "-arch=x64 -host_arch=x64"`

Quick verification
- `where cl`
- `where msbuild`
- `where cmake`

If `cl` is not found
You are not in a VS Developer shell. Run Enter-VsDevShell again.

Do not rely on PATH for tool discovery
Use fixed paths only, as specified in Build. If a command fails, call it by full path.

Common configure failures and fixes

CMake cannot find a compiler
Cause
Not in Developer PowerShell or generator mismatch
Fix
1. Enter the Developer shell
2. Re run configure with the Visual Studio generator
   - Use `-G "Visual Studio 17 2022" -A x64`

vcpkg not found or packages not found
Cause
VCPKG_ROOT not set correctly
Fix
1. Confirm VCPKG_ROOT points to the vcpkg folder
2. Re run configure with `-DVCPKG_ROOT="C:/vcpkg"`

CMake cache stuck or wrong toolchain
Fix
Delete the build folder and reconfigure
- `Remove-Item -Recurse -Force build`

Common build failures and fixes

MSBuild fails due to locked files
Cause
A running process is holding an exe or dll open
Fix
1. Close the app using the build output
2. Close Visual Studio instances using the build folder
3. If still locked, reboot or kill the locking process

Linker cannot open output file
Same root cause as above. A process is using the binary.

LNK2038 mismatch or runtime mismatch
Cause
Mixing Debug and Release or different runtime settings
Fix
1. Build Release consistently
2. Delete build folder if needed and reconfigure

PowerShell command footguns

Backtick escapes characters in PowerShell
Fix
Prefer single quotes in patterns and avoid backticks in command strings.

Viewing a file with line numbers
Use this instead of nl
- `Get-Content <path> | ForEach-Object -Begin { $i=1 } -Process { "{0,5}  {1}" -f $i++, $_ }`

Piping large output
Use Out-Host -Paging for long files
- `git show origin/main:Docs/TODO.md | Out-Host -Paging`

Worktree constraints

If main is checked out in another worktree
Do not `git switch main` in this worktree.

Use origin main as the reference
- `git fetch origin --prune`
- `git show origin/main:Docs/TODO.md`

Branch deletion may fail locally due to worktree constraints
If deletion fails, delete remote branch only and skip local deletion
- `git push origin --delete <branch>`

GitHub CLI issues

gh not authenticated
Fix
- `gh auth login`

gh pr merge branch deletion triggers local worktree errors
Fix
Do not use auto delete on merge in multi worktree setups.
Delete remote branch manually after merge if desired.

vcpkg binary path at runtime

If the app runs but fails to find OpenCV dlls
Cause
OpenCV bin folder not on PATH for runtime
Fix options
1. Add `C:\vcpkg\installed\x64-windows\bin` to PATH for the current shell before running
2. Or copy required dlls next to the built exe as a one time workaround

## Build
Use Visual Studio MSBuild only. Do not use non VS toolchains.

Toolchain fixed paths
- MSBuild `C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe`
- CMake `C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`
- CTest `C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe`

Developer PowerShell recommended
- `Import-Module "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"`
- `Enter-VsDevShell -VsInstallPath "C:\Program Files\Microsoft Visual Studio\18\Community" -SkipAutomaticLocation -DevCmdArguments "-arch=x64 -host_arch=x64"`

Build commands from repo root
1. Configure
   - `& "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -S . -B build -G "Visual Studio 17 2022" -A x64 -DVCPKG_ROOT="C:/vcpkg"`
2. Build
   - `& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" "build\DropletAnalyzer.sln" /m /p:Configuration=Release`
3. Tests
   - `& "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe" -C Release --output-on-failure`

## Dependencies
Verify these directories exist.

OpenCV via vcpkg
- `C:\vcpkg\installed\x64-windows\include\opencv4`
- `C:\vcpkg\installed\x64-windows\bin`

Hamamatsu DCAM SDK
- `C:\Users\goals\Codex\CNN for Droplet Sorting\python_pipeline\Hamamatsu_DCAMSDK4_v25056964\dcamsdk4\inc`
- `C:\Users\goals\Codex\CNN for Droplet Sorting\python_pipeline\Hamamatsu_DCAMSDK4_v25056964\dcamsdk4\lib\win64`

If paths differ, update the CMake cache and reconfigure, but still build with MSBuild.

## If you hit a new environment issue
Add it to the gotchas log with
1. Symptom
2. Root cause
3. Fix steps
So other agents do not repeat it.
```
