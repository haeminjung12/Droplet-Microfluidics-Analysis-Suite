param(
    [Parameter(Mandatory)][string] $Directory,
    [Parameter()][ValidateRange(1, [int]::MaxValue)][int] $SampleStride = 10,
    [Parameter()][ValidateRange(0, [int]::MaxValue)][int] $MaxSamples = 0,
    [Parameter()][ValidateRange(1, [int]::MaxValue)][int] $CacheSize = 64,
    [Parameter()][ValidateRange(0, [int]::MaxValue)][int] $CacheSimRepeatLastN = 50,
    [Parameter()][ValidateRange(0, [int]::MaxValue)][int] $CacheSimLoops = 10
)

$ErrorActionPreference = 'Stop'

Import-Module "$PSScriptRoot/ImageSequencePerf.psm1" -Force

function Get-SimulatedLruCacheHitRate {
    param(
        [Parameter(Mandatory)][int] $FrameCount,
        [Parameter(Mandatory)][int] $Capacity,
        [Parameter(Mandatory)][int[]] $AccessPattern
    )

    $cache = New-Object System.Collections.Generic.List[int]
    $hits = 0
    $misses = 0

    foreach ($id in $AccessPattern) {
        $existingIndex = $cache.IndexOf($id)
        if ($existingIndex -ge 0) {
            $hits += 1
            $cache.RemoveAt($existingIndex)
            $cache.Insert(0, $id)
            continue
        }

        $misses += 1
        $cache.Insert(0, $id)
        if ($cache.Count -gt $Capacity) {
            $cache.RemoveAt($cache.Count - 1)
        }
    }

    $total = $hits + $misses
    $hitRate = if ($total -eq 0) { 0.0 } else { [double]$hits / [double]$total }
    return [pscustomobject]@{
        FrameCount = $FrameCount
        Capacity  = $Capacity
        Hits      = $hits
        Misses    = $misses
        Total     = $total
        HitRate   = $hitRate
    }
}

$metadataProvider = { param([string] $Path) Get-TiffMetadataFast -Path $Path }

$scanStopwatch = [System.Diagnostics.Stopwatch]::StartNew()
$scanOnly = Get-ImageSequenceScanResult -Directory $Directory -SampleStride $SampleStride -MaxSamples 1
$scanStopwatch.Stop()

$sampleStopwatch = [System.Diagnostics.Stopwatch]::StartNew()
$scan = Get-ImageSequenceScanResult -Directory $Directory -SampleStride $SampleStride -MaxSamples $MaxSamples -MetadataProvider $metadataProvider
$sampleStopwatch.Stop()

$totalSeconds = $scanStopwatch.Elapsed.TotalSeconds + $sampleStopwatch.Elapsed.TotalSeconds

$frameCount = $scan.OrderedFiles.Count
$sampleCount = $scan.SampledFiles.Count
$mismatchCount = $scan.FormatConsistency.Mismatches.Count

Write-Host "ImageSequenceSource prototype (Phase 0.4)"
Write-Host "Directory: $Directory"
Write-Host "Frames found: $frameCount"
Write-Host "Sample stride: $SampleStride (sampled $sampleCount)"
Write-Host "Max samples: $MaxSamples (0 = unlimited)"
Write-Host ("Scan+sort time (approx): {0:N3} s" -f ($scanStopwatch.Elapsed.TotalSeconds))
Write-Host ("Sample+metadata time: {0:N3} s" -f ($sampleStopwatch.Elapsed.TotalSeconds))
Write-Host ("Total time: {0:N3} s" -f $totalSeconds)
Write-Host "Format consistency: $([bool]$scan.FormatConsistency.IsConsistent) (mismatches: $mismatchCount)"

if ($mismatchCount -gt 0) {
    Write-Host "First mismatch:"
    $first = $scan.FormatConsistency.Mismatches | Select-Object -First 1
    Write-Host "  Index: $($first.Index)"
    Write-Host "  Path:  $($first.Path)"
    if ($first.Error) {
        Write-Host "  Error: $($first.Error)"
    }
    else {
        Write-Host "  Diffs: $($first.Differences.Keys -join ', ')"
    }
}

if ($frameCount -gt 0) {
    $accessPattern = New-Object System.Collections.Generic.List[int]
    for ($i = 0; $i -lt $frameCount; $i += 1) {
        $accessPattern.Add($i)
    }
    $repeatStart = [Math]::Max(0, $frameCount - $CacheSimRepeatLastN)
    for ($loop = 0; $loop -lt $CacheSimLoops; $loop += 1) {
        for ($i = $repeatStart; $i -lt $frameCount; $i += 1) {
            $accessPattern.Add($i)
        }
    }

    $cacheSim = Get-SimulatedLruCacheHitRate -FrameCount $frameCount -Capacity $CacheSize -AccessPattern $accessPattern.ToArray()
    Write-Host ("Simulated LRU cache hit rate (capacity {0}, repeat last {1} for {2} loops): {3:P2} ({4} hits / {5} total)" -f $CacheSize, $CacheSimRepeatLastN, $CacheSimLoops, $cacheSim.HitRate, $cacheSim.Hits, $cacheSim.Total)
}

[pscustomobject]@{
    ScanResult = $scan
    Seconds    = $totalSeconds
}
