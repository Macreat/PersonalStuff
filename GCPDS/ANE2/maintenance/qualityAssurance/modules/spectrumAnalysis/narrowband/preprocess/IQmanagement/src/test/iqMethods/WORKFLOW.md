Workflow to select and port best IQ method

Prerequisites
- Python 3.8+, pip packages: papermill, nbformat, numpy
- gcc (native) and/or cross-compiler aarch64-linux-gnu-gcc for Pi
- /usr/bin/time and perf (on Raspberry Pi) for metrics

Steps
1. Execute notebook to produce metrics:
   ./run_notebook.sh compare_iq_methods.ipynb metrics_output.json
   (run on host; uses papermill to inject parameters and capture outputs)
2. Select best method:
   python select_best_method.py metrics_output.json --out chosen_method.json
   This script selects based on accuracy/time tradeoff.
3. Implement selected algorithm in C:
   cd c_project && make
4. Run C binary with measurement harness:
   ./build_and_run_c.sh --input-db "../../../../db" --method chosen_method.json
   Results CSV and perf outputs saved under c_project/results/
5. Compare C vs Python:
   python compare_results.py --py metrics_output.json --c c_project/results/metrics_c.json

Outputs
- metrics_output.json: JSON of method metrics from notebook
- chosen_method.json: JSON describing the selected algorithm and parameters
- c_project/results/: CSV/JSON with timing, RSS, perf counters
