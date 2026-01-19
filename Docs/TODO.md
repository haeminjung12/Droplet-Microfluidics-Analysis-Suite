# TODO - DropletAnalyzer v1.0.0 - Integrated Implementation & Test Plan

**Project:** DropletAnalyzer: Microfluidics Image Processing & Analysis Suite
**Version:** 1.0.0
**Source Spec:** `Docs/TECHSPEC_V2.md` (diffed against `Docs/TECHSPEC_V1.md`)

---

**Camera scope note:** Camera control is already implemented in `C:\Users\goals\Codex\CNN for Droplet Sorting\cpp_pipeline_github\qt_hama_gui`. All camera work in this plan is integration and verification using those existing assets (e.g., `dcam_controller.*`, `frame_grabber.*`, `frame_types.h`, and UI patterns in `main.cpp`), not reimplementation.

---

## Phase 0: Camera Asset Validation & Early Verification (Weeks 0-1)

**Goal:** Proactively investigate and de-risk critical technical challenges before committing to the full development cycle. These are short, focused prototypes to confirm feasibility and performance.

- [x] **0.1: Camera Asset Inventory (qt_hama_gui)**
    -   **Task:** Identify and list reusable camera assets from `qt_hama_gui` (controller, grabber, frame types, UI patterns) and their dependencies.
    -   **Verification:** Inventory includes source filenames, required Qt/DCAM SDK dependencies, and notes on UI elements to reuse.
    -   **Output:** `Docs/camera_asset_inventory_qt_hama_gui.md`
    -   Done by agent01, commit 5c10c63, PR 1
- [x] **0.2: DCAM Connectivity & Streaming Test (Using Existing Assets)**
    -   **Task:** Build/run the existing `qt_hama_gui` camera pipeline to connect to ORCA-Fusion and stream raw frames.
    -   **Verification:** The existing pipeline connects, streams at 23.3 FPS (full frame, 2304x2304, 16-bit, 1 ms exposure), and sustains acquisition for 60 seconds with zero `DCAMERR_LOSTFRAME` errors.
    -   **Output:** `Docs/dcam_connectivity_streaming_test_qt_hama_gui.md`
    -   **Optional patch:** `Docs/patches/qt_hama_gui_log_lostframe.patch`
    -   Done by agent02, commit 3afd7f4, PR TBD (user confirmed streaming test passed)
- [x] **0.3: DCAM Settings Round-Trip Verification (Using Existing Assets)**
    -   **Task:** Use the existing `qt_hama_gui` camera controls to set and read back parameters (resolution, binning, ROI, FPS, exposure, gain, bit depth, trigger mode).
    -   **Verification:** Requested vs actual settings are logged and match within acceptable tolerances.
    -   Done by agent02, commit b1c7739, PR TBD (user requested mark complete)
- [ ] **0.4: Image Sequence Loading & Validation Performance Test**
    -   **Task:** Implement a prototype for `ImageSequenceSource` that scans a directory, sorts filenames alphabetically (no frame index parsing), and performs format consistency checks (sampling every 10th file). Focus on performance for large sequences.
    -   **Verification:** Using a test directory containing ~233 full-resolution TIFF files (2304x2304, 16-bit), the prototype completes directory scanning, sorting, and format consistency sampling in under 5 seconds. Log output includes total parse time and reported "cache hit rate" (simulated or actual) remains within acceptable bounds for an LRU cache.
- [x] **0.5: File Order & Uniform Timestamping Robustness**
    -   **Task:** Validate that file order is alphabetical and timestamps are computed as `file_order_index / fps_manual`.
    -   **Verification:**
        -   **Ordering Test:** Provide filenames out of numeric order (e.g., `img_10.tif`, `img_2.tif`, `img_1.tif`) and assert the load order is alphabetical.
        -   **Timing Test:** With FPS=23.3, assert timestamps are `0.0`, `1/23.3`, `2/23.3`, ... based on file order index, with no gap handling or index parsing.
- [x] **0.6: `FrameCache` Memory Management Test**
    -   **Task:** Implement a functional `FrameCache` (LRU) with a configurable memory budget.
    -   **Verification:** Configure the `FrameCache` with a 512 MB memory budget. Load frames (simulated or actual `cv::Mat` objects) exceeding this budget. Monitor allocated memory. Assert that the cache never exceeds its configured memory budget, and that the LRU eviction policy correctly removes the least recently used frames.
    -   Done by agent04, commit 747fce7, PR 6

---

## Phase 1: Core Library & Command-Line Validation

**Goal:** Create a UI-independent core library (`libdroplet`) that handles all data processing. Prove its functionality with comprehensive unit tests and a rudimentary command-line interface (CLI) for end-to-end testing.

