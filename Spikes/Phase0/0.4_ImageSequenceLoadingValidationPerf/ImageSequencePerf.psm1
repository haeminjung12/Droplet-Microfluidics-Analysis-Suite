$ErrorActionPreference = 'Stop'

function Get-TiffMetadataFast {
    [CmdletBinding()]
    param([Parameter(Mandatory)][string] $Path)

    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "File not found: $Path"
    }

    $fs = [System.IO.File]::Open($Path, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::ReadWrite)
    try {
        $br = New-Object System.IO.BinaryReader($fs)
        try {
            $b0 = $br.ReadByte()
            $b1 = $br.ReadByte()

            $isLittleEndian = $false
            if ($b0 -eq [byte][char]'I' -and $b1 -eq [byte][char]'I') {
                $isLittleEndian = $true
            }
            elseif ($b0 -eq [byte][char]'M' -and $b1 -eq [byte][char]'M') {
                $isLittleEndian = $false
            }
            else {
                throw "Not a TIFF header (expected 'II' or 'MM')."
            }

            function Read-U16 {
                param([Parameter(Mandatory)][System.IO.BinaryReader] $Reader, [Parameter(Mandatory)][bool] $LittleEndian)
                $bytes = $Reader.ReadBytes(2)
                if ($bytes.Length -ne 2) { throw "Unexpected EOF while reading UInt16." }
                if (-not $LittleEndian) { [array]::Reverse($bytes) }
                return [System.BitConverter]::ToUInt16($bytes, 0)
            }

            function Read-U32 {
                param([Parameter(Mandatory)][System.IO.BinaryReader] $Reader, [Parameter(Mandatory)][bool] $LittleEndian)
                $bytes = $Reader.ReadBytes(4)
                if ($bytes.Length -ne 4) { throw "Unexpected EOF while reading UInt32." }
                if (-not $LittleEndian) { [array]::Reverse($bytes) }
                return [System.BitConverter]::ToUInt32($bytes, 0)
            }

            $magic = Read-U16 -Reader $br -LittleEndian $isLittleEndian
            if ($magic -ne 42) {
                if ($magic -eq 43) {
                    throw "BigTIFF (43) is not supported by this fast parser."
                }
                throw "Invalid TIFF magic number: $magic"
            }

            $ifdOffset = Read-U32 -Reader $br -LittleEndian $isLittleEndian
            if ($ifdOffset -lt 8) {
                throw "Invalid IFD offset: $ifdOffset"
            }

            $fs.Seek([int64]$ifdOffset, [System.IO.SeekOrigin]::Begin) | Out-Null
            $entryCount = Read-U16 -Reader $br -LittleEndian $isLittleEndian

            $width = $null
            $height = $null
            $bitsPerSample = $null

            function Get-TypeSizeBytes {
                param([Parameter(Mandatory)][UInt16] $Type)
                switch ($Type) {
                    1 { return 1 }  # BYTE
                    2 { return 1 }  # ASCII
                    3 { return 2 }  # SHORT
                    4 { return 4 }  # LONG
                    5 { return 8 }  # RATIONAL
                    default { return 0 }
                }
            }

            for ($i = 0; $i -lt $entryCount; $i += 1) {
                $tag = Read-U16 -Reader $br -LittleEndian $isLittleEndian
                $type = Read-U16 -Reader $br -LittleEndian $isLittleEndian
                $count = Read-U32 -Reader $br -LittleEndian $isLittleEndian
                $valueOrOffset = Read-U32 -Reader $br -LittleEndian $isLittleEndian

                $typeSize = Get-TypeSizeBytes -Type $type
                if ($typeSize -eq 0) { continue }

                $valueBytesCount = [uint64]$typeSize * [uint64]$count

                function Read-ValueInlineU16 {
                    param([Parameter(Mandatory)][UInt32] $ValueField, [Parameter(Mandatory)][bool] $LittleEndian)
                    $bytes = [System.BitConverter]::GetBytes($ValueField)
                    if (-not $LittleEndian) { [array]::Reverse($bytes) }
                    return [System.BitConverter]::ToUInt16($bytes, 0)
                }

                function Read-ValueInlineU32 {
                    param([Parameter(Mandatory)][UInt32] $ValueField)
                    return $ValueField
                }

                if ($tag -eq 256 -and $width -eq $null) {
                    if ($count -eq 1) {
                        if ($type -eq 4) {
                            $width = Read-ValueInlineU32 -ValueField $valueOrOffset
                        }
                        elseif ($type -eq 3) {
                            $width = [uint32](Read-ValueInlineU16 -ValueField $valueOrOffset -LittleEndian $isLittleEndian)
                        }
                    }
                }
                elseif ($tag -eq 257 -and $height -eq $null) {
                    if ($count -eq 1) {
                        if ($type -eq 4) {
                            $height = Read-ValueInlineU32 -ValueField $valueOrOffset
                        }
                        elseif ($type -eq 3) {
                            $height = [uint32](Read-ValueInlineU16 -ValueField $valueOrOffset -LittleEndian $isLittleEndian)
                        }
                    }
                }
                elseif ($tag -eq 258 -and $bitsPerSample -eq $null) {
                    if ($type -eq 3 -and $count -eq 1) {
                        $bitsPerSample = Read-ValueInlineU16 -ValueField $valueOrOffset -LittleEndian $isLittleEndian
                    }
                    elseif ($type -eq 3 -and $count -ge 1 -and $valueBytesCount -gt 4) {
                        $pos = $fs.Position
                        try {
                            $fs.Seek([int64]$valueOrOffset, [System.IO.SeekOrigin]::Begin) | Out-Null
                            $first = Read-U16 -Reader $br -LittleEndian $isLittleEndian
                            $bitsPerSample = $first
                        }
                        finally {
                            $fs.Seek($pos, [System.IO.SeekOrigin]::Begin) | Out-Null
                        }
                    }
                }

                if ($width -ne $null -and $height -ne $null -and $bitsPerSample -ne $null) {
                    break
                }
            }

            if ($width -eq $null -or $height -eq $null -or $bitsPerSample -eq $null) {
                throw "Missing required TIFF tags (width/height/bits-per-sample)."
            }

            return [pscustomobject]@{
                WidthPx  = [int]$width
                HeightPx = [int]$height
                BitDepth = [int]$bitsPerSample
                Format   = 'tiff'
            }
        }
        finally {
            $br.Dispose()
        }
    }
    finally {
        $fs.Dispose()
    }
}

function Get-ImageSequenceScanResult {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)][string] $Directory,
        [Parameter()][ValidateRange(1, [int]::MaxValue)][int] $SampleStride = 10,
        [Parameter()][scriptblock] $MetadataProvider,
        [Parameter()][ValidateRange(0, [int]::MaxValue)][int] $MaxSamples = 0
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

        if ($MaxSamples -gt 0 -and $sampledFiles.Count -ge $MaxSamples) {
            break
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

Export-ModuleMember -Function Get-ImageSequenceScanResult, Get-TiffMetadataFast
