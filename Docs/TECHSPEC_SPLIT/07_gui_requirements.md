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
