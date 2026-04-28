# Error Handling Quickstart for ANE FM Notebooks

Audience: Data scientists and engineers maintaining SDR FM analysis notebooks
Scope: Practical implementation patterns for robust, auditable notebook execution

---

## 1. Why This Guide Exists

Most FM notebook failures are not catastrophic crashes. They are silent quality failures:
1. data gets discarded without explanation.
2. unknown exceptions are swallowed and treated as normal.
3. outputs are generated without enough context to reproduce them.

A robust notebook is not one that only avoids crashing. It is one that can explain every important decision after execution.

---

## 2. The Error Handling Pyramid (Implementation Priority)

Implement bottom to top.

1. Mathematical stability (must-have).
2. Guard clauses (must-have).
3. Specific exceptions (must-have).
4. Graceful degradation (important).
5. Data quality reporting (important).
6. Structured logging (important).
7. Synthetic testing and regression (nice-to-have but strongly recommended).

If the base layers are weak, upper layers only create noisy diagnostics.

---

## 3. Must-Have Layer 1: Mathematical Stability

### 3.1 Unsafe Pattern
```python
pxx_db = 10 * np.log10(lin_avg)
```
If `lin_avg` contains zeros, the output contains `-inf` and contaminates downstream logic.

### 3.2 Safe Pattern
```python
pxx_db = 10.0 * np.log10(np.maximum(lin_avg, 1e-30))
if not np.all(np.isfinite(pxx_db)):
    raise ValueError("non_finite_psd")
```

### 3.3 Required Numerical Guards
1. clip interpolation offsets to configured windows.
2. verify frequency vectors stay within configured band.
3. reject NaN/Inf before peak detection.
4. use robust fallbacks for degenerate fits.

---

## 4. Must-Have Layer 2: Guard Clauses

Guard clauses prevent invalid states from propagating.

### 4.1 Data Loading Guards
```python
if df is None:
    return None
if df.empty:
    return None
if "pxx" not in df.columns:
    return None
```

### 4.2 Detection Guards
```python
if p.size < 10:
    reject("insufficient_samples")
    continue
if not np.all(np.isfinite(p)):
    reject("non_finite_psd")
    continue
```

### 4.3 Output Guards
Return typed empty outputs instead of ambiguous nulls where pipeline chaining benefits from it.

---

## 5. Must-Have Layer 3: Specific Exceptions

### 5.1 Anti-Pattern
```python
try:
    df = pd.read_csv(path)
except Exception:
    pass
```

### 5.2 Correct Pattern
```python
try:
    df = pd.read_csv(path)
except FileNotFoundError:
    logger.error(f"file_not_found: {path}")
    return None
except pd.errors.ParserError as exc:
    logger.error(f"csv_parser_error: {path}: {exc}")
    return None
except UnicodeDecodeError as exc:
    logger.error(f"encoding_error: {path}: {exc}")
    return None
except Exception as exc:
    logger.critical(f"unexpected_error: {type(exc).__name__}: {exc}")
    raise
```

### 5.3 Policy
1. expected exceptions are handled locally with explicit reason codes.
2. unexpected exceptions are escalated.

---

## 6. Important Layer 4: Graceful Degradation

Graceful degradation means keeping the run alive while preserving quality transparency.

### 6.1 Clock Correction Cascade
1. estimate ppm from reference channel.
2. fallback to configured override.
3. fallback to zero correction.

Each fallback step must be logged.

### 6.2 Confidence as Quality Contract
Always return confidence plus quality flags.
Low confidence is a valid output, not a crash condition.

### 6.3 Degradation Rules
1. do not convert hard failures into silent defaults.
2. use defaults only when explicitly documented.
3. attach fallback metadata to output records.

---

## 7. Important Layer 5: Data Quality Reporting

### 7.1 Required Counters
Per run:
1. files_attempted.
2. files_loaded.
3. files_failed.
4. rows_parsed.
5. rows_rejected.
6. rejection_reasons (Counter).

### 7.2 Required Stage Metrics
For Loading, Detection, Compliance, Export:
1. stage input size.
2. stage output size.
3. stage rejection count.
4. top rejection causes.

### 7.3 Recommended Rejection Codes
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

This taxonomy is critical for diagnosing recurring campaign data issues.

---

## 8. Important Layer 6: Structured Logging

### 8.1 Logging Requirements
1. one file per execution, timestamped.
2. DEBUG to file, INFO to console.
3. consistent structured format.

### 8.2 Setup Pattern
```python
def setup_logger(name, log_dir="./audit_logs"):
    Path(log_dir).mkdir(exist_ok=True)
    logger = logging.getLogger(name)
    logger.setLevel(logging.DEBUG)

    fh = logging.FileHandler(
        Path(log_dir) / f"{name}_{dt.datetime.now().strftime('%Y%m%d_%H%M%S')}.log"
    )
    fh.setLevel(logging.DEBUG)

    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)

    fmt = logging.Formatter(
        "%(asctime)s [%(levelname)s] %(name)s - %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
    )
    fh.setFormatter(fmt)
    ch.setFormatter(fmt)

    logger.addHandler(fh)
    logger.addHandler(ch)
    return logger
```

### 8.3 Minimum Log Events
1. run start with version.
2. effective config snapshot.
3. stage start and end.
4. per-file and per-node failures.
5. summary totals.
6. artifact export paths.

---

## 9. Layer 7: Testing and Validation

### 9.1 Synthetic Tests to Add First
1. clean synthetic carrier should be detected and compliant.
2. low-SNR synthetic carrier should be flagged low confidence.
3. missing reference should trigger ppm fallback.
4. malformed rows should be counted and reported.

### 9.2 Regression Baseline
Keep at least one stable dataset to compare:
1. total detections.
2. violations count.
3. warnings count.
4. reject reason distribution.

---

## 10. Implementation Plan (Ready to Execute)

### P0 - Mandatory Stability Block
1. logger bootstrap.
2. specific exceptions for I/O and parsing.
3. finite checks and core invariants.

### P1 - Observability Block
1. implement `DataQualityReport`.
2. add stage metric recording.
3. print and persist final audit summary.

### P2 - Reproducibility Block
1. store run metadata (version, timestamp, environment, config).
2. export standardized artifacts to timestamped directory.

### P3 - Assurance Block
1. synthetic edge-case tests.
2. simple regression checks against baseline dataset.

---

## 11. Documentation Template for Every Change

For each change, always record:
1. What changed.
2. Why it was needed.
3. Which risk it removes.
4. Evidence generated.
   - before/after metric.
   - representative log line.
   - resulting artifact path.
5. Operational impact.

Example:
"Added rejection counters by reason to eliminate silent data loss. Every discarded row is now attributable, improving regulatory traceability and reducing incident diagnosis time."

---

## 12. Definition of Done (Operational)

A run is done only when:
1. a persistent log exists.
2. every rejection has a reason code.
3. stage-wise loss summary is available.
4. no unexpected exception is silently hidden.
5. metadata plus outputs plus audit report are saved.
6. the team can explain how much data was lost, where, why, and under which configuration.

---

## 13. Common Anti-Patterns to Avoid

1. broad `except Exception` without escalation.
2. `print`-only diagnostics.
3. unsafe log transforms.
4. continuing on non-finite arrays.
5. exporting outputs without metadata.
6. changing thresholds without versioned trace.

---

## 14. Final Principle

Error handling is not just exception catching.
It is:
1. prevention of unstable states.
2. controlled degradation.
3. explicit quality accounting.
4. complete audit evidence.

That is the minimum standard for ANE-grade notebook operations.
