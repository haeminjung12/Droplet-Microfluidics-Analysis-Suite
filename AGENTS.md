# AGENTS

This repo supports many parallel agents.

## Keep context small

- Specs are split into Docs\TECHSPEC_SPLIT.
- Do not open TECHSPEC.md unless a split file is missing content.
- Read only the spec files needed for the task.

## Minimum reads before starting any task

1. Docs\TODO.md
2. Docs\TECHSPEC_SPLIT\TECHSPEC_INDEX.md
3. Docs\AGENTS\01_workflow.md

## Pick task specific spec files

Use the spec map to pick the 1 or 2 spec files relevant to your task

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
