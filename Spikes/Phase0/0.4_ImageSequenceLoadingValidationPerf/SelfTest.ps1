$ErrorActionPreference = 'Stop'

Import-Module "$PSScriptRoot/ImageSequencePerf.psm1" -Force

function Assert-Equal {
    param(
        [Parameter(Mandatory)] $Expected,
        [Parameter(Mandatory)] $Actual,
        [Parameter(Mandatory)][string] $Message
    )

    if ($Expected -is [System.Array] -or $Actual -is [System.Array]) {
        $expectedText = ($Expected | ForEach-Object { "$_" }) -join "`n"
        $actualText = ($Actual | ForEach-Object { "$_" }) -join "`n"
        if ($expectedText -ne $actualText) {
            throw "$Message`nExpected:`n$expectedText`nActual:`n$actualText"
        }
        return
    }

    if ($Expected -ne $Actual) {
        throw "$Message`nExpected: $Expected`nActual: $Actual"
    }
}

function New-TempDirectory {
    $path = Join-Path $env:TEMP ([System.Guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $path | Out-Null
    return $path
}

function New-EmptyFile {
    param([Parameter(Mandatory)][string] $Path)
    New-Item -ItemType File -Path $Path | Out-Null
}

$metadataProvider = {
    param([Parameter(Mandatory)][string] $Path)
    return [pscustomobject]@{
        WidthPx  = 2304
        HeightPx = 2304
        BitDepth = 16
        Format   = 'tiff'
    }
}

Write-Host "SelfTest: fast TIFF metadata parsing..."
$dir0 = New-TempDirectory
try {
    $tiffPath = Join-Path $dir0 'synthetic.tif'
    $fs = [System.IO.File]::Open($tiffPath, [System.IO.FileMode]::CreateNew, [System.IO.FileAccess]::Write, [System.IO.FileShare]::None)
    try {
        $bw = New-Object System.IO.BinaryWriter($fs)
        try {
            # Minimal little-endian TIFF header + one IFD with width/height/bits-per-sample.
            # Header: "II" 42, IFD offset = 8
            $bw.Write([byte][char]'I')
            $bw.Write([byte][char]'I')
            $bw.Write([UInt16]42)
            $bw.Write([UInt32]8)

            # IFD at offset 8: 3 entries
            $bw.Write([UInt16]3)

            # Tag 256 (ImageWidth), Type 4 (LONG), Count 1, Value 2304
            $bw.Write([UInt16]256)
            $bw.Write([UInt16]4)
            $bw.Write([UInt32]1)
            $bw.Write([UInt32]2304)

            # Tag 257 (ImageLength), Type 4 (LONG), Count 1, Value 2304
            $bw.Write([UInt16]257)
            $bw.Write([UInt16]4)
            $bw.Write([UInt32]1)
            $bw.Write([UInt32]2304)

            # Tag 258 (BitsPerSample), Type 3 (SHORT), Count 1, Value 16 (stored in low 2 bytes of the 4-byte value field)
            $bw.Write([UInt16]258)
            $bw.Write([UInt16]3)
            $bw.Write([UInt32]1)
            $bw.Write([UInt16]16)
            $bw.Write([UInt16]0)

            # Next IFD offset = 0
            $bw.Write([UInt32]0)
        }
        finally {
            $bw.Dispose()
        }
    }
    finally {
        $fs.Dispose()
    }

    $meta = Get-TiffMetadataFast -Path $tiffPath
    Assert-Equal 2304 $meta.WidthPx "Fast TIFF parser should read ImageWidth."
    Assert-Equal 2304 $meta.HeightPx "Fast TIFF parser should read ImageLength."
    Assert-Equal 16 $meta.BitDepth "Fast TIFF parser should read BitsPerSample."
}
finally {
    Remove-Item -Recurse -Force $dir0
}

Write-Host "SelfTest: alphabetical ordering..."
$dir1 = New-TempDirectory
try {
    New-EmptyFile (Join-Path $dir1 'img_10.tif')
    New-EmptyFile (Join-Path $dir1 'img_2.tif')
    New-EmptyFile (Join-Path $dir1 'img_1.tif')

    $result = Get-ImageSequenceScanResult -Directory $dir1 -SampleStride 10 -MetadataProvider $metadataProvider
    $orderedNames = $result.OrderedFiles | ForEach-Object { [System.IO.Path]::GetFileName($_) }

    Assert-Equal @('img_1.tif', 'img_10.tif', 'img_2.tif') $orderedNames "Should sort files alphabetically by filename."
}
finally {
    Remove-Item -Recurse -Force $dir1
}

Write-Host "SelfTest: sampling every 10th file..."
$dir2 = New-TempDirectory
try {
    0..22 | ForEach-Object {
        New-EmptyFile (Join-Path $dir2 ("img_{0:D2}.tif" -f $_))
    }

    $result = Get-ImageSequenceScanResult -Directory $dir2 -SampleStride 10 -MetadataProvider $metadataProvider
    Assert-Equal @(0, 10, 20) $result.SampledIndices "Should sample indices 0, 10, 20 for 23 frames with stride=10."
}
finally {
    Remove-Item -Recurse -Force $dir2
}

Write-Host "SelfTest: format consistency sampling..."
$dir3 = New-TempDirectory
try {
    0..25 | ForEach-Object {
        New-EmptyFile (Join-Path $dir3 ("img_{0:D2}.tif" -f $_))
    }

    $metadataProviderWithMismatch = {
        param([Parameter(Mandatory)][string] $Path)
        $name = [System.IO.Path]::GetFileNameWithoutExtension($Path)
        $idxText = $name.Substring($name.Length - 2, 2)
        $idx = [int]$idxText
        $width = if ($idx -eq 10) { 2000 } else { 2304 }
        return [pscustomobject]@{
            WidthPx  = $width
            HeightPx = 2304
            BitDepth = 16
            Format   = 'tiff'
        }
    }

    $result = Get-ImageSequenceScanResult -Directory $dir3 -SampleStride 10 -MetadataProvider $metadataProviderWithMismatch
    Assert-Equal $false $result.FormatConsistency.IsConsistent "Should report inconsistency when sampled TIFF metadata differs."
    Assert-Equal 1 $result.FormatConsistency.Mismatches.Count "Should report exactly one mismatch for the sampled index 10."
}
finally {
    Remove-Item -Recurse -Force $dir3
}

Write-Host "SelfTest: PASS"
