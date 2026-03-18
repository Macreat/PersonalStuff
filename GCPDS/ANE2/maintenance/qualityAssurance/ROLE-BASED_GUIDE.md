# ROLE-BASED DOCUMENTATION & COLLABORATION GUIDE
**Version:** 0.2 | **Date:** March 18, 2026

---

## OVERVIEW

This document provides **role-specific navigation paths** for collaborators working on the ANE2-Calibration-SDR project. Each role (wide band, narrow band and service voice) has dedicated entry points, documentation, and code modules.

---

## QUICK ROLE SELECTOR

| Role | Entry Point | Primary Challenge | Key Module | Status |
|------|-------------|-------------------|-----------|--------|
| wide band | [Link](#wideband) | Multi-frequency analysis | `processing.normalization` | Planning |
| narrow band | [Link](#narrowband) | Signal quality validation | `processing.noise_floor` |  Planning  |
| voice services | [Link](#voice-services) | Network sync & metrics | `processing.correlation` |  Planning |
---

## WIDEBAND

**Focus:** Multi-frequency spectrum analysis, bandwidth optimization, cross-band correlation

### Starting Point
1. **First Read:** [README.md - wide band section](README.md###-Wideband)
3. **Implementation Notebook:** `src/example-campaign_nodes.ipynb`

### Code Modules
```
src/
├── libs/data_request.py           ← Campaign data fetching
├── processing/
│   ├── normalization.py           ← Cross-band PSD alignment
│   ├── correlation.py             ← Band-to-band correlation
│   └── noise_floor.py             ← Per-band baseline
└── visualization/
    ├── spectrum_plots.py          ← Multi-band comparison plots
    └── correlation_heatmaps.py    ← Band relationship visualization
```

### Key Workflows

#### 1. Load Campaign Data Across Bands
```python
from libs.data_request import DataRequest

dr = DataRequest()

# Multiple campaigns = multiple frequency bands
campaigns = {
    'band_240hz': 1,      # 240 Hz RBW
    'band_960hz': 2,      # 960 Hz RBW
    'band_1920hz': 3      # 1920 Hz RBW
}

# Load for all 11 nodes
node_ids = list(range(1, 12))
multi_band_data = dr.load_campaigns_and_nodes(campaigns, node_ids)

# Structure: {band_name: {node_name: DataFrame}}
```

#### 2. Normalize Spectra Across Bands
```python
from processing.normalization import SpectrumNormalizer

# Each band has different gain, RBW
# Normalize to common reference
normalizer = SpectrumNormalizer(reference_band='band_960hz')
normalized = normalizer.normalize_all_bands(multi_band_data)
```

#### 3. Analyze Band Correlation
```python
from processing.correlation import CorrelationAnalyzer

analyzer = CorrelationAnalyzer()

# Find which bands are correlated
band_correlation = analyzer.compute_band_correlation(normalized)

# Example output:
# {
#   ('band_240hz', 'band_960hz'): 0.87,   ← Strong correlation
#   ('band_240hz', 'band_1920hz'): 0.42,  ← Weak
#   ('band_960hz', 'band_1920hz'): 0.91   ← Very strong
# }
```

### Testing Requirements
- [ ] `tests/test_normalization.py` — Verify normalization consistency
- [ ] `tests/test_correlation.py` — Band correlation calculations
- [ ] Coverage: ≥ 80%

### Code Quality Standards
- Follow [developStandars.md](docs/reference/developStandars.md)
- Type hints on all functions
- Docstring: Google format
- Commits: `feat(banda-ancha): ...`

### Documentation to Write
- **Frequency band reference table** (what each band measures)
- **Normalization methodology** (mathematical basis)
- **Interpretation guide** (how to read band correlation results)

### Timeline
- Week 1: Core normalization module
- Week 2: Correlation analysis
- Week 3: Testing & visualization

---

## NARROWBAND

**Focus:** Signal quality validation, IQ distortion, noise floor calibration, software compensation

### Starting Point
1. **First Read:** [README.md - narrow band section](README.md###-Narrowband)
2. **Real-time Notebook implementation:** `src/example-realtime.ipynb`

### Code Modules 
```
src/
├── processing/                    ← YOUR PRIMARY RESPONSIBILITY
│   ├── noise_floor.py            ← WEEK 1 - Implement
│   │   ├── NoiseFloorEstimator
│   │   └── SNR calculation
│   │
│   ├── iq_calibration.py         ← WEEK 2 - Implement
│   │   ├── IQImbalanceParams
│   │   └── IQCalibrator
│   │
│   ├── gain_tuning.py            ← WEEK 2 - Implement
│   │   ├── GainTuner
│   │   └── Automatic AGC
│   │
│   ├── synchronization.py        ← WEEK 3 - Implement
│   │   ├── SynchronizationValidator
│   │   └── Node alignment
│   │
│   └── anomaly_detection.py      ← WEEK 3 - Implement
│       ├── AnomalyDetector (ML)
│       └── Feature extraction
│
├── pipeline.py                    ← Orchestrates all modules
└── visualization/
    └── quality_dashboard.py       ← Your quality metrics plots
```

### CRITICAL: The 5 Signal Quality Challenges

Read [the main maintenance readme](../README.md) PHASES SECTION FOR understand implementation about:
1. **Orthogonal Signal** (Noise floor too high)
2. **IQ Distortion** (Phase & amplitude imbalance)
3. **Inadequate Power** (Gain not optimized)
4. **Synchronization** (Multi-node timing)
5. **Anomalies** (Exogenous events not detected)

### Implementation Checklist (3 Weeks)

#### WEEK 1: Noise Floor & SNR
- [ ] Implement `NoiseFloorEstimator.histogram_mode()`
- [ ] Implement `NoiseFloorEstimator.estimate_snr()`
- [ ] Create per-node baseline (save to database)
- [ ] Unit tests: `tests/test_noise_floor.py`
- [ ] Validation: All nodes SNR > 20 dB
- **Deliverable:** Noise floor report

#### WEEK 2: IQ Calibration & Gain Tuning
- [ ] Implement `IQCalibrator.estimate_imbalance()`
- [ ] Implement `IQCalibrator.correct_iq_samples()`
- [ ] Implement `GainTuner.estimate_optimal_gain()`
- [ ] Implement `GainTuner.validate_no_clipping()`
- [ ] Unit tests: `tests/test_iq_*.py`, `tests/test_gain_*.py`
- [ ] Validation: Phase < 5°, gain < 2 dB, no clipping
- **Deliverable:** Calibration profiles, before/after plots

#### WEEK 3: Sync & Anomalies
- [ ] Implement `SynchronizationValidator`
- [ ] Implement `AnomalyDetector` (with ML training)
- [ ] Create `SpectrumQualityPipeline` orchestrator
- [ ] Create quality dashboard visualization
- [ ] Integration tests: `tests/test_pipeline.py`
- [ ] Achieve 80% overall code coverage
- **Deliverable:** Complete pipeline, metrics dashboard

### Daily Validation Checklist
```
Before Each Commit:
- [ ] pytest tests/test_*.py --cov=src/processing
- [ ] All tests pass
- [ ] Coverage > 80%
- [ ] black src/processing/
- [ ] mypy src/processing/
- [ ] Commit message: feat(banda-angosta): ...
```

###  Database Structure (NEW)
Design and implement:
```python
# Noise floor baseline per node
{
    'node_id': int,
    'measurements': [float],  # dB values
    'mean': float,
    'std': float,
    'timestamp': datetime,
    'context': str  # environment description
}

# IQ calibration parameters per node/band
{
    'node_id': int,
    'frequency_mhz': float,
    'phase_error_deg': float,
    'gain_imbalance_db': float,
    'dc_offset_i': float,
    'dc_offset_q': float,
    'timestamp': datetime
}

# Anomaly model state
{
    'model_version': str,
    'training_samples': int,
    'contamination_rate': float,
    'feature_names': [str],
    'pickle_file': str  # Serialized sklearn model
}
```

### Testing Strategy
- **Unit tests:** Each function isolated, fixtures in `tests/conftest.py`
- **Integration tests:** Full pipeline on sample data
- **Performance tests:** < 2 seconds for 6 nodes × 104 records
- **Validation tests:** Against laboratory reference measurements

### Documentation You Must Write
- **Noise floor estimation**: Why histogram mode? When to use percentile method?
- **IQ calibration procedures**: Step-by-step hardware + software compensation
- **Gain tuning algorithm**: Design logic, parameters, lookup tables
- **Synchronization protocol**: How to detect and fix clock drift
- **Anomaly detection model**: Feature engineering, training procedure, interpretation

### Code Review Checklist (For PRs)
Your PR to `main` must have:
- [ ] All tests passing
- [ ] ≥ 80% coverage for new code
- [ ] Every function has Google-style docstring
- [ ] Type hints on all parameters & returns
- [ ] No hardcoded values (use config or constants)
- [ ] Before/after documentation (for algorithmic changes)

### Slack/Meeting Talking Points
- Daily: Brief 5-min standup on blockers
- Twice weekly: 15-min sync with Banda Ancha team (correlation work)
- Weekly: Overall quality scores across all 11 nodes

---

##  VOICE SERVICES

**Focus:** Network synchronization, quality metrics, multi-node anomaly correlation

###  Starting Point
1. **First Read:** [README.md - Voice Services Section](README.md###voice-Services)
2. **Notebook implementation** tests or implementation about network sync, quality metrics, node anomaly correlation 
### Your Code Modules
```
src/
├── processing/
│   ├── correlation.py             ← Node synchronization
│   ├── synchronization.py         ← Timestamp alignment
│   └── anomaly_detection.py       ← Multi-node anomalies
│
├── services/                       ← YOUR DOMAIN
│   ├── quality_metrics.py         ← Compute service-level metrics
│   ├── alert_system.py            ← Alert thresholds & notifications
│   └── reporting.py               ← Quality reports for management
│
└── dashboards/
    └── voice_quality_dashboard.py ← Real-time monitoring
```

### Key Workflows

#### 1. Monitor Network Health
```python
from services.quality_metrics import VoiceQualityMonitor

monitor = VoiceQualityMonitor(log=log)

# Continuously check quality across all nodes
metrics = monitor.compute_network_metrics(
    node_ids=range(1, 12),
    time_window_minutes=15
)

# Example output:
# {
#   'overall_snr_db': 23.5,
#   'sync_error_ms': 0.3,
#   'nodes_healthy': 11,
#   'anomalies_detected': 0,
#   'timestamp': datetime.now()
# }

# Save for dashboard
monitor.publish_metrics(metrics)
```

#### 2. Detect Service Degradation
```python
from services.alert_system import AlertManager

alert_mgr = AlertManager(
    snr_threshold_db=20,
    sync_error_threshold_ms=1.0,
    clipping_threshold_pct=0.1
)

# Alert if metrics exceed thresholds
alert_mgr.check_and_alert(metrics)

# Alerts published to:
# - Email (team)
# - Slack (channel)
# - Database (historical log)
```

#### 3. Generate Quality Reports
```python
from services.reporting import QualityReporter

reporter = QualityReporter()

# Weekly report
report = reporter.generate_report(
    start_date='2026-03-05',
    end_date='2026-03-12',
    nodes=range(1, 12)
)

# Export formats
reporter.export_pdf('weekly_report.pdf', report)
reporter.export_csv('metrics.csv', report)
reporter.publish_to_web('https://ane-internal.gov.co/reports/v0.2.0')
```

### Team Assignments - USE TICKETS ¿? 
- **Person A:** Network synchronization implementation
- **Person B:** Quality metrics calculation & database
- **Person C:** Alert system & thresholds
- **Person D:** Reporting & dashboard UI

### Testing Requirements
- [ ] `tests/test_quality_metrics.py`
- [ ] `tests/test_alert_system.py`
- [ ] `tests/test_reporting.py`
- [ ] Integration test: End-to-end monitoring workflow

### Timeline
- Week 1: Setup & data collection pipeline
- Week 2: Metrics engine & thresholds
- Week 3: Alerts & reporting, dashboard



---

## CROSS-TEAM COLLABORATION MATRIX

| Activity | Banda Ancha | Banda Angosta | Voz | API |
|----------|------------|--------------|-----|-----|
| **Data loading** | ✓ | ✓ | ✓ | ✓ |
| **Signal processing** | ✓ | ✓✓ | ✓ | PROVIDES |
| **Quality metrics** | ✓ | ✓✓ | ✓✓ | EXPOSES |
| **Testing** | ALL | ALL | ALL | ALL |
| **Documentation** | wideband docs | narrowband docs | voice services docs | API docs |


**Key Dependencies:**
1. API Deployment team → All others (provides infrastructure)
2. Narrow band → wide band (signal quality affects band analysis)
3. 3 MODULES → (feeds metrics system)
4. Voz → API (exposes quality endpoints)

---



## SUCCESS METRICS (BY ROLE)

###  wide band
- [ ] 2 normalization algorithms implemented
- [ ] Band correlation analysis working
- [ ] Integration tests passing (80%+ coverage)
- [ ] Documentation: "Band Analysis Guide" published

###  narrow band
- [ ] All 5 signal quality challenges addressed
- [ ] Per-node baseline established
- [ ] Quality score > 85 for all nodes
- [ ] Documentation: "Software Validation Guide" published

###  voice services
- [ ] Quality metrics API endpoint deployed
- [ ] Alert system detects degradation (< 5 min latency)
- [ ] Weekly quality report generated
- [ ] Dashboard updated in real-time

###  API Deployment
- [ ] REST API running with 99.9% uptime
- [ ] Docker image pushed to registry
- [ ] CI/CD pipeline fully automated
- [ ] API documentation (OpenAPI) complete

---

## TROUBLESHOOTING

### I don't know where to start
1. Find your role above 
2. read "Starting Point"
3. Read first priority document
4. Join role-specific Slack channel

### My PR was rejected - what now?
See the [Code Review Checklist](#code-review-checklist-for-prs) in your role section.

### I found a bug in someone else's code
- Report in #ane2-bugs Slack channel
- @mention the responsible team
- Reference CODE_AUDIT.md if security-related

---

_**Last Updated:** March 18, 2026_
