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
