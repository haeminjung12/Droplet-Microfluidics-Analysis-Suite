# Camera Asset Inventory: `qt_hama_gui`

Source repo (local path): `C:\Users\goals\Codex\CNN for Droplet Sorting\cpp_pipeline_github\qt_hama_gui`

## Reusable camera components

### Core acquisition + settings (DCAM)
- `dcam_controller.h` / `dcam_controller.cpp`
  - Lifecycle: `dcamapi_init` → `dcamdev_open` → `dcamwait_open` → `dcambuf_alloc` → `dcamcap_start` / `dcamcap_stop` → cleanup/uninit.
  - Settings application via DCAM properties:
    - ROI/subarray (`DCAM_IDPROP_SUBARRAY*`)
    - Binning (`DCAM_IDPROP_BINNING*`, incl. independent H/V)
    - Pixel type + bit depth (`DCAM_IDPROP_IMAGE_PIXELTYPE`, `DCAM_IDPROP_BITSPERCHANNEL`)
    - Exposure (`DCAM_IDPROP_EXPOSURETIME`)
    - Readout speed (`DCAM_IDPROP_READOUTSPEED`)
    - Optional frame bundle (`DCAM_IDPROP_FRAMEBUNDLE_*`)
  - Frame access: `dcambuf_lockframe(iFrame=-1)` and conversion to `QImage` (mono8 direct; mono16 converted for display).

### Frame delivery (threading + UI decoupling)
- `frame_grabber.h` / `frame_grabber.cpp`
  - `QThread` loop waits on `DCAMWAIT_CAPEVENT_FRAMEREADY` (`dcamwait_start`) then pulls the latest frame via `DcamController`.
  - Emits `frameReady(QImage, FrameMeta, fps)` with a throttled UI update rate and `QImage::copy()` to avoid holding camera buffers on the UI thread.
  - Optional `recordHook` callback for recording/capture pipelines.

### Shared types
- `frame_types.h`
  - `ApplySettings`: width/height, binning, bits/pixel type, exposure, readout speed, subarray enable, frame-bundle options.
  - `FrameMeta`: width/height/bits/binning, delivered count, derived FPS, readout speed.

## UI patterns worth reusing (Camera Control Module)

- `main.cpp`
  - Settings panel patterns: resolution presets + custom size, binning (incl. independent H/V), bit depth selection, exposure (ms) with property min/max query, readout speed combo.
  - Read-back logging pattern (requested vs actual via `dcamprop_getvalue`).
  - Live view widget: zoom/pan + scrollbars; display LUT/range adaptation by bit depth.
  - Stats overlay: resolution/binning/bits/FPS, delivered/dropped, readout speed.
  - Capture/recording patterns: single-frame save and sequence recording; metadata text export (e.g., `capture_info.txt`), session logging (`session_log.txt`).
  - Robustness patterns: “viewer-only” startup mode if camera init fails; crash logging/minidump setup (Windows + `DbgHelp`).

## Build/runtime dependencies (camera-relevant)

- Qt: `Qt6::Widgets` (also uses `QtCore`, `QtGui`).
- Hamamatsu DCAM SDK:
  - Headers: `dcamapi4.h`, `dcamprop.h` (include path `${DCAM_SDK_DIR}/inc`)
  - Libs: `dcamapi` (link path `${DCAM_SDK_DIR}/lib/win64`)
  - Runtime: system-installed DCAM runtime DLLs (not bundled by default in `qt_hama_gui` packaging).

## Additional dependencies in `qt_hama_gui` (pipeline tab; not required for v1.0 Camera Control)

- OpenCV (via vcpkg): `core`, `imgproc`, `imgcodecs`
- ONNX Runtime (required in that repo’s CMake)
- NI-DAQmx (optional; compile-time gated)

## Related (parent repo) files referenced by `qt_hama_gui/CMakeLists.txt`

These are pulled into the `droplet_pipeline` GUI build in `cpp_pipeline_github`, but are not required for a camera-only module:
- `..\dcam_camera.*`: additional DCAM camera wrapper used by the pipeline.
- `..\cli_runner.*`: headless runner integration.
- `..\event_detector.*`, `..\fast_event_detector.*`, `..\onnx_classifier.*`, `..\metadata_loader.*`: inference pipeline.
- `..\daq_trigger.*`: NI-DAQ trigger support (optional).

