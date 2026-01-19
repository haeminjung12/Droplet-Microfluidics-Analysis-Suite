```markdown
# Technical Specification Sheet
## DropletAnalyzer: Microfluidics Image Processing & Analysis Suite

**Product Version:** 1.0.0  
**Specification Version:** 1.0  
**Date:** 2024  
**Status:** IMPLEMENTATION READY  
**Project Goal:** Build a droplet microfluidics analysis tool with offline batch processing (video sequences, single images, fluorescence quantification) and basic real-time camera control for the Hamamatsu ORCA-Fusion. Designed with extensible module architecture for future real-time analysis add-ons.  
**Technology Stack:** C++20, Qt6 (Widgets), OpenCV 4.x, CMake  
**Target Platform:** Windows 10/11 (64-bit)  
**License:** GPL v3 (open source)

---

## 1. Scope and Objectives

### 1.1 Primary Objectives (v1.0.0 - Minimum Viable Product)

#### 1.1.1 Offline Analysis Modules (Core)

**Module 1: Video - Droplet Tracking**
- Load **video as image sequence** (directory of sequential TIFF files).
- Detect droplets using semi-automated parameter optimization (Auto-Tune Mode).
- Track droplets frame-to-frame (nearest-neighbor centroid association).
- Compute geometric metrics: diameter, area, circularity, velocity, frequency.
- Export per-droplet CSV and summary statistics JSON.
- Export annotated video with customizable overlays.

**Module 2: Image - Brightfield Analysis**
- Load single brightfield images (TIFF 8-bit grayscale).
- Detect droplets using Auto-Tune or Expert Mode.
- Compute geometric metrics.
- Export results CSV.

**Module 3: Image - Fluorescence Analysis**
- Load single fluorescence images (TIFF 8/16-bit grayscale).
- Detect droplets (or use ROI from brightfield).
- Compute fluorescence metrics: Mean, integrated, min/max, background-corrected, SBR.
- Export results CSV.

**Module 4: Image - Multi-Channel Population Analysis**
- Load multi-page TIFF (1 detection channel + up to 2 fluorescence channels, max 3 pages total).
- User manually specifies page assignments.
- Detect droplets on detection channel; measure fluorescence on all channels.
- Population-level statistics: histograms, gating (fraction above threshold).
- Export multi-channel CSV.

#### 1.1.2 Real-Time Camera Control Module (v1.0.0)
- Connect to **Hamamatsu ORCA-Fusion C15440-20UP** via DCAM-API SDK.
- Display live camera feed (raw frames, no processing).
- **Camera Control Testing:**
  - Resolution (full frame 2304×2304, partial frame, custom ROI).
  - Frame rate (up to 23.3 FPS at full resolution 2304×2304, 16-bit, 1ms exposure).
  - Exposure time (ms, range read from camera).
  - Gain (range read from camera).
  - Bit depth (8/12/16-bit, capability-driven).
  - Trigger mode (internal/external, test only).
- **Purpose:** Validate camera connection, test all settings, ensure stable acquisition at achievable USB speeds.
- **No droplet detection, no analysis, no logging in v1.0.0.**

#### 1.1.3 Key Design Principles
- **Semi-automated workflow:** Auto-Tune Mode for ease of use, Expert Mode for advanced users.
- **Extensible module architecture:** Add-ons (v1.1+) can be added without modifying core.
- **Reproducibility:** Session files with full provenance (git commit, input hash, config hash).
- **Performance:** Multi-threaded offline processing (frame-level parallelism).
- **Robustness:** Strict TIFF validation, graceful error handling, memory-efficient lazy loading.

### 1.2 Explicitly Deferred to Future Versions

#### v1.1 (Highest Priority - Real-Time Analysis Add-Ons)
- **Real-time droplet detection module** (process live feed, overlay contours).
- **Real-time frequency/diameter module** (compute metrics on live stream).
- **Real-time recording/logging** (save raw/annotated video, CSV data log).
- **Generator QA module** (stability, drift, CV metrics).
- **NI-DAQ plugin** (hardware triggering, analog I/O, optional runtime-loaded).

#### v1.5 (Future Enhancements)
- **Merger QA module** (pairing/merging efficiency with ROI-based state machine).
- **Sorter QA module** (hit/waste ratio, fluorescence-based sorting validation).
- **Kalman filter tracking** (occlusion handling).
- **Advanced fluorescence metrics** (median, percentiles, std, CV, SNR, radial profiles).
- **OME-TIFF import** with metadata parsing.
- **GPU acceleration** (CUDA, NVIDIA RTX).

#### v2.0 (Advanced Features)
- 3D droplet reconstruction, FRET, co-localization, PDF reports, Parquet/HDF5 export, automated population clustering.

---

## 2. System Architecture

### 2.1 Input Source Abstraction

**Design Goal:** Decouple image/video I/O from analysis logic for testability and extensibility.

```cpp
// Abstract base class for all input sources
class InputSource {
public:
    enum class Type { SingleImage, ImageSequence, MultiPageTIFF, CameraStream };
    
    virtual ~InputSource() = default;
    virtual Type getType() const = 0;
    virtual size_t getTotalFrames() const = 0;  // 0 for streams
    virtual cv::Mat getFrame(size_t logical_index) = 0;  // Indexed access with caching
    virtual double getTimestamp(size_t logical_index) const = 0;  // Based on file order and FPS
};
```

#### Implementation 1: Image Sequence Source (Video as Directory)

```cpp
class ImageSequenceSource : public InputSource {
public:
    ImageSequenceSource(const std::string& directory, double fps_manual);
    
    // Scans directory, collects all TIFF files
    bool load(std::string& error_message);
    
    cv::Mat getFrame(size_t logical_index) override;  // Uses LRU cache
    double getTimestamp(size_t logical_index) const override {
        // Uniform timestamp spacing based on file order and FPS
        return logical_index / fps_manual_;
    }
    
private:
    struct FrameFile {
        std::string path;
        // Note: No frame_index parsing. Files are loaded in sorted order.
    };
    
    std::vector<FrameFile> frames_;  // Sorted by filename (alphabetical)
    double fps_manual_;
    
    FrameCache cache_;  // Memory-budget based LRU cache
};
```

**Frame Cache (Memory-Budget Based):**
```cpp
class FrameCache {
public:
    FrameCache(size_t memory_budget_bytes = 512 * 1024 * 1024);  // Default 512 MB
    
    cv::Mat get(size_t frame_id, std::function<cv::Mat()> loader);
    void clear();
    size_t getMaxFrames() const { return max_frames_; }
    void setMemoryBudget(size_t bytes);
    
private:
    size_t memory_budget_;
    size_t frame_size_bytes_;  // Computed from first frame loaded
    size_t max_frames_;        // budget / frame_size
    
    std::unordered_map<size_t, cv::Mat> cache_;
    std::list<size_t> lru_order_;  // Front = MRU, Back = LRU
    
    void evict();  // Remove LRU frame if cache full
};
```

#### Implementation 2: Multi-Page TIFF Source
```cpp
class MultiPageTIFFSource : public InputSource {
public:
    MultiPageTIFFSource(const std::string& tiff_path);
    
    bool load(std::string& error_message);  // Validates pages, checks consistency
    
    cv::Mat getFrame(size_t page_index) override;  // Loads page using libtiff
    size_t getTotalFrames() const override { return num_pages_; }
    
