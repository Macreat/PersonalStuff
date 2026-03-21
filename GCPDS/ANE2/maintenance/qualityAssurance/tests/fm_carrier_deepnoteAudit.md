# 📋 Audit: FM Carrier Frequency Estimation Framework
**Notebook:** `fm_carrier_deepnote.ipynb`  
**Sensor Target:** ANE6  
**Compliance Standards:** ITU-R BS.450-3, ITU-R SM.328-11, ETSI EN 302 018-1, MINTIC Res. 415/2010  
**Date Audited:** 2026-03-21

---

## 🎯 Executive Summary

This notebook implements a **comprehensive RF signal analysis and compliance testing framework** for FM radio frequency carriers. It combines:
- **PSD parsing** from CSV measurements  
- **Spectral peak detection** with sub-bin interpola tion  
- **Quality metrics** (SNR, prominence, confidence scoring)  
- **Regulatory compliance** assessment (MINTIC ±2 kHz tolerance)  
- **Batch multi-node analysis** with aggregated reporting  
- **Rich visualizations** and data export (CSV/JSON)

The framework is production-ready, well-structured, and designed for Deepnote (cloud-based Jupyter environment).

---

## 📐 Core Conceptual Implementations

### 1. **Three-Tier Data Architecture**

The notebook defines three core data structures:

#### 1.1 `PSDRecord` (Data Input Layer)
```python
@dataclass
class PSDRecord:
    sensor_id, timestamp, latitude, longitude
    freq_hz: np.ndarray          # Frequency axis
    power_dbm: np.ndarray        # Power spectrum values
    freq_resolution_hz: float    # Calculated from median(diff(freq))
    (metadata: freq range, power range, raw CSV headers)
```
**Purpose:** Encapsulates one complete PSD sweep with full metadata.  
**Key Design:** Stores raw numpy arrays to minimize data transformation overhead.

#### 1.2 `CarrierEstimate` (Estimation Output Layer)
```python
@dataclass
class CarrierEstimate:
    # Identification
    sensor_id, timestamp, nominal_freq_hz
    # Estimation
    estimated_freq_hz, peak_bin_freq_hz, deviation_hz, deviation_khz, deviation_ppm
    # Spectral Shape
    spike_prominence_db, spike_width_hz, snr_db, noise_floor_dbm
    # Quality
    confidence, confidence_flags
    # Compliance
    compliance_status, tolerance_hz, notes
```
**Purpose:** Complete measurand set for a single detected carrier.  
**Key Design:** All values pre-rounded to appropriate precision for export. Flags captured separately for structured analysis.

#### 1.3 `FrameworkConfig` (Tunable Parameters)
```python
@dataclass
class FrameworkConfig:
    fm_band_min_hz = 87.5e6          # Band limits
    channel_raster_hz = 100e3         # ITU-R 100 kHz grid
    min_prominence_db = 5.0           # Detection threshold
    min_peak_distance_hz = 100e3      # Peak separation
    interpolation_method = "quadratic"  # Sub-bin method
    noise_guard_band_hz = 200e3       # Guard for noise floor
    frequency_tolerance_hz = 2000.0   # MINTIC ±2 kHz limit
    warning_threshold_hz = 1000.0     # Soft warning ±1 kHz
```
**Purpose:** Single source of truth for all algorithm parameters.  
**Key Design:** Avoids scattered magic numbers; enables batch parameter sweeps.

---

### 2. **CSV Parser with Regex Header Extraction**

**Function:** `parse_psd_csv(path: Path) → PSDRecord`

**Pattern Matching Strategy:**
- Regex patterns for Spanish/Unicode variants (e.g., `Número de puntos`, `Frecuencia mínima`)
- Flexible date parsing with multiple format fallbacks
- UTF-8 BOM handling and no-break space normalization

**Two-Phase Parsing:**
1. **Header Phase:** Extract metadata (sensor_id, timestamp, lat/lng, freq/power ranges)  
2. **Data Phase:** Accumulate frequency/power pairs into numpy arrays  

