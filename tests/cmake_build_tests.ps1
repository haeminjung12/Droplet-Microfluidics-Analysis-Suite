$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")

$cmake = "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin\\cmake.exe"
if (-not (Test-Path -LiteralPath $cmake)) {
  throw "cmake.exe not found at expected path: $cmake"
}

$vcpkgToolchain = "C:\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake"
if (-not (Test-Path -LiteralPath $vcpkgToolchain)) {
  throw "vcpkg toolchain not found at expected path: $vcpkgToolchain"
}

$qtPrefix = "C:\\Qt\\6.10.1\\msvc2022_64"
if (-not (Test-Path -LiteralPath $qtPrefix)) {
  throw "Qt prefix not found at expected path: $qtPrefix"
}

$buildDir = Join-Path $repoRoot "build\\agent03_task_1_1"
New-Item -ItemType Directory -Force $buildDir | Out-Null

& $cmake -S $repoRoot -B $buildDir -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE="$vcpkgToolchain" `
  -DCMAKE_PREFIX_PATH="$qtPrefix"
if ($LASTEXITCODE -ne 0) {
  throw "CMake configure failed with exit code $LASTEXITCODE"
}

& $cmake --build $buildDir --config Release --target droplet_analyzer_tests
if ($LASTEXITCODE -ne 0) {
  throw "CMake build failed with exit code $LASTEXITCODE"
}

Write-Host "OK: configured and built droplet_analyzer_tests (Release)"
