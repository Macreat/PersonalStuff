param(
    [string]$DbPath = "D:/wnOs/wsp/CODE/work/PersonalStuff/GCPDS/ANE2/maintenance/qualityAssurance/modules/db/DataBase-IQ-FM-88MHz-108MHz",
    [int]$Files = 6,
    [int]$MaxComplex = 262144,
    [int]$Nperseg = 512,
    [double]$Overlap = 0.5,
    [int]$ChunkBytes = 65536,
    [string]$JsonOut = "qa_after.json",
    [string]$CsvOut = "qa_after.csv",
    [switch]$NativeOpt
)

$ErrorActionPreference = "Stop"

$cmakeCmd = Get-Command cmake -ErrorAction SilentlyContinue

if ($cmakeCmd) {
    $cmakeArgs = @("-S", ".", "-B", "build", "-DCMAKE_BUILD_TYPE=Release")
    if ($NativeOpt) {
        $cmakeArgs += "-DIQBENCH_NATIVE_OPT=ON"
    }
    cmake @cmakeArgs
    cmake --build build --config Release
}
else {
    Write-Host "[INFO] cmake no disponible. Compilando con gcc..."
    $gccArgs = @("-O3", "-Wall", "-Wextra", "-Wpedantic", "-std=c11", "-Iinclude", "src/main.c", "src/fs_utils.c", "src/sigmf_io.c", "src/method3.c", "src/welch.c", "src/profiler.c", "-lpsapi", "-lm", "-o", "iq_method3_bench.exe")
    if ($NativeOpt) {
        $gccArgs = @("-O3", "-march=native", "-mtune=native", "-Wall", "-Wextra", "-Wpedantic", "-std=c11", "-Iinclude", "src/main.c", "src/fs_utils.c", "src/sigmf_io.c", "src/method3.c", "src/welch.c", "src/profiler.c", "-lpsapi", "-lm", "-o", "iq_method3_bench.exe")
    }
    gcc @gccArgs
}

$exe1 = Join-Path $PWD "build/Release/iq_method3_bench.exe"
$exe2 = Join-Path $PWD "build/iq_method3_bench.exe"
$exe3 = Join-Path $PWD "iq_method3_bench.exe"

$runArgs = @("--db", $DbPath, "--files", $Files, "--max-complex", $MaxComplex, "--nperseg", $Nperseg, "--overlap", $Overlap, "--chunk-bytes", $ChunkBytes, "--json-out", $JsonOut, "--csv-out", $CsvOut)

if (Test-Path $exe1) {
    & $exe1 @runArgs
} elseif (Test-Path $exe2) {
    & $exe2 @runArgs
} elseif (Test-Path $exe3) {
    & $exe3 @runArgs
} else {
    throw "Executable not found in build outputs."
}