- [ ] **1.1: Build System and Project Scaffolding**
    - [ ] Create project directory layout: `src/`, `include/`, `tests/`, `third_party/`, `cli/`, `resources/`, `docs/`, `cmake/`.
        -   **Verification:** Directory structure matches the specification.
    - [ ] Establish top-level `CMakeLists.txt` with C++20 standard, MSVC/clang-cl flags, and output directories.
        -   **Verification:** `CMakeLists.txt` successfully configures the project. Compilation on Windows 10/11 using MSVC or clang-cl targeting C++20 standard passes without errors.
    - [ ] Add `cmake/` helper modules for locating Qt6, OpenCV, libtiff, spdlog, nlohmann/json, GoogleTest.
        -   **Verification:** `cmake/` modules correctly find and link to required external libraries. `cmake --build .` completes successfully for a minimal executable that links these libraries.
    - [ ] Configure `spdlog` for file and console logging.
        -   **Verification:** A test application using `spdlog` successfully writes messages to both a specified log file and the console output. Log levels are configurable.
    - [ ] Define build-time option for DCAM SDK presence (`WITH_DCAM_SDK`).
        -   **Verification:** A test build with `WITH_DCAM_SDK=OFF` successfully compiles, and the resulting binary does not contain any DCAM-related symbols. A separate test build with `WITH_DCAM_SDK=ON` (assuming SDK is present) successfully compiles and includes DCAM symbols.
    - [ ] Add build targets: `DropletAnalyzer` (app), `libdroplet` (static library), and `droplet_analyzer_tests`.
        -   **Verification:** Running `cmake --build .` successfully generates `libdroplet.lib`, `DropletAnalyzer.exe`, and `droplet_analyzer_tests.exe` (or platform equivalents).

- [ ] **1.2: Core Interfaces and Data Models**
    - [ ] Implement `InputSource` abstract base class (`include/InputSource.h`).
        -   **Verification:** A derived dummy class can be instantiated and implements all pure virtual functions defined in `InputSource`. Unit tests ensure `getType()`, `getTotalFrames()`, `getFrame()`, and `getTimestamp()` signatures are correctly defined.
    - [ ] Implement `IAnalysisModule` abstract base class (`include/IAnalysisModule.h`).
        -   **Verification:** A derived dummy class can be instantiated. Unit tests confirm `getName()`, `getDisplayName()`, `configure()`, `run()`, `createControlPanel()`, and `exportResults()` signatures are correctly defined.
    - [ ] Define core data structures in `include/DataModels.h`: `Detection`, `FluorescenceMetrics`, `FrameDetections`, `Track`.
        -   **Verification:** Unit tests confirm that `Detection`, `FluorescenceMetrics`, `FrameDetections`, and `Track` structs can be instantiated, populated with data, and their members accessed correctly. Ensure `cv::Point2f` and `cv::Rect` are handled properly.
    - [ ] Define `AnalysisResults` data structure to hold module outputs.
        -   **Verification:** A unit test instantiates `AnalysisResults`, populates it with dummy `FrameDetections` and `Track` data, and verifies data integrity.
    - [ ] Define `ProgressCallback` `std::function` type.
        -   **Verification:** A unit test defines a lambda function matching `ProgressCallback` signature, passes it to a dummy processing function, and verifies the callback is invoked with expected `current`, `total`, and `status` parameters.

- [ ] **1.3: Core Utilities**
    - [ ] Implement `HashUtils` for SHA-256 hashing of files and configurations.
        -   **Verification:** A unit test computes the SHA-256 hash of a known test file (`test_file.txt` with "hello world") and asserts the output matches the pre-computed hash `b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9`. Another test computes the hash of a known JSON string.
    - [ ] Implement `TimeUtils` for timestamp handling.
        -   **Verification:** A unit test provides a `frame_index` of 100 and an `fps_manual` of 50.0, and asserts that `TimeUtils::inferredTimestamp(100, 50.0)` returns `2.0`. Additional tests cover edge cases like 0 FPS.
    - [ ] Implement `MathUtils` for geometric calculations (circularity, etc.).
        -   **Verification:** Unit tests for `MathUtils::calculateCircularity(area, perimeter)` with known values (e.g., perfect circle, elongated ellipse) assert the returned circularity is within a small epsilon of the expected value. Similarly for area, diameter, and aspect ratio.

