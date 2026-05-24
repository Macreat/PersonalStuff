#!/bin/bash

# deployRaspi.sh: Sync and deploy to Raspberry Pi sensor node

# Target configuration (Modify as needed)
RPI_USER="pi"
RPI_HOST="raspberrypi.local"
RPI_DIR="/home/pi/liquid_sdr_bench"

echo "--- 1. Syncing Workspace to $RPI_HOST ---"
# Create remote directory
ssh $RPI_USER@$RPI_HOST "mkdir -p $RPI_DIR"

# Sync files (excluding local build and result artifacts)
rsync -avz --exclude 'build' --exclude 'results' --exclude '.git' ./ $RPI_USER@$RPI_HOST:$RPI_DIR/

echo "--- 2. Building Remotely on Raspberry Pi ---"
ssh $RPI_USER@$RPI_HOST "cd $RPI_DIR && mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make -j4"

echo "--- 3. Deployment Successful ---"
echo "You can now run the benchmark on the Pi using: ssh $RPI_USER@$RPI_HOST 'cd $RPI_DIR && ./scripts/build_and_run.sh'"
