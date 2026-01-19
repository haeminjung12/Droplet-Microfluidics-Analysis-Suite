## 12. Deliverables

### 12.1 Software
- **Desktop GUI:** `DropletAnalyzer.exe` with NSIS installer (includes Qt, OpenCV, libtiff DLLs).
- **Installer does NOT include:** DCAM-API, NIDAQ-API (user downloads separately).

### 12.2 Documentation
1. **User Manual** (PDF + HTML, ~70 pages):
   - Installation (including DCAM-API setup).
   - Module-by-module workflows.
   - Auto-Tune vs Expert Mode.
   - Camera control guide (USB bandwidth limitations, real-time acquisition).
   - Troubleshooting, FAQ.
2. **Algorithm Reference** (PDF, ~30 pages):
   - Detection pipeline, auto-tune algorithm, fluorescence quantification.
   - USB bandwidth constraints and implications.
3. **Developer Documentation** (Doxygen HTML):
   - API reference, module architecture, build instructions.

### 12.3 Example Data
- Benchmark videos (Tier A: 2304×2304 23.3 FPS, Tier B: 1152×1152 100 FPS reference).
- Annotated video with ground truth.
- Multi-page TIFF (brightfield + GFP + RFP).
- Session files for each module.

---