    double getTimestamp(size_t page_index) const override {
        // Multi-page TIFF: No inherent timestamps, return frame index
        return static_cast<double>(page_index);
    }
    
private:
    TIFF* tiff_handle_;
    size_t num_pages_;
    std::unordered_map<size_t, cv::Mat> cache_;  // Small cache (10 pages max)
};
```

#### Implementation 3: Camera Source (DCAM)
```cpp
class CameraSource : public InputSource {
public:
    CameraSource(const std::string& camera_id);
    
    bool connect();
    void disconnect();
    bool setSettings(const CameraSettings& requested, CameraSettings& actual);
    
    bool grabNextFrame(cv::Mat& output) override;  // Non-blocking grab with timeout
    double getTimestamp(size_t) const override {
        return std::chrono::duration<double>(last_frame_time_ - acquisition_start_).count();
    }
    
    std::string getLastError() const;
    
private:
    HDCAM dcam_handle_;
    std::chrono::high_resolution_clock::time_point acquisition_start_;
    std::chrono::high_resolution_clock::time_point last_frame_time_;
    std::string last_error_;
};
```

### 2.2 Intermediate Data Model (For Parallel Processing)

```cpp
// Single detected droplet (output of detection on one frame)
struct Detection {
    cv::Point2f centroid;
    float area_px2;
    float perimeter_px;
    float diameter_eq_px;
    float major_axis_px, minor_axis_px, angle_deg;
    float circularity;
    float aspect_ratio;
    cv::Rect bounding_box;  // For fast spatial queries
    std::vector<cv::Point> contour;  // Full contour (for fluorescence masking)
    
    // Fluorescence metrics (computed in parallel if multi-channel)
    std::unordered_map<std::string, FluorescenceMetrics> fluorescence;
    // Example: {"gfp": {mean: 450.2, integrated: 54000, ...}, "rfp": {...}}
};

struct FluorescenceMetrics {
    float mean;
    float integrated;
    float min;
    float max;
    float bg_corrected_mean;
    float sbr;
    std::string bg_method;  // "local_annulus" / "global_roi" / "failed"
};

// All detections from one frame (output of parallel detection step)
struct FrameDetections {
    size_t frame_index_logical;   // Position in loaded sequence (0, 1, 2, ...)
    double timestamp_inferred_s;  // frame_index_logical / fps_manual
    std::vector<Detection> detections;
};

// A tracked droplet across multiple frames (output of tracking step)
struct Track {
    size_t track_id;  // Unique track ID
    std::vector<size_t> droplet_ids;  // Which Detection (by global droplet_id) belongs to this track
    std::vector<size_t> frame_indices;  // Frame indices where this track appears
    std::vector<cv::Point2f> centroids;  // Centroid history
    std::vector<double> timestamps;  // Timestamp history
    std::vector<float> velocities;  // Velocity at each frame (except first)
    bool crossed_line;  // Did this track cross measurement line?
    size_t line_crossing_frame;  // Frame index where crossing occurred (if crossed_line=true)
};
```

### 2.3 Module Interface (Revised - UI-Independent Core)

```cpp
// Abstract interface for analysis modules
class IAnalysisModule {
public:
    virtual ~IAnalysisModule() = default;
    
    virtual QString getName() const = 0;  // Internal name (e.g., "VideoTrackingModule")
    virtual QString getDisplayName() const = 0;  // UI name (e.g., "Video: Droplet Tracking")
    
    // Core processing (UI-independent, testable)
    virtual void configure(const nlohmann::json& params) = 0;
    virtual void run(InputSource& input, AnalysisResults& output, ProgressCallback callback) = 0;
    
    // UI factory (returns new widget, caller takes ownership)
    virtual QWidget* createControlPanel() = 0;
    
    // Export
    virtual void exportResults(const AnalysisResults& results, const QString& path) = 0;
};