- [ ] **1.4: Foundational Image Processing Algorithms (in `libdroplet`)**
    - [ ] Implement background subtraction modes: "None", "Static" (median), "Running" (exponential update).
        -   **Verification:**
            -   **"None"**: Unit test runs background subtraction with "None" mode on a test image; output image is identical to input.
            -   **"Static"**: Unit test uses a synthetic image stack where a constant background is present over a moving foreground object. "Static" mode with N=50 frames should produce a clear background image (median) that accurately represents the constant background, subtracting the moving object.
            -   **"Running"**: Unit test feeds a synthetic video sequence (e.g., a white square moving over a dark background) into the "Running" background mode with a small alpha. The background should gradually converge to the dark background, leaving only the moving square as foreground.
    - [ ] Implement core droplet detection pipeline: Gaussian blur, adaptive threshold, morphology, contour finding.
        -   **Verification:** A test suite uses synthetic images containing known, perfectly circular droplets (e.g., 5 circles of 10px radius on a dark background). The detection pipeline is applied. The test asserts that:
            1.  The number of detected contours matches the known number of droplets.
            2.  The area, circularity, and centroid of each detected droplet are within a 5% tolerance of the known ground truth.
            3.  Noise added to the image (e.g., salt-and-pepper) does not lead to spurious detections.
    - [ ] Implement fluorescence quantification logic: local annulus and global ROI background correction, SBR computation.
        -   **Verification:**
            -   **Mean/Integrated:** Unit test with a synthetic image (e.g., a known grayscale value within a mask). Assert that mean/integrated values match expectations.
            -   **Local Annulus:** Unit test with a synthetic image of a fluorescent droplet (high intensity center) surrounded by a dimmer background. The annulus should be defined in the dimmer region. Assert that `bg_corrected_mean` is calculated correctly. Test edge case where annulus is too small/intersects other droplets; verify "failed" `bg_method` is set.
            -   **Global ROI:** Unit test with a synthetic image and a predefined global background ROI. Assert `bg_corrected_mean` uses the mean intensity of this ROI.
            -   **SBR:** Unit test asserts `SBR` is `mean / I_bg`.
    - [ ] Implement nearest-neighbor centroid tracking algorithm.
        -   **Verification:** Unit tests use synthetic sequences of droplets moving with constant velocity. The tracking algorithm should correctly assign track IDs across frames. Tests should include scenarios with:
            1.  Overlapping bounding boxes.
            2.  Droplets entering/exiting the field of view.
            3.  Reappearance after disappearance assigns a new track ID.
    - [ ] Implement two-stage Auto-Tune parameter optimization algorithm.
        -   **Verification:** A unit test for Auto-Tune uses a synthetic image sequence with varying noise and droplet definitions. Repeated runs on the same dataset should yield consistent optimal parameter sets. The `Q` score should show clear peaks for visually correct detections and penalties for over/under-segmentation.

- [ ] **1.5: Input Source Implementations (in `libdroplet`)**
    - [ ] Implement `ImageSequenceSource`:
        -   **Verification:**
            -   **Non-recursive scan:** A unit test scans a directory with nested folders and asserts only `.tif`/`.tiff` files in the top-level directory are found.
            -   **Alphabetical ordering:** Unit test asserts files are loaded in alphabetical order regardless of numeric tokens in names.
            -   **No naming convention required:** Unit test includes `data.tif` and confirms it loads as a valid frame.
            -   **Validation (File count):** Empty directory returns `false` with an explicit "no TIFF files found" error.
            -   **Validation (Max files):** Directory with >100,000 TIFFs is rejected with an explicit limit error.
            -   **Validation (Format Consistency):** Unit test provides a sequence where `frame_1.tif` is 8-bit and `frame_50.tif` is 16-bit. `load()` returns `false` and `error_message` indicates bit depth mismatch. This test samples every 10th file.
            -   **Validation (TIFF compliance):** Unit tests reject compressed (LZW/JPEG/ZIP), tiled, non-grayscale, BigTIFF, and >8192×8192 files.
    - [ ] Implement `MultiPageTIFFSource` using `libtiff`.
        -   **Verification:** Unit tests with:
            1.  A valid 3-page TIFF file asserts `getTotalFrames()` returns 3 and `getFrame(0)`, `getFrame(1)`, `getFrame(2)` return valid `cv::Mat` objects.
            2.  A TIFF with inconsistent page dimensions/bit depths asserts `load()` returns `false` and `error_message` indicates the mismatch.
            3.  A TIFF with >10 pages is rejected with a clear error.
            4.  `getTimestamp(page_index)` returns `page_index` as a double.
            5.  Accessing `getFrame()` with an invalid page index (e.g., 5 for a 3-page TIFF) throws an appropriate exception or returns an empty `cv::Mat`.
    - [ ] Implement `FrameCache` for memory-budgeted LRU caching of frames.
        -   **Verification:** A unit test instantiates `FrameCache` with a budget (e.g., 10MB). It then repeatedly calls `get()` with `cv::Mat` objects. The test monitors the memory footprint and asserts that the cache's total memory usage never exceeds the budget. `get()` should correctly retrieve cached frames and update LRU order. `clear()` releases all frames. `setMemoryBudget()` updates the max frames and can trigger eviction.

