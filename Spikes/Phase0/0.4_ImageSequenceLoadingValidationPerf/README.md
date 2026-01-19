# Phase 0.4 – Image Sequence Loading & Validation Performance (Prototype)

This spike prototype scans a directory of TIFF files, sorts filenames alphabetically (no frame index parsing), samples every Nth file for format consistency checks, and reports timing plus a simulated LRU cache hit rate.

## Run

From repo root:

```powershell
& ".\Spikes\Phase0\0.4_ImageSequenceLoadingValidationPerf\Invoke-ImageSequencePerfTest.ps1" -Directory "C:\path\to\tiff_sequence"
```

Optional parameters:

- `-SampleStride 10` (default) – sample indices `0, 10, 20, ...`
- `-MaxSamples 0` (default) – cap sampled files (useful for very large sequences)
- `-CacheSize 64` – simulated LRU capacity (frames)
- `-CacheSimRepeatLastN 50` and `-CacheSimLoops 10` – cache access pattern controls

## Self-test (no TIFFs required)

```powershell
& ".\Spikes\Phase0\0.4_ImageSequenceLoadingValidationPerf\SelfTest.ps1"
```
