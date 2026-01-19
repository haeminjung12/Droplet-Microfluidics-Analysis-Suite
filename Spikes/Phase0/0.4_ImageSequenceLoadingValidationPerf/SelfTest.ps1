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
