# build_and_run.ps1: Complete build, benchmark, and report automation for Windows

$ErrorActionPreference = "Continue"

# Configuration
# Use a short absolute path for the build to avoid MAX_PATH issues
$ShortBuildDir = "D:/bld/liquidSDR"
$ProjectSourceDir = (Get-Location).Path
$BinName = "liquid_sdr_bench.exe"
$ResultsDir = "results"
$Iterations = 5

Write-Host "--- 1. Searching for Dependencies ---" -ForegroundColor Cyan
$cmakeCmd = "C:\msys64\mingw64\bin\cmake.exe"

# Set up compiler environment to point to MSYS2 MinGW
$env:CC = "C:\msys64\mingw64\bin\gcc.exe"
$env:CXX = "C:\msys64\mingw64\bin\g++.exe"

Write-Host "--- 2. Building in Short Path: $ShortBuildDir ---" -ForegroundColor Cyan
if (!(Test-Path $ShortBuildDir)) { New-Item -ItemType Directory -Path $ShortBuildDir | Out-Null }
Set-Location $ShortBuildDir

# Configure and build
& $cmakeCmd $ProjectSourceDir -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
& $cmakeCmd --build . --config Release

Set-Location $ProjectSourceDir

Write-Host "--- 3. Preparing Results Directory ---" -ForegroundColor Cyan
if (!(Test-Path $ResultsDir)) { New-Item -ItemType Directory -Path $ResultsDir | Out-Null }

Write-Host "--- 4. Running Benchmark Suite ($Iterations iterations) ---" -ForegroundColor Cyan
# Binary will be in the short build path
$BinPath = "$ShortBuildDir/liquid_sdr_bench.exe"

if (!(Test-Path $BinPath)) {
    Write-Error "Could not find binary at $BinPath. Build might have failed."
    exit 1
}

for ($i = 1; $i -le $Iterations; $i++) {
    Write-Host "Running Iteration $i/$Iterations..."
    & $BinPath --files 6 --json-out "$ResultsDir/metrics_iter_$i.json" | Out-Null
}

if (Test-Path "$ResultsDir/metrics_iter_1.json") {
    Copy-Item "$ResultsDir/metrics_iter_1.json" "$ResultsDir/metrics_liquid.json" -Force
}

Write-Host "--- 5. Generating HTML Dashboard ---" -ForegroundColor Cyan
$pythonCmd = Get-Command python -ErrorAction SilentlyContinue
if (-not $pythonCmd) { $pythonCmd = Get-Command python3 -ErrorAction SilentlyContinue }

if ($pythonCmd) {
    & $pythonCmd scripts/generate_report.py
} else {
    Write-Warning "Python not found, skipping visual report generation."
}

Write-Host "--- 6. Done ---" -ForegroundColor Green
Write-Host "Metrics saved in: $ResultsDir/"
Write-Host "Visual report available at: web/index.html"
