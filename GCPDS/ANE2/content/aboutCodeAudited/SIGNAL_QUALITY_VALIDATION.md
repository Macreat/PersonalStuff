# SIGNAL QUALITY VALIDATION & IMPROVEMENT ROADMAP
**Created:** March 11, 2026 | **Focus:** Banda Angosta (Narrowband) Software Validation Path

---

## EXECUTIVE SUMMARY

From the 070326Lecture.md session notes, the project identified **5 main signal quality challenges** that must be addressed to achieve **better measurement quality signal**. This document maps those challenges to code issues and provides systematic validation strategies.

---

## 1. SIGNAL QUALITY CHALLENGES MAPPING

### Challenge #1: Orthogonal Signal Between Ports (Ortogo nal signal between pot → close to noise)

**Issue:** 
- Signals measured between sensor ports are orthogonal
- Values too close to noise floor, making detection unreliable
- Anomalies/exogenous events are hidden in measurement noise

**Root Cause Analysis:**
- No noise floor estimation implemented
- Gain settings not optimized for signal strength
- No SNR (Signal-to-Noise Ratio) calculation
- Baseline measurement drift not tracked

**Current Code Status:** ❌ NOT IMPLEMENTED

**Code Entry Point:** `src/libs/data_request.py` → Raw PSD loaded without analysis

**Implementation Plan:**

```python
# STEP 1: Implement Noise Floor Estimator
# File: src/processing/noise_floor.py

class NoiseFloorEstimator:
    """Estimate baseline noise from PSD measurements."""
    
    @staticmethod
    def histogram_mode(pxx: np.ndarray, nbins='sturges') -> float:
        """
        Estimate noise floor using histogram mode (Sturges' rule).
        
        Theory:
        - Lowest bins represent noise
        - Highest frequency bin = mode = noise floor
        - Works for complex spectra with signals
        
        Args:
            pxx: Power array, shape (4096,) in dB
            
        Returns:
            Noise floor estimate in dB
        """
        hist, edges = np.histogram(pxx, bins=nbins)
        # Mode is the bin with highest count
        mode_idx = np.argmax(hist)
        return edges[mode_idx]
    
    @staticmethod
    def percentile_method(pxx: np.ndarray, percentile=10) -> float:
        """
        Estimate noise using low-percentile value.
        Assumes bottom 10% of power values are noise.
        """
        return np.percentile(pxx, percentile)
    
    @staticmethod
    def estimate_snr(pxx: np.ndarray, signal_band: tuple) -> float:
        """
        Calculate Signal-to-Noise Ratio.
        Args:
            pxx: Power array
            signal_band: Tuple (start_idx, end_idx) of signal region
        Returns:
            SNR in dB
        """
        noise_floor = NoiseFloorEstimator.histogram_mode(pxx)
        signal_power = np.max(pxx[signal_band[0]:signal_band[1]])
        return signal_power - noise_floor
```

**Validation Checklist:**
- [ ] Noise floor < -100 dB for Banda Angosta
- [ ] SNR > 20 dB for detectable signals
- [ ] Noise floor stable across time (σ < 2 dB)
- [ ] Per-node baseline documented

---

### Challenge #2: IQ Distortion & Signal Asymmetry (Distorsión IQ - Simetry)

**Issue:**
- I/Q phase imbalance causes asymmetric spectrum representation
- Amplitude gain imbalance reduces effective measurement range
- Hardware-level DC offset not compensated

**Root Cause Analysis:**
- No IQ calibration implemented
- DC offset removal not performed
- ADC/mixer issues not characterized

**Current Code Status:** ❌ NOT IMPLEMENTED

**Code Entry Point:** `src/libs/data_request.py` → Raw IQ samples not processed

**Implementation Plan:**