// Progress callback (thread-safe)
using ProgressCallback = std::function<void(size_t current, size_t total, const std::string& status)>;
```

**Benefits:**
- `run()` takes `InputSource&` (works for images, sequences, camera).
- `createControlPanel()` returns new widget (UI optional, testable without GUI).
- `ProgressCallback` decouples progress reporting from Qt signals (can be used in CLI tool).

---

## 3. Functional Requirements

### 3.1 Image Sequence (Video) Loading

#### Strict Contract and Validation

**FR-IS1: Directory Selection**
- User clicks "Select Video Directory..." → selects folder containing TIFF files.
- System scans directory for `.tif` and `.tiff` files (case-insensitive, recursive off).

**FR-IS2: File Collection Rule**
- **System collects all TIFF files from the directory.**
- Files are sorted **alphabetically by filename**.
- **No frame index parsing is performed.**
- Files are loaded sequentially: 1st file (alphabetically) → frame 0, 2nd file → frame 1, etc.
- Example: If directory contains `frame_001.tif`, `frame_002.tif`, `frame_004.tif`, `frame_005.tif`:
  - Frame 0: `frame_001.tif`
  - Frame 1: `frame_002.tif`
  - Frame 2: `frame_004.tif`
  - Frame 3: `frame_005.tif`
  - **Gaps in filenames are ignored; no special handling.**

**FR-IS3: Validation on Load**
System performs validation:
1. **Check file count:** At least 1 TIFF file in directory.
2. **Check format consistency:**
   - Load first file, extract: resolution, bit depth, compression type.
   - Validate all other files match (sample every 10th file for performance).
   - If mismatch detected → **ERROR** (reject load).
3. **Check for duplicates:** No validation; assume user manages filenames correctly.

**FR-IS4: Validation Report (Display to User)**
```
┌────────────────────────────────────────────────┐
│   Image Sequence Validation                   │
├────────────────────────────────────────────────┤
│ Directory: C:\Data\experiment_001\             │
│ Files found: 2998 TIFFs                        │
│                                                │
│ Resolution: 2304×2304 px (validated)           │
│ Bit depth: 16-bit (validated)                  │
│ Compression: Uncompressed (validated)          │
│                                                │
│ [Load Sequence]  [Cancel]                      │
└────────────────────────────────────────────────┘
```

**FR-IS5: Uniform Frame Timing**
- User must enter FPS (float, >0).
- Label: "Enter frame rate (FPS) of the video sequence:"
- **Timestamps are computed uniformly based on file order:** `timestamp = file_order_index / fps_manual`
  - 1st file (file_order_index=0): timestamp = 0.00 s
  - 2nd file (file_order_index=1): timestamp based on FPS entered
  - 3rd file (file_order_index=2): timestamp based on FPS entered
  - Etc.
- **Files are processed in the order they are loaded from the directory (alphabetically sorted).**
- Note: "Timestamps will be computed based on the file order in the folder and FPS. Ensure your TIFF files are sorted correctly before loading."

**FR-IS6: File Format Mismatch (ERROR)**
```
┌────────────────────────────────────────────────┐
│   ERROR: Inconsistent Image Format             │
├────────────────────────────────────────────────┤
│ Not all TIFFs have the same format:            │
│                                                │
│ frame_001.tif: 2304×2304, 16-bit, Uncompressed│
│ frame_050.tif: 2304×2304, 8-bit, Uncompressed │
│         ↑ Bit depth mismatch                   │
│                                                │
│ Please convert all files to the same format.   │
│                                                │
│ [OK]                                           │
└────────────────────────────────────────────────┘
```

---

### 3.2 TIFF Support Matrix (Strict Definition)

#### Supported TIFF Types (v1.0.0)

**Single-Page TIFF (Images):**
- **Bit depth:** 8-bit or 16-bit grayscale only.
- **Compression:** **Uncompressed only** (no LZW, no JPEG, no ZIP).
  - Rationale: Maximize decode speed and compatibility; LZW adds complexity without major size benefit for analysis images.
- **Layout:** Strip-based (not tiled).
- **Color:** Grayscale only (`PHOTOMETRIC_MINISBLACK`).
- **BigTIFF:** **Not supported** (files must be <4 GB).
  - Rationale: v1.0.0 focuses on typical single-image sizes (<100 MB). BigTIFF adds complexity for minimal v1.0 benefit.
- **Max dimensions:** 8192×8192 px.

**Multi-Page TIFF (Fluorescence, Max 3 Pages):**
- **Max pages:** 10 (v1.0.0 uses max 3: detection + 2 fluorescence; higher limit for future flexibility).
- **Bit depth:** 8-bit or 16-bit grayscale.
- **Compression:** Uncompressed only.
- **Pages must have:** Same resolution, same bit depth.
- **BigTIFF:** **Not supported**.

**Image Sequence (Video as Directory):**
- **Each file:** Must meet single-page TIFF requirements.
- **Naming:** No specific naming convention required; files sorted alphabetically.
- **Max files:** 100,000 (practical limit: ~100 GB total size at 1 MB/frame).

#### Unsupported (v1.0.0)
- **LZW, JPEG, ZIP, PackBits compression** (uncompressed only for decode speed and simplicity).
- **Tiled TIFF** (strip-based only).
- **RGB, palette, CMYK** (grayscale only).
- **BigTIFF** (files >4 GB; deferred to v1.5 if needed).
- **Pyramidal TIFF** (multi-resolution).
- **OME-TIFF metadata** (treat as standard TIFF, manual page assignment).

#### Validation (libtiff)
```cpp
bool validateTIFF(const std::string& path, std::string& error) {
    TIFF* tif = TIFFOpen(path.c_str(), "r");
    if (!tif) {
        error = "Cannot open file (corrupted or not a TIFF)";
        return false;
    }
    
    uint16 compression, bits_per_sample, photometric;
    uint32 width, height;
    TIFFGetField(tif, TIFFTAG_COMPRESSION, &compression);
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
    TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
    
    // Check if tiled
    uint32 tile_width = 0;
    TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tile_width);
    bool is_tiled = (tile_width > 0);
    
    // Check if BigTIFF
    bool is_bigtiff = TIFFIsBigTIFF(tif);
    
    TIFFClose(tif);
    
    // Validate
    if (compression != COMPRESSION_NONE) {
        error = "Unsupported compression (must be uncompressed). Use ImageJ: Image → Type → 8-bit, File → Save As → TIFF";
        return false;
    }
    if (bits_per_sample != 8 && bits_per_sample != 16) {
        error = "Unsupported bit depth (must be 8-bit or 16-bit grayscale)";
        return false;
    }
    if (photometric != PHOTOMETRIC_MINISBLACK) {
        error = "Unsupported color space (must be grayscale). Convert to grayscale in ImageJ.";
        return false;
    }
    if (is_tiled) {
        error = "Tiled TIFF not supported (must be strip-based). Convert in ImageJ or FIJI.";
        return false;
    }
    if (is_bigtiff) {
        error = "BigTIFF not supported in v1.0.0 (file >4 GB). Split into smaller files.";
        return false;
    }
    if (width > 8192 || height > 8192) {
        error = "Image too large (max 8192×8192 px)";
        return false;
    }
    
    return true;
}
```

---

### 3.3 Auto-Tune Parameter Optimization (Revised - Two-Stage + Single Image)

#### Scoring Function (Refined)
$$
Q = \frac{N_{valid} \cdot \bar{C}}{1 + \text{CV}_d + \alpha \cdot (N_{boundary} / N_{valid}) + \beta \cdot f_{outlier}}
$$

**Where:**
- $N_{valid}$: Number of detected droplets passing filters.
- $\bar{C}$: Mean circularity.
- $\text{CV}_d = \sigma_d / \bar{d}$: Coefficient of variation of diameter (penalizes polydispersity or over-segmentation).
- $N_{boundary}$: Droplets touching ROI boundary.
- $f_{outlier} = \max(0, |N_{valid} - N_{expected}| / N_{expected} - 0.5)$ (penalty if count far from expected).
- $\alpha = 0.3$, $\beta = 0.2$ (penalty weights).

**Note:** CV penalty only applied if $N_{valid} \geq 5$ (avoid division by zero or unstable CV with very few droplets).

#### Two-Stage Search (Video)

**Stage 1: Coarse Search (Fast Exploration)**
- **Sample:** 10 frames (uniformly sampled from first 100 frames: frames 0, 11, 22, ..., 99).
- **Search space:**
  - Gaussian sigma: [0, 1.0, 2.0]
  - Adaptive block size: [9, 13]
  - Adaptive C: [0, 2]
  - Morph open kernel: [3, 5]
  - Morph close kernel: [3, 5]
  - **Total:** 3 × 2 × 2 × 2 × 2 = **48 combinations**
- **Compute:** 48 × 10 = **480 evaluations**.
- **Time estimate:** ~5 seconds (parallel, 8 cores, ~100 ms per frame detection).
- **Output:** Top 5 parameter sets by Q score.

**Stage 2: Fine Search (Refinement)**
- **Sample:** 50 frames (uniformly sampled from entire video).
- **Search space:** Top 5 from Stage 1 + **local neighbors** (vary each param by ±1 step).
  - Example: If top candidate is {sigma=1.0, block=11, C=2, open=5, close=5}:
    - Neighbors: {sigma=[0.5, 1.0, 1.5], block=[9, 11, 13], C=[0, 2, 4], ...}
  - Generate ~15 neighbors × 5 top candidates = ~**75 combinations** (duplicates removed).
- **Compute:** 75 × 50 = **3,750 evaluations**.
- **Time estimate:** ~15 seconds.
- **Output:** Best parameter set by Q score.

**Total Auto-Tune Time (Video):** ~20 seconds (Stage 1: 5 sec, Stage 2: 15 sec).

#### Single-Stage Search (Single Image)

**Sample:** Single image only (no temporal sampling).
- **Search space (same as Stage 1 coarse):**
  - 48 combinations.
- **Compute:** 48 evaluations on one image.
- **Time estimate:** ~2–3 seconds.

**User Option: Sampling ROI (Advanced)**
- **UI:** [Checkbox: Use Sampling ROI for Tuning (Faster)] (unchecked by default).
- If checked: [Button: Draw Sampling ROI]
- User draws a small region (e.g., 600×600 px crop of 1800×1800 ROI).
- Auto-tune runs on sampling ROI only (9× faster for this example).
- After tuning, parameters are applied to full ROI for analysis.

#### Failure Mode Output

If $Q_{max} < 5$:
```
┌────────────────────────────────────────────────┐
│   Auto-Tune: No Good Parameters Found          │
├────────────────────────────────────────────────┤
│ Top 3 candidates (with diagnostics):           │
│                                                │
│ 1. Q=4.2 | 8 droplets detected                 │
│    Issues: Too few droplets (expected ~25)     │
│    Params: sigma=0, block=9, C=0, ...          │
│    [Preview Frame with These Params]           │
│                                                │
│ 2. Q=3.8 | 142 droplets detected               │
│    Issues: High diameter CV (45%), likely      │
│            over-segmentation (debris)          │
│    Params: sigma=0, block=7, C=-2, ...         │
│    [Preview]                                   │
│                                                │
│ 3. Q=3.5 | 25 droplets detected                │
│    Issues: Low mean circularity (0.58),        │
│            droplets poorly defined             │
│    Params: sigma=2.0, block=15, C=4, ...       │
│    [Preview]                                   │
│                                                │
│ [Accept Best (#1)] [Switch to Expert Mode]    │
│ [Cancel]                                       │
└────────────────────────────────────────────────┘
```

---

### 3.4 Offline Processing (Multi-Threaded)

**Algorithm (Frame-Level Parallelism):**

**Step 1: Parallel Detection**
```cpp
std::vector<FrameDetections> all_detections = detectAllFramesParallel(
    input_source,
    detection_params,
    roi,
    progress_callback
);
```

**Step 2: Sequential Tracking**
```cpp
std::vector<Track> tracks = performTracking(
    all_detections,
    tracking_params,
    measurement_line
);
```

**Step 3: Flatten to CSV**
```cpp
// Assign global droplet_id to each detection
size_t global_droplet_id = 1;
for (auto& frame : all_detections) {
    for (auto& det : frame.detections) {
        det.droplet_id = global_droplet_id++;
    }
}

// Write CSV rows (one per detection, with track_id from tracking step)
for (const auto& frame : all_detections) {
    for (const auto& det : frame.detections) {
        size_t track_id = findTrackForDetection(det.droplet_id, tracks);
        writeCsvRow(csv_file, frame, det, track_id);
    }
}
```

---

### 3.5 Real-Time Camera Control (v1.0.0 - Detailed)

#### Acceptance Criteria (Revised - USB Bandwidth Limited)

**AC-R1: Camera Detection**
- System detects ORCA-Fusion at startup (via `dcamapi_init()` and device enumeration).
- Display in dropdown: "ORCA-Fusion C15440-20UP [Serial: XXXXXX]"

**AC-R2: Settings Control**
- User can set: Resolution, FPS, Exposure, Gain, Bit Depth, Trigger Mode.
- After "Apply Settings", system reads back **actual values** from camera.
- Display actual vs requested (if different, show warning).

**AC-R3: Full Frame Acquisition (2304×2304, 23.3 FPS)**
- Sustain 23.3 FPS acquisition at full resolution (2304×2304, 16-bit, 1ms exposure) for **60 seconds**.
- **Zero DCAM-reported dropped frames** (`DCAMERR_LOSTFRAME` never occurs).
- UI displays frames at 23.3 Hz (all frames displayed in real-time).
- Memory usage: <1 GB for 10-frame ring buffer.
- **Limitation:** USB 3.0 bandwidth restricts full-resolution acquisition. This is the practical maximum for typical USB cable connections.

**AC-R4: High-Speed Acquisition with Reduced Resolution**
- Support extreme high-speed modes for testing and specialized use cases (e.g., 2304×4 at 8938 FPS with 8-bit, 0.07ms exposure).
- Implementation: Camera settings panel allows full range of capabilities; bandwidth warnings displayed if unavailable.

**AC-R5: Camera Disconnect Handling**
- If camera disconnected during live feed: Acquisition thread detects error, stops cleanly, displays dialog.
- User can reconnect camera and restart live feed without restarting application.

**Dropped Frame Definition (Clarified):**
- **DCAM-reported drop:** `dcambuf_lockframe()` returns `DCAMERR_LOSTFRAME`, or frame index metadata (`iFrame`) has discontinuity.
  - This is a **failure** (driver could not keep up with available bandwidth).
  - Acceptance: **Zero** DCAM-reported drops at standard 23.3 FPS full-resolution operation.
- **UI frame display:** All frames acquired are displayed (no intentional decimation at 23.3 FPS).
  - This is **real-time preview** (1:1 frame display).
- **Queue overflow:** If processing (v1.1+) cannot keep up, oldest frame in queue is discarded.
  - This is **acceptable** (backpressure handling).

---

## 4. Performance Requirements (Revised - Testable Benchmarks)

### 4.1 Test Hardware
- **CPU:** Intel Core i9-13900HX (24 cores: 8 P-cores @ 5.4 GHz, 16 E-cores @ 3.9 GHz)
- **RAM:** 32 GB DDR5
- **Storage:** NVMe SSD (read >3 GB/s)
- **OS:** Windows 11 (64-bit)
- **GPU:** NVIDIA RTX 4070 Laptop (8 GB) — **Not used in v1.0.0**
- **Camera Connection:** USB 3.0 (practical bandwidth ~400 MB/s)

### 4.2 Standard Benchmark Videos (Two Tiers)

#### Tier A: Typical Use Case (Primary Benchmark - USB Bandwidth Limited)
- **Input:** Image sequence (directory of TIFFs).
- **Resolution:** 2304×2304 px (full frame, practical maximum for USB 3.0).
- **Bit depth:** **16-bit grayscale** (standard acquisition mode).
- **FPS (user-entered):** 23.3 FPS (USB 3.0 practical maximum for full resolution, 16-bit).
- **Duration:** 10 seconds (233 frames).
- **Exposure time:** 1 ms (standard setting).
- **File size per frame:** ~10.6 MB (uncompressed TIFF).
- **Total size:** ~2.5 GB.
- **Content:** 15–25 droplets per frame (typical microfluidic droplet generator output).
- **ROI:** 1800×1800 px (typical 80% of full frame).

#### Tier B: Reduced-Size High-Speed Case (Secondary Benchmark - Optional)
- **Resolution:** 1152×1152 px (2×2 binning or ROI, for reference).
- **Bit depth:** 8-bit grayscale.
- **FPS:** 100 FPS (user-entered, simulated or external trigger).
- **Duration:** 5 seconds (500 frames).
- **ROI:** 900×900 px.
- **Content:** 15 droplets/frame.
- **File size:** ~1.3 MB/frame, ~6.5 GB total.
- **Note:** Tier B is for offline analysis only; not achievable in real-time over USB 3.0 at full 2304×2304.

### 4.3 Performance Targets (CPU-only, Multi-Threaded)

#### Tier A (Primary: 2304×2304, 16-bit, ROI 1800×1800, 23.3 FPS data)

| **Mode**                                      | **Target**    | **Measured Metric**                     |
|-----------------------------------------------|---------------|-----------------------------------------|
| Detection only (no tracking, no fluorescence) | ≥100 FPS      | Process 233 frames in ≤2.5 seconds      |
| Detection + Tracking                          | ≥80 FPS       | Process 233 frames in ≤3 seconds        |
| Detection + Tracking + Fluorescence (1 ch)    | ≥60 FPS       | Process 233 frames in ≤4 seconds        |

**Rationale:**
- 16-bit images (larger than 8-bit).
- Full frame resolution 2304×2304 (larger spatial extent).
- Multi-threading (8 P-cores) provides 4–6× speedup for detection.
- i9-13900HX P-cores @ 5.4 GHz are exceptionally fast.
- Offline processing is decoupled from USB bandwidth; performance limited by CPU.

#### Tier B (Secondary: 1152×1152, 8-bit, ROI 900×900, for reference)

| **Mode**                                      | **Target**    | **Measured Metric**                     |
|-----------------------------------------------|---------------|-----------------------------------------|
| Detection only                                | ≥400 FPS      | Process 500 frames in ≤1.3 seconds      |
| Detection + Tracking                          | ≥300 FPS      | Process 500 frames in ≤1.7 seconds      |

#### Export Benchmark (Separate Pass, Not Included in Processing Time)
- **Task:** Render overlays on 233 frames (Tier A data) + encode to MJPEG AVI (quality 85).
- **Target:** ≥50 FPS (complete in ≤5 seconds).
- **Note:** This is I/O and encoding bound, not algorithm bound. Measured separately.

### 4.4 Real-Time Camera Control (v1.0.0)

| **Metric**                             | **Target**              | **Acceptance**                              |
|----------------------------------------|-------------------------|---------------------------------------------|
| Full frame (2304×2304, 16-bit, 1ms)    | 23.3 FPS sustained      | 60 sec, zero DCAM drops, USB 3.0 compliant |
| High-speed modes (reduced resolution)  | Up to 8938 FPS (2304×4) | Camera limit, expert mode only              |
| UI canvas refresh                      | ~23 Hz (real-time)      | Display all frames as acquired              |
| Latency (grab to display)              | <100 ms                 | Live preview acceptable                     |

**USB Bandwidth Notes:**
- **Full resolution 2304×2304, 16-bit, 23.3 FPS:** ~500 MB/s (at frame buffer level).
- **USB 3.0 limit:** ~400 MB/s theoretical, typically 300–350 MB/s practical.
- **Result:** 23.3 FPS is practical maximum; higher rates require reduced resolution or bit depth.
- **Expert Mode:** Allows user to attempt higher speeds; camera will drop frames if bandwidth insufficient (DCAM-reported drops expected if misconfigured).

### 4.5 Memory Usage

| **Scenario**                          | **Peak Memory**       | **Acceptance**          |
|---------------------------------------|-----------------------|-------------------------|
| Offline (Tier A, 233 frames)          | <2.5 GB               | Includes 512 MB cache   |
| Offline (Tier B, 500 frames)          | <1.5 GB               | Includes 512 MB cache   |
| Real-time (23.3 FPS, 60 sec)          | <800 MB               | 10-frame ring buffer    |

**Cache Budget:**
- Default: 512 MB (user-configurable in Preferences: 128 MB – 2 GB).
- Adaptive frame count: $N_{cache} = \lfloor 512\text{MB} / \text{frame\_size} \rfloor$.
  - Example: 2304×2304 16-bit (~10.6 MB/frame) → ~48 frames cached.
  - Example: 1152×1152 8-bit (~1.3 MB/frame) → ~390 frames cached.

---

## 5. Fluorescence Quantification

### 5.1 Background Correction (Self-Contained, Edge Cases)

#### Local Annulus Method (Default)

**Algorithm:**
```cpp
FluorescenceMetrics computeFluorescence(
    const cv::Mat& fluor_image,
    const std::vector<cv::Point>& droplet_contour,
    const cv::Mat& all_droplets_mask,  // Binary mask of all droplets (dilated)
    int annulus_width = 5
) {
    // Create droplet mask
    cv::Mat droplet_mask = cv::Mat::zeros(fluor_image.size(), CV_8U);
    cv::drawContours(droplet_mask, {droplet_contour}, 0, cv::Scalar(255), cv::FILLED);
    
    // Create annulus (dilated - original)
    cv::Mat dilated;
    cv::dilate(droplet_mask, dilated, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(annulus_width*2+1, annulus_width*2+1)));
    cv::Mat annulus = dilated - droplet_mask;
    
    // Exclude pixels inside other droplets
    cv::Mat valid_annulus;
    cv::bitwise_and(annulus, ~all_droplets_mask, valid_annulus);
    
    // Count valid annulus pixels
    int annulus_pixel_count = cv::countNonZero(valid_annulus);
    
    FluorescenceMetrics metrics;
    
    // Edge case: Annulus too small
    if (annulus_pixel_count < 10) {
        // Try expanding annulus
        annulus_width *= 2;  // e.g., 5 → 10
        cv::dilate(droplet_mask, dilated, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(annulus_width*2+1, annulus_width*2+1)));
        annulus = dilated - droplet_mask;
        cv::bitwise_and(annulus, ~all_droplets_mask, valid_annulus);
        annulus_pixel_count = cv::countNonZero(valid_annulus);
        
        if (annulus_pixel_count < 10) {
            // Fallback to global background (if available) or report failure
            metrics.bg_method = "failed";
            metrics.bg_corrected_mean = metrics.mean;  // Use raw mean
            metrics.sbr = NAN;
            return metrics;
        }
    }
    
    // Compute background mean
    cv::Scalar bg_mean = cv::mean(fluor_image, valid_annulus);
    float I_bg = static_cast<float>(bg_mean[0]);
    
    // Compute droplet metrics
    cv::Scalar droplet_mean = cv::mean(fluor_image, droplet_mask);
    double droplet_sum = cv::sum(fluor_image.mul(droplet_mask / 255.0))[0];  // Integrated intensity
    double min_val, max_val;
    cv::minMaxLoc(fluor_image, &min_val, &max_val, nullptr, nullptr, droplet_mask);
    
    metrics.mean = static_cast<float>(droplet_mean[0]);
    metrics.integrated = static_cast<float>(droplet_sum);
    metrics.min = static_cast<float>(min_val);
    metrics.max = static_cast<float>(max_val);
    metrics.bg_corrected_mean = metrics.mean - I_bg;
    metrics.sbr = metrics.mean / I_bg;
    metrics.bg_method = "local_annulus";
    
    // Edge case: Negative corrected mean
    if (metrics.bg_corrected_mean < 0) {
        metrics.bg_corrected_mean = 0.0f;  // Clamp to zero
        metrics.bg_corrected_negative_flag = true;
    }
    
    // Edge case: Saturated pixels
    float max_value = (fluor_image.depth() == CV_8U) ? 255.0f : 65535.0f;
    cv::Mat saturated_mask = (fluor_image == max_value);
    int saturated_count = cv::countNonZero(saturated_mask & droplet_mask);
    int droplet_pixel_count = cv::countNonZero(droplet_mask);
    if (saturated_count > droplet_pixel_count * 0.1) {  // >10% saturated
        metrics.saturated_flag = true;
    }
    
    return metrics;
}
```

#### Global Background ROI (Fallback)
- User draws background ROI in region with no droplets.
- Compute: $\bar{I}_{bg}^{global} = \text{mean}(\text{background ROI})$.
- Apply to all droplets: $\bar{I}_{corr} = \bar{I}_{droplet} - \bar{I}_{bg}^{global}$.

---

## 6. Data Model and File Formats

### 6.1 Session JSON (With Provenance)

```json
{
  "session": {
    "product_version": "1.0.0",
    "spec_version": "1.0",
    "schema_version": "2024.1",
    "created": "2024-01-15T10:30:00Z",
    "software": {
      "name": "DropletAnalyzer",
      "version": "1.0.0",
      "git_commit": "a3f2e9c",
      "build_id": "2024-01-12T09:15:00Z"
    }
  },
  "module": {
    "name": "VideoTrackingModule"
  },
  "input": {
    "type": "image_sequence",
    "directory": "C:\\Data\\experiment_001\\",
    "file_pattern": "frame_*.tif",
    "total_files": 233,
    "fps_manual": 23.3,
    "first_file_hash_sha256": "e5f9c3a..."
  },
  "calibration": {
    "um_per_px": 0.58,
    "method": "manual"
  },
  "roi": {
    "enabled": true,
    "type": "rectangle",
    "x": 252,
    "y": 252,
    "width": 1800,
    "height": 1800
  },
  "parameters": {
    "mode": "auto_tuned",
    "auto_tune": {
      "stage1_time_s": 4.8,
      "stage2_time_s": 14.2,
      "quality_score": 32.5,
      "selected_params": {
        "gaussian_sigma": 1.0,
        "adaptive_block_size": 11,
        "adaptive_c": 2,
        "morph_open_kernel": 5,
        "morph_close_kernel": 5
      }
    }
  },
  "config_hash_sha256": "c6e9b4a..."
}
```

### 6.2 Per-Droplet CSV (Video, Wide Format)

**Column Definitions:**
- `timestamp_inferred_s`: Computed as `file_order_index / fps_manual` (where file_order_index is the position of the file in the sorted directory listing, starting at 0).
- `frame_index_logical`: Position in the sorted file list from the directory (0, 1, 2, 3, ...).
- `droplet_id`: Globally unique ID for each detection (never repeats: 1, 2, 3, ..., N).
- `track_id`: Unique ID for tracked droplet (persistent across frames; equals `droplet_id` if tracking fails).

**CSV Header (with calibration):**
```csv
# DropletAnalyzer v1.0.0 | Calibration: 0.58 um/px | FPS: 23.3 | Total frames: 233 | Resolution: 2304×2304 | Bit depth: 16-bit
timestamp_inferred_s,frame_index_logical,droplet_id,track_id,centroid_x_px,centroid_y_px,centroid_x_um,centroid_y_um,area_px2,area_um2,diameter_eq_um,major_axis_um,minor_axis_um,angle_deg,circularity,aspect_ratio,velocity_um_s,crossed_line
```

**Example Rows (files loaded in alphabetical order):**
```csv
0.0000,0,1,1,1150.2,1200.5,667.6,696.3,280.0,94.2,10.96,12.5,10.8,15.3,0.89,1.16,NaN,0
0.0429,1,2,1,1155.8,1205.1,670.4,699.0,282.0,94.9,11.00,12.6,10.9,16.1,0.88,1.15,1250.5,0
0.0858,2,3,1,1160.2,1210.0,672.8,701.8,285.0,95.9,11.06,12.7,11.0,15.8,0.89,1.15,1245.3,0
0.1287,3,4,2,1165.5,1214.8,675.8,704.6,284.5,95.7,11.05,12.6,10.9,15.5,0.89,1.16,1240.2,0
```

**Notes:**
- Timestamps are uniformly spaced (0.0000, 0.0429, 0.0858, 0.1287, ...) based on FPS (23.3 Hz) and file order position in directory.
- Velocity is NaN for the first detection of each track (cannot compute without prior frame).
- Files are loaded in alphabetical order; gaps in filenames are ignored.

### 6.3 Summary Statistics JSON

```json
{
  "summary": {
    "module": "VideoTrackingModule",
    "duration_s": 10.0,
    "total_frames": 233,
    "fps_actual": 23.3
  },
  "droplets": {
    "total_detected": 4500,
    "total_tracked": 2800,
    "frequency_hz": {
      "mean": 450.0,
      "std": 15.2
    },
    "diameter_um": {
      "mean": 10.96,
      "median": 10.94,
      "std": 0.42,
      "cv_percent": 3.83,
      "min": 9.50,
      "max": 12.80
    }
  }
}
```

---

## 7. GUI Requirements

### 7.1 Control Panel - Detection Tab (Expert Mode)

**Background Subtraction (Clarified for Image Modules):**

**Video Module:**
- [Dropdown: Background Method] (None, Static, Running)
- If Static: [Spinbox: Median Frames] (10–200, default 50)
- If Running: [Slider: Alpha] (0.01–0.2, default 0.05)

**Image Modules (Brightfield, Fluorescence):**
- [Dropdown: Background Method] (None, Static)
- **Default:** None (adaptive threshold only, robust to gradients).
- If Static (Expert Mode only):
  - [Button: Load Blank Image...] (user provides separate TIFF)
  - Use case: Correct severe vignetting or sensor artifacts.

### 7.2 Plot Downsampling Rules (Explicit)

**Time-Series Plots:**
- Max points: **300**.
- If incoming data >300 points: Uniform decimation (keep every Nth point).
- Update trigger: Only when new batch results available (not every 5 Hz timer tick if no new data).

**Histograms:**
- Max bins: **200** (adaptive bin width).
- Update: Every 1 sec OR every 100 new droplets (whichever is less frequent).

**Scatter Plots (Population Module):**
- Max points: **2000**.
- If >2000 droplets: Randomly sample 2000 for display (all data still in CSV).
- Update: Only when analysis completes (not streaming).

**Implementation (Pseudo-code):**
```cpp
void updateTimeSeries(const std::vector<DataPoint>& new_data) {
    all_data_.insert(all_data_.end(), new_data.begin(), new_data.end());
    
    if (all_data_.size() > 300) {
        // Decimate: keep every Nth point
        int N = all_data_.size() / 300;
        std::vector<DataPoint> decimated;
        for (size_t i = 0; i < all_data_.size(); i += N) {
            decimated.push_back(all_data_[i]);
        }
        plot->setData(decimated);
    } else {
        plot->setData(all_data_);
    }
}
```

---

## 8. Software Stack and Dependencies

### 8.1 Required Libraries

- **Qt 6.5+** (Core, Widgets, Gui, Concurrent)
  - License: LGPL v3.
- **OpenCV 4.8+** (core, imgproc, imgcodecs, videoio)
  - License: Apache 2.0.
  - **Note:** `video` module (MOG2 background subtraction) not needed; custom implementation used.
- **libtiff 4.5+**
  - License: BSD-like.
  - For robust TIFF I/O (multi-page, validation).
  - **BigTIFF support:** Disabled (not needed for v1.0.0).
  - CMake: Link against libtiff, ensure `-DBIG_TIFF=OFF` or use standard build.
- **Hamamatsu DCAM-API SDK 24.1+**
  - License: Proprietary.
  - **Not bundled in installer** (external prerequisite, see §8.3).
- **QCustomPlot 2.1+**
  - License: GPL v3 (source included in project).
- **GoogleTest 1.12+** (testing, BSD 3-Clause)
- **spdlog 1.10+** (logging, MIT)
- **nlohmann/json 3.11+** (JSON, MIT)

### 8.2 Removed Dependencies (v1.0.0)
- **NI-DAQmx:** Removed entirely from v1.0.0 (no link, no package). Will be added as optional runtime plugin in v1.1.

### 8.3 Licensing and Compliance

**Product License:** GPL v3
- **Reason:** QCustomPlot is GPL v3; viral license requires entire application to be GPL v3.
- **Implications:**
  - Source code available on GitHub under GPL v3.
  - Users can modify and redistribute under GPL v3.
  - Cannot be used in proprietary/commercial products without QCustomPlot commercial license.

**Proprietary Dependencies (External Prerequisites):**

**Hamamatsu DCAM-API SDK:**
- **License:** Proprietary (Hamamatsu).
- **Redistribution:** **Not permitted without written permission** (assumed; must verify EULA).
- **v1.0.0 Installer:** Does **not** bundle `dcamapi4.dll`.
- **User requirement:** Download and install DCAM-API SDK from Hamamatsu website before using Camera Control Module.
- **Fallback:** If DCAM-API not detected at launch, Camera Control Module is disabled (grayed out), offline modules remain functional.

**Detection at Launch:**
```cpp
// At startup, check for dcamapi4.dll
QLibrary dcam_lib("dcamapi4");
if (dcam_lib.load()) {
    // DCAM available, enable Camera Control Module
    ui->moduleCameraControl->setEnabled(true);
} else {
    // DCAM not found, disable module
    ui->moduleCameraControl->setEnabled(false);
    QMessageBox::information(this, "Camera Support Unavailable",
        "Hamamatsu DCAM-API not detected.\n\n"
        "Camera Control Module is disabled.\n"
        "Offline analysis modules remain available.\n\n"
        "To enable camera support:\n"
        "1. Download DCAM-API SDK from hamamatsu.com\n"
        "2. Install SDK\n"
        "3. Restart DropletAnalyzer");
}
```

**README.md (GitHub):**
```markdown
## License
DropletAnalyzer is licensed under **GPL v3**.

