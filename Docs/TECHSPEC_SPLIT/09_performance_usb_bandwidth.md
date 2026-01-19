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
