# ANE2-SDR- QA 
> Python toolkit for SDR sensor network calibration & RF spectrum monitoring with **better measurement quality signal**.


**Version:** 0.2.0-dev 



| **Objective:** Improve RF signal quality through systematic calibration and anomaly detection across  distributed sensors given the [DataBase-RF-FM-88MHz-108MHz-Bogota-Funza](database/Database-FM-10Nodes/) database.

---


## PROJECT OBJECTIVE & GOAL

> **Better Measurement Quality Signal** 

Current challenges affecting signal quality:
-  **High Noise Floor (Piso de Ruido Alto)** — Cannot distinguish genuine signals
-  **IQ Distortion (Asymmetry)** — Signal representation artifacts  
-  **Inadequate Power Levels (Potencia Inadecuada)** — Gain tuning suboptimal
-  **Synchronization Issues** — Multi-node timing misalignment
- **Exogenous Events** — Unclassified anomalies in data

**Solution:** Systematic calibration + parameter tuning + quality validation

---

## CORE TOPICS & INFERENCE

### 1. **SENSOR CALIBRATION**
Given a specific context (RF environment), place sensors under defined circumstances and evaluate performance.

**Implementation:**
- Per-node baseline calibration against reference standards
- Gain profile mapping: frequency → optimal LNA/VGA settings
- Statistical inference on PSD distribution (non-linear models)

**Outcome:** Consistent measurement baseline across 11 sensors

### 2. **QUALITY EMISSIONS (RF)**
Validate transmitted RF quality meets specifications.

**Implementation:**
- IQ calibration/compensation (hardware + software)
- Spectral mask validation
- Power level conformance testing

**Outcome:** Certified, high-fidelity RF measurements

### 3. **SIGNAL PROCESSING ALGORITHMS**
Optimize SDR configuration parameters for signal acquisition.

**Inputs:** 
- Pxx (Power Spectral Density) [shape: 4096]
- Spatial location (latitude, longitude)
- Reference database parameters

**Outputs:**
- Optimized RBW, span, sample rate
- Noise floor estimate
- Signal quality metrics

### 4. **IQ DISTORTION HANDLING**
Detect and compensate for I/Q phase and amplitude imbalance.

**Methods:**
- Hardware-level: Pre-acquisition correction (attenuate DC offset, balance amplifiers)
- Software-level: Post-acquisition compensation (numerical correction)
- Validation: Cross-check symmetry across frequency bins

### 5. **BETTER ACQUISITION CONDITIONS**
Through empirical testing, establish optimal measurement setup.

**Steps:**
- Categorize measurement errors (electronic design, parameter tuning, exogenous events)
- Document parameter dictionary: context → settings
- Build ML classifier for signal state recognition

---

## REPOSITORY STRUCTURE


```bash
QualityAssurance/
├── README.md                          [This file: Main entry point; purpose, scope, structure]
├── DICTIONARY.md                      [Canonical variable definitions (~25 fields)]
├── DB/
│   └── Database-FM-10Nodes/           [11 sensors × 104 records × multiple campaigns
│       └── Measurements               [timestamp, pxx[4096], location, config]
│       ├── DataAcq.ipynb              [ data acquisition  & preprocessing RULES]
│       ├── Database-RF-FM-88...-RF/   [Per-node CSVs (10 nodes)]
│       └── README.md                  [DB structure & ingestion guide]
├── docs/                              [basis documentation & referece notes]
│   └── notes/  
│       ├── notes.md                   [reference notes]
│   └── reference/
│       ├── referenceDocs.md           [Modular reference docs]
├── modules                            [Modular QA implementations]
│   ├── wideband                       [validate Multi-freq spectrum, noise floor]
│       ├── dataAcq/               [calibration directory for each module]
│       ├── preprocessing/                [validation directory for each module]
│   ├── narrowband                     [IQ calibration, spectral masks]
│       ├── dataAcq/               [calibration directory for each module]
│       ├── preprocessing/                [validate IQ calibration, spectral masks]
│   └── voice service                  [Sync checks, service metrics]
│       ├── dataAcq/               [calibration directory for each module]
│       ├── preprocessing/                [validate Sync checks, service metrics]
├── validators/                        [Schema & unit checkers]
│   ├── schema.py                      [Validate CSV/Dict structure]
│   └── units.py                       [Enforce unit consistency]
└── tests/                             [Unit & integration tests]
    ├── test_schema.py
    ├── test_routes.py
    └── test_integration.py
```

---


## QUICK START 