- [ ] **1.6: Unit Testing (Phase 1 Comprehensive)**
    - [ ] **Test: Geometry metrics:** Unit tests for area, circularity, diameter, major/minor axis, aspect ratio. These tests will use known synthetic contours (e.g., perfect circles, ellipses) and assert the calculated metrics are within a small tolerance of expected values.
    - [ ] **Test: Adaptive threshold and morphology:** Unit tests apply adaptive thresholding and morphological operations (open, close) to synthetic grayscale images with known features and noise. Assert that features are correctly segmented and noise is removed.
    - [ ] **Test: Tracking association and lifecycle:** Unit tests for tracking with synthetic data:
        -   Constant velocity droplets (assert track ID persistence).
        -   Droplets appearing/disappearing (assert new track IDs).
        -   Track start (assert velocity is NaN for first detection in each track).
    - [ ] **Test: Fluorescence metrics and background correction:** Unit tests for mean, integrated, min, max, background-corrected mean, and SBR. Test various background methods (local annulus, global ROI) with synthetic images, including edge cases like annuli overlapping other droplets or being too small.
    - [ ] **Test: Auto-tune scoring and parameter selection:** Unit tests for the auto-tune quality scoring function, ensuring it correctly penalizes over-segmentation, under-detection, and irregular shapes. Repeated auto-tune runs on static synthetic images should yield consistent optimal parameters.
    - [ ] **Test: TIFF validation rules:** Unit tests for `ImageSequenceSource` and `MultiPageTIFFSource` that cover TIFF compliance (uncompressed, strip-based, grayscale, no BigTIFF, max dimensions) and format consistency across pages.

- [ ] **1.7: Command-Line Interface (CLI) Tool**
    - [ ] Create a CLI application (`cli/DropletAnalyzerCLI`) linking `libdroplet` to test the full pipeline.
        -   **Verification:** The `DropletAnalyzerCLI.exe` executable is successfully built.
    - [ ] The CLI should accept: `--module`, `--input`, `--output`, and an optional `--config` (JSON file).
        -   **Verification:** Running `DropletAnalyzerCLI --help` displays correct usage information. Invalid arguments result in an error message and non-zero exit code.
    - [ ] Use the CLI to run a full `VideoTrackingModule` analysis (detect, track, export CSV).
        -   **Verification:** `DropletAnalyzerCLI --module VideoTracking --input <TierA_data_path> --output <output_dir> --config <video_config.json>` successfully runs the analysis.
- [ ] **MILESTONE (CLI):** Successfully process the Tier A benchmark video (233 frames, 2304x2304, 16-bit, 23.3 FPS) from the command line.
        -   **Verification:** The CLI completes the Tier A benchmark without error. The output CSV and summary JSON match the expected format and values. The processing time should be logged and compared against the Phase 3 performance target.

---

## Phase 2: Basic GUI and Single-Image Modules

**Goal:** Build the main Qt application shell and implement the simpler, single-image analysis modules to validate the UI architecture and library integration.

- [ ] **2.1: Qt Application Skeleton**
    - [ ] Set up `main.cpp` and `MainWindow` class.
        -   **Verification:** A minimal Qt application launches, displaying an empty `MainWindow`.
    - [ ] Integrate Qt6 (Widgets, Concurrent, GUI).
        -   **Verification:** All necessary Qt6 modules are linked and available for UI development.
    - [ ] Implement main window layout with module selection, control panel, and results/display area.
        -   **Verification:** The main window displays a clear layout with placeholder widgets for module selection (e.g., QListWidget), a control panel area (e.g., QStackedWidget), and a results/display area (e.g., QVBoxLayout).
    - [ ] Add status bar for messages and progress UI with cancel support.
        -   **Verification:** The status bar displays messages (e.g., "Ready"), and a QProgressBar updates during a simulated long operation. A "Cancel" button, when clicked, stops the simulated operation and resets the progress bar.
    - [ ] Integrate QCustomPlot as a `third_party` widget.
        -   **Verification:** A basic QCustomPlot widget is successfully embedded in the UI and can display a simple sine wave plot.

