# CODE AUDIT: ANE2-Calibration-SDR
**March , 2026**  | **Focus:** Signal Quality Measurement & Improvement
 
---

## EXECUTIVE SUMMARY

The ANE2-Calibration-SDR project aims to **improve RF signal measurement quality** across a distributed network of 11 SDR sensors (Bogotá-Funza). Current code exhibits architectural gaps, incomplete implementations, and data quality issues that prevent optimal calibration and signal processing.

**Main Objective:** Categorize and eliminate measurement errors to achieve better quality signal acquisition and processing.

---

## 1. CRITICAL ISSUES IDENTIFIED

### 1.1 Signal Quality & Calibration Issues

#### ✗ **IQ Distortion & Asymmetry**
- **Issue:** No IQ calibration/compensation mechanism implemented
- **Impact:** Asymmetrical signal representation, reduced measurement precision
- **Severity:** CRITICAL for RF measurements
- **Location:** Missing from `data_request.py` and processing pipeline
- **Solution:**
  - Implement IQ calibration module: `src/processing/iq_calibration.py`
  - Add hardware-level correction (pre-acquisition)
  - Add software-level compensation (post-acquisition)

```python
# RECOMMENDED IMPLEMENTATION
class IQCalibrator:
    """Compensate for I/Q phase and amplitude imbalance."""
    
    @staticmethod
    def estimate_iq_imbalance(pxx: np.ndarray) -> dict:
        """Estimate I/Q phase error and gain imbalance."""
        pass
    
    @staticmethod
    def correct_iq_samples(iq_samples: np.ndarray, params: dict) -> np.ndarray:
        """Apply I/Q correction to raw samples."""
        pass
```

