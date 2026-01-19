# Phase 0.2: DCAM Connectivity & Streaming Test (Using Existing Assets)

This repo does not reimplement camera control yet. This task verifies camera connectivity and stable streaming using the existing `qt_hama_gui` assets.

Source repo (local path): `C:\Users\goals\Codex\CNN for Droplet Sorting\cpp_pipeline_github\qt_hama_gui`

## Goal / Acceptance (from `Docs/TECHSPEC.md`)

- Connect to Hamamatsu ORCA-Fusion via DCAM.
- Stream raw frames at **23.3 FPS** (full frame **2304×2304**, **16-bit**, **1 ms exposure**) for **60 seconds**.
- Zero DCAM-reported dropped frames (`DCAMERR_LOSTFRAME` does not occur).

## Prereqs

- Camera connected (USB 3.0) and powered on.
- Hamamatsu DCAM SDK + runtime installed (so `dcamapi.dll` resolves at runtime).
- Qt 6 (MSVC 2022 build), OpenCV via vcpkg, ONNX Runtime (required by `qt_hama_gui`’s CMake).

Known local paths (adjust if different):
- Qt: `C:\Qt\6.10.1\msvc2022_64`
- vcpkg: `C:\vcpkg`
- ONNX Runtime: `C:\onnxruntime-gpu`
- DCAM SDK headers/libs:
  - `C:\Users\goals\Codex\CNN for Droplet Sorting\python_pipeline\Hamamatsu_DCAMSDK4_v25056964\dcamsdk4\inc`
  - `C:\Users\goals\Codex\CNN for Droplet Sorting\python_pipeline\Hamamatsu_DCAMSDK4_v25056964\dcamsdk4\lib\win64`

## Build (CMake configure + MSBuild build)

From PowerShell:

```powershell
$qtHamaGui = "C:\Users\goals\Codex\CNN for Droplet Sorting\cpp_pipeline_github\qt_hama_gui"
$buildDir  = Join-Path $qtHamaGui "build"
$qtPrefix  = "C:\Qt\6.10.1\msvc2022_64"
$onnxDir   = "C:\onnxruntime-gpu"

cmake -S $qtHamaGui -B $buildDir -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_PREFIX_PATH=$qtPrefix `
  -DONNXRUNTIME_DIR=$onnxDir

& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
  (Join-Path $buildDir "qt_hama_gui.sln") /m /p:Configuration=Release
```

Expected output: `droplet_pipeline.exe` under `...\qt_hama_gui\build\Release\`.

## Enable `DCAMERR_LOSTFRAME` logging (recommended)

`qt_hama_gui` currently does not surface `DCAMERR_LOSTFRAME` clearly in the UI/logs. Apply the optional patch below to log every `DCAMERR_LOSTFRAME` occurrence to `session_log.txt`.

- Patch file in this repo: `Docs/patches/qt_hama_gui_log_lostframe.patch`
- Apply in the `qt_hama_gui` repo:

```powershell
cd "C:\Users\goals\Codex\CNN for Droplet Sorting\cpp_pipeline_github\qt_hama_gui"
git apply "<path-to-this-repo>\\Docs\\patches\\qt_hama_gui_log_lostframe.patch"
```

Replace `<path-to-this-repo>` with the workspace path that contains this file (run `pwd` from the repo root).

Rebuild after applying.

## Run + Verify (manual)

1. Launch `droplet_pipeline.exe`.
2. Select preset `2304 x 2304`, bit depth `16`, exposure `1.000 ms` (or as close as the camera allows).
3. Click `Start`.
4. Let it run for **60 seconds**.
5. Verify:
   - UI shows FPS ~ **23.3** (Cam FPS close to 23.3).
   - With the optional patch applied: `session_log.txt` contains **zero** `DCAMERR_LOSTFRAME` lines.

Log check:

```powershell
$log = "...\qt_hama_gui\build\Release\session_log.txt"
Select-String -Path $log -Pattern "DCAMERR_LOSTFRAME"
```
