# detailMetricsComparison

## Scope
This report compares your latest Python benchmark outputs against the C executable outputs for Method 3 IQ and provides inference on performance, signal metrics, and fairness of comparison.

## Source Snapshots
### Python (Notebook)
- Files: 6
- Samples total: 157,286,400
- Total runtime: 39,544.823 ms
- Throughput: 3,977,420.76 samples/s
- SNR mean: 3.0049 dB
- Noise mean: -74.4821 dBm
- Center mean: -71.4771 dBm
- Stage share: Load 2.21%, Convert 6.27%, Welch 91.50%, Metric ~0%, KDE 0.01%

### C (Executable)
- Files: 6
- Samples total: 1,572,864
- Total runtime: 126.0436 ms
- Throughput: 12,478,729.58021 samples/s
- SNR mean: 13.0018 dB
- Noise mean: -73.2616 dBm
- Center mean: -60.2598 dBm
- Stage share: Load 1.87%, Convert 1.87%, Preproc 9.37%, Welch 83.84%, Metric 0.04%
- Leak status: OK (alloc_count == free_count, bytes_current == 0)

## Key Inference
## 1) Raw runtime is not directly comparable
Python processes about 100x more samples than C:
- Python: 157,286,400
- C: 1,572,864

So direct total runtime (39.5 s vs 0.126 s) overstates C speedup.

## 2) Throughput comparison is fairer
Using samples per second:
- Python: 3.98 Msps
- C: 12.48 Msps

Inference: C is about 3.14x faster in end-to-end throughput under current settings.

## 3) Normalized cost per sample
Approximate runtime per sample:
- Python: 39,544.823 ms / 157,286,400 = 0.000251 ms/sample
- C: 126.0436 ms / 1,572,864 = 0.000080 ms/sample

Inference: C runtime cost per sample is about 3.14x lower.

## 4) Same dominant bottleneck in both pipelines
Welch PSD is the main bottleneck in both implementations:
- Python: 91.50%
- C: 83.84%

Inference: further optimization impact is highest in Welch path (FFT config, windowing, memory locality, vectorization).

## 5) Signal quality metrics are not yet equivalent
SNR and center power differ strongly:
- SNR mean: Python 3.00 dB vs C 13.00 dB
- Center mean: Python -71.48 dBm vs C -60.26 dBm

Inference: current Python and C pipelines are not computing exactly the same signal chain/estimator settings, so signal KPI deltas should not be treated as implementation error yet.

## Why SNR differs so much right now
Most likely causes seen from your configuration:
1. Different sample volume: Python uses full files; C uses truncated chunks (`max_complex`).
2. Different PSD config: Python uses `nperseg=2048`; C run shown uses `nperseg=512`.
3. Different preprocessing fidelity: C applies full Method 3 (DC + RMS normalization), while Python code path shown only guarantees DC centering before Welch.
4. Potential estimator differences: window/scaling/bin handling may differ between SciPy Welch and your C Welch implementation.

## Resource/Operational Inference
- C CPU percent around 100% means single-core saturation during benchmark, which is expected for a compute-heavy loop.
- C RAM percent (~87%) is system-wide memory pressure, not process-only memory growth.
- C process RSS peak is low (~6.34 MB), indicating bounded-memory behavior is working.
- Memory leak gate passes in C, so lifecycle cleanup is currently correct.

## Practical Conclusion
- Performance objective: achieved. C is significantly faster per sample and bounded in memory.
- Numerical parity objective: not yet achieved. Current signal metrics are not apples-to-apples due different benchmark settings and preprocessing behavior.

## Recommended Next Step (for fair parity test)
Run both Python and C with identical settings:
1. Same samples per file (either full-file both or same truncation both).
2. Same `nperseg` and overlap.
3. Same exact Method 3 preprocessing (DC + RMS normalization).
4. Same PSD scaling/window conventions.

Then compare:
- `snr_mean_db`
- `noise_mean_dbm`
- `center_mean_dbm`
- throughput and per-stage percentages

A good acceptance target for parity is absolute error under about 0.5 dB for noise/center/SNR means on the same sample slices.

## Bottom Line
Your C executable is already in good shape for performance and reliability profiling. The remaining gap is metric equivalence calibration between Python and C, not core runtime engineering.
