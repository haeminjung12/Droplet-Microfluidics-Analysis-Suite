# Build and tests

Build

Use Visual Studio MSBuild only.
Do not run CMake.
If build/DropletAnalyzer.sln is missing, stop and ask the user.

Run from repo root in PowerShell

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" /p:Platform=x64
```

Common MSBuild issues

1. MSB8013 with Release Win32
   1. Re run with Platform x64

2. MSB8020 toolset v180 not found
   1. Retarget to the installed toolset, commonly v145
   2. Or edit the vcxproj PlatformToolset field

Tests

1. Prefer headless tests
   1. Unit tests with GoogleTest
   2. CLI runs that exercise end to end behavior

2. GUI testing rule
   1. Ask for GUI testing only when the task is GUI facing
   2. List exact steps and expected results
   3. Ask for pass or fail and a screenshot on failure

Evidence for a PR or merge note

1. Unit tests
   1. Command used
   2. Pass result

2. Performance claims
   1. Dataset used
   2. Timing numbers
   3. Machine notes
