# deploy_docs.ps1 - Automated Documentation Deployment for GCPDS
# This script regenerates Doxygen HTML from the 'progress' branch and deploys it to the personal domain (gh-newpage).

$ErrorActionPreference = "Stop"

Write-Host "`n[1/4] Regenerating Doxygen Documentation..." -ForegroundColor Cyan
# Run Doxygen using the consolidated configuration
doxygen GCPDS/agentCLI/Doxyfile

Write-Host "[2/4] Staging changes for deployment..." -ForegroundColor Cyan
# Stage Markdown files, the generated docs folder, and the deployment script itself
git add GCPDS/
git add docs/
git add deploy_docs.ps1

$timestamp = Get-Date -Format "yyyy-MM-dd HH:mm"
$commitMsg = "docs: automatic update and deployment from progress branch ($timestamp)"

# Check if there are changes to commit
$status = git status --porcelain
if ($null -eq $status -or $status -eq "") {
    Write-Host "No changes detected. Skipping commit and push." -ForegroundColor Yellow
    exit
}

Write-Host "[3/4] Committing changes to progress branch..." -ForegroundColor Cyan
git commit -m $commitMsg

Write-Host "[4/4] Pushing to branches..." -ForegroundColor Cyan

# Push the current branch (progress) to its remote origin
Write-Host "-> Pushing progress to origin..." -ForegroundColor Gray
git push origin progress

# Update the deployment branch (gh-newpage) with the new documentation
Write-Host "-> Updating gh-newpage branch..." -ForegroundColor Gray
git push origin HEAD:gh-newpage --force

Write-Host "`nDocumentation restructured and deployed successfully!" -ForegroundColor Green
Write-Host "Check your domain at: https://macreat.github.io/PersonalStuff/index.html" -ForegroundColor Green
