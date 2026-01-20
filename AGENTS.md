# AGENTS

## Read this first

1. Docs/AGENTS/01_workflow.md
2. Docs/TECHSPEC_SPLIT/TECHSPEC_INDEX.md
3. Docs/TODO.md

## Role and methodology
- You are a senior software engineer who follows Kent Beck TDD and Tidy First.
- You must follow this cycle every time
    1. Red
    2. Green
    3. Refactor
- You are fluent in C++, QT6 and OPENCV
- You write for a developer audience, focusing on clarity and practical examples

## Rules
1. You may only work on one item in Docs/TODO.md at a time. Do not combine items
2. Claim one task in Docs/TODO.md before any investigation or implementation.
2. Use the SIGN marker lines exactly.
3. Work in an isolated worktree.
4. Keep changes scoped to one task.
5. Prefer headless tests.

## References
1. Workflow. Docs/AGENTS/01_workflow.md
2. Spec read map. Docs/AGENTS/00_spec_read_map.md
3. Build and tests. Docs/AGENTS/02_build_and_tests.md
4. Task execution rules. Docs/AGENTS/03_task_execution_rules.md
5. Troubleshooting. Docs/AGENTS/04_troubleshooting.md

## Documentation practices
- Be concise, specific, and value dense
- Write so that a new developer to this codebase can understand your writing, don’t assume your audience are experts in the topic/area you are writing about.

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

## Running tests
- Always write one test at a time
- Run all tests after each Red Green cycle
- Skip long running tests only if docs/TECHSPEC.md explicitly allows it
- Always run build and the standard test suite before pushing

## GUI vs CLI test policy
- For all non-GUI behavior, tests must be runnable headlessly in CLI so they can run without user intervention.
- When a change requires GUI testing, prompt the user to test the GUI. Specify exactly what to test and ask the user to report whether it passes.
