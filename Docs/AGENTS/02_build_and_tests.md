# Build and tests

## Build

Use Visual Studio MSBuild only.
Do not run CMake. If `build\DropletAnalyzer.sln` is missing, stop and ask the user.

Run from repo root in PowerShell

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" "build\DropletAnalyzer.sln" /m /p:Configuration=Release
```

## Tests

1. Prefer headless tests.
   * Unit tests with GoogleTest
   * CLI runs that exercise end to end behavior

2. GUI testing rule
   * Only ask the user to test the GUI when the task is GUI facing
   * When you ask, list exactly what to click and what should be observed
   * Ask for pass or fail and any screenshot if it fails

3. Evidence required in PR

1. For unit tests
   * Command used
   * Pass result
2. For performance claims
   * Dataset used
   * Timing numbers
   * Machine notes
