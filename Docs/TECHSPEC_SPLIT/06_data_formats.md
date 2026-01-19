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
