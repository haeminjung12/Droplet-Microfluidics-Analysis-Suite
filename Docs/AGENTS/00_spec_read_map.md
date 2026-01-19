# Spec read map

Purpose

- Keep context small for each agent.
- Avoid loading the original TECHSPEC.md.

## Default reads for any task

Read these first

1. TODO.md
2. Docs/TECHSPEC_SPLIT/TECHSPEC_INDEX.md

Read these only if your task needs them

- Docs/TECHSPEC_SPLIT/00_overview.md
- Docs/TECHSPEC_SPLIT/02_system_architecture.md
- Docs/TECHSPEC_SPLIT/10_testing_validation.md

## Task to spec file map

Open only the files that match your task

- Data formats, sessions, hashing, export
  - Docs/TECHSPEC_SPLIT/06_data_formats.md

- Core algorithms and offline modules
  - Docs/TECHSPEC_SPLIT/03_functional_requirements.md

- Fluorescence quantification
  - Docs/TECHSPEC_SPLIT/05_fluorescence.md

- GUI, plotting, UX
  - Docs/TECHSPEC_SPLIT/07_gui_requirements.md

- Performance targets and bandwidth
  - Docs/TECHSPEC_SPLIT/04_performance_benchmarks.md
  - Docs/TECHSPEC_SPLIT/09_performance_usb_bandwidth.md

- Dependencies, licensing, packaging
  - Docs/TECHSPEC_SPLIT/08_dependencies_licensing.md

- Roadmap, deliverables, success criteria
  - Docs/TECHSPEC_SPLIT/11_development_roadmap.md
  - Docs/TECHSPEC_SPLIT/12_deliverables.md
  - Docs/TECHSPEC_SPLIT/14_success_criteria.md

- Open questions and missing requirements
  - Docs/TECHSPEC_SPLIT/13_open_questions.md

## When requirements are missing

- Add or refine requirements in Docs/TECHSPEC_SPLIT/13_open_questions.md
- Then link the new note from TODO.md