**Robustness Features:**
- Graceful handling of missing data (uses defaults or calculated values)
- Automatic frequency resolution inference from median bin spacing
- Validation gate: raises error if <2 data points

**Output Quality:** Parsed PSD indexed by frequency with associated metadata for full context.

---

### 3. **Peak Detection & Sub-Bin Interpolation**

**Two-Stage Frequency Estimation:**

#### 3.1 Coarse Detection
```python
def detect_fm_carriers(psd, cfg):
    # Restrict to FM band [87.5, 108.0] MHz
    # Apply find_peaks with:
    #   - prominence threshold (default 5 dB)
    #   - minimum distance (default 100 kHz)
    # Return bin indices of peaks
```

**Logic:** Uses SciPy's peak finder to locate local maxima with prominence filtering.  
**Robustness:** Falls back to full sweep if no FM-band data detected.

#### 3.2 Sub-Bin Interpolation (Three Selectable Methods)
All methods refine the bin estimate using local neighborhood samples.

##### **Quadratic (Default)**
```
Point cloud: (x ∈ [-hw, hw], y = power_dbm[idx+x])
Fit: y = ax² + bx + c
Offset: -b/(2a) ∈ [-hw, hw]
Frequency: freq_hz[peak_idx] + offset × freq_resolution_hz
```
**Advantages:** Fast, stable, good for narrow peaks.  
**Limitations:** Assumes parabolic shape; poor for asymmetric profiles.

##### **Gaussian**
```
Fit: y(x) = a·exp(-0.5·((x-μ)/σ)²)
Estimate: μ (peak offset from bin center)
Uses: scipy.optimize.curve_fit with bounded search window
```
**Advantages:** Better for broadband/asymmetric peaks.  
**Limitations:** More compute; can fail to converge on noise.

##### **Centroid (Linear Average)**
```
Offset = Σ(x · 10^((p[x]-p[idx])/10)) / Σ(10^(...))
(Power-weighted center-of-mass in linear scale)
```
**Advantages:** Robust to noise; no fitting required.  
**Limitations:** Biased toward larger neighborhood.

**Cross-Method Comparison:**
| Method | Speed | Precision | Noise-Robust | Best Use Case |
|--------|-------|-----------|--------------|---------------|
| Quadratic | Fast | Good | Moderate | Narrow, clean peaks |
| Gaussian | Moderate | Excellent | Good | Broadband, asymmetric |
| Centroid | Fast | Moderate | Excellent | Very noisy data |

---

### 4. **Spectral Metrics Extraction**

**Function:** `compute_spectral_metrics(psd, peak_idx, cfg) → dict`

#### 4.1 Prominence (Peak Height Above Baseline)
```
SciPy find_peaks → properties["prominences"][i]
Fallback: manual estimation (peak_power - min(surrounding))
```
**Significance:** Indicates how distinct the carrier is from noise.

#### 4.2 Width at -3 dB (Spectral Occupancy)
```python
scipy.signal.peak_widths(power_dbm, [peak_idx], rel_height=0.5)
# rel_height=0.5 → width at half-height-and-depth
```
**Significance:** Correlates with modulation bandwidth and filtering.

#### 4.3 Noise Floor (Local Baseline)
```
Guard band: ±200 kHz around peak
Window: ±500 kHz from guard band edge
Noise floor = median(power in guard + window regions)
```
**Logic:** Avoids sidebands and adjacent channels; uses robust median.

#### 4.4 Signal-to-Noise Ratio
```
SNR_dB = peak_power_dbm - noise_floor_dbm
```
**Interpretation:**
- SNR > 20 dB → Full confidence
- SNR < 10 dB → LOW_SNR flag, reduced confidence

---

### 5. **Confidence Scoring (Weighted Multi-Factor)**

**Function:** `compute_confidence(snr_db, prominence_db, width_hz, freq_res, cfg) → (float, [flags])`

**Score Composition:**
```
confidence = 0.45 × s_snr + 0.40 × s_prom + 0.15 × s_sharp

where:
  s_snr   = min(1, max(0, snr_db / 20))           [45% weight]
  s_prom  = min(1, max(0, prominence_db / 10))   [40% weight]
  s_sharp = max(0, 1 - max(0, width - 5*res)/(10*res))  [15% weight, penalizes wide peaks]
```

