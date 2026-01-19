# TODO - DropletAnalyzer v1.0.0 - Integrated Implementation & Test Plan

**Project:** DropletAnalyzer: Microfluidics Image Processing & Analysis Suite
**Version:** 1.0.0
**Source Spec:** `Docs/TECHSPEC_V2.md` (diffed against `Docs/TECHSPEC_V1.md`)

---

**Camera scope note:** Camera control is already implemented in `C:\Users\goals\Codex\CNN for Droplet Sorting\cpp_pipeline_github\qt_hama_gui`. All camera work in this plan is integration and verification using those existing assets (e.g., `dcam_controller.*`, `frame_grabber.*`, `frame_types.h`, and UI patterns in `main.cpp`), not reimplementation.

---

## Phase 0: Camera Asset Validation & Early Verification (Weeks 0-1)

**Goal:** Proactively investigate and de-risk critical technical challenges before committing to the full development cycle. These are short, focused prototypes to confirm feasibility and performance.

1. [x] **Camera Asset Inventory (qt_hama_gui):** Identify and list reusable camera assets from `qt_hama_gui` (controller, grabber, frame types, UI patterns) and their dependencies.
2. [x] **DCAM Connectivity & Streaming Test (Using Existing Assets):** Build/run the existing `qt_hama_gui` camera pipeline to connect to ORCA-Fusion and stream raw frames.
3. [x] **DCAM Settings Round-Trip Verification (Using Existing Assets):** Use the existing `qt_hama_gui` camera controls to set and read back parameters (resolution, binning, ROI, FPS, exposure, gain, bit depth, trigger mode).
4. [ ] **Image Sequence Loading & Validation Performance Test:** Implement a prototype for `ImageSequenceSource` that scans a directory, sorts filenames alphabetically (no frame index parsing), and performs format consistency checks (sampling every 10th file). Focus on performance for large sequences.
5. [x] **File Order & Uniform Timestamping Robustness:** Validate that file order is alphabetical and timestamps are computed as `file_order_index / fps_manual`.
6. [x] **`FrameCache` Memory Management Test:** Implement a functional `FrameCache` (LRU) with a configurable memory budget.

---

## Phase 1: Core Library & Command-Line Validation

**Goal:** Create a UI-independent core library (`libdroplet`) that handles all data processing. Prove its functionality with comprehensive unit tests and a rudimentary command-line interface (CLI) for end-to-end testing.

7. [ ] **Create project directory layout:** `src/`, `include/`, `tests/`, `third_party/`, `cli/`, `resources/`, `docs/`, `cmake/`.
8. [ ] **Establish top-level `CMakeLists.txt`:** with C++20 standard, MSVC/clang-cl flags, and output directories.
9. [ ] **Add `cmake/` helper modules:** for locating Qt6, OpenCV, libtiff, spdlog, nlohmann/json, GoogleTest.
10. [ ] **Configure `spdlog`:** for file and console logging.
11. [ ] **Define build-time option for DCAM SDK presence (`WITH_DCAM_SDK`).**
12. [ ] **Add build targets:** `DropletAnalyzer` (app), `libdroplet` (static library), and `droplet_analyzer_tests`.
13. [ ] **Implement `InputSource` abstract base class (`include/InputSource.h`).**
14. [ ] **Implement `IAnalysisModule` abstract base class (`include/IAnalysisModule.h`).**
15. [ ] **Define core data structures in `include/DataModels.h`:** `Detection`, `FluorescenceMetrics`, `FrameDetections`, `Track`.
16. [ ] **Define `AnalysisResults` data structure to hold module outputs.**
17. [ ] **Define `ProgressCallback` `std::function` type.**
18. [ ] **Implement `HashUtils` for SHA-256 hashing of files and configurations.**
19. [ ] **Implement `TimeUtils` for timestamp handling.**
20. [ ] **Implement `MathUtils` for geometric calculations (circularity, etc.).**
21. [ ] **Implement background subtraction modes:** "None", "Static" (median), "Running" (exponential update).
22. [ ] **Implement core droplet detection pipeline:** Gaussian blur, adaptive threshold, morphology, contour finding.
23. [ ] **Implement fluorescence quantification logic:** local annulus and global ROI background correction, SBR computation.
24. [ ] **Implement nearest-neighbor centroid tracking algorithm.**
25. [ ] **Implement two-stage Auto-Tune parameter optimization algorithm.**
26. [ ] **Implement `ImageSequenceSource`.**
27. [ ] **Implement `MultiPageTIFFSource` using `libtiff`.**
28. [ ] **Implement `FrameCache` for memory-budgeted LRU caching of frames.**
29. [ ] **Test: Geometry metrics.**
30. [ ] **Test: Adaptive threshold and morphology.**
31. [ ] **Test: Tracking association and lifecycle.**
32. [ ] **Test: Fluorescence metrics and background correction.**
33. [ ] **Test: Auto-tune scoring and parameter selection.**
34. [ ] **Test: TIFF validation rules.**
35. [ ] **Create a CLI application (`cli/DropletAnalyzerCLI`) linking `libdroplet` to test the full pipeline.**
36. [ ] **CLI should accept: `--module`, `--input`, `--output`, and an optional `--config` (JSON file).**
37. [ ] **Use the CLI to run a full `VideoTrackingModule` analysis (detect, track, export CSV).**
38. [ ] **MILESTONE (CLI):** Successfully process the Tier A benchmark video (233 frames, 2304x2304, 16-bit, 23.3 FPS) from the command line.

