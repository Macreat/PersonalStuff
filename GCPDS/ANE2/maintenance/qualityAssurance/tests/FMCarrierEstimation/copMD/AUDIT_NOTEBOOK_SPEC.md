=# FM Carrier Frequency Estimation v4 - Functional Specification and Implementation Protocol

Project: ANE Spectrum Monitoring (HackRF One)
Notebook Scope: Carrier frequency estimation, compliance assessment, and audit-grade reporting
Target Notebook Family: `fm_carrier_v3_audit.ipynb`, `fm_carrier_deepnote.ipynb`, `fm_carrier_v4.ipynb`

---

## 1. Executive Summary

### 1.1 Purpose
This document defines the full technical contract for the FM carrier estimation notebook pipeline used in ANE monitoring operations. It describes architecture, function-level responsibilities, expected inputs and outputs, error handling behavior, and required audit artifacts.

### 1.2 Primary Goals
1. Estimate FM carrier frequencies from PSD sweeps with sub-bin refinement.
2. Evaluate regulatory compliance using hard and warning thresholds.
3. Provide node-level and channel-level aggregation for operational decisions.
4. Persist sufficient evidence to explain all pipeline decisions after execution.

### 1.3 Regulatory and Technical Constants
1. FM band: 87.5 MHz to 108.0 MHz.
2. Channel raster: 100 kHz.
3. Typical frequency resolution: approximately 1220 Hz/bin (20 MHz span, 16384 bins).
4. Hard tolerance: plus or minus 2 kHz.
5. Warning zone: plus or minus 1 kHz.
6. Hardware baseline: HackRF One TCXO with potential ppm drift.

---

## 2. Architecture Overview

### 2.1 End-to-End Data Flow

```text
CSV files (multi-node)
    -> parse_pxx_cell
    -> load_node_psd (linear averaging)
    -> clock offset estimation and correction
    -> DC/LO leakage suppression
    -> peak detection
    -> per-peak estimation (frequency, SNR, prominence, width, confidence)
    -> compliance classification
    -> channel and node aggregation
    -> export CSV + audit report + logs + plot
```

### 2.2 Stage Model
The pipeline is implemented and audited in four stages:
1. Loading.
2. Detection.
3. Compliance.
4. Export.

Each stage must provide:
1. input volume.
2. output volume.
3. rejection reasons.
4. execution status.

### 2.3 Design Principles
1. Deterministic behavior with explicit fallbacks.
2. Data quality transparency over silent continuation.
3. Reproducibility through timestamped outputs and configuration snapshots.
4. Failure isolation so one bad file or node does not collapse the entire run.

---

## 3. Functional Modules and Contracts

## 3.1 Parsing Module

### Function: `parse_pxx_cell(cell)`
Purpose:
Parse one raw PSD cell into a numeric array.

Expected Input:
1. serialized list-like PSD representation in text form.

Output:
1. `np.ndarray` with float values.
2. parse status or reason in enhanced implementations.

Failure Modes:
1. malformed serialization.
2. empty payload.
3. non-numeric values.

Contract:
1. never return malformed numeric arrays.
2. produce explicit parse failure reason when parsing fails.

---

### Function: `load_node_psd(csv_path, cfg)`
Purpose:
Load one node CSV, parse PSD rows, normalize shape, average in linear scale, and attach metadata.

Expected Input:
1. path to CSV file.
2. validated spectral configuration object.

Output on Success:
1. node identifier.
2. frequency axis.
3. averaged PSD in dBm.
4. sweep count.
5. optional timestamp and geo metadata.
6. parsing quality counters.

Output on Failure:
1. `None` or typed empty structure (implementation dependent).
2. reason recorded in quality report.

Required Behavior:
1. validate required columns.
2. reject empty files.
3. reject inconsistent arrays after mode-length filtering.
4. reject non-finite PSD after averaging.