**Flag Generation Rules:**
- `LOW_SNR(X dB)` if s_snr < 0.5  
- `LOW_PROMINENCE(X dB)` if s_prom < 0.5  
- `WIDE_SPIKE(X kHz)` if width > 5× frequency resolution  

**Output Range:** [0, 1] — unitless scalar with diagnostic breadcrumbs.

**Design Rationale:**
- **45% SNR:** Strongest indicator of signal validity  
- **40% Prominence:** Prevents false positives on noise floor ripples  
- **15% Sharpness:** Penalizes broadband interference; favors narrow channels  

---

### 6. **Compliance Assessment Framework**

**Function:** `assess_compliance(deviation_hz, cfg) → str`

```
IF dev_hz is None:
  → UNVERIFIABLE (carrier not mapped to ITU-R channel)
ELSE IF |dev_hz| ≤ 1.0 kHz:
  → COMPLIANT ✅ (within soft warning threshold)
ELSE IF |dev_hz| ≤ 2.0 kHz:
  → WARNING ⚠️ (within MINTIC hard limit, but drifting)
ELSE:
  → VIOLATION 🚨 (exceeds frequency tolerance)
```

**Mapping to ITU-R Channels:**
```python
def nearest_channel(est_hz, cfg):
    ch = round((est_hz - 87.5e6) / 100e3)  # Quantize to grid
    nom = 87.5e6 + ch × 100e3
    return nom if |est_hz - nom| ≤ 5×tolerance else None
```
**Logic:** Finds nearest 100 kHz FM raster; returns None if too far (>±10 kHz).

**Deviation Computation:**
```
deviation_hz  = estimated_freq - nominal_freq
deviation_khz = deviation_hz / 1000
deviation_ppm = deviation_hz / nominal_freq × 1e6
```

---

### 7. **Batch Multi-Node Processing Pipeline**

**Architecture:**
```
CSV files → load_node_psd() → {node_name: {freqs_mhz, pxx, n_sweeps, ...}}
  ↓
run_batch_detection() → scan all nodes → detect peaks → estimate each
  ↓
pd.DataFrame (summary_df) — one row per detected carrier
  ↓
Aggregation → carrier_summary() → per-channel cross-node stats
           → node_summary()    → per-node QA metrics
```

#### 7.1 Temporal Averaging (Per-Node)
```python
def load_node_psd(csv_path):
    # 1. Read CSV → DataFrame
    # 2. Parse all 'pxx' cells → list of arrays
    # 3. Filter to mode length (most common array size)
    # 4. Vertical stack: (n_sweeps × n_bins)
    # 5. Average in linear scale (energy averaging):
    #    pxx_avg = 10·log₁₀(mean(10^(stack/10)))
    # 6. Reconstruct frequency axis from start/end
```

**Why Linear Average?**  
Energy averaging preserves spectral structure better than dB-domain arithmetic. dB-domain averaging can suppress peaks and exaggerate noise floors.

**Output:** Single averaged PSD per node, metadata (n_sweeps, timestamp range, location).

#### 7.2 Batch Detection
```python
for each node:
  peaks ← find_peaks(pxx, prominence_min, distance_min)
  for each peak:
    estimate ← _estimate_one(node, freqs, pxx, peak_idx, cfg)
    records.append(estimate)
return pd.DataFrame(records)
```

---

### 8. **Summary Statistics & Aggregations**

#### 8.1 Per-Channel Cross-Node Summary
```python
df.groupby('nominal_mhz').agg({
    'node': 'nunique',              # Number of nodes detecting this channel
    'deviation_hz': ['mean', 'std', 'max_abs'],  # Frequency accuracy
    'snr_db': ['mean', 'min'],      # Signal quality
    'spike_prominence_db': 'mean',  # Peak distinctness
    'confidence': ['mean', 'min'],  # Measurement confidence
})
# Worst compliance status per channel
```

**Use Case:** Identifies problematic channels (high variance, low SNR, violations).