This program uses QCustomPlot (GPL v3), which requires the entire application to be GPL-licensed.

## Dependencies

### Open Source (Included)
- Qt 6 (LGPL v3, dynamically linked)
- OpenCV (Apache 2.0)
- libtiff (BSD-like)
- QCustomPlot (GPL v3, source included)

### Proprietary (External Prerequisites)
**Hamamatsu DCAM-API SDK** (required for Camera Control Module)
- License: Proprietary (Hamamatsu)
- Download: https://www.hamamatsu.com/[link]
- Installation: Required before using camera features
- Offline analysis modules work without DCAM-API

## Source Code
Available at: https://github.com/[your-org]/DropletAnalyzer
```

---

## 9. Performance Requirements (Revised - USB Bandwidth Limited, Realistic Targets)

### 9.1 Benchmark Videos (Two Tiers, USB 3.0 Limited)

#### Tier A: Typical Use Case (Primary Benchmark - USB Bandwidth Limited)
- **Resolution:** 2304×2304 px (full frame, USB 3.0 practical maximum).
- **Bit depth:** **16-bit grayscale** (standard acquisition).
- **FPS:** 23.3 FPS (USB 3.0 practical maximum, 1ms exposure).
- **Duration:** 10 seconds (233 frames).
- **Content:** 15–25 droplets/frame (typical microfluidic output).
- **ROI:** 1800×1800 px.
- **File size:** ~10.6 MB/frame, ~2.5 GB total.

#### Tier B: Reduced-Size Offline Case (Secondary Benchmark - Reference Only)
- **Resolution:** 1152×1152 px (2×2 binning or ROI).
- **Bit depth:** 8-bit grayscale.
- **FPS:** 100 FPS (user-entered, offline simulation).
- **Duration:** 5 seconds (500 frames).
- **ROI:** 900×900 px.
- **Content:** 15 droplets/frame.
- **File size:** ~1.3 MB/frame, ~6.5 GB total.
- **Note:** Tier B is for offline analysis reference only; not achievable in real-time at full resolution.

### 9.2 Performance Targets (Multi-Threaded, i9-13900HX, Decoupled from USB)

#### Tier A Targets (2304×2304, 16-bit, ROI 1800×1800, 233 frames)

| **Mode**                                      | **Target**    | **Measured Metric**                     |
|-----------------------------------------------|---------------|-----------------------------------------|
| Detection only (no tracking, no fluorescence) | ≥100 FPS      | Process 233 frames in ≤2.5 seconds      |
| Detection + Tracking                          | ≥80 FPS       | Process 233 frames in ≤3 seconds        |
| Detection + Tracking + Fluorescence (1 ch)    | ≥60 FPS       | Process 233 frames in ≤4 seconds        |

**Rationale:**
- 16-bit images (larger than 8-bit).
- Full frame resolution 2304×2304.
- Multi-threading (8 P-cores) provides 4–6× speedup.
- **Offline processing is decoupled from USB acquisition speed.**

#### Tier B Targets (1152×1152, 8-bit, ROI 900×900, 500 frames)

| **Mode**                                      | **Target**    | **Measured Metric**                     |
|-----------------------------------------------|---------------|-----------------------------------------|
| Detection only                                | ≥400 FPS      | Process 500 frames in ≤1.3 seconds      |
| Detection + Tracking                          | ≥300 FPS      | Process 500 frames in ≤1.7 seconds      |

#### Export Benchmark (Separate Pass, Not Included in Processing Time)
- **Task:** Render overlays on 233 frames (Tier A) + encode to MJPEG AVI (quality 85).
- **Target:** ≥50 FPS (complete in ≤5 seconds).
- **Note:** I/O and encoding bound, not algorithm bound.

### 9.3 Real-Time Camera Control Targets (v1.0.0)

| **Metric**                                    | **Target**              | **Acceptance**                              |
|-----------------------------------------------|-------------------------|---------------------------------------------|
| Full frame (2304×2304, 16-bit, 1ms exposure) | 23.3 FPS sustained      | 60 sec, zero DCAM drops, USB 3.0 compliant |
| UI canvas refresh                             | ~23 Hz (real-time)      | Display all frames as acquired              |
| Latency (grab to display)                     | <100 ms                 | Live preview acceptable                     |
| Expert Mode high-speed (reduced resolution)  | Up to 8938 FPS (2304×4) | Camera limit, not USB-dependent             |

**USB Bandwidth Constraints:**
- **Full resolution acquisition limited to ~23.3 FPS** by USB 3.0 practical throughput.
- **Expert Mode allows reduced resolution/bit depth for higher speeds** (camera capability, not typical use).
- **Bandwidth warnings** displayed if user attempts configurations exceeding USB limits.

### 9.4 Memory Usage Targets

| **Scenario**                          | **Peak Memory**       |
|---------------------------------------|-----------------------|
| Offline (Tier A, 233 frames)          | <2.5 GB               |
| Offline (Tier B, 500 frames)          | <1.5 GB               |
| Real-time (23.3 FPS, 60 sec)          | <800 MB               |

**Cache Budget:**
- Default: 512 MB (user-configurable: 128 MB – 2 GB).
- Adaptive frame count: $N_{cache} = \lfloor 512\text{MB} / \text{frame\_size} \rfloor$.
  - 2304×2304 16-bit (~10.6 MB/frame) → ~48 frames cached.
  - 1152×1152 8-bit (~1.3 MB/frame) → ~390 frames cached.

---

## 10. Testing and Validation

### 10.1 Unit Testing (GoogleTest)
- Coverage: 80% for core algorithms.
- Test categories: Geometry, filtering, tracking, fluorescence, auto-tune scoring, frame cache, TIFF validation.

### 10.2 Synthetic Image Testing
- 10 test scenes (S1–S10) validating detection accuracy, tracking continuity.

### 10.3 Real Data Validation

**User-Provided Datasets (Minimum 3):**
1. **Full-resolution USB-limited video (image sequence):** 2304×2304, 16-bit, 23.3 FPS, 10 sec (~233 frames), ~20 droplets/frame.
2. **Fluorescence multi-page TIFF:** Brightfield + GFP + RFP, ~50 droplets, 16-bit.
3. **Reduced-size offline reference:** 1152×1152, 8-bit, 100 FPS simulated, 5 sec, ~15 droplets/frame.

**Ground Truth Annotation (Built-In Tool):**
- Simple click-to-mark tool: User clicks droplet centroids on sampled frames.
- Export as JSON: `{"frame_1": [{"x": 550, "y": 620}, ...], ...}`
- External Python script computes precision/recall/MAE.

**Validation Metrics (Precise Definitions):**
- **Detection:**
  - Matching tolerance: 5 px Euclidean distance.
  - Precision = TP / (TP + FP), target ≥95%.
  - Recall = TP / (TP + FN), target ≥90%.
- **Diameter:**
  - MAE < 0.5 µm or < 5% of mean diameter (whichever is larger).
  - Only computed if ground truth includes diameter.
- **Frequency:**
  - Ground truth = manual count of line-crossings / duration.
  - Error < 5% (computed over ≥10 sec window).
- **Fluorescence:**
  - MAE < 5% or < 100 units (16-bit) vs ImageJ measurements.

### 10.4 Performance Benchmarking
- Automated benchmark tool (Tools → Run Benchmark).
- Runs Tier A and Tier B, displays results table (FPS, ms/frame, memory).

---

## 11. Development Roadmap (Revised - 16 Weeks)

### Phase 0: Risk Mitigation Spikes (Weeks 1–2)

**Spike 1: DCAM Camera Integration (Week 1)**
- **Goal:** Validate ORCA-Fusion connection, USB 3.0 bandwidth limits, stable acquisition at 23.3 FPS.
- **Tasks:**
  - Minimal app: Connect camera, grab frames, display in Qt window.
  - Test full-frame acquisition (2304×2304, 16-bit, 23.3 FPS).
  - Measure USB bandwidth utilization, confirm zero DCAM drops at 23.3 FPS.
  - Test high-speed reduced-resolution modes in Expert Mode (optional reference).
- **Deliverable:** Working `CameraSource` class, bandwidth documentation, lessons learned.

**Spike 2: libtiff Image Sequence Loader (Week 2)**
- **Goal:** Validate directory scanning, lazy loading, memory-efficient caching.
- **Tasks:**
  - Load directory with 233 full-resolution TIFFs (~2.5 GB).
  - Implement `ImageSequenceSource` with LRU cache (512 MB budget).
  - Test with varying file patterns (alphabetical sorting, no frame index parsing).
  - Measure: Load time, decode speed, cache hit rate.
- **Deliverable:** `ImageSequenceSource` prototype validated.

---

### Phase 1: Core Framework & Video Module (Weeks 3–7)
- **M1.1 (Week 3):** Project skeleton, CMake, module architecture (`IAnalysisModule`, `InputSource` abstraction).
- **M1.2 (Week 4):** Video module UI, image sequence loader (use Spike 2 code), detection pipeline (manual params).
- **M1.3 (Week 5):** Auto-Tune (two-stage search, refined scoring), Expert Mode with processing metrics.
- **M1.4 (Week 6):** Tracking (parallel detection + sequential tracking), measurement line, frequency calculation.
- **M1.5 (Week 7):** CSV/JSON export, session save/load, overlay customization tab, multi-threaded processing (`QtConcurrent`).

**Deliverable:** Video Tracking Module functional, Tier A performance targets met.

---

### Phase 2: Image Modules & Fluorescence (Weeks 8–10)
- **M2.1 (Week 8):** Brightfield Image Module (single TIFF, Auto-Tune for single image, detection, CSV export).
- **M2.2 (Week 9):** Fluorescence Image Module (intensity metrics, local annulus with edge case handling, saturation detection).
- **M2.3 (Week 10):** Population Module (multi-page TIFF, page assignments, multi-channel quantification, gating, histograms, scatter plot).

**Deliverable:** All offline modules functional.

---

### Phase 3: Real-Time Camera Control (Weeks 11–12)
- **M3.1 (Week 11):** Camera Control Module UI (use Spike 1 code), settings panel (capability-driven UI), USB bandwidth warnings.
- **M3.2 (Week 12):** Live feed display (23 Hz real-time), settings read-back validation, error handling (disconnect, buffer failure), 23.3 FPS sustained acquisition test with zero DCAM drops.

**Deliverable:** Camera Control Module functional (no processing, live preview only).

---

### Phase 4: Testing, Documentation, Release (Weeks 13–16)
- **M4.1 (Week 13–14):** Unit tests, synthetic tests, real-data validation (user datasets), ground truth annotation tool.
- **M4.2 (Week 14):** Performance benchmarking on i9-13900HX, USB bandwidth analysis.
- **M4.3 (Week 15):** User manual (70 pages), algorithm reference (30 pages), Doxygen docs, USB limitations guide.
- **M4.4 (Week 16):** NSIS installer, packaging, example datasets, final QA testing.

**Deliverable:** DropletAnalyzer v1.0.0 released on GitHub.

---

## 12. Deliverables

### 12.1 Software
- **Desktop GUI:** `DropletAnalyzer.exe` with NSIS installer (includes Qt, OpenCV, libtiff DLLs).
- **Installer does NOT include:** DCAM-API, NIDAQ-API (user downloads separately).

### 12.2 Documentation
1. **User Manual** (PDF + HTML, ~70 pages):
   - Installation (including DCAM-API setup).
   - Module-by-module workflows.
   - Auto-Tune vs Expert Mode.
   - Camera control guide (USB bandwidth limitations, real-time acquisition).
   - Troubleshooting, FAQ.
2. **Algorithm Reference** (PDF, ~30 pages):
   - Detection pipeline, auto-tune algorithm, fluorescence quantification.
   - USB bandwidth constraints and implications.
3. **Developer Documentation** (Doxygen HTML):
   - API reference, module architecture, build instructions.

### 12.3 Example Data
- Benchmark videos (Tier A: 2304×2304 23.3 FPS, Tier B: 1152×1152 100 FPS reference).
- Annotated video with ground truth.
- Multi-page TIFF (brightfield + GFP + RFP).
- Session files for each module.

---

## 13. Open Questions
1. **Branding:** Final logo, color scheme, splash screen design.
2. **DCAM redistribution:** Confirm with Hamamatsu if bundling is permitted (if yes, can simplify installer).
3. **USB cable specifications:** Should documentation recommend USB 3.0 Gen 2 cables or specific vendors for best performance?

---

## 14. Success Criteria (v1.0.0)

1. **Functional:** All 4 offline modules + Camera Control Module work as specified.
2. **Performance (Offline):** Tier A ≥80 FPS (detection + tracking), 233 frames in ≤3 seconds.
3. **Performance (Real-Time):** 23.3 FPS sustained full-frame acquisition with zero DCAM drops.
4. **Validation:** Precision ≥95%, recall ≥90%, diameter MAE <0.5 µm, frequency error <5%.
5. **Stability:** No crashes in 10 hours, graceful error handling.
6. **USB Compliance:** Achieves 23.3 FPS full-frame on standard USB 3.0 connections.
7. **Documentation:** User manual complete with USB limitation guidance, algorithm reference complete.
8. **Installer:** Works on clean Windows 11, prompts for DCAM-API if missing.

---

**End of Technical Specification - Product v1.0.0, Spec v1.0 (IMPLEMENTATION READY)**
```