Mandatory Numeric Logic:
```python
lin_avg = np.mean(10.0 ** (stack / 10.0), axis=0)
pxx_avg = 10.0 * np.log10(np.maximum(lin_avg, 1e-30))
```

---

## 3.2 Clock Correction Module

### Function: `_estimate_clock_ppm(freqs_hz, pxx, cfg)`
Purpose:
Estimate clock drift using a reference broadcast channel.

Strategy:
1. search in a configurable window around reference channel.
2. pick peak by highest prominence.
3. fallback to strongest bin if peak extractor does not return candidates.
4. fallback to configured override when reference is absent.

Output:
1. ppm estimate used to correct frequency axis.

Failure Strategy:
1. return fallback ppm and log warning.
2. never produce NaN/Inf ppm.

### Function: `apply_clock_correction(freqs_hz, cfg)`
Transformation:
`f_corrected = f_measured * (1 - ppm / 1e6)`

Validation:
1. skip correction for negligible ppm magnitude if configured.
2. log applied ppm at node-level granularity.

---

## 3.3 DC Mask Module

### Function: `apply_dc_mask(freqs_hz, pxx, cfg)`
Purpose:
Suppress central DC/LO leakage to avoid false detections.

Method:
1. locate central bin or configured center frequency region.
2. mask plus or minus configured bin width.
3. interpolate across mask boundaries.
4. fallback to robust baseline if interpolation edge case occurs.

Required Constraints:
1. must not create discontinuities that bias peak detection.
2. must not write non-finite values.

---

## 3.4 Spectral Estimation Helpers

### `_quadratic_offset(p, idx, hw)`
Purpose:
Sub-bin frequency refinement around detected peak.

Rules:
1. use quadratic fit in dBm domain.
2. reject convex fits for peak correction.
3. clip offset to interpolation window.

### `_noise_floor(p, idx, df_hz, cfg)`
Purpose:
Estimate local baseline from side windows outside guard region.

Fallback:
1. global median if local samples are unavailable.

### `_prominence(p, idx, df_hz, cfg)`
Purpose:
Compute peak prominence via detector output or fallback baseline method.

### `_confidence(snr_db, prom_db, width_bins, cfg)`
Purpose:
Compute confidence score and quality flags.

Expected Output:
1. normalized score in [0, 1].
2. list of diagnostic flags (low SNR, low prominence, broad peak).

---

## 3.5 Per-Carrier Estimation and Batch Execution

### Function: `_estimate_one(...)`
Produces one final carrier row including:
1. estimated and nominal frequency.
2. deviation in Hz, kHz, ppm.
3. peak power, noise floor, SNR.
4. prominence and width.
5. confidence and flags.
6. compliance status.

### Function: `run_batch_detection(pxx_node, cfg)`
Purpose:
Run the full detection loop across loaded nodes.

Mandatory Robustness:
1. skip nodes with insufficient samples.
2. stop node processing on non-finite PSD.
3. isolate per-node failures and continue batch.
4. collect rejection reasons by stage.

---

## 4. Compliance Logic

Status decision contract:
1. COMPLIANT if absolute deviation is less than or equal to 1000 Hz.
2. WARNING if absolute deviation is greater than 1000 Hz and less than or equal to 2000 Hz.
3. VIOLATION if absolute deviation is greater than 2000 Hz.
4. UNVERIFIABLE if nominal channel cannot be matched.

Why UNVERIFIABLE matters:
1. this is a quality classification, not a hard error.
2. prevents forced, misleading compliance assignment.

---

## 5. Error Handling Architecture

### 5.1 Expected Exception Classes
1. `FileNotFoundError`
2. `PermissionError`
3. `pd.errors.ParserError`
4. `UnicodeDecodeError`
5. `ValueError`

### 5.2 Unexpected Exception Policy
1. log as CRITICAL with context.
2. re-raise to avoid masking defects.

