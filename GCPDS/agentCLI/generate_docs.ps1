<#
generate_docs.ps1
Creates output dir and runs Doxygen with the included Doxyfile.
Usage: Open PowerShell as normal user and run: .\generate_docs.ps1
Requires: Doxygen installed and available on PATH.
#>

$RepoRoot = "D:\wnOs\wsp\CODE\work\PersonalStuff\GCPDS\agentCLI"
$Doxyfile = Join-Path $RepoRoot 'Doxyfile'
$OutDir = Join-Path $RepoRoot 'docs\doxygen'

Write-Host "Repo root: $RepoRoot"
Write-Host "Doxyfile: $Doxyfile"
Write-Host "Output directory: $OutDir"

if (-not (Test-Path $Doxyfile)) {
    Write-Error "Doxyfile not found at $Doxyfile"
    exit 2
}

if (-not (Test-Path $OutDir)) {
    Write-Host "Creating output directory: $OutDir"
    New-Item -ItemType Directory -Path $OutDir -Force | Out-Null
}

# Check for doxygen
$DoxyCmd = Get-Command doxygen -ErrorAction SilentlyContinue
if (-not $DoxyCmd) {
    Write-Host "Doxygen not found on PATH. Install instructions:"
    Write-Host "  - Windows (Chocolatey): choco install doxygen -y"
    Write-Host "  - Windows (scoop): scoop install doxygen"
    Write-Host "  - macOS (brew): brew install doxygen"
    Write-Host "  - Ubuntu/WSL: sudo apt update && sudo apt install -y doxygen"
    exit 3
}

Write-Host "Running: doxygen $Doxyfile"
$doxygenExit = & doxygen $Doxyfile

if ($LASTEXITCODE -eq 0) {
    $HtmlPath = Join-Path $OutDir 'html'
    Write-Host "Doxygen completed successfully. Generated HTML at: $HtmlPath"
    Write-Host "Open index.html in that folder to view the documentation."
    exit 0
} else {
    Write-Error "Doxygen failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}
