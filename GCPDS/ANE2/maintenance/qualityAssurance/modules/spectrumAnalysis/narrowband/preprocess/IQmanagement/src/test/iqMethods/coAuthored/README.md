IQ Methods Test Directory

Purpose
- Provide a minimal, precise workflow to run and benchmark the IQ database located at modules/db against the algorithms prototyped in compare_iq_methods.ipynb.

Contents
- WORKFLOW.md: step-by-step instructions to run selection, profiling, and verification
- run_notebook.sh: execute the notebook to generate metrics (uses papermill)
- select_best_method.py: pick the best method by balancing accuracy and runtime
- metrics_collector.py: script to run C binary and collect compute metrics
- c_project/: C implementation skeleton with Makefile to compile measureable binary

Usage summary
1. Populate modules/db with IQ CSV/binary files (already present).
2. Run run_notebook.sh to execute compare_iq_methods.ipynb and save its metrics to src/test/iqMethods/metrics.json.
3. Run select_best_method.py to choose the algorithm to port to C.
4. Use c_project/ to implement the chosen algorithm in C and run build_and_run_c.sh to collect CPU/time/RSS metrics.
