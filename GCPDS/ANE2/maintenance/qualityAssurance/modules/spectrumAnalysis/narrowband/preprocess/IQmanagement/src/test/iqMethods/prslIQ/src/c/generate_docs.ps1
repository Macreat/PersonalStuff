param(
    [switch]$Open,
    [switch]$InstallTools
)

$ErrorActionPreference = "Stop"

function Test-Tool {
    param([string]$Name)
    return $null -ne (Get-Command $Name -ErrorAction SilentlyContinue)
}

function Install-WithScoop {
    if (-not (Test-Tool "scoop")) {
        return $false
    }

    Write-Host "[INFO] Instalando doxygen y graphviz con scoop..."
    scoop install doxygen graphviz
    return $true
}

function Install-WithChoco {
    if (-not (Test-Tool "choco")) {
        return $false
    }

    Write-Host "[INFO] Instalando doxygen.install y graphviz con chocolatey..."
    choco install doxygen.install graphviz -y
    return $true
}

if (-not (Test-Path "docs/Doxyfile")) {
    throw "No se encontro docs/Doxyfile"
}

$hasDoxygen = Test-Tool "doxygen"
$hasDot = Test-Tool "dot"

if (($InstallTools) -and (-not ($hasDoxygen -and $hasDot))) {
    $ok = (Install-WithScoop)
    if (-not $ok) {
        $ok = (Install-WithChoco)
    }

    if (-not $ok) {
        throw "No fue posible instalar herramientas automaticamente. Instala doxygen y graphviz manualmente."
    }

    $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
    $hasDoxygen = Test-Tool "doxygen"
    $hasDot = Test-Tool "dot"
}

if (-not $hasDoxygen) {
    throw "doxygen no esta disponible en PATH. Ejecuta: .\\generate_docs.ps1 -InstallTools"
}

if (-not $hasDot) {
    Write-Warning "graphviz (dot) no esta disponible. Se generara HTML sin graficas de llamadas."
}

Write-Host "[INFO] Generando documentacion HTML..."
doxygen docs/Doxyfile

$indexPath = Join-Path $PWD "docs/_build/html/index.html"
if (-not (Test-Path $indexPath)) {
    throw "No se encontro el index HTML generado: $indexPath"
}

Write-Host "[OK] Documentacion generada en: $indexPath"

if ($Open) {
    Start-Process $indexPath
}