---

## Phase 2: Basic GUI and Single-Image Modules

**Goal:** Build the main Qt application shell and implement the simpler, single-image analysis modules to validate the UI architecture and library integration.

39. [ ] **Set up `main.cpp` and `MainWindow` class.**
40. [ ] **Integrate Qt6 (Widgets, Concurrent, GUI).**
41. [ ] **Implement main window layout with module selection, control panel, and results/display area.**
42. [ ] **Add status bar for messages and progress UI with cancel support.**
43. [ ] **Integrate QCustomPlot as a `third_party` widget.**
44. [ ] **Implement `BrightfieldModule` inheriting from `IAnalysisModule`.**
45. [ ] **Implement `BrightfieldModuleControlPanel` widget.**
46. [ ] **Connect UI controls to the `run` method via `QtConcurrent::run`.**
47. [ ] **Implement single TIFF loader (8-bit grayscale).**
48. [ ] **Implement export to per-droplet CSV.**
49. [ ] **Test: Brightfield module loads 8-bit TIFF and exports CSV with required columns.**
50. [ ] **Test: Brightfield module rejects unsupported TIFF variants.**
51. [ ] **Implement `FluorescenceModule` and its control panel (8/16-bit TIFFs).**
52. [ ] **Test: Fluorescence module loads 8-bit TIFF and computes valid fluorescence metrics.**
53. [ ] **Test: Fluorescence module loads 16-bit TIFF and handles saturation detection.**
54. [ ] **Test: Fluorescence module rejects unsupported TIFF variants.**
55. [ ] **Implement `MultiChannelModule` and its control panel.**
56. [ ] **Add UI for multi-page channel assignment.**
57. [ ] **Add plotting capabilities: Histograms for population analysis (max 200 bins) & Scatter plots for channel-pair analysis (max 2000 points).**
58. [ ] **Test: Histogram rendering uses <= 200 bins and updates at 1 s or 100 droplet cadence.**
59. [ ] **Test: Scatter plot displays <= 2000 points and samples when exceeded.**
60. [ ] **Implement export CSV with multi-channel metrics.**
61. [ ] **Test: Population module exports multi-channel CSV with correct columns.**
62. [ ] **Test: Multi-Channel module rejects unsupported TIFF variants.**
63. [ ] **MILESTONE (Single-Image GUI):** All three single-image modules are functional within the GUI.

---

## Phase 3: Advanced Offline Video Module

**Goal:** Integrate the most complex offline feature: the video tracking module, with its full suite of UI controls and visualizations.

