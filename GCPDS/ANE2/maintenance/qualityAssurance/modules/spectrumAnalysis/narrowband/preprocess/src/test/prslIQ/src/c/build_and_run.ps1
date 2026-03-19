param(
    [string]$DbPath = "D:/wnOs/wsp/CODE/work/PersonalStuff/GCPDS/ANE2/maintenance/qualityAssurance/modules/db/DataBase-IQ-FM-88MHz-108MHz",
    [int]$Files = 6,
    [int]$MaxComplex = 262144,
    [int]$Nperseg = 512,
    [double]$Overlap = 0.5
)

$ErrorActionPreference = "Stop"

cmake -S . -B build
cmake --build build --config Release

$exe1 = Join-Path $PWD "build/Release/iq_method3_bench.exe"
$exe2 = Join-Path $PWD "build/iq_method3_bench.exe"

if (Test-Path $exe1) {
    & $exe1 --db $DbPath --files $Files --max-complex $MaxComplex --nperseg $Nperseg --overlap $Overlap
} elseif (Test-Path $exe2) {
    & $exe2 --db $DbPath --files $Files --max-complex $MaxComplex --nperseg $Nperseg --overlap $Overlap
} else {
    throw "Executable not found in build outputs."
}