```python
# STEP 2: Implement IQ Calibration
# File: src/processing/iq_calibration.py

from dataclasses import dataclass
import numpy as np

@dataclass
class IQImbalanceParams:
    """Estimated I/Q imbalance parameters."""
    phase_error_deg: float  # Degrees
    gain_imbalance_db: float  # dB
    dc_offset_i: float  # Counts
    dc_offset_q: float  # Counts

class IQCalibrator:
    """Detect and compensate for I/Q imbalance."""
    
    @staticmethod
    def estimate_imbalance(iq_samples: np.ndarray) -> IQImbalanceParams:
        """
        Estimate I/Q phase error and gain imbalance.
        
        Method:
        - Separate I and Q components
        - Compute phase angle between them
        - Calculate amplitude ratio
        
        Args:
            iq_samples: Complex IQ samples [shape: (n_samples,)]
            
        Returns:
            IQImbalanceParams with phase error and gain imbalance
        """
        i_vals = np.real(iq_samples)
        q_vals = np.imag(iq_samples)
        
        # Remove DC offset first
        i_centered = i_vals - np.mean(i_vals)
        q_centered = q_vals - np.mean(q_vals)
        
        # Estimate phase error using angle between I and Q
        phase_error = np.arctan2(q_centered, i_centered).mean()
        phase_error_deg = np.degrees(phase_error)
        
        # Gain imbalance: ratio of RMS values
        i_rms = np.sqrt(np.mean(i_centered**2))
        q_rms = np.sqrt(np.mean(q_centered**2))
        gain_ratio = q_rms / i_rms if i_rms > 0 else 1.0
        gain_imbalance_db = 20 * np.log10(gain_ratio)
        
        return IQImbalanceParams(
            phase_error_deg=phase_error_deg,
            gain_imbalance_db=gain_imbalance_db,
            dc_offset_i=np.mean(i_vals),
            dc_offset_q=np.mean(q_vals)
        )
    
    @staticmethod
    def correct_iq_samples(
        iq_samples: np.ndarray, 
        params: IQImbalanceParams
    ) -> np.ndarray:
        """
        Apply I/Q correction to raw samples.
        
        Compensation steps:
        1. Remove DC offset
        2. Rotate by phase error
        3. Scale Q channel by gain ratio
        
        Args:
            iq_samples: Complex samples [shape: (n_samples,)]
            params: Calibration parameters
            
        Returns:
            Corrected IQ samples
        """
        # 1. DC removal
        corrected = iq_samples - (params.dc_offset_i + 1j * params.dc_offset_q)
        
        # 2. Phase rotation
        phase_correction = np.exp(-1j * np.radians(params.phase_error_deg))
        corrected = corrected * phase_correction
        
        # 3. Gain balancing
        gain_correction = 10 ** (params.gain_imbalance_db / 20)
        corrected = corrected.real + 1j * (corrected.imag / gain_correction)
        
        return corrected
```

**Validation Checklist:**
- [ ] Phase error < 5° (< 0.087 radians)
- [ ] Gain imbalance < 2 dB
- [ ] DC offset < 1% of peak signal
- [ ] Symmetry verified on corrected spectrum
- [ ] Before/after comparison plots created

---

### Challenge #3: Inadequate Power Levels (Potencia Inadecuada)

**Issue:**
- Some signals too weak to detect (below noise floor)
- Some signals saturate ADC (clipping)
- Gain settings not optimized for specific contexts
- No automatic gain control (AGC) mechanism

**Root Cause Analysis:**
- LNA/VGA gains hard-coded, not adaptive
- No feedback loop from power measurements
- No lookup table for frequency → gain mapping
- ADC saturation not detected

**Current Code Status:** ⚠️ PARTIALLY IMPLEMENTED (ConfigParams exists but unused)

**Code Entry Point:** `src/libs/data_request.py` → ConfigParams.lna_gain, vga_gain

**Implementation Plan:**

