$ErrorActionPreference = "Stop"

function Assert-PathExists([string]$Path) {
  if (-not (Test-Path -LiteralPath $Path)) {
    throw "Missing required path: $Path"
  }
}

try {
  Assert-PathExists "Docs"
  Assert-PathExists "src"
  Assert-PathExists "include"
  Assert-PathExists "tests"
  Assert-PathExists "third_party"
  Assert-PathExists "cli"
  Assert-PathExists "resources"
  Assert-PathExists "cmake"
  Assert-PathExists "CMakeLists.txt"

  Write-Host "OK: scaffold layout present"
  exit 0
} catch {
  Write-Error $_
  exit 1
}

