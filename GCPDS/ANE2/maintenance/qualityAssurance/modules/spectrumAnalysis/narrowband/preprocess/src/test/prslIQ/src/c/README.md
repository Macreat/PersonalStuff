# IQ Method 3 C Benchmark (Minimal README)

Minimal C benchmark for IQ Method 3 (DC removal + RMS normalization) with runtime and memory audit.

## Scope
- int8 interleaved IQ -> complex float32
- Method 3 preprocessing
- Welch PSD (reference implementation)
- Metrics: noise floor, center power, SNR
- Cost metrics: load, convert, preprocess, welch, metric, total
- Memory checks: RSS + internal allocation tracker

## Project Files
- `include/iq_bench.h`: shared types and interfaces
- `src/main.c`: orchestration, CLI, audit printout
- `src/sigmf_io.c`: metadata parse and raw IQ load
- `src/method3.c`: Method 3 implementation
- `src/welch.c`: Welch PSD and metric extraction
- `src/profiler.c`: timing and memory tracking
- `src/fs_utils.c`: local SigMF pair discovery

## Build (Windows GCC)
```powershell
cd D:/wnOs/wsp/CODE/work/PersonalStuff/GCPDS/ANE2/maintenance/qualityAssurance/modules/spectrumAnalysis/narrowband/preprocess/src/test/prslIQ/src/c
gcc -O3 -Wall -Wextra -std=c11 -Iinclude src/main.c src/fs_utils.c src/sigmf_io.c src/method3.c src/welch.c src/profiler.c -lpsapi -lm -o iq_method3_bench.exe
```

## Run
```powershell
.\iq_method3_bench.exe --db "D:/wnOs/wsp/CODE/work/PersonalStuff/GCPDS/ANE2/maintenance/qualityAssurance/modules/db/DataBase-IQ-FM-88MHz-108MHz" --files 6 --max-complex 262144 --nperseg 512 --overlap 0.5
```

Quick smoke test:
```powershell
.\iq_method3_bench.exe --files 2 --max-complex 32768 --nperseg 256 --overlap 0.5
```

## Inference From Current Output
If your output is similar to the validated run:
- `welch_ms` dominates (`~97%`), so PSD is the critical bottleneck.
- `convert_ms` and `preprocess_ms` are small, so Method 3 itself is not the runtime problem.
- `bytes_current = 0` and `alloc_count = free_count` indicates no internal leak in tracked allocations.
- `rss_delta_mb` positive but modest means temporary working buffers are expected and released.

Conclusion: optimize Welch/FFT path first, then acquisition buffering.

## Acquisition Process Audit (Minimal)
Apply these constraints to improve performance and reliability:

The right next optimization target is FFT-based Welch and reusable buffers.

1. Bounded memory
- Keep `--max-complex` explicit.
- Process one file at a time.
- Reuse buffers across files.

2. Deterministic dataflow
- Discover pair -> parse meta -> load raw -> convert -> Method3 -> Welch -> metrics -> free.
- No cross-file retained signal arrays.

3. Leak control
- Keep one cleanup path per file stage.
- Treat tracker mismatch as build blocker.
- Validate with sanitizer/valgrind in debug workflows.

4. Measurement quality
- Always report stage percentages, not only total time.
- Keep benchmark args fixed when comparing revisions.

5. Numerical stability
- Keep epsilon in log conversion.
- Keep float32 for throughput path; compare against Python only for validation windows.

## Next Technical Upgrades
1. Replace naive Welch in `src/welch.c` with FFT backend (FFTWf/KissFFT).
2. Add chunked loader in `src/sigmf_io.c` for larger captures.
3. Pre-allocate window and PSD scratch once in `main.c` and pass reusable workspace.
4. Add CSV/JSON output mode for automated regression benchmarks.

## About External Chat Reference
The shared Qwen URL was not machine-readable from this environment (landing page only), so this audit is based on your current code, run outputs, and Method 3 constraints already defined in the project guides.