```python
# STEP 3: Implement Adaptive Gain Control
# File: src/processing/gain_tuning.py

from typing import Dict, Tuple
import numpy as np

class GainTuner:
    """Optimize LNA/VGA gains based on measured signal."""
    
    # Lookup table: frequency range → recommended gain strategy
    GAIN_PROFILES = {
        'low_power': {'lna_gain': 20, 'vga_gain': 0},
        'medium_power': {'lna_gain': 10, 'vga_gain': 10},
        'high_power': {'lna_gain': 0, 'vga_gain': 20},
    }
    
    @staticmethod
    def estimate_optimal_gain(
        measured_pxx: np.ndarray,
        target_headroom_db: float = 3.0
    ) -> Dict[str, int]:
        """
        Calculate optimal gain settings based on measured power.
        
        Strategy:
        - Measure current power level
        - Compare to target range
        - Calculate gain adjustment needed
        - Return new settings
        
        Target: Peak signal at -3dB below ADC saturation
        
        Args:
            measured_pxx: Current power spectrum [dB]
            target_headroom_db: Desired safety margin
            
        Returns:
            Dictionary with 'lna_gain' and 'vga_gain' values
        """
        peak_power = np.max(measured_pxx)
        adc_saturation = 0  # dB (full scale)
        target_peak = adc_saturation - target_headroom_db
        
        gain_error_db = target_peak - peak_power
        
        # Simple algorithm: adjust VGA by gain error
        # (VGA has finer resolution than LNA)
        if gain_error_db > 10:
            profile = 'low_power'
        elif gain_error_db < -10:
            profile = 'high_power'
        else:
            profile = 'medium_power'
        
        return GainTuner.GAIN_PROFILES[profile]
    
    @staticmethod
    def validate_no_clipping(
        iq_samples: np.ndarray,
        clip_threshold: float = 0.95
    ) -> Tuple[bool, float]:
        """
        Check if ADC is clipping (saturation).
        
        Args:
            iq_samples: Complex samples
            clip_threshold: Fraction of max value to flag as clipping (0-1)
            
        Returns:
            (is_clipping, percentage_clipped)
        """
        adc_max = np.max(np.abs(iq_samples))
        clipped_samples = np.sum(np.abs(iq_samples) > clip_threshold * adc_max)
        percentage = 100 * clipped_samples / len(iq_samples)
        
        is_clipping = percentage > 0.1  # Flag if >0.1% clipped
        return is_clipping, percentage
```

**Validation Checklist:**
- [ ] Peak signal -3 dB below saturation
- [ ] No ADC clipping detected (< 0.1%)
- [ ] Gain adjustments automatic & logged
- [ ] Lookup table covers all frequency bands
- [ ] Per-node gain profile generated

---

### Challenge #4: Synchronization Issues (Sincronización)

**Issue:**
- Centralized acquisition with timing gaps
- Multi-node timestamps misaligned
- Phase relationships across nodes lost
- Cannot correlate signals properly

**Root Cause Analysis:**
- Timestamp validation not implemented
- No NTP sync verification
- No timestamp difference checking
- Network latency not accounted for

**Current Code Status:** ❌ NOT IMPLEMENTED

**Code Entry Point:** `src/libs/data_request.py` → Timestamp field in measurements

**Implementation Plan:**

