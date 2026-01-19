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
