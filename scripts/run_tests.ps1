param(
    [string]$BuildDir = (Join-Path $PSScriptRoot "..\\build")
)

$vcpkgRoot = $env:VCPKG_ROOT
if ($vcpkgRoot) {
    $binDir = Join-Path $vcpkgRoot "installed\\x64-windows\\bin"
    $debugBinDir = Join-Path $vcpkgRoot "installed\\x64-windows\\debug\\bin"

    if (Test-Path $binDir) {
        if (Test-Path $debugBinDir) {
            $env:PATH = "$binDir;$debugBinDir;$env:PATH"
        } else {
            $env:PATH = "$binDir;$env:PATH"
        }
    }
}

if (-not (Test-Path $BuildDir)) {
    Write-Error "Build directory not found: $BuildDir"
    exit 1
}

$testExe = Get-ChildItem -Path $BuildDir -Recurse -Filter "droplet_analyzer_tests.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $testExe) {
    Write-Error "droplet_analyzer_tests.exe not found under $BuildDir. Build the test target first."
    exit 1
}

& $testExe.FullName
exit $LASTEXITCODE