```python
# STEP 4: Implement Synchronization Validation
# File: src/processing/synchronization.py

from datetime import datetime, timedelta
import pandas as pd
import numpy as np

class SynchronizationValidator:
    """Validate and improve multi-node timestamp synchronization."""
    
    MAX_TIMESTAMP_ERROR_MS = 1.0  # 1 millisecond tolerance
    
    @staticmethod
    def validate_timestamps(df: pd.DataFrame) -> Dict[str, any]:
        """
        Check timestamp validity and drift.
        
        Args:
            df: DataFrame with 'timestamp' column (assume dtype: datetime64)
            
        Returns:
            Dictionary with validation results
        """
        if df.empty or 'timestamp' not in df.columns:
            return {'is_valid': False, 'error': 'No timestamp column'}
        
        # Calculate time deltas between consecutive samples
        time_deltas = df['timestamp'].diff().dt.total_seconds() * 1000  # Convert to ms
        
        # Expected interval (assume uniform sampling)
        expected_interval = time_deltas.median()
        
        # Find outliers (jitter)
        deviation = np.abs(time_deltas - expected_interval)
        max_deviation = deviation.max()
        jitter_samples = (deviation > SynchronizationValidator.MAX_TIMESTAMP_ERROR_MS).sum()
        
        return {
            'is_valid': max_deviation < SynchronizationValidator.MAX_TIMESTAMP_ERROR_MS,
            'max_deviation_ms': max_deviation,
            'jitter_count': jitter_samples,
            'expected_interval_ms': expected_interval,
            'total_samples': len(df)
        }
    
    @staticmethod
    def align_nodes(
        nodes_data: Dict[str, pd.DataFrame],
        tolerance_ms: float = 1.0
    ) -> Dict[str, pd.DataFrame]:
        """
        Align timestamps across multiple nodes.
        
        Strategy:
        - Find common time window
        - Interpolate to common timestamp grid
        - Drop nodes with excessive drift
        
        Args:
            nodes_data: Dictionary {node_name: DataFrame}
            tolerance_ms: Acceptable clock drift
            
        Returns:
            Dictionary with synchronized DataFrames
        """
        if not nodes_data:
            return {}
        
        # Find common time range
        min_time = max(df['timestamp'].min() for df in nodes_data.values())
        max_time = min(df['timestamp'].max() for df in nodes_data.values())
        
        # Create common timestamp grid
        first_df = next(iter(nodes_data.values()))
        time_step = first_df['timestamp'].diff().median()
        common_times = pd.date_range(min_time, max_time, freq=time_step)
        
        aligned = {}
        for node_name, df in nodes_data.items():
            # Interpolate to common grid
            aligned[node_name] = df.set_index('timestamp').reindex_with_fill_value(
                common_times, method='nearest'  # Or 'linear' for better precision
            ).reset_index()
        
        return aligned
```

**Validation Checklist:**
- [ ] Timestamp misalignment < 1 ms across nodes
- [ ] Time drift < 1 ppm (part per million)
- [ ] NTP sync verified on all nodes
- [ ] Phase coherence between nodes maintained
- [ ] Temporal correlation analysis possible

---

### Challenge #5: Exogenous Events & Anomaly Detection (Eventos Exógenos)

**Issue:**
- Anomalies not detected or classified
- No anomaly database
- Cannot distinguish electronic noise from external interference
- Signal states not modeled

**Root Cause Analysis:**
- No statistical model for PSD distribution
- No ML classifier for signal states
- No baseline behavior established
- No anomaly scoring mechanism

**Current Code Status:** ❌ NOT IMPLEMENTED

**Code Entry Point:** New module → `src/processing/anomaly_detection.py`

**Implementation Plan:**

