# Task execution rules

## Scope control

1. Read only the spec files needed for your task.
2. Do not read the original TECHSPEC.md unless a split file is missing content.
3. If you need new requirements then add them as an open question in
   * Docs/TECHSPEC_SPLIT/13_open_questions.md

## Spec reference rule

When you implement a task, add a short comment near the implementation that points to the relevant split spec file.

Example

1. Hashing utilities
   * Reference Docs/TECHSPEC_SPLIT/06_data_formats.md
2. Fluorescence
   * Reference Docs/TECHSPEC_SPLIT/05_fluorescence.md
3. GUI
   * Reference Docs/TECHSPEC_SPLIT/07_gui_requirements.md

## Sign marker rule

Every agent must claim a task with a sign marker.

1. Claim
   * SIGN <branch-name> ACTIVE task NN
2. Complete
   * SIGN <branch-name> DONE task NN
3. Blocked
   * SIGN <branch-name> BLOCKED task NN reason short

Place the marker in

1. The PR description
2. The Docs/TODO.md task entry

Additional rules

1. You may only do the minimum reads before claiming.
   * After that, any task-specific repo investigation (searching, opening code/spec files, etc.) requires an ACTIVE marker in `Docs/TODO.md` first.
2. Keep `Docs/TODO.md` in sync and visible.
   * Before editing `Docs/TODO.md`, ensure it matches `origin/main` (per `Docs/AGENTS/01_workflow.md`).
   * After adding an ACTIVE marker, push/open a draft PR so other agents can see the claim.