#### 8.2 Per-Node Summary
```python
df.groupby('node').agg({
    'estimated_mhz': 'count',           # Total carriers detected
    'compliance_status': value_counts,  # Compliant/Warning/Violation/Unverifiable breakdown
    'snr_db': ['mean', 'min'],          # Node-wide signal quality
    'confidence': ['mean', 'min'],      # Measurement reliability
    'deviation_hz': ['mean_abs', 'max_abs'],  # Frequency accuracy
})
```

**Use Case:** Node-level diagnostics (Is this sensor malfunctioning? Does it need recalibration?).

---

### 9. **Export & Reporting**

#### 9.1 CSV Export
```python
def export_csv(estimates, path):
    fields = [sensor_id, timestamp, nominal_freq_hz, estimated_freq_hz, ..., notes]
    # Row-by-row write → portable, importable to Excel/Pandas
```

**Fields Preserved:** 18 core fields covering identification, estimation, metrics, compliance.

#### 9.2 JSON Export
```python
{
  "metadata": {
    "sensor_id": "ANE6",
    "timestamp": "2026-03-03T19:44:01",
    "location": {lat, lng},
    "sweep_range": {freq_min_hz, freq_max_hz},
    "framework_version": "1.0.0"
  },
  "carriers": [
    {all CarrierEstimate fields as dict},
    ...
  ]
}
```

**Advantage:** Preserves nested structure; suitable for programmatic downstream analysis.

#### 9.3 Visualizations (Plotly)

##### **8.1 Full PSD with Carrier Annotations**
- X-axis: Frequency (MHz)
- Y-axis: Power (dBm)
- Vertical lines: Detected carriers, color-coded by compliance status
- Annotations: Estimated frequency, hover tooltips

##### **8.2 Per-Carrier Detail (Zoom ±500 kHz)**
- Multi-panel grid (1–3 columns)
- PSD curve + carrier line + noise floor reference
- Deviation, SNR, confidence metrics overlaid
- Color-coded by compliance

##### **8.3 Quality Metrics Dashboard**
- 3 subplots: SNR (dB), Prominence (dB), Confidence (0–1)
- Bars per carrier, color-coded by compliance
- Enables quick visual scan for poor-quality detections

---

## 🔬 Technical Strengths

1. **Modular Design**
   - Clean separation: parsing → detection → estimation → compliance → reporting
   - Easy to swap interpolation methods or adjust parameters
   - Every function has a single responsibility

2. **Regulatory Compliance Embedded**
   - MINTIC Res. 415/2010 integrated (±2 kHz hard limit)
   - ITU-R raster (100 kHz FM grid) baked in
   - Distinction between COMPLIANT (soft threshold) and WARNING (hard limit)

3. **Robust Error Handling(also coded oriented)**
   - Malformed CSV fallbacks (date parsing, string literals)
   - Missing metadata → defaults or computed values
   - Peak fitting failures → graceful degradation
   - Verbose logging for debugging
      e.g : 

         - Si malloc devuelve NULL → abortar con mensaje claro.

         - Si la metadata no tiene sample_rate → detener y reportar error.

         - Si el cálculo de log10 produce NaN → aplicar epsilon para estabilizar.


4. **Multi-Scale Analysis**
   - Single-sweep estimation
   - Batch multi-node across time
   - Per-channel and per-node aggregations
   - Cross-node statistical comparison

5. **Data Quality Metrics Built-In**
   - Confidence scoring with diagnostic flags
   - SNR, prominence, sharpness factored separately
   - Allows filtering low-quality detections

6. **Export Flexibility**
   - CSV for spreadsheet/pivot tables
   - JSON for REST APIs or data lakes
   - Plotly for interactive exploration
   - Tabulated console output for quick review

---

## ⚠️ Potential Limitations & Considerations

1. **Frequency Resolution**
   - Sub-bin interpolation assumes local parabolic/smooth structure
   - In presence of adjacent strong carriers, sideband bleed can distort estimates
   - Quadratic method biased toward symmetric peaks

2. **Confidence Scoring**
   - Weights (45%, 40%, 15%) are heuristic; no formal derivation
   - Does not account for adjacent-channel interference
   - Flags are binary (present/absent), not weighted