### 5.3 Guard Clauses
Guards are required immediately after:
1. file read.
2. schema validation.
3. parse stage.
4. averaging stage.
5. pre-detection stage.

### 5.4 Numerical Invariants
1. no NaN/Inf in PSD used for detection.
2. frequency vectors remain in configured band.
3. output arrays must be shape-consistent after filtering.
4. confidence must remain inside [0, 1].

---

## 6. Data Quality and Auditability

### 6.1 Required Data Quality Counters
1. files_attempted.
2. files_loaded.
3. files_failed.
4. rows_parsed.
5. rows_rejected.
6. rejection reasons counter.

### 6.2 Stage Metrics
For each stage (Loading, Detection, Compliance, Export), record:
1. input count.
2. output count.
3. rejected count.
4. key reason breakdown.

### 6.3 Rejection Taxonomy (Recommended)
1. missing_required_column.
2. empty_csv.
3. pxx_parse_failed.
4. array_length_mismatch.
5. non_finite_psd.
6. invalid_frequency_metadata.
7. insufficient_samples.
8. clock_estimation_failed.
9. peak_detection_failed.
10. peak_estimation_failed.

---

## 7. Logging and Export Specification

### 7.1 Logging Requirements
1. persistent file logger for complete trace.
2. console logger for concise operator feedback.
3. timestamped log name per run.

### 7.2 Output Layout
Recommended run directory:
`results/YYYYMMDD_HHMMSS/`

Required files:
1. `01_carrier_detections.csv`
2. `02_carrier_summary.csv`
3. `03_node_summary.csv`
4. `04_violations.csv`
5. `00_AUDIT_REPORT.txt`
6. `05_analysis_plot.png`

Log location:
`audit_logs/YYYYMMDD_HHMMSS.log`

### 7.3 Run Metadata Snapshot
Persist:
1. notebook version.
2. run timestamp.
3. python version.
4. dependency versions.
5. effective config values.

---

## 8. Implementation Protocol for New Notebook Versions

### 8.1 Standard Notebook Layout
1. environment setup and dependency checks.
2. imports and config class.
3. logging and quality-report infrastructure.
4. data loading and validation.
5. signal processing functions.
6. aggregation and compliance.
7. execution block.
8. export and final report.

### 8.2 Configuration Rules
1. use a single config dataclass.
2. keep all frequencies in Hz internally.
3. expose only presentation conversions (kHz/MHz) in reports.
4. validate config before processing data.

### 8.3 Versioning Rules
1. maintain semantic version in code.
2. document fixes per version.
3. include version in all run artifacts.

---

## 9. Performance and Scalability Guidance

1. track stage execution times.
2. measure file and row throughput.
3. avoid repeated conversions for large datasets.
4. use vectorized operations where feasible.
5. maintain deterministic behavior under partial data corruption.

---

## 10. Testing and Validation Requirements

### 10.1 Synthetic Tests (Minimum)
1. clean synthetic peak should be detected and compliant.
2. low-SNR peak should trigger low confidence flags.
3. missing reference channel should trigger clock fallback.
4. corrupted arrays should be rejected with explicit reason.

### 10.2 Regression Tests
1. compare run outputs across versions on fixed datasets.
2. ensure no unintended drift in compliance counts.
3. ensure rejection taxonomy stability.

---

## 11. Acceptance Criteria (Operational Done)

A run is considered production-ready only if:
1. no unhandled exceptions terminate execution.
2. all rejections include reason codes.
3. stage-wise loss summary is available.
4. compliance results are traceable at record level.
5. metadata and artifacts are fully reproducible.
6. audit log and report are persisted and searchable.

---

## 12. Key Operational Outcome

The notebook should answer, for every run:
1. what was processed.
2. what was discarded.
3. why it was discarded.
4. what configuration was active.
5. where to find complete audit evidence.

That outcome is the core requirement for ANE-grade technical and regulatory defensibility.
