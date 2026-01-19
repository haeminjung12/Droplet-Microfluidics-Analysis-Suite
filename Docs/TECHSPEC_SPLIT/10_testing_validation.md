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
