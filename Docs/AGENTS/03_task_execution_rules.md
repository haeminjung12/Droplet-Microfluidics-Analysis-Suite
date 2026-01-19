# Task execution rules

Scope control

1. Read only the spec files needed for your task.
2. Do not read TECHSPEC.md unless a split file is missing content.
3. If you need new requirements, add them in Docs/TECHSPEC_SPLIT/13_open_questions.md.

Spec reference rule

When you implement a task, add a short comment near the implementation that points to the relevant split spec file.

Sign marker rule

Every agent must claim a task with a sign marker.

1. Claim
   1. SIGN <branch name> ACTIVE task NN

2. Complete
   1. SIGN <branch name> DONE task NN

3. Blocked
   1. SIGN <branch name> BLOCKED task NN reason short

Place the marker in Docs/TODO.md under the task entry.

Claim visibility rule

1. The claim is valid only after it is pushed to the shared local origin main.
2. No task work starts until the push succeeds.

One task per branch

1. One task per branch.
2. No unrelated refactors.
3. If you touch public headers, update or add unit tests.
