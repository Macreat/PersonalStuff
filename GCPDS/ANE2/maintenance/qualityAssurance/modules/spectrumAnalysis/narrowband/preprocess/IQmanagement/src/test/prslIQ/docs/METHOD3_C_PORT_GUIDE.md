# Method 3 (DC + RMS Norm) - Conceptual Design and C Porting Guide

## 1) Purpose
Method 3 is the selected preprocessing strategy because it improves stability before PSD and metric extraction.
In this project, Method 3 means:

1. Convert interleaved int8 IQ to float32 complex samples.
2. Remove DC offset from I and Q.
3. Normalize by RMS magnitude.
4. Compute Welch PSD.
5. Extract noise floor, center power, SNR.
6. Optionally compute KDE for distribution visualization.

what we have to preserve when porting to C and what parameters most affect performance and correctness.

---

## 2) Conceptual Strategy of Method 3

## 2.1 Signal conditioning steps
- DC removal:
  - Compute mean of complex vector.
  - Subtract mean from each sample.
  - Goal: remove center spike and low-frequency bias.
- RMS normalization:
  - Compute RMS = sqrt(mean(|x|^2)).
  - If RMS > eps, divide all samples by RMS.
  - Goal: make amplitude scale consistent across files.

## 2.2 Why this method is robust
- Better comparability between captures with different gains.
- More stable SNR estimates across files.
- Reduced sensitivity to static bias from RF frontend.

---

## 3) Pipeline Architecture (Recommended for C)

Use a streaming architecture with strict ownership of buffers.

1. File IO Layer
- Read .sigmf-meta once per file.
- Read .sigmf-data in chunks or full file depending on RAM budget.

2. Decode Layer
- Convert interleaved int8 to I/Q float32 arrays.
- Handle odd-length input safely (drop last byte).

3. Preprocess Layer (Method 3)
- DC removal in-place.
- RMS normalization in-place.

4. Spectral Layer
- Welch PSD with fixed nperseg, fixed overlap.
- FFT backend (FFTWf or KissFFT).

5. Metrics Layer
- noise_floor_dbm = mean(PSD_dB)
- center_power_dbm = PSD_dB[argmin(|f-f_center|)]
- snr_center_db = center_power_dbm - noise_floor_dbm

6. Reporting Layer
- Runtime per stage.
- Memory usage (RSS + internal allocations).
- KPI summary and optional JSON export.

---

## 4) Parameters That Matter Most

## 4.1 Algorithmic parameters
- sample_rate (from metadata): required for correct frequency axis.
- nperseg (Welch segment length): biggest speed/variance tradeoff.
- overlap ratio (usually 50%): impacts both accuracy and runtime.
- window type (Hann recommended): affects PSD bias/leakage.
- epsilon before log10: avoid -inf / NaN in PSD dB conversion.

## 4.2 Throughput parameters
- N_FILES: number of captures benchmarked.
- file_size_bytes: drives IO and conversion cost.
- chunk_size: critical if streaming to reduce peak RAM.

## 4.3 Numeric precision parameters
- float32 for DSP path: best speed/memory compromise.
- float64 only for validation mode if needed.

---

## 5) Buffer Sizing and Memory Budget

Let N be number of complex samples in one file.
Raw int8 IQ data stores I and Q interleaved:
- raw_bytes = 2 * N * sizeof(int8) = 2N bytes

Typical arrays in C (float32):
- I array: N * 4 bytes
- Q array: N * 4 bytes
- or complex struct {float re, im}: N * 8 bytes

If using one complex buffer in-place:
- complex_bytes = 8N

Welch/FFT scratch (approx):
- O(nperseg) per segment + window + PSD accumulator
- practical budget: 5 to 12 * nperseg * 4 bytes

Rule of thumb peak RAM (single-file streaming):
- peak_bytes ~= raw_chunk + complex_buffer + fft_scratch + psd_accumulator

For safe embedded operation:
- process one file at a time.
- prefer chunked read if full-file allocation is risky.
- pre-allocate buffers once and reuse.

---

## 6) Memory Leak and Safety Checklist

## 6.1 Ownership model
- Module that allocates buffer must free buffer in same module or clearly transfer ownership.
- Avoid hidden allocation in helper functions unless documented.

## 6.2 Leak prevention practices
- Centralized cleanup label pattern in C:
  - allocate
  - if error -> goto cleanup
  - cleanup frees non-null pointers
- After free, set pointer to NULL.
- Keep one allocation table in comments (name, size, owner, lifetime).

## 6.3 Runtime tooling
- Linux: valgrind --leak-check=full
- GCC/Clang debug builds: -fsanitize=address,undefined -fno-omit-frame-pointer
- Windows alternative: Dr. Memory or Visual Studio diagnostics

## 6.4 Common failure points
- Forgetting to free FFT plans/buffers.
- Realloc paths that lose original pointer on failure.
- Early returns before cleanup.
- Writing past buffer on odd input length.

---

## 7) C Data Structures (Suggested)

```c
typedef struct {
    uint32_t sample_rate;
    uint64_t center_freq;
    int lna_db;
    int vga_db;
    int amp_enabled;
} iq_meta_t;

typedef struct {
    float noise_floor_dbm;
    float center_power_dbm;
    float snr_center_db;
} iq_metrics_t;

typedef struct {
    double t_load_ms;
    double t_convert_ms;
    double t_preproc_ms;
    double t_welch_ms;
    double t_metric_ms;
    double t_kde_ms;
    double t_total_ms;
    size_t peak_bytes;
} perf_report_t;
```

---

## 8) Correctness Risks During Port

1. Welch behavior mismatch
- Python scipy.signal.welch has defaults (window, overlap, scaling).
- Reproduce same settings in C for fair comparison.

2. Frequency-axis indexing mismatch
- Ensure FFT shift and center-bin selection are equivalent.

3. Log scale instability
- Always use epsilon: 10*log10(pxx + eps).

4. Normalization mismatch
- Confirm RMS definition matches Python exactly.

5. int8 conversion mismatch
- Preserve sign and ordering: I=raw[0::2], Q=raw[1::2].

---

## 9) Validation Strategy Against Python

Use the same 3 to 6 files and compare C vs Python outputs.

Acceptance targets (practical):
- abs(delta_noise_floor_dbm) <= 0.2 dB
- abs(delta_center_power_dbm) <= 0.2 dB
- abs(delta_snr_center_db) <= 0.25 dB
- runtime improvement >= 4x initially, then optimize toward higher target

Also compare stage timing percentages, not only total runtime.

---

## 10) Optimization Priority (Based on Current Benchmark)

From current profiling, most time is in Welch PSD.
Priority order:
1. Welch kernel and FFT backend
2. int8 -> float32 conversion
3. file IO buffering
4. KDE and metric extraction (already negligible)

This means C work should start with spectral kernel optimization first.

---

## 11) Minimal Porting Plan

1. Port loader + int8 conversion.
2. Port Method 3 preprocessing (DC + RMS) in-place.
3. Port Welch with fixed params.
4. Port metric extraction.
5. Add timing and memory instrumentation.
6. Validate against Python reference values.
7. Iterate on FFT/window/overlap for speed.

---

## 12) Practical Defaults for First C Version

- float32 path only
- nperseg = 2048
- overlap = 50%
- window = Hann
- epsilon = 1e-12f
- one-file streaming
- reusable buffers
- JSON output with timing + KPIs

These defaults maximize reproducibility with your current notebook while keeping memory and complexity controlled.