- [ ] **2.2: Brightfield Analysis Module**
    - [ ] Implement `BrightfieldModule` inheriting from `IAnalysisModule`.
        -   **Verification:** The `BrightfieldModule` class can be instantiated and registers itself with the module selection system.
    - [ ] Implement `BrightfieldModuleControlPanel` widget.
        -   **Verification:** The control panel contains a "Select Image" button, parameter inputs (Gaussian sigma, threshold, etc.), and an "Analyze" button.
    - [ ] Connect UI controls to the `run` method via `QtConcurrent::run`.
        -   **Verification:** Clicking "Analyze" initiates the `BrightfieldModule`'s `run` method in a separate thread. The UI remains responsive during the analysis, and the `ProgressCallback` updates the status bar.
    - [ ] Implement single TIFF loader (8-bit grayscale).
        -   **Verification:** Clicking "Select Image" opens a file dialog filtered for TIFFs. Selecting a valid 8-bit grayscale TIFF loads it successfully, displaying its resolution in the UI. Invalid TIFFs are rejected with an error message.
    - [ ] Implement export to per-droplet CSV.
        -   **Verification:** After analysis, clicking an "Export CSV" button saves a CSV file. The CSV contains columns for `droplet_id`, `centroid_x_px`, `centroid_y_px`, `area_px2`, `diameter_eq_px`, `circularity`, etc., for each detected droplet.
    - [ ] **Test: Brightfield module loads 8-bit TIFF and exports CSV with required columns.**
        -   **Verification:** Load a known synthetic 8-bit grayscale TIFF with predefined droplets via the UI. Run analysis. Export CSV. Assert that the CSV contains the expected number of droplets and their metrics match ground truth within tolerance.
    - [ ] **Test: Brightfield module rejects unsupported TIFF variants.**
        -   **Verification:** Attempt to load compressed, tiled, RGB/palette, or BigTIFF images and confirm the module blocks load with a clear validation error.

- [ ] **2.3: Fluorescence & Multi-Channel Population Modules**
    - [ ] Implement `FluorescenceModule` and its control panel (8/16-bit TIFFs).
        -   **Verification:** The `FluorescenceModule` UI includes "Select Image" and fluorescence-specific parameter inputs (e.g., background correction method). It successfully loads both 8-bit and 16-bit grayscale TIFFs.
    - [ ] **Test: Fluorescence module loads 8-bit TIFF and computes valid fluorescence metrics.**
        -   **Verification:** Load a synthetic 8-bit fluorescent image with known droplet and background intensities. Run analysis. Assert that `mean`, `integrated`, `bg_corrected_mean`, and `SBR` metrics for detected droplets match pre-computed values within tolerance.
    - [ ] **Test: Fluorescence module loads 16-bit TIFF and handles saturation detection.**
        -   **Verification:** Load a synthetic 16-bit fluorescent image where some droplets have pixels at the maximum value (65535). Run analysis. Assert that droplets with saturated pixels have the `saturated_flag` set in their `FluorescenceMetrics`.
    - [ ] **Test: Fluorescence module rejects unsupported TIFF variants.**
        -   **Verification:** Attempt to load compressed, tiled, RGB/palette, or BigTIFF images and confirm the module blocks load with a clear validation error.
    - [ ] Implement `MultiChannelModule` and its control panel.
        -   **Verification:** The UI allows selection of a multi-page TIFF and offers controls for assigning each page to a specific channel (e.g., detection, GFP, RFP).
    - [ ] Add UI for multi-page channel assignment.
        -   **Verification:** Selecting a multi-page TIFF populates a dropdown or similar widget, allowing the user to map "Page 0", "Page 1", etc., to "Detection", "Fluorescence 1", "Fluorescence 2". Changes to these assignments correctly influence the analysis.
    - [ ] Add plotting capabilities: Histograms for population analysis (max 200 bins) & Scatter plots for channel-pair analysis (max 2000 points).
        -   **Verification:** After multi-channel analysis, "Histograms" and "Scatter Plot" buttons are enabled. Clicking them displays respective QCustomPlot widgets. Histograms for a known dataset correctly show distributions. Scatter plots (e.g., GFP vs. RFP) display detected droplets.
    - [ ] **Test: Histogram rendering uses <= 200 bins and updates at 1 s or 100 droplet cadence.**
        -   **Verification:** Run multi-channel analysis on a large dataset. Monitor plot updates. Assert that histogram plots do not exceed 200 bins. For streaming data (simulated), assert updates occur no more frequently than once per second or every 100 new droplets.
    - [ ] **Test: Scatter plot displays <= 2000 points and samples when exceeded.**
        -   **Verification:** Run multi-channel analysis on a dataset with >2000 droplets. Assert that the scatter plot displays exactly 2000 points. Visually confirm random sampling by comparing multiple runs on the same data.
    - [ ] Implement export CSV with multi-channel metrics.
        -   **Verification:** Exported CSV from the Multi-Channel module contains columns for each channel's fluorescence metrics (e.g., `gfp_mean`, `gfp_integrated`, `rfp_mean`).
    - [ ] **Test: Population module exports multi-channel CSV with correct columns.**
        -   **Verification:** Load a synthetic 3-page TIFF (Detection, GFP, RFP). Assign channels. Run analysis. Export CSV. Assert that the CSV contains columns for detection metrics AND `gfp_mean`, `rfp_mean`, etc., for each droplet.
    - [ ] **Test: Multi-Channel module rejects unsupported TIFF variants.**
        -   **Verification:** Attempt to load multi-page TIFFs that are compressed, tiled, BigTIFF, or have page format mismatches; confirm the module blocks load with a clear validation error.
    - [ ] **MILESTONE (Single-Image GUI):** All three single-image modules are functional within the GUI.
        -   **Verification:** Load example images for each of the Brightfield, Fluorescence, and Multi-Channel modules. Successfully run analyses and verify the correct output CSVs and plot visualizations.