```python
# STEP 5: Implement Anomaly Detection (ML-based)
# File: src/processing/anomaly_detection.py

from sklearn.ensemble import IsolationForest
import pandas as pd
import numpy as np

class AnomalyDetector:
    """Detect and classify anomalous spectrum measurements."""
    
    def __init__(self, contamination: float = 0.05):
        """
        Initialize anomaly detector.
        
        Args:
            contamination: Expected proportion of anomalies (5% default)
        """
        self.model = IsolationForest(contamination=contamination, random_state=42)
        self.is_trained = False
    
    @staticmethod
    def extract_features(pxx: np.ndarray) -> np.ndarray:
        """
        Extract features from power spectrum.
        
        Features:
        - Peak power
        - Spectral width (bandwidth)
        - Skewness (asymmetry)
        - Kurtosis (peakedness)
        - Entropy (signal complexity)
        
        Args:
            pxx: Power array [shape: (4096,)]
            
        Returns:
            Feature vector [shape: (5,)]
        """
        from scipy.stats import entropy, skew, kurtosis
        
        peak_power = np.max(pxx)
        spectral_width = np.std(pxx)
        skewness = skew(pxx)
        kurt = kurtosis(pxx)
        ent = entropy(pxx - np.min(pxx) + 1e-10)  # Normalize for entropy
        
        return np.array([peak_power, spectral_width, skewness, kurt, ent])
    
    def train(self, pxx_samples: np.ndarray):
        """
        Train anomaly detector on baseline measurements.
        
        Args:
            pxx_samples: Multiple power spectra [shape: (n_samples, 4096)]
        """
        features = np.array([
            self.extract_features(pxx[i]) for i in range(pxx_samples.shape[0])
        ])
        self.model.fit(features)
        self.is_trained = True
    
    def detect(self, pxx: np.ndarray) -> Dict[str, any]:
        """
        Detect anomalies in a spectrum.
        
        Returns:
            Dictionary with:
            - is_anomaly: Boolean
            - anomaly_score: -1 (anomaly) to 1 (normal)
            - confidence: Probability
        """
        if not self.is_trained:
            raise ValueError("Model must be trained first")
        
        features = self.extract_features(pxx).reshape(1, -1)
        prediction = self.model.predict(features)[0]
        anomaly_score = self.model.score_samples(features)[0]
        
        return {
            'is_anomaly': prediction == -1,
            'anomaly_score': anomaly_score,
            'confidence': abs(anomaly_score)  # Confidence in prediction
        }
```

**Validation Checklist:**
- [ ] ML model trained on 100+ baseline spectra
- [ ] Detection accuracy > 90%
- [ ] False positive rate < 5%
- [ ] Anomaly database created
- [ ] Classification tags: electronic, interference, weather-related, etc.

---

## 2. INTEGRATED VALIDATION WORKFLOW

### Complete Signal Quality Pipeline

```python
# File: src/pipeline.py
# Orchestrates all quality validation modules

class SpectrumQualityPipeline:
    """End-to-end signal quality validation."""
    
    def __init__(self, log):
        self.log = log
        self.noise_estimator = NoiseFloorEstimator()
        self.iq_calibrator = IQCalibrator()
        self.gain_tuner = GainTuner()
        self.sync_validator = SynchronizationValidator()
        self.anomaly_detector = AnomalyDetector()
    
    def validate_signal(self, iq_samples: np.ndarray, node_id: int) -> Dict:
        """
        Comprehensive signal quality check (all 5 challenges).
        
        Returns quality report with all metrics.
        """
        pxx = self._compute_psd(iq_samples)
        
        # 1. Noise floor analysis
        noise_floor = self.noise_estimator.histogram_mode(pxx)
        snr = self.noise_estimator.estimate_snr(pxx, (100, 200))
        
        # 2. IQ calibration check
        iq_params = self.iq_calibrator.estimate_imbalance(iq_samples)
        iq_corrected = self.iq_calibrator.correct_iq_samples(iq_samples, iq_params)
        
        # 3. Gain optimization
        optimal_gain = self.gain_tuner.estimate_optimal_gain(pxx)
        is_clipping, clip_pct = self.gain_tuner.validate_no_clipping(iq_samples)
        
        # 4. Synchronization (if timestamp available)
        # sync_report = self.sync_validator.validate_timestamps(df)
        
        # 5. Anomaly detection
        anomaly_result = self.anomaly_detector.detect(pxx)
        
        # Compile report
        return {
            'node_id': node_id,
            'noise_floor_db': noise_floor,
            'snr_db': snr,
            'iq_phase_error_deg': iq_params.phase_error_deg,
            'iq_gain_imbalance_db': iq_params.gain_imbalance_db,
            'adc_clipping': is_clipping,
            'clipping_percentage': clip_pct,
            'optimal_gain': optimal_gain,
            'is_anomaly': anomaly_result['is_anomaly'],
            'anomaly_score': anomaly_result['anomaly_score'],
            'quality_score': self._compute_quality_score(
                noise_floor, snr, iq_params, is_clipping, anomaly_result
            )
        }
    
    def _compute_quality_score(self, ...):
        """
        Compute overall quality score (0-100).
        
        Weighted metrics:
        - SNR > 20 dB: 100%
        - IQ phase < 5°: 100%
        - No clipping: 100%
        - Not anomaly: 100%
        """
        pass
    
    def _compute_psd(self, iq_samples):
        """Compute power spectral density from IQ samples."""
        pass
```

