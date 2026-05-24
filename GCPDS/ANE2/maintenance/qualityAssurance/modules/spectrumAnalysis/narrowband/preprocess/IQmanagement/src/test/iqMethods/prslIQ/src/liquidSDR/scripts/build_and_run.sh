#!/bin/bash

# build_and_run.sh: Complete build, benchmark, and report automation

# Exit on error
set -e

# Configuration
BUILD_DIR="build"
BIN_NAME="liquid_sdr_bench"
RESULTS_DIR="results"
ITERATIONS=5

echo "--- 1. Building Liquid-DSP Benchmark ---"
mkdir -p $BUILD_DIR
cd $BUILD_DIR
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc || echo 1)
cd ..

echo "--- 2. Preparing Results Directory ---"
mkdir -p $RESULTS_DIR

echo "--- 3. Running Benchmark Suite ($ITERATIONS iterations) ---"
for i in $(seq 1 $ITERATIONS); do
    echo "Running Iteration $i/$ITERATIONS..."
    ./$BUILD_DIR/bin/$BIN_NAME --files 6 --json-out $RESULTS_DIR/metrics_iter_$i.json > /dev/null
done

# Sync the primary result for the report generator
if [ -f "$RESULTS_DIR/metrics_iter_1.json" ]; then
    cp $RESULTS_DIR/metrics_iter_1.json $RESULTS_DIR/metrics_liquid.json
fi

echo "--- 4. Generating HTML Dashboard ---"
if command -v python3 &>/dev/null; then
    python3 scripts/generate_report.py
elif command -v python &>/dev/null; then
    python scripts/generate_report.py
else
    echo "[WARN] Python not found, skipping visual report generation."
fi

echo "--- 5. Done ---"
echo "Metrics saved in: $RESULTS_DIR/"
echo "Visual report available at: web/index.html"
