# Troubleshooting

If you are confused or blocked, stop and follow this order.

1. Re read AGENTS.md and Docs/AGENTS/01_workflow.md.
2. Run git fetch and check if origin main moved.
3. Identify the failure class. Git conflict, build error, test failure.

Local Git workflow issues

1. git push origin HEAD:main fails during a claim
   1. Another agent pushed first.
   2. Run git fetch origin and git rebase origin/main.
   3. If rebase conflicts in Docs/TODO.md, run git rebase abort.
   4. Re read Docs/TODO.md and choose a different available task.

2. git worktree add fails
   1. Target directory exists and is not empty.
   2. Branch name already exists.

3. Main does not look up to date in an agent worktree
   1. Do not git switch main in a worktree.
   2. Always sync with git fetch origin and git rebase origin/main.

Build and test issues

1. MSB8020 toolset mismatch
   1. Retarget to the installed toolset.

2. Unit test failure
   1. Treat as regression.
   2. Fix before merging.

If still blocked

Stop. Ask the user with

1. The task you were doing.
2. The exact command.
3. The full output.
4. What you already tried.