---

## Phase 3: Advanced Offline Video Module

**Goal:** Integrate the most complex offline feature: the video tracking module, with its full suite of UI controls and visualizations.

- [ ] **3.1: Video Tracking Module UI**
    - [ ] Implement `VideoTrackingModule` and its comprehensive control panel.
        -   **Verification:** The `VideoTrackingModule` appears in the module selection. Its control panel is laid out with logically grouped settings for input, detection, tracking, visualization, and export.
    - [ ] Add UI for video directory selection and FPS input.
        -   **Verification:** A "Select Video Directory" button opens a folder picker. A QLineEdit allows entering manual FPS (validated for positive float values).
    - [ ] Display file-order timestamp warning text near FPS input.
        -   **Verification:** UI text states that timestamps are computed from alphabetical file order and FPS, and advises users to sort files correctly.
    - [ ] Implement validation feedback dialogs for sequence loading (file count, format mismatch).
        -   **Verification:**
            -   **Empty directory:** Attempt to load a directory with no TIFF files. An error dialog appears indicating no files found.
            -   **Format mismatch:** Attempt to load a sequence with inconsistent TIFF formats. An error dialog appears, detailing the mismatch, and blocks the load.
    - [ ] Add UI for Auto-Tune vs. Expert Mode, with parameter controls for Expert Mode.
        -   **Verification:** A toggle button (e.g., QRadioButton) switches between "Auto-Tune" and "Expert Mode." In Expert Mode, parameter spinboxes/sliders for Gaussian sigma, adaptive threshold, morphology kernels, etc., are enabled.
    - [ ] Add interactive image widget for drawing ROI and measurement line.
        -   **Verification:** An image display area allows users to draw a rectangular ROI (mouse drag) and a line (two clicks). The coordinates of these elements are captured and update the module's parameters.

- [ ] **3.2: Visualization and Results Display**
    - [ ] Display annotated frames in the image widget (contours, track IDs).
        -   **Verification:** After analysis, playing an annotated video in the UI correctly overlays detected droplet contours, their unique `droplet_id`s, and `track_id`s on each frame.
    - [ ] Implement time-series plots for metrics like diameter and frequency.
        -   **Verification:** Plots of diameter, velocity, and frequency over time (x-axis: timestamp) are displayed using QCustomPlot.
    - [ ] **Test: Time-series plots respect 300-point max and decimation rules.**
        -   **Verification:** Analyze a long video sequence producing >300 data points. Assert that time-series plots display exactly 300 points (or fewer if total is less). Visually confirm uniform decimation (e.g., every Nth point is plotted) rather than just taking the first/last 300.
    - [ ] Add UI for customizing overlays (color, thickness, toggles).
        -   **Verification:** UI controls (checkboxes, color pickers, spinboxes) allow toggling visibility of contours/IDs, changing their color, and adjusting line thickness on the annotated display.

- [ ] **3.3: Data Export and Session Management**
    - [ ] Implement the full export pipeline:
        - [ ] Per-droplet CSV (wide format, with `frame_index_logical`, `track_id`, etc.).
        - [ ] Summary statistics JSON.
        - [ ] Annotated video export to MJPEG AVI.
        -   **Verification:** Export buttons correctly trigger the saving of each file type.
    - [ ] **Test: Per-droplet CSV schema matches spec (columns, header metadata, calibrated values).**
        -   **Verification:** Export a CSV from a processed video. Assert the metadata header includes calibration, FPS, total frames, resolution, and bit depth. Assert the column header includes `timestamp_inferred_s`, `frame_index_logical`, `droplet_id`, `track_id`, `centroid_x_px`, `centroid_x_um`, etc. Verify `velocity_um_s` is NaN for the first frame of each track and `_um` columns are correctly calibrated.
    - [ ] **Test: Summary JSON contains module, duration, FPS, total frames, and statistics fields.**
        -   **Verification:** Export summary JSON. Assert the JSON schema matches the spec, including `module`, `duration_s`, `total_frames`, `fps_actual`, and aggregate statistics for `diameter_um`, `frequency_hz`.
    - [ ] Implement Session JSON I/O: Save/load all parameters, ROI, calibration, and provenance data.
        -   **Verification:** Save a session after analysis. Close the app. Re-open and load the session. Assert that all UI parameters (detection, tracking, ROI, calibration) are restored to their saved state.
    - [ ] **Test: Session JSON includes input hash, config hash, ROI, calibration, and git commit.**
        -   **Verification:** Open a saved session JSON. Assert that it contains `input.first_file_hash_sha256`, `config_hash_sha256`, `roi`, `calibration`, and `software.git_commit` (if available at build time).
    - [ ] **Test: Annotated MJPEG AVI export produces playable file with overlays.**
        -   **Verification:** Export an annotated video. Verify the AVI file can be opened and played by a standard video player. Visually inspect for correct overlays (contours, IDs) and quality (MJPEG, quality 85).

