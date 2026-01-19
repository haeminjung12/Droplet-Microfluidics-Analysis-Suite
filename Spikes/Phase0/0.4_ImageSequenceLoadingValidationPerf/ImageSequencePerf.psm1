$ErrorActionPreference = 'Stop'

function Get-ImageSequenceScanResult {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)][string] $Directory,
        [Parameter()][ValidateRange(1, [int]::MaxValue)][int] $SampleStride = 10,
        [Parameter()][scriptblock] $MetadataProvider
    )

    if (-not (Test-Path -LiteralPath $Directory -PathType Container)) {
        throw "Directory not found: $Directory"
    }

    $files =
        Get-ChildItem -LiteralPath $Directory -File |
        Where-Object {
            $ext = $_.Extension
            $ext -and ($ext.Equals('.tif', 'InvariantCultureIgnoreCase') -or $ext.Equals('.tiff', 'InvariantCultureIgnoreCase'))
        } |
        Sort-Object -Property Name

    $orderedFiles = $files | ForEach-Object { $_.FullName }

    $sampledIndices = New-Object System.Collections.Generic.List[int]
    $sampledFiles = New-Object System.Collections.Generic.List[string]
    $sampledMetadata = New-Object System.Collections.Generic.List[object]
    $formatMismatches = New-Object System.Collections.Generic.List[object]
    $baselineMetadata = $null

    for ($i = 0; $i -lt $orderedFiles.Count; $i += 1) {
        if (($i % $SampleStride) -ne 0) {
            continue
        }

        $sampledIndices.Add($i)
        $sampledFiles.Add($orderedFiles[$i])

        if ($MetadataProvider) {
            $metadata = $null
            $metadataError = $null
            try {
                $metadata = & $MetadataProvider $orderedFiles[$i]
            }
            catch {
                $metadataError = $_.Exception.Message
            }
            $sampledMetadata.Add($metadata)

            if ($null -eq $metadata) {
                $formatMismatches.Add([pscustomobject]@{
                        Index       = $i
                        Path        = $orderedFiles[$i]
                        Differences = @{}
                        Error       = $metadataError
                    })
                continue
            }

            if ($null -eq $baselineMetadata) {
                $baselineMetadata = $metadata
            }
            else {
                $diffs = @{}
                foreach ($field in @('WidthPx', 'HeightPx', 'BitDepth', 'Format')) {
                    $baselineValue = $baselineMetadata.$field
                    $value = $metadata.$field
                    if ($baselineValue -ne $value) {
                        $diffs[$field] = [pscustomobject]@{ Baseline = $baselineValue; Actual = $value }
                    }
                }

                if ($diffs.Count -gt 0) {
                    $formatMismatches.Add([pscustomobject]@{
                            Index       = $i
                            Path        = $orderedFiles[$i]
                            Differences = $diffs
                            Error       = $null
                        })
                }
            }
        }
    }

    $formatConsistency = [pscustomobject]@{
        IsConsistent = ($formatMismatches.Count -eq 0)
        Baseline     = $baselineMetadata
        Mismatches   = $formatMismatches.ToArray()
    }

    return [pscustomobject]@{
        OrderedFiles    = $orderedFiles
        SampledIndices  = $sampledIndices.ToArray()
        SampledFiles    = $sampledFiles.ToArray()
        SampledMetadata = $sampledMetadata.ToArray()
        FormatConsistency = $formatConsistency
    }
}

Export-ModuleMember -Function Get-ImageSequenceScanResult
