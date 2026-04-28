# FM Carrier Audit Checklist - Current vs Recommended

Notebook family: FM carrier estimation and compliance workflow
Objective: identify implementation gaps by severity, then execute highest-value fixes first

---

## 1. How to Use This Checklist

Use this file in three phases:
1. baseline audit before modifications.
2. in-progress validation during implementation.
3. release gate validation before declaring notebook ready.

This checklist is designed to prioritize reliability, traceability, and regulatory defensibility.

---

## 2. Baseline Assessment Snapshot

Historical baseline from prior audit package:
1. overall score: 6.2/10.
2. strongest areas: mathematical stability and guard clauses.
3. weakest areas: persistent logging and data quality reporting.

Interpretation:
1. the core signal workflow was viable.
2. the major risk was insufficient visibility into failures and data loss.

---

## 3. Category Scorecard Framework

Use the following categories and score each from 0 to 10.

1. Mathematical stability.
2. Guard clauses.
3. Specific exceptions.
4. Graceful degradation.
5. Data quality reporting.
6. Structured logging.
7. Validation and invariants.
8. Reproducibility.
9. Synthetic testing.
10. Export completeness.

Suggested interpretation:
1. 0-2: missing.
2. 3-5: partial and risky.
3. 6-8: implemented with moderate gaps.
4. 9-10: robust and auditable.

---

## 4. Detailed Audit Sections

## 4.1 Mathematical Stability
Checks:
- [ ] log transform includes floor guard.
- [ ] no non-finite arrays flow into detector.
- [ ] interpolation offsets are clipped.
- [ ] degenerate fit fallbacks are deterministic.

Expected outcome:
Numerical stage cannot silently poison compliance stage.

## 4.2 Guard Clauses
Checks:
- [ ] required columns validated early.
- [ ] empty CSV and empty parse handled cleanly.
- [ ] insufficient sample nodes skipped with reason.
- [ ] typed empty outputs returned where applicable.

Expected outcome:
Invalid input states are stopped before expensive downstream processing.

## 4.3 Specific Exceptions
Checks:
- [ ] no critical path still uses broad catches as final handler.
- [ ] expected I/O and parse exceptions are explicit.
- [ ] unknown exceptions are logged critical and re-raised.

Expected outcome:
Defects are visible; expected bad data is handled.

## 4.4 Graceful Degradation
Checks:
- [ ] clock ppm fallback cascade is implemented.
- [ ] fallback usage is logged.
- [ ] confidence flags expose low-quality detections.

Expected outcome:
Pipeline continues when possible without hiding quality loss.

## 4.5 Data Quality Reporting
Checks:
- [ ] files counters exist.
- [ ] row counters exist.
- [ ] rejection reasons are aggregated.
- [ ] stage-wise metrics are recorded.

Expected outcome:
Operator can quantify where and why data was lost.

## 4.6 Structured Logging
Checks:
- [ ] file-based logger enabled.
- [x] timestamped per-run log file.
- [ ] stage transitions logged.
- [ ] per-file and per-node errors logged with context.

Expected outcome:
Searchable, persistent execution narrative.

## 4.7 Validation and Invariants
Checks:
- [ ] frequency vectors stay in configured band.
- [ ] PSD range sanity checks exist.
- [ ] no shape inconsistency after filtering.
- [ ] confidence stays in [0,1].

Expected outcome:
Invalid internal states become explicit failures, not silent corruption.

## 4.8 Reproducibility
Checks:
- [ ] version and timestamp are persisted.
- [ ] python and dependency versions are captured.
- [ ] effective config snapshot is saved.
- [ ] outputs are stored in timestamped run directory.

Expected outcome:
Any result can be recreated and defended.

## 4.9 Synthetic Testing
Checks:
- [ ] synthetic clean peak test exists.
- [ ] low-SNR edge case test exists.
- [ ] missing-reference fallback test exists.
- [ ] malformed-row rejection test exists.

Expected outcome:
Critical behavior remains stable across revisions.

## 4.10 Export Completeness
Checks:
- [ ] detections export exists.
- [ ] channel summary export exists.
- [ ] node summary export exists.
- [ ] violations export exists.
- [ ] audit report export exists.
- [ ] analysis plot export exists.

Expected outcome:
Operational package is complete for downstream review.

---

## 5. Highest-Cost Failure Risks

1. Silent data loss.
Without rejection attribution, compliance conclusions lose credibility.

2. Generic exception masking.
Unknown defects become recurring hidden incidents.

3. Missing persistent audit trail.
Post-run investigation becomes speculative.

4. Incomplete reproducibility.
Results cannot be defended to QA or regulatory review.

---

## 6. Top Improvements by Impact-to-Effort Ratio

1. Add structured file logging.
- impact: high.
- effort: low.
- operational gain: immediate traceability.

2. Add stage-based data quality report.
- impact: high.
- effort: medium.
- operational gain: quantified loss diagnostics.

3. Replace generic exceptions with typed handlers.
- impact: medium-high.
- effort: low.
- operational gain: faster root-cause analysis.

4. Add metadata snapshot and timestamped exports.
- impact: medium-high.
- effort: low.
- operational gain: reproducibility.

5. Add synthetic edge-case tests.
- impact: medium.
- effort: medium.
- operational gain: regression safety.

---

## 7. Execution Roadmap (P0 to P3)

### P0 - Mandatory Foundation
1. logger bootstrap.
2. specific exception policy.
3. finite and range invariants.

### P1 - Observability
1. quality report class.
2. rejection taxonomy.
3. stage metrics and final summary.

### P2 - Reproducibility
1. run metadata capture.
2. timestamped artifact export.

### P3 - Assurance
1. synthetic scenario tests.
2. baseline regression comparison.

---

## 8. Evidence Package Required for Closure

To close a notebook hardening task, attach:
1. log excerpt showing stage transitions.
2. rejection breakdown table by reason.
3. list of generated run artifacts.
4. compliance summary counts.
5. sample records for VIOLATION and UNVERIFIABLE.

---

## 9. Change Justification Template

For every implemented item:
1. Change.
2. Why.
3. Risk removed.
4. Evidence.
5. Operational impact.

Example statement:
"Implemented rejection counters by reason to eliminate silent data loss. Each discarded record is now attributable, improving regulatory traceability and reducing incident triage time."

---

## 10. Release Gates

Do not mark the notebook as release-ready unless all gates pass.

Gate A - Robustness:
1. no unhandled crashes on representative dataset.

Gate B - Traceability:
1. persistent log and audit report generated.

Gate C - Data Quality:
1. stage-wise loss and reasons available.

Gate D - Regulatory Explainability:
1. every compliance decision is reconstructable.

Gate E - Reproducibility:
1. same input and config yields explainable repeatable results.

---

## 11. Final Acceptance Criteria

1. A per-run persistent log exists.
2. Every rejected row/file has reason code.
3. Stage-wise data loss summary exists.
4. Unexpected exceptions are not silently swallowed.
5. Full metadata plus outputs plus audit report are exported.
6. Team can explain for each run:
   - how much data was lost,
   - where it was lost,
   - why it was lost,
   - which configuration produced the result.

Meeting all six criteria is the minimum standard for operational and audit acceptance.