- [ ] **3.4: Performance Benchmarking (Phase 3)**
    - [ ] Implement a "Tools -> Run Benchmark" menu action.
        -   **Verification:** The menu item exists and triggers a benchmark test.
    - [ ] **Test: Annotated video export meets Tier A export target (≥50 FPS).**
        -   **Verification:** Export overlays for 233 Tier A frames (MJPEG, quality 85) and confirm completion in ≤5 seconds.
    - [ ] **Test: Tier A detection-only pipeline meets ≥100 FPS.**
        -   **Verification:** Run Tier A detection-only and confirm processing completes in ≤2.5 seconds for 233 frames.
    - [ ] **Test: Tier A fluorescence pipeline meets ≥60 FPS (1 channel).**
        -   **Verification:** Run Tier A with one fluorescence channel and confirm processing completes in ≤4 seconds for 233 frames.
    - [ ] **Test: Tier B detection + tracking meets ≥300 FPS.**
        -   **Verification:** Run Tier B (1152x1152, 8-bit, 500 frames, ROI 900x900) and confirm processing completes in ≤1.7 seconds.
- [ ] **MILESTONE (Video GUI):** Video Tracking module is fully functional, meeting the Tier A performance target (≥80 FPS Det+Track) from within the GUI.
        -   **Verification:** Run the Tier A benchmark (233 frames, 2304x2304, 16-bit, 1800x1800 ROI). Log the processing time for Detection + Tracking. Assert that the average FPS is ≥80.

---

## Phase 4: Camera Module Integration (Reuse qt_hama_gui Assets)

**Goal:** Integrate the existing Hamamatsu camera implementation into DropletAnalyzer (no reimplementation).

- [ ] **4.1: Camera Asset Integration**
    - [ ] Import and adapt `dcam_controller.*`, `frame_grabber.*`, `frame_types.h`, and relevant UI patterns from `qt_hama_gui`.
        -   **Verification:** DropletAnalyzer builds with camera assets when `WITH_DCAM_SDK=ON`, and builds without camera symbols when `WITH_DCAM_SDK=OFF`.
    - [ ] If needed, wrap existing controller/grabber behind the `IAnalysisModule`/UI integration points.
        -   **Verification:** Camera module can be instantiated and attached to the main window without code duplication.

- [ ] **4.2: Camera Control UI Wiring (Reuse Patterns)**
    - [ ] Wire camera controls to `DcamController::apply()` and `readProps()`; use `FrameGrabber` for the live feed.
        -   **Verification:** UI controls update camera settings and read-back values without errors.
    - [ ] **Test: If `dcamapi4.dll` missing, camera module is disabled and info dialog appears.**
        -   **Verification:** On a system without `dcamapi4.dll`, the Camera Control module is disabled and an informative dialog explains the missing dependency.
    - [ ] **Test: If DCAM present, camera module is enabled and device list populates.**
        -   **Verification:** With an ORCA-Fusion connected, the device list populates and settings reflect camera defaults/ranges.
    - [ ] Display live feed at ~23 Hz and show requested vs actual settings.
        -   **Verification:** Live feed updates at ~23 Hz (full-frame USB-limited); requested vs actual values are shown and mismatches are warned.
    - [ ] **Test: USB bandwidth warning appears for unsupported settings.**
        -   **Verification:** Request full-frame FPS above 23.3 or settings exceeding USB 3.0 throughput and confirm a bandwidth warning is displayed.

- [ ] **4.3: Stability and Validation (Reuse)**
    - [ ] **Test: Disconnect/reconnect during live feed recovers without crash.**
        -   **Verification:** Live feed stops cleanly on disconnect and resumes after reconnect without app restart.
    - [ ] **Test: 23.3 FPS sustained for 60 s with zero DCAM drop errors.**
        -   **Verification:** Full-frame 2304x2304, 16-bit acquisition runs 60 s with zero `DCAMERR_LOSTFRAME`.
    - [ ] **Test: Grab-to-display latency stays below 100 ms.**
        -   **Verification:** Measure timestamp at frame capture and first paint; assert median latency <100 ms.
    - [ ] **Test: Expert Mode reduced-resolution high-speed configuration applies without errors.**
        -   **Verification:** Apply a reduced-height configuration (e.g., 2304x4) and confirm the camera reports a higher FPS mode without DCAM errors.
    - [ ] **MILESTONE (Camera Control):** Integrated camera module provides stable live feed and control of all specified settings.
        -   **Verification:** All tests in Phase 4.2 and 4.3 pass.