3. **Noise Floor Estimation**
   - Guard band (200 kHz) + window (500 kHz) fixed
   - May underestimate noise if sweep contains many strong carriers
   - Median-based; vulnerable to outliers in edges of PSD

4. **Temporal Averaging**
   - Linear-scale averaging assumes stationary spectrum
   - May alias transient bursts into noise floor
   - No outlier rejection before averaging

5. **CSV Parser**
   - Hardcoded header regex patterns; assumes Spanish field names
   - Limited date format coverage (fallback to dateutil helps)
   - No encoding detection; assumes UTF-8-sig

6. **Scaling**
   - Batch processing loads all CSVs into memory
   - For very large datasets (>1000 sweeps), consider streaming

---

## 🎓 Recommended Extensions

1. **Adaptive Noise Flooring**
   - Estimate noise floor percentile (e.g., 10th) rather than fixed guard band
   - Account for interference profile

2. **Adjacent-Channel Rejection**
   - Check if neighbor bins show strong power (i.e., sidelobe vs. independent carrier)
   - Penalize confidence if asymmetry indicates bleed

3. **Confidence Calibration**
   - Use labeled validation set to optimize weights via logistic regression
   - Generate ROC curves against ground truth

4. **Frequency Drift Analysis**
   - Trend per-node deviation over time
   - Detect drift patterns (temperature, aging effects)

5. **Automatic Interpolation Method Selection**
   - Try all three; choose based on fit residuals or peak shape metrics
   - Adaptive per peak based on local SNR

6. **Batch Visualization Dashboard**
   - Plotly Dash app for interactive filtering, drilling down by node/channel
   - Real-time summary statistics

---

## 📊 Data Flow Diagram

```
CSV Files (Raw Survey Data)
    ↓
 parse_psd_csv()
    ↓
PSDRecord {freq_hz, power_dbm, metadata}
    ├→ detect_fm_carriers() → [peak_indices]
    │   ↓
    │   interpolate_carrier() → estimated_freq_hz
    │   compute_spectral_metrics() → {prominence, width, snr, noise_floor}
    │   compute_confidence() → (score, flags)
    │   nearest_channel() → nominal_freq_hz
    │   assess_compliance() → status
    │   ↓
    ├→ CarrierEstimate {all 19 fields}
    │
    └→ [estimates] → export_csv/json → files
                  → visualizations → Plotly
                  → summaries → pd.DataFrame
                        ├→ carrier_summary() → per-channel cross-node stats
                        └→ node_summary() → per-node diagnostics
```

---

## 🔐 Code Quality Assessment

| Aspect | Rating | Notes |
|--------|--------|-------|
| **Documentation** | ⭐⭐⭐⭐⭐ | Clear docstrings, inline comments, section headers |
| **Modularity** | ⭐⭐⭐⭐⭐ | Every function has single responsibility |
| **Error Handling** | ⭐⭐⭐⭐ | Good coverage; some edge cases untested |
| **Testability** | ⭐⭐⭐⭐ | Pure functions; easy mocking with config objects |
| **Performance** | ⭐⭐⭐⭐ | Vectorized numpy; memory-efficient for moderate datasets |
| **Readability** | ⭐⭐⭐⭐⭐ | Clean variable names, logical flow |
| **Compliance** | ⭐⭐⭐⭐⭐ | Regulatory thresholds embedded correctly |

---

## ✅ Conclusion

This notebook is a **well-engineered, production-grade RF signal analysis framework** suitable for:
- ✅ Regulatory compliance testing (MINTIC, ITU-R)  
- ✅ Multi-node spectrum monitoring  
- ✅ Frequency accuracy diagnostics  
- ✅ Quality assurance workflows  
- ✅ Research & development of SDR-based sensors (HackRF One, USRP)  

**Strengths:** Modular, robust, standards-aware, with rich diagnostics and visualizations.  
**Maturity:** beta→production ready; recommend unit testing suite & validation data.

---

**Audit Signed:** GitHub Copilot | 2026-03-21
