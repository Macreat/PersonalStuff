C Project Usage

Build locally:
  cd c_project
  make
Run measurement:
  ./build_and_run_c.sh --input-db "../../../../db" --method ../chosen_method.json

Cross-compile for Raspberry Pi:
  Use c_bench/cross_build.sh or Dockerfile.cross in c_bench/ to build aarch64 binaries

Collect metrics
- Results will be saved under c_project/results/ as time and stdout files
- Use compare_results.py to compare Python vs C outputs