#### ✗ **High Noise Floor (Piso de Ruido Alto)**
- **Issue:** Baseline noise not properly estimated or tracked
- **Impact:** Cannot distinguish signal from noise, poor SNR
- **Severity:** CRITICAL
- **Location:** `libs/data_request.py` loads raw PSD without noise floor analysis
- **Solution:**
  - Implement noise floor estimation: `src/processing/noise_floor.py`
  - Use histogram mode (Sturges' rule) or quantile-based methods
  - Per-node baseline calibration against reference standard

#### ✗ **Inadequate Power Levels (Potencia Inadecuada)**
- **Issue:** Gain settings (LNA, VGA) not automatically optimized
- **Impact:** Some signals too weak, others saturated
- **Severity:** HIGH
- **Current State:** Hard-coded gain values in `ConfigParams`
- **Solution:**
  - Implement adaptive gain tuning algorithm
  - Add feedback loop from power measurements
  - Create lookup table: frequency → optimal gain mapping

#### ✗ **No Power Spectral Density (PSD) Analysis**
- **Issue:** Raw PSD data loaded but not analyzed for distribution
- **Impact:** Cannot detect anomalies or model variations
- **Severity:** HIGH
- **Solution:**
  - Implement PSD statistical inference
  - Model PDF (probability density function) per node
  - Use ML to classify signal states

---

### 1.2 Architecture & Code Quality Issues

#### ✗ **Incomplete Module Architecture**
**Current State:**
```
src/
├── cfg.py           ✓ (Basic config)
├── main.py          ✓ (Entry point only)
├── libs/
│   ├── __init__.py  ✗ (Empty)
│   └── data_request.py  ⚠ (Partial - no processing)
```

**Expected State (from docs):**
```
src/
├── rf_spectrum/
│   ├── __init__.py
│   ├── config.py         ← Expand from cfg.py
│   ├── logger.py         ← Extract from cfg.py
│   ├── data/
│   │   ├── loader.py     ← Move data_request logic
│   │   └── validator.py  ← New
│   ├── processing/
│   │   ├── noise_floor.py      ← New
│   │   ├── iq_calibration.py   ← New
│   │   ├── normalization.py    ← New
│   │   └── correlation.py      ← New
│   └── visualization/
│       ├── spectrum_plots.py   ← New
│       └── correlation_plots.py ← New
├── tests/
│   ├── test_data/
│   └── test_processing/
└── notebooks/
    ├── 01_data_exploration.ipynb
    ├── 02_pipeline_demo.ipynb
    └── 03_results_analysis.ipynb
```

**Recommendations:**
- [ ] Refactor `libs/` into `src/rf_spectrum/` package structure
- [ ] Implement missing data validation layer
- [ ] Add processing pipeline orchestrator

#### ✗ **Inconsistent API Client Design**
**Location:** `src/libs/data_request.py`

**Issues:**
1. **Mixed responsibilities:**
   - HTTP requests + dataframe operations = hard to test
   - Data fetching + data parsing tightly coupled

2. **Incomplete pagination:**
   ```python
   # Current: Simple while loop, no error handling
   while True:
       data = requests.get(url, params=params, verify=False).json()
       signals.extend(data['measurements'])
       if not data.get('pagination', {}).get('has_next'):
           break
   ```
   - No timeout handling
   - No retry logic
   - No graceful degradation

3. **Hard-coded MAC addresses:**
   ```python
   self.node_macs = {
       1: 'd8:3a:dd:f7:1d:f2', 
       2: 'd8:3a:dd:f4:4e:26',
       # ... 8 more nodes
   }
   ```
   - Should be loaded from config file or database
   - No validation of MAC format

4. **Missing type hints:**
   - `get_realtime_signal()` returns `RealtimeSignal` (undefined class)
   - `get_campaign_params()` returns `CampaignParams` (no import shown)

**Recommendations:**
```python
# IMPROVED DESIGN
class APIClient:
    """Pure HTTP client - no data transformation."""
    def get_signals(self, endpoint: str) -> dict:
        """Returns raw JSON."""
        pass

class SignalParser:
    """Transform raw JSON → dataframes."""
    @staticmethod
    def parse_campaign(data: dict) -> CampaignParams:
        pass

class NodeRegistry:
    """Centralized node/MAC management."""
    def __init__(self, config_path: str):
        self.nodes = self._load_from_config(config_path)
```

---

### 1.3 Data Handling Issues

#### ✗ **No Data Validation Pipeline**
- **Issue:** Raw API data loaded directly without verification
- **Impact:** Silent failures, corrupted analyses
- **Severity:** HIGH
- **Missing:** Validation layer between fetch → process

#### ✗ **Inadequate Error Handling**
- **Location:** `data_request.py`
- **Issues:**
  - No connection timeout handling
  - No retry logic for failed requests
  - No validation of response structure
  - Silent failures in signal parsing

#### ✗ **No Database Backend**
- **Issue:** Results stored in memory/DataFrames only
- **Impact:** Cannot persist calibration data, poor reproducibility
- **Severity:** MEDIUM
- **Solution:** Implement SQLite backend for storing:
  - Calibration parameters per node
  - Historical measurements
  - Anomaly logs
  - Quality metrics

---

### 1.4 Configuration & Environment Issues

#### ⚠ **Environment Variables Not Validated**
- **Location:** `src/cfg.py`
- **Issues:**
  ```python
  API_URL = os.getenv("API_URL", "https://rsm/ane.gov.co/api")  # ← URL typo!
  ```
  - Invalid default URL (should be `rsm.ane.gov.co`)
  - No type validation (e.g., URL format)
  - Boolean parsing uses manual string comparison

#### ⚠ **Logging Configuration Too Simple**
- Only console output, no file logging
- No log rotation configured
- No structured logging (no JSON output for analysis)

#### ✗ **Missing .env Validation**
- No schema validation
- No required variable checks
- Silent defaults for critical settings

---

### 1.5 Testing & Quality Assurance

#### ✗ **No Test Suite**
- **Missing:**
  - `tests/` directory completely absent
  - No unit tests for data loading
  - No integration tests for pipeline
  - No performance benchmarks
- **Target Coverage:** ≥ 80% (from development standards)

#### ✗ **No Type Checking**
- No `mypy` configuration
- Inconsistent type hints
- Document suggests 100% type coverage, actual: <30%

#### ✗ **No Code Formatting Standards**
- No `black` configuration
- No `flake8` linting
- Inconsistent docstring format

---

### 1.6 Documentation Issues

#### ⚠ **Duplicate Documentation**
- Same content in multiple files (architectureModules.md, developStandars.md, etc.)
- No single source of truth
- Inconsistent formatting

#### ⚠ **Incomplete Notebook Examples**
- Two notebooks defined but purpose unclear:
  - `example-campaign_nodes.ipynb`
  - `example-realtime.ipynb`
- No documentation of how to use them

#### ⚠ **Missing Role-Based Documentation Paths**
- Roles mentioned but not implemented:
  - Banda Ancha
  - Banda Angosta
  - Servicio de Voz
  - API Deployment
- No clear guidance for different user types

---

## 2. ROOT CAUSE ANALYSIS

### Why Signal Quality is Suboptimal

**From 070326Lecture.md notes:**

1. **Orthogonal Signal Issues**
   - Signals measured between channels too close to noise floor
   - Cannot distinguish genuine signal from artifacts
   - **Cause:** No noise floor calibration implemented

2. **Design Electronics Problems (Problemas de Diseño Electrónico)**
   - IQ imbalance in hardware not addressed
   - DC offset not handled
   - **Cause:** No compensation mechanism in code

3. **Parameter Tuning (Parameter Tunning HW+SW)**
   - Hard-coded gains don't adapt to signal strength
   - No automatic optimization of RBW, span, sample rate
   - **Cause:** No adaptive algorithm + no parameter dictionary

4. **Synchronization Issues**
   - Centralized acquisition with timing gaps
   - Multiple nodes not phase-synchronized
   - **Cause:** No timestamp validation, no sync protocol

5. **Exogenous Events (Eventos Exógenos)**
   - Anomalies not detected or classified
   - No anomaly database
   - **Cause:** No statistical model for signal states

---

## 3. PRIORITY FIXES (3-Week Timeline)

### WEEK 1: Foundations
- [ ] Refactor code architecture (package structure)
- [ ] Implement noise floor estimation
- [ ] Add data validation layer
- [ ] Fix environment configuration

### WEEK 2: Signal Processing
- [ ] Implement IQ calibration
- [ ] Add PSD normalization
- [ ] Create signal quality metrics
- [ ] Implement anomaly detection

### WEEK 3: Testing & Integration
- [ ] Add 80% test coverage
- [ ] Create integrated pipeline
- [ ] Document role-based paths
- [ ] Deploy release v0.2.0

---

## 4. CODE METRICS SUMMARY

| Metric | Current | Target | Gap |
|--------|---------|--------|-----|
| Test Coverage | 0% | 80% | -80% |
| Docstring Coverage | ~40% | 100% | -60% |
| Type Hint Coverage | ~30% | 100% | -70% |
| Architecture Modularity | 2/8 modules | 8/8 modules | -75% |
| Error Handling | Poor (0 try/catch blocks) | Comprehensive | CRITICAL |
| Config Validation | None | Full schema validation | CRITICAL |
| Signal Processing Features | 1/5 (raw load only) | 5/5 | -80% |

---

## 5. QUALITY SIGNAL CHECKLIST

To achieve **better measurement quality signal**, verify:

- [ ] **Noise Floor**: Estimated, logged, < -100 dB for band
- [ ] **IQ Balance**: Phase error < 5°, amplitude imbalance < 2 dB
- [ ] **Gain Optimization**: Automatic adjustment per signal strength
- [ ] **Synchronization**: Timestamp misalignment < 1ms
- [ ] **Anomaly Detection**: ML model classifies signal states
- [ ] **Data Integrity**: 100% validation before processing
- [ ] **Reproducibility**: Fixes seeds, logs parameters, archival ready
- [ ] **Performance**: Full analysis < 2 seconds for 6 nodes × 104 records

---

## 6. NEXT STEPS

1. **Create session memory** with codebase improvements
2. **Implement refactored architecture** (Week 1)
3. **Add processing modules** (Week 2)
4. **Full test coverage** (Week 3)
5. **Deploy version 0.2.0** with signal quality improvements

---

**Signed:** Code Audit Agent | March 11, 2026
