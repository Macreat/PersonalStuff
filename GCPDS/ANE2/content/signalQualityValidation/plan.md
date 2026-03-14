# Signal Quality Validation — Implementation Plan

## Executive Summary
Establish a standardized, scalable framework for RF signal quality assurance (QA) across distributed SDR sensors. Deliver canonical data sources, consistent naming conventions, and modular QA routes for wideband, narrowband, and voice services.

---

## Goals & Success Criteria

### Primary Goals
1. **Canonical Data Dictionary** — Single source of truth for all signal QA variables (names, types, units, sources)
2. **Structured Documentation** — Clear README and protocol guides for users and developers
3. **Modular QA Routes** — Separate, testable implementations for three service types (wideband, narrowband, voice)
4. **Code Consistency** — All modules use identical naming and units; validated via linting and schema checks

### Success Criteria
- ✅ README.md: Clear purpose, scope, structure, and quick pointers
- ✅ DICTIONARY.md: All ~25 variables defined (name, type, unit, description, source, importance)
- ✅ Database ingestion: CSVs validated against dictionary schema
- ✅ QA modules: Each route (wideband, narrowband, voice) imports from canonical dictionary
- ✅ Automated checks: Linters catch undefined/misnamed variables; unit mismatches flagged
- ✅ Version control: Each significant change tagged with schema version

---

## Architecture Overview

```
signalQualityValidation/
├── README.md                          [Main entry point; purpose, scope, structure]
├── DICTIONARY.md                      [Canonical variable definitions (~25 fields)]
├── plan.md                            [This file; high-level strategy]
├── DB/
│   └── Database-FM-10Nodes/
│       ├── DataAcq.ipynb              [Raw data acquisition & preprocessing]
│       ├── Database-RF-FM-88...-RF/   [Per-node CSVs (10 nodes)]
│       └── README.md                  [DB structure & ingestion guide]
├── routes/                            [Modular QA implementations]
│   ├── wideband.py                    [Multi-freq spectrum, noise floor]
│   ├── narrowband.py                  [IQ calibration, spectral masks]
│   └── voice.py                       [Sync checks, service metrics]
├── validators/                        [Schema & unit checkers]
│   ├── schema.py                      [Validate CSV/Dict structure]
│   └── units.py                       [Enforce unit consistency]
└── tests/                             [Unit & integration tests]
    ├── test_schema.py
    ├── test_routes.py
    └── test_integration.py
```

---

## Phase 1: Documentation & Dictionary Finalization (CURRENT)

### Tasks
- [x] **analyze-db** — Inspect DataAcq.ipynb and Database-RF; extract all variables and rules
- [x] **extract-dictionary** — Create canonical dictionary with ~25 fields (name, type, unit, description, source, importance)
- [x] **draft-readme** — Write minimal README: purpose, scope, structure, quick pointers
- [ ] **review-with-user** — Present draft README and DICTIONARY to user for feedback
- [ ] **finalize-readme** — Incorporate feedback and commit to version control

### Deliverables
- **README.md** — 30 lines; titled "signal quality validation"; covers purpose, scope (3 routes), DB sources, canonical dictionary, naming scheme, quick pointers, version control, next steps
- **DICTIONARY.md** — Table format (name | type | unit | description | source | importance); ~25 core variables; final notes on unit consistency and primary keys
- **plan.md** — This file; strategy, phases, success criteria

**Status:** Ready for review and finalization.

---

## Phase 2: Data Validation & Ingestion (PLANNED)

### Tasks
1. **schema-validator** — Python module to validate CSV columns against DICTIONARY.md schema
2. **unit-converter** — Helper functions for dB ↔ linear, Hz conversions, timestamp normalization
3. **csv-ingestion** — Load per-node CSVs; validate schema; ingest into consolidated structure
4. **test-ingestion** — Unit tests for schema and unit validation

### Success Criteria
- All 10 per-node CSVs load without schema errors
- Timestamps normalized to ISO8601 (UTC)
- All numeric columns cast to correct types
- Linter catches undefined DICTIONARY variables

---

## Phase 3: QA Route Implementation (PLANNED)

