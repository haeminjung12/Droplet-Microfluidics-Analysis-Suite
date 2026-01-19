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
