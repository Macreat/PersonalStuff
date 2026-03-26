#!/bin/bash
# Run notebook with papermill to produce metrics JSON
NB_PATH="../../compare_iq_methods.ipynb"
OUT_METRICS="metrics_output.json"
if [ "$1" != "" ]; then NB_PATH="$1"; fi
if [ "$2" != "" ]; then OUT_METRICS="$2"; fi

# Ensure papermill installed
command -v papermill >/dev/null 2>&1 || { echo "Install papermill: pip install papermill"; exit 1; }

# Run notebook and save executed notebook
papermill "$NB_PATH" executed_notebook.ipynb -p OUTPUT_METRICS "$OUT_METRICS"
# Note: the notebook should be adapted to write metrics to OUTPUT_METRICS path as JSON

echo "Executed notebook. Metrics expected at $OUT_METRICS"