### Route: Wideband
- **Input:** Multi-frequency PSD arrays (e.g., 88–108 MHz)
- **Processing:** Aggregate PSD across nodes, estimate noise floor, detect interference
- **Output:** Noise floor (dB), SNR estimates, frequency maps
- **Test:** Verify PSD aggregation logic; compare against DataAcq reference

### Route: Narrowband
- **Input:** Per-node IQ samples; frequency masks; capture metadata
- **Processing:** DC blocking, spectral mask check, per-node PSD normalization
- **Output:** IQ health (clean/degraded), mask compliance (pass/fail), per-node PSD
- **Test:** Validate DC-blocker math; check IQ quality metrics

### Route: Voice
- **Input:** Timestamp alignment, sample rate, per-node latency data
- **Processing:** Synchronization checks, service quality metrics
- **Output:** Sync status, latency distribution, anomaly flags
- **Test:** Simulate clock drift; verify anomaly detection

---

## Phase 4: Integration & Deployment (PLANNED)

### Tasks
1. **route-orchestrator** — Main entry point; loads dictionary, routes data to appropriate handler
2. **logging & metrics** — Structured logging; output metrics to CSV/JSON
3. **deployment-guide** — Instructions for running QA on new datasets
4. **automated-tests** — CI/CD checks (linting, schema validation, unit tests)

### Success Criteria
- All 3 routes run end-to-end without errors
- Metrics output matches expected schema
- Documentation is current and complete

---

## Key Decisions & Rationale

### 1. Single Canonical Dictionary
**Why:** Eliminates variable name mismatches and unit inconsistencies across routes and code.
**How:** DICTIONARY.md is the single source of truth; all code imports/validates against it.

### 2. Modular Routes
**Why:** Each service type (wideband, narrowband, voice) has different inputs, processing logic, and outputs.
**How:** Separate modules under `routes/`; shared validators and utilities in `validators/`.

### 3. CSV as Primary Source
**Why:** Per-node data already in CSV; minimal parsing overhead; easy versioning.
**How:** Schema validator ensures consistency; ingestion pipeline normalizes units and types.

### 4. ISO8601 Timestamps
**Why:** Standard, sortable, human-readable; avoids timezone ambiguity.
**How:** Ingestion pipeline auto-converts; DICTIONARY specifies UTC timezone.

---

## Dependencies & External References

- **DataAcq.ipynb** — Reference for acquisition rules, DC-blocker parameters (alpha, cutoff_hz), IQ conversion
- **per-node CSVs** — 10 nodes, each with ~50–100 rows of capture data (timestamp, PSD, metrics)
- **Python 3.9+** — Required for async processing and type hints
- **NumPy/SciPy** — PSD computation, FFT, signal processing
- **Pandas** — CSV ingestion and time-series alignment

---

## Timeline & Milestones

| Phase | Tasks | Target | Status |
|-------|-------|--------|--------|
| 1 | Documentation & dictionary | ✅ Pending review | In progress |
| 2 | Data validation & ingestion | Q2 2026 | Planned |
| 3 | QA route implementation | Q2–Q3 2026 | Planned |
| 4 | Integration & deployment | Q3 2026 | Planned |

---

## Risk & Mitigation

| Risk | Impact | Mitigation |
|------|--------|-----------|
| Schema changes require code updates | High | Version DICTIONARY; use schema validation layer |
| Per-node data inconsistency | High | Strict CSV schema checks; unit validators |
| Performance on large datasets | Medium | Vectorized NumPy/Pandas; async processing |
| Integration test failures | Medium | Test each route independently first; CI/CD pipeline |

---

## Next Steps

1. **Review & approve plan.md, README.md, DICTIONARY.md** — User feedback
2. **Finalize documentation** — Incorporate feedback
3. **Begin Phase 2** — Implement schema validator and ingestion pipeline
4. **Track progress** — Update plan.md as phases complete

---

## Questions & Open Items

- [ ] Confirm canonical dictionary is complete (any missing variables?)
- [ ] Approve unit choices (dB vs. linear for PSD; Hz for frequencies)
- [ ] Timeline confirmation for Phase 2 & 3 start dates
- [ ] Any additional routes or QA metrics required?

---

**Last Updated:** 2026-03-14  
**Version:** 1.0  
**Owner:** Signal Quality Validation Team  