### *for each module*
- [Getting Started](#installation)
- [Configuration Guide](#environment-configuration-env)
- [architecture by modules](docs/reference/architectureModules.md)
- [Development Standards](docs/reference/developStandars.md)  
- [Code Audit Report](CODE_AUDIT.md)
- [deployment Guide](docs/reference/deploymentGuide.md)

then, see the - [role base guide](ROLE-BASED_GUIDE.md).


###  **Wideband**
**Focus:** Multi-frequency analysis, bandwidth optimization, spectrum mapping  

**Key Modules:**
- `data_request.DataRequest` — Campaign data fetching
- `processing.normalization` — Cross-band spectrum alignment
- `visualization.spectrum_plots` — Multi-node comparisons

---

###  **Narrowband**
**Focus:** Signal quality validation, IQ distortion, noise floor calibration  

**Key Modules:**
- `processing.iq_calibration` — I/Q compensation (TBD)
- `processing.noise_floor` — Baseline estimation
- `processing.correlation` — Signal interference analysis

---

###  **Voice Services**
**Focus:** Network synchronization, quality metrics, anomaly detection  

**Key Modules:**
- `processing.correlation` — Node synchronization  
- Anomaly detection (ML-based, TBD)
- Quality reporting dashboard (TBD)

---


## ARCHITECTURE & DATA FLOW

### Current State
```
┌─────────────────────────────────────────────────────────┐
│ REST API (rsm.ane.gov.co:12443/api and DB)              │
│ → Campaign data & Real-time signals                     │
└──────────────────────┬──────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│ DATA LAYER (libs/data_request.py)                       │
│ → Fetch & parse signals into pandas DataFrames          │
└──────────────────────┬──────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│ NOTEBOOKS (Jupyter interactive analysis)                │
│  No modular processing pipeline Yet (to implement)      │
└──────────────────────┬──────────────────────────────────┘
```

### Target State (v0.2.0)
```
┌─────────────────────────────────────────────────────────┐
│ REST API                                                 │
└───────────────────────┬─────────────────────────────────┘
                        │
┌───────────────────────▼─────────────────────────────────┐
│ rf_spectrum.data.APIClient + SignalParser               │
│ → Pure HTTP + data transformation                        │
└───────────────────────┬─────────────────────────────────┘
                        │
┌───────────────────────▼─────────────────────────────────┐
│ rf_spectrum.data.Validator                              │
│ → Data integrity checks, anomaly detection               │
└───────────────────────┬─────────────────────────────────┘
                        │
┌───────────────────────▼─────────────────────────────────┐
│ rf_spectrum.processing (PIPELINE)                        │
│ ├── NoiseFloorEstimator                                  │
│ ├── IQCalibrator                                         │
│ ├── Normalizer                                           │
│ └── CorrelationAnalyzer                                  │
└───────────────────────┬─────────────────────────────────┘
                        │
┌───────────────────────▼─────────────────────────────────┐
│ rf_spectrum.visualization                               │
│ ├── SpectrumPlots                                        │
│ └── CorrelationHeatmaps                                  │
└─────────────────────────────────────────────────────────┘
```
## INSTALLATION

### Prerequisites
- **Python 3.11+**
- **pip** (or conda)
- **Git**

### Linux / macOS
```bash
git clone https://github.com/.git
cd clonning project
bash install.sh              # Create venv, install deps
source venv/bin/activate
python test/main.py           # Test installation
```

### Windows (PowerShell)
```powershell
git clone https://github.com/.git
cd clonning project
.\install.ps1                # Create .venv, install deps
.\venv\Scripts\Activate.ps1
python src/main.py           # Test installation
```

### Both platform install scripts perform:
1. Create virtual environment (`venv/` or `.venv/`)
2. Upgrade pip/setuptools
3. Install `requirements.txt` dependencies
4. Create `.env` from `.env.example` (if missing)
5. Print system diagnostics

## ENVIRONMENT CONFIGURATION (`.env`)

Create `.env` in project root:

```bash
# API CONFIGURATION
API_URL=https://rsm.ane.gov.co:12443/api

# APPLICATION
APP_NAME=ANE2-QA 
APP_VERSION=0.2.0
COUNTRY=America/Bogota

# DEBUG & LOGGING
DEBUG=false              # Enable detailed logging
VERBOSE=true             # Print all log levels
DEVELOPMENT=false        # Development mode features
```

**Configuration Priority:**
1. Environment variables (`.env` file)
2. Hardcoded defaults in `src/cfg.py`
3. CLI arguments (TBD)

---

## CONFIGURATION MODULE (`cfg.py`)
...

---


## DEVELOPMENT PROCESS

### Standard Workflow

```
┌─── New Feature ────┐
│                    │
├→ Create branch: git checkout -b feature/nombre-descriptivo
├→ Implement (follow developStandars.md)
├→ Test (≥80% coverage per testingProtocol.md)
├→ Create PR (code review required)
├→ Merge to main
└→ Tag release
```

### Code Quality Standards (from developStandars.md)

**Python Code:**
- **PEP 8:** Max 100 chars/line
- **Type Hints:** 100% coverage (PEP 484)
- **Docstrings:** Google format, every function
- **Naming:** snake_case for functions/vars, UPPER_CASE for constants

**Testing:**
- **Minimum Coverage:** 80%
- **Test Types:** Unit + integration + performance
- **Before Commit:** `pytest`, `black`, `flake8`, `mypy`

**Commits:**
- Format: `feat(module): description` (Conventional Commits)
- Atomic commits with clear messages
- No secrets in commits (use .gitignore)

### More Details
See [Full Development Standards](docs/reference/developStandars.md)

---

## TESTING & QUALITY ASSURANCE

From [Testing Protocol](docs/reference/developStandars.md):

### Quick Commands
```bash
# Run all tests with coverage
pytest tests/ --cov=src --cov-report=html

# Check code style
black --check src/
flake8 src/
mypy src/

# Before pushing
pytest && black src/ && flake8 src/
```

---

## DEPLOYMENT & RELEASES

From [Deployment Guide](docs/reference/deploymentGuide.md):

### Release Checklist
- [ ] All tests passing (`pytest tests/ --cov=src`)
- [ ] Code coverage ≥ 80%
- [ ] Documentation updated
- [ ] `CHANGELOG.md` updated
- [ ] Version bumped in `setup.py`
- [ ] Git tag created: `git tag v0.2.0`
- [ ] Release notes in GitHub

### Deploy Steps
```bash
git checkout main && git pull
pytest tests/ --cov=src       # Verify coverage
python setup.py sdist bdist_wheel
git tag v0.2.0 && git push origin v0.2.0
```

---

## CONTRIBUTING

See [workFlowProtocol.md](docs/reference/deploymentGuide.md) for:
- Branch naming conventions
- Pull request process
- Code review checklist
- Release versioning

**Quick Branch Checklist:**
```bash
git checkout -b feature/your-feature
# ... make changes, test locally ...
pytest tests/ && black src/ && flake8 src/  # Validate
git push origin feature/your-feature
# → Create PR on GitHub
```

---

## PROJECT TIMELINE (3 Weeks)

### Week 1: Refactor Architecture and data acquisiton 
- [ ] Restructure code into `src/rf_spectrum/` package
- [ ] Implement data validation layer
- [ ] Fix environment configuration
- [ ] Target: v0.1.1 (hotfix release)

### Week 2: Signal Processing
- [ ] Implement noise floor estimation
- [ ] Add IQ calibration modules
- [ ] Create quality metrics
- [ ] Add anomaly detection (ML)
- [ ] Target: v0.2.0-beta

### Week 3: Testing & Integration
- [ ] Achieve 80% test coverage
- [ ] Create integrated pipeline
- [ ] Document role-based paths
- [ ] Final release: v0.2.0


---



## DOCUMENTATION ROADMAP

| Document | Status | Purpose |
|----------|--------|---------|
| [architectureModules.md](docs/reference/architectureModules.md) |  Complete | Module design + API reference |
| [developStandars.md](docs/reference/developStandars.md) |  In progress | Code quality standards |
| [deploymentGuide.md](docs/reference/deploymentGuide.md) |  Complete | Release & deployment |
| [CODE_AUDIT.md](CODE_AUDIT.md) | Complete | Full code audit + quality issues |

---



## SUPPORT by MODULES

**About:**
-  **wide band** → See `GCPDS\ANE2\maintenance\signalQualityValidation\QA(qualityAssurance)\modules\spectrum\wideBand`
-  **narrow band** → See `GCPDS\ANE2\maintenance\signalQualityValidation\QA(qualityAssurance)\modules\spectrum\narrowBand\`
-  **voice services** → See `GCPDS\ANE2\maintenance\signalQualityValidation\QA(qualityAssurance)\modules\voiceService\`
-  **API Deployment** → See `GCPDS\ANE2\software\`
-  **Code Audit Issues** → See `signalQualityValidation/CODE_AUDIT.md`

---

## VALIDATION & QUALITY CHECKLIST

### To Achieve **Better Measurement Quality Signal**

 **Signal Quality Validation:**
- [ ] Noise floor estimated (< -100 dB for your band)
- [ ] IQ balance checked (phase error < 5°, amplitude < 2 dB)
- [ ] Gain automatically optimized per signal strength
- [ ] Node synchronization verified (< 1ms misalignment)
- [ ] Anomalies detected and classified (ML model to update) 

 **Data Integrity:**
- [ ] clear and concise dictionary for principal reference 
- [ ] 100% CSV validation before processing (REVIEW LECTURE SigMF-BINARIO)
- [ ] PSD values in realistic range ([-120, -40] dB) (MANAGE DATA RANGE)
- [ ] No missing or corrupted measurements
- [ ] Cross-node consistency verified

 **System Performance:**
- [ ] Full analysis completes < 2 seconds (6 nodes, 104 records)
- [ ] Memory usage < 500 MB (to define)
- [ ] Reproducibility guaranteed (fixed seeds, logged parameters)

---

## VERSION HISTORY

**v0.2.0** (In Progress - March 2026)
-  Signal quality improvement framework
-  Refactored architecture
-  IQ calibration module (TBD)
-  80% test coverage
- [Full Audit Report](CODE_AUDIT.md)

**v0.1.0** (Initial Release)
- Basic data request API
- Campaign/real-time example notebooks
- Simple configuration management

---

Last Updated: March 18, 2026 | Status: **Active Development**

