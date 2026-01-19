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
