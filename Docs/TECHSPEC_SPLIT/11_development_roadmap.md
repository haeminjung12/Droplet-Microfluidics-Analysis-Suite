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