64. [ ] **Implement `VideoTrackingModule` and its comprehensive control panel.**
65. [ ] **Add UI for video directory selection and FPS input.**
66. [ ] **Display file-order timestamp warning text near FPS input.**
67. [ ] **Implement validation feedback dialogs for sequence loading (file count, format mismatch).**
68. [ ] **Add UI for Auto-Tune vs. Expert Mode, with parameter controls for Expert Mode.**
69. [ ] **Add interactive image widget for drawing ROI and measurement line.**
70. [ ] **Display annotated frames in the image widget (contours, track IDs).**
71. [ ] **Implement time-series plots for metrics like diameter and frequency.**
72. [ ] **Test: Time-series plots respect 300-point max and decimation rules.**
73. [ ] **Add UI for customizing overlays (color, thickness, toggles).**
74. [ ] **Implement the full export pipeline:** Per-droplet CSV, Summary JSON, Annotated video (MJPEG AVI).
75. [ ] **Test: Per-droplet CSV schema matches spec (columns, header metadata, calibrated values).**
76. [ ] **Test: Summary JSON contains module, duration, FPS, total frames, and statistics fields.**
77. [ ] **Implement Session JSON I/O: Save/load all parameters, ROI, calibration, and provenance data.**
78. [ ] **Test: Session JSON includes input hash, config hash, ROI, calibration, and git commit.**
79. [ ] **Test: Annotated MJPEG AVI export produces playable file with overlays.**
80. [ ] **Implement a "Tools -> Run Benchmark" menu action.**
81. [ ] **Test: Annotated video export meets Tier A export target (≥50 FPS).**
82. [ ] **Test: Tier A detection-only pipeline meets ≥100 FPS.**
83. [ ] **Test: Tier A fluorescence pipeline meets ≥60 FPS (1 channel).**
84. [ ] **Test: Tier B detection + tracking meets ≥300 FPS.**
85. [ ] **MILESTONE (Video GUI):** Video Tracking module is fully functional, meeting the Tier A performance target (≥80 FPS Det+Track) from within the GUI.

---

## Phase 4: Camera Module Integration (Reuse qt_hama_gui Assets)

**Goal:** Integrate the existing Hamamatsu camera implementation into DropletAnalyzer (no reimplementation).

86. [ ] **Import and adapt `dcam_controller.*`, `frame_grabber.*`, `frame_types.h`, and relevant UI patterns from `qt_hama_gui`.**
87. [ ] **If needed, wrap existing controller/grabber behind the `IAnalysisModule`/UI integration points.**
88. [ ] **Wire camera controls to `DcamController::apply()` and `readProps()`; use `FrameGrabber` for the live feed.**
89. [ ] **Test: If `dcamapi4.dll` missing, camera module is disabled and info dialog appears.**
90. [ ] **Test: If DCAM present, camera module is enabled and device list populates.**
91. [ ] **Display live feed at ~23 Hz and show requested vs actual settings.**
92. [ ] **Test: USB bandwidth warning appears for unsupported settings.**
93. [ ] **Test: Disconnect/reconnect during live feed recovers without crash.**
94. [ ] **Test: 23.3 FPS sustained for 60 s with zero DCAM drop errors.**
95. [ ] **Test: Grab-to-display latency stays below 100 ms.**
96. [ ] **Test: Expert Mode reduced-resolution high-speed configuration applies without errors.**
97. [ ] **MILESTONE (Camera Control):** Integrated camera module provides stable live feed and control of all specified settings.

---

## Phase 5: Final Validation, Documentation, and Release

**Goal:** Finalize the application with comprehensive testing, documentation, and a distributable installer.

98. [ ] **Create a simple annotation tool or script to generate ground truth data for test videos.**
99. [ ] **Prepare validation datasets per TECHSPEC_V2.**
100. [ ] **Run validation scripts to measure Detection Precision (≥95%) and Recall (≥90%).**
101. [ ] **Test: Diameter MAE < 0.5 um or < 5% of mean diameter.**
102. [ ] **Test: Frequency error < 5% over >= 10 s window.**
103. [ ] **Test: Fluorescence MAE < 5% or < 100 units vs ImageJ.**
104. [ ] **Write User Manual (~70 pages) covering workflows for all modules.**
105. [ ] **Write Algorithm Reference (~30 pages) detailing the image processing pipelines.**
106. [ ] **Generate Doxygen API documentation for `libdroplet`.**
107. [ ] **Test: README includes GPL v3, dependency list, and DCAM SDK prerequisite.**
108. [ ] **Create NSIS installer script.**
109. [ ] **Bundle required runtime DLLs (Qt, OpenCV, libtiff).**
110. [ ] **Test: Installer includes example datasets and session files.**
111. [ ] **Test: Installer excludes DCAM DLLs and prompts if SDK missing.**
112. [ ] **Perform final code review and cleanup.**
113. [ ] **Test: App runs 10 hours continuously without crash.**
114. [ ] **Test: All four offline modules pass smoke tests end-to-end.**
115. [ ] **Test: Camera control module passes stability test at 23.3 FPS for 60 s.**
116. [ ] **Tag `v1.0.0` in git.**
117. [ ] **RELEASE:** Build and sign the final installer.