---

## Phase 5: Final Validation, Documentation, and Release

**Goal:** Finalize the application with comprehensive testing, documentation, and a distributable installer.

- [ ] **5.1: Final Validation and Ground Truth**
    - [ ] Create a simple annotation tool or script to generate ground truth data for test videos (if not already done in Phase 1 or 2).
        -   **Verification:** The tool/script correctly generates JSON files representing ground truth droplet positions, sizes, and labels for synthetic or real datasets.
    - [ ] Prepare validation datasets per TECHSPEC_V2 (Tier A full-res USB-limited, fluorescence multi-page, reduced-size offline reference).
        -   **Verification:** Datasets match the stated resolutions, bit depths, and frame counts: 2304x2304 16-bit @ 23.3 FPS (233 frames), multi-page TIFF (3 pages, 16-bit), and 1152x1152 8-bit @ 100 FPS (500 frames).
    - [ ] Run validation scripts to measure Detection Precision (≥95%) and Recall (≥90%).
        -   **Verification:** An external Python script processes output CSVs against ground truth. It reports detection precision and recall. Assert these metrics meet or exceed their targets.
    - [ ] **Test: Diameter MAE < 0.5 um or < 5% of mean diameter.**
        -   **Verification:** The validation script calculates Mean Absolute Error (MAE) for droplet diameters against ground truth. Assert MAE is less than 0.5 µm or 5% of the mean diameter.
    - [ ] **Test: Frequency error < 5% over >= 10 s window.**
        -   **Verification:** For a video with a known, consistent droplet generation frequency, the script calculates the relative error of the detected frequency over a 10-second window. Assert this error is less than 5%.
    - [ ] **Test: Fluorescence MAE < 5% or < 100 units vs ImageJ.**
        -   **Verification:** For synthetic fluorescent images with known intensities, the script compares DropletAnalyzer's fluorescence measurements (mean, integrated) against values obtained from ImageJ's measurement tools. Assert MAE is less than 5% or 100 intensity units (for 16-bit).

- [ ] **5.2: Documentation**
    - [ ] Write User Manual (~70 pages) covering workflows for all modules.
        -   **Verification:** Manual is drafted, covers all modules, includes USB bandwidth limitations, and is ~70 pages.
    - [ ] Write Algorithm Reference (~30 pages) detailing the image processing pipelines.
        -   **Verification:** Reference is drafted, explains key algorithms and USB constraints, and is ~30 pages.
    - [ ] Generate Doxygen API documentation for `libdroplet`.
        -   **Verification:** `doxygen Doxyfile` runs without errors and produces comprehensive API documentation.
    - [ ] **Test: README includes GPL v3, dependency list, and DCAM SDK prerequisite.**
        -   **Verification:** `README.md` file is present in the repository root and contains all specified information.

- [ ] **5.3: Packaging and Deployment**
    - [ ] Create NSIS installer script.
        -   **Verification:** NSIS script compiles successfully without warnings.
    - [ ] Bundle required runtime DLLs (Qt, OpenCV, libtiff).
        -   **Verification:** A test installation on a clean Windows VM confirms all necessary DLLs are present in the installation directory.
    - [ ] **Test: Installer includes example datasets and session files.**
        -   **Verification:** The installed application contains an "Examples" folder with sample images, videos, and `.dasession` files.
    - [ ] **Test: Installer excludes DCAM DLLs and prompts if SDK missing.**
        -   **Verification:** On a clean Windows VM, run the installer. Verify that `dcamapi4.dll` is *not* bundled. Launch the installed application. If DCAM SDK is not present, assert that the application displays a prompt or message informing the user of the missing dependency for camera functionality.

- [ ] **5.4: Release Readiness**
    - [ ] Perform final code review and cleanup.
        -   **Verification:** Codebase adheres to coding standards. No outstanding TODOs or FIXMEs.
    - [ ] **Test: App runs 10 hours continuously without crash.**
        -   **Verification:** The application runs the Video Tracking module on a long video sequence (e.g., 10 hours worth of frames) without user intervention. Monitor memory usage and CPU. Assert no crashes or memory leaks.
    - [ ] **Test: All four offline modules pass smoke tests end-to-end.**
        -   **Verification:** Each offline module (Brightfield, Fluorescence, Multi-Channel, Video Tracking) is run with a standard test dataset from start to finish. Output files are generated correctly, and no errors occur.
    - [ ] **Test: Camera control module passes stability test at 23.3 FPS for 60 s.**
        -   **Verification:** The test from Phase 0.2 is re-run and passes, confirming no regression.
    - [ ] Tag `v1.0.0` in git.
        -   **Verification:** `git tag v1.0.0` is executed on the final release commit.
    - [ ] **RELEASE:** Build and sign the final installer.