---

## 3. QUALITY METRICS DASHBOARD

**Proposed visualization** (for Jupyter notebooks):

```python
# Complete quality report visualization
import matplotlib.pyplot as plt

fig, axes = plt.subplots(2, 3, figsize=(15, 10))

# 1. Noise Floor Trend
axes[0, 0].plot(nodes_noise_floor)
axes[0, 0].axhline(y=-100, color='r', linestyle='--', label='Target')
axes[0, 0].set_ylabel('Noise Floor [dB]')

# 2. SNR Distribution
axes[0, 1].hist(snr_values, bins=30)
axes[0, 1].axvline(x=20, color='g', linestyle='--', label='Min acceptable')
axes[0, 1].set_xlabel('SNR [dB]')

# 3. IQ Phase Error
axes[0, 2].scatter(nodes, iq_phase_errors)
axes[0, 2].axhline(y=5, color='r', linestyle='--', label='Max 5°')
axes[0, 2].set_ylabel('Phase Error [degrees]')

# 4. Gain Optimization
axes[1, 0].bar(node_ids, vga_gains)
axes[1, 0].set_ylabel('VGA Gain [dB]')

# 5. Clipping Detection
axes[1, 1].bar(nodes[clipped], clip_pcts[clipped], color='red')
axes[1, 1].set_ylabel('Clipping %')

# 6. Anomaly Scores
axes[1, 2].scatter(nodes, anomaly_scores, c=anomaly_flags, cmap='RdYlGn')
axes[1, 2].set_ylabel('Anomaly Score')

plt.suptitle('Signal Quality Dashboard - All Nodes')
plt.tight_layout()
plt.savefig('quality_report.png', dpi=150)
```

---

## 4. VALIDATION TIMELINE & MILESTONES

### Week 1: Foundation (March 11-15, 2026)
- [ ] Implement NoiseFloorEstimator
- [ ] Create per-node noise floor baseline
- [ ] Validate SNR calculations
- **Deliverable:** Noise floor report for all 11 nodes

### Week 2: Calibration (March 18-22, 2026)
- [ ] Implement IQCalibrator
- [ ] Implement GainTuner
- [ ] Create gain lookup tables by frequency
- **Deliverable:** Calibration profiles, before/after comparison plots

### Week 3: Advanced (March 25-29, 2026)
- [ ] Implement SynchronizationValidator
- [ ] Implement AnomalyDetector with ML model
- [ ] Create quality dashboard
- [ ] Full integration test
- **Deliverable:** Complete quality validation pipeline, metrics report

---

## 5. SUCCESS CRITERIA

The project achieves its goal when:

✅ **All 5 Challenges Addressed:**
1. ✓ Orthogonal signal detection: SNR > 20 dB
2. ✓ IQ distortion compensation: Phase < 5°, gain < 2 dB  
3. ✓ Power optimization: Peak -3dB, no clipping
4. ✓ Synchronization: < 1ms misalignment across nodes
5. ✓ Anomaly detection: > 90% accuracy

✅ **Quality Metrics Met:**
- Noise floor < -100 dB (Banda Angosta)
- SNR > 25 dB (minimum)
- Full pipeline execution < 2 seconds
- 80% test coverage
- 100% documentation

✅ **Production Ready:**
- Code audit passed (CODE_AUDIT.md complete)
- All modules tested
- API deployment guide created
- Role-based documentation paths established

---

**Status:** ROADMAP READY | Next: Implement Week 1 items  
**Assigned:** Banda Angosta (Software Validation) Team
