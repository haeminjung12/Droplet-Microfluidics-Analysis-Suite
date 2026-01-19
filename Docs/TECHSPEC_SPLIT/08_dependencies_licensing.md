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
