# deployRaspi.ps1: Sync and deploy to Raspberry Pi sensor node from Windows

$ErrorActionPreference = "Stop"

# Target configuration (Modify as needed)
$RpiUser = "pi"
$RpiHost = "raspberrypi.local"
$RpiDir = "/home/pi/liquid_sdr_bench"

Write-Host "--- 1. Syncing Workspace to $RpiHost ---" -ForegroundColor Cyan
# Create remote directory
ssh "$RpiUser@$RpiHost" "mkdir -p $RpiDir"

# Sync files using rsync (Requires Git for Windows or similar in PATH)
# Using -avz to preserve attributes and compress data
rsync -avz --exclude 'build' --exclude 'results' --exclude '.git' ./ "$RpiUser@$RpiHost`:$RpiDir/"

Write-Host "--- 2. Building Remotely on Raspberry Pi ---" -ForegroundColor Cyan
ssh "$RpiUser@$RpiHost" "cd $RpiDir && mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make -j4"

Write-Host "--- 3. Deployment Successful ---" -ForegroundColor Green
Write-Host "You can now run the benchmark on the Pi using: ssh $RpiUser@$RpiHost 'cd $RpiDir && ./scripts/build_and_run.sh'"
