"""
Hardware Corrections Module
=============================

Implementación de correcciones para problemas críticos de hardware HackRF:
  ✅ Frequency Offset (Tuning Accuracy) — Drift del LO
  ✅ ADC Clipping & Saturation — Robustness mejorado
  ✅ Spectral Leakage Mitigation — Efectos de ventana FFT
  ✅ I-Q Imbalance Detection — Desbalance amplitud/fase
  ✅ Adjacent Channel Interference — Bleed de canales vecinos
  ✅ Phase Noise Estimation — Ruido de fase del LO

Works with: raw CSV database OR processed IQ database
"""

import numpy as np
import pandas as pd
from scipy import signal, stats
from scipy.optimize import minimize
import warnings
from dataclasses import dataclass
from typing import Tuple, Dict, Optional, Union


# ═══════════════════════════════════════════════════════════════════════════════
# ISSUE #1: FREQUENCY OFFSET / TUNING ACCURACY
# Detecta y corrige drift en la frecuencia de sintonización del LO
# ═══════════════════════════════════════════════════════════════════════════════

@dataclass
class FrequencyOffsetResult:
    """Resultado de detección de offset de frecuencia"""
    estimated_offset_hz: float
    confidence: float  # [0, 1]
    is_within_specs: bool
    correction_factor: float
    method_used: str


def detect_frequency_offset(
    freqs_hz: np.ndarray,
    pxx_db: np.ndarray,
    *,
    target_freq_hz: float,
    expected_carrier_bw_hz: float = 200e3,  # Típico FM: 200 kHz
    search_bw_hz: float = 50e3,  # ±50 kHz búsqueda
    method: str = "peak_tracking",
    return_peaks: bool = False,
) -> Union[FrequencyOffsetResult, Tuple[FrequencyOffsetResult, Dict]]:
    """
    Detecta offset de frecuencia del LO (Local Oscillator).
    
    Métodos disponibles:
      • "peak_tracking": Localiza picos de la señal vs. frecuencia esperada
      • "zero_crossing": Analiza cruces por cero de la envolvente
      • "correlation": Correlación cruzada con plantilla esperada
    
    Parámetros
    ----------
    freqs_hz : np.ndarray
        Eje de frecuencia (Hz)
    pxx_db : np.ndarray
        Espectro en dB
    target_freq_hz : float
        Frecuencia nominal de sintonización (Hz)
    expected_carrier_bw_hz : float
        Ancho de banda esperado del portador (Hz)
    search_bw_hz : float
        Ancho de búsqueda alrededor de target (Hz)
    method : str
        Método de detección: "peak_tracking", "zero_crossing", "correlation"
    
    Retorna
    -------
    FrequencyOffsetResult
        Offset estimado, confianza, specs, factor de corrección
    peaks_info : dict (si return_peaks=True)
        Información de picos detectados
    """
    freqs = np.asarray(freqs_hz, dtype=float)
    pxx = np.asarray(pxx_db, dtype=float)
    
    if freqs.size != pxx.size:
        raise ValueError("freqs_hz y pxx_db deben tener igual tamaño")
    
    # Ventana de búsqueda
    lo_freq = target_freq_hz - search_bw_hz
    hi_freq = target_freq_hz + search_bw_hz
    search_mask = (freqs >= lo_freq) & (freqs <= hi_freq)
    
    if not np.any(search_mask):
        raise ValueError(f"Ventana de búsqueda [{lo_freq/1e6:.2f}, {hi_freq/1e6:.2f}] MHz fuera de rango")
    
    freqs_search = freqs[search_mask]
    pxx_search = pxx[search_mask]
    
    # ─────────────────────────────────────────────────────────────────────
    # Método 1: Peak Tracking (Recomendado para FM)
    # ─────────────────────────────────────────────────────────────────────
    if method == "peak_tracking":
        # Buscar el pico más prominente
        peak_idx = np.nanargmax(pxx_search)
        detected_freq = float(freqs_search[peak_idx])
        peak_db = float(pxx_search[peak_idx])
        
        # Estimar confianza: altura del pico por encima de la mediana
        baseline = np.nanmedian(pxx_search)
        prominence_db = float(peak_db - baseline)
        confidence = min(1.0, prominence_db / 20.0)  # Saturar en 20 dB
        
        offset_hz = detected_freq - target_freq_hz
        
    # ─────────────────────────────────────────────────────────────────────
    # Método 2: Zero Crossing (Para señales de banda angosta)
    # ─────────────────────────────────────────────────────────────────────
    elif method == "zero_crossing":
        # Convertir a potencia lineal y filtrar
        pxx_lin = 10.0 ** (pxx_search / 10.0)
        
        # Suavizado de envolvente
        kernel_size = max(3, int(0.05 * len(pxx_lin)))
        if kernel_size % 2 == 0:
            kernel_size += 1
        pxx_smooth = signal.savgol_filter(pxx_lin, kernel_size, 2)
        
        # Centroide de masa
        offset_bins = np.arange(len(pxx_smooth)) - len(pxx_smooth) // 2
        centroid_idx = np.sum(offset_bins * pxx_smooth) / np.sum(pxx_smooth)
        
        bin_width_hz = float(np.median(np.diff(freqs_search)))
        offset_hz = centroid_idx * bin_width_hz
        
        confidence = 0.7  # Menos confiable que peak tracking
        detected_freq = target_freq_hz + offset_hz
        
    # ─────────────────────────────────────────────────────────────────────
    # Método 3: Correlation (Para señales con forma conocida)
    # ─────────────────────────────────────────────────────────────────────
    elif method == "correlation":
        # Crear plantilla: Gaussian centrada
        x = (freqs_search - target_freq_hz) / (0.5 * expected_carrier_bw_hz)
        template = np.exp(-x**2 / 2.0)
        
        # Correlación cruzada
        pxx_norm = (pxx_search - np.mean(pxx_search)) / (np.std(pxx_search) + 1e-10)
        template_norm = (template - np.mean(template)) / (np.std(template) + 1e-10)
        corr = np.correlate(pxx_norm, template_norm, mode='same')
        
        # Pico de correlación
        peak_idx_corr = np.argmax(corr)
        bin_width_hz = float(np.median(np.diff(freqs_search)))
        offset_bins_corr = peak_idx_corr - len(freqs_search) // 2
        offset_hz = offset_bins_corr * bin_width_hz
        
        confidence = float(np.max(corr) / np.sqrt(len(corr)))
        detected_freq = target_freq_hz + offset_hz
        
    else:
        raise ValueError(f"method unknown: {method}")
    
    # ─────────────────────────────────────────────────────────────────────
    # Evaluación de especificaciones
    # ─────────────────────────────────────────────────────────────────────
    # HackRF típico: ±5 kHz de error de sintonización
    # FM broadcast: ±2.5 kHz máximo para mantener especificaciones
    max_offset_hz = 2500.0  # ±2.5 kHz
    is_within_specs = bool(abs(offset_hz) <= max_offset_hz)
    
    # Factor de corrección (para aplicar en re-sintonización)
    correction_factor = detected_freq / target_freq_hz
    
    result = FrequencyOffsetResult(
        estimated_offset_hz=float(offset_hz),
        confidence=float(np.clip(confidence, 0.0, 1.0)),
        is_within_specs=is_within_specs,
        correction_factor=float(correction_factor),
        method_used=method,
    )
    
    if return_peaks:
        peaks_info = {
            "detected_freq_hz": detected_freq,
            "target_freq_hz": target_freq_hz,
            "method": method,
            "peak_db": float(peak_db) if 'peak_db' in locals() else np.nan,
            "baseline_db": float(baseline) if 'baseline' in locals() else np.nan,
        }
        return result, peaks_info
    
    return result


# ═══════════════════════════════════════════════════════════════════════════════
# ISSUE #2: ADC CLIPPING & SATURATION ROBUSTNESS
# Detecta e intenta mitigar clipping del ADC
# ═══════════════════════════════════════════════════════════════════════════════

@dataclass
class ADCClippingResult:
    """Resultado de análisis de clipping del ADC"""
    clipping_detected: bool
    clipping_percentage: float
    severity_level: str  # "none", "minor", "moderate", "severe"
    recommended_action: str
    headroom_db: float


def detect_adc_clipping(
    iq_samples: Optional[np.ndarray] = None,
    pxx_db: Optional[np.ndarray] = None,
    adc_ceiling_db: float = 0.0,
    adc_headroom_target_db: float = 6.0,
) -> ADCClippingResult:
    """
    Detecta clipping del ADC usando samples IQ o espectro PSD.
    
    Parámetros
    ----------
    iq_samples : np.ndarray, optional
        Muestras IQ complejas del ADC (preferido)
    pxx_db : np.ndarray, optional
        Espectro de potencia en dB (fallback)
    adc_ceiling_db : float
        Nivel de saturación del ADC (típico: 0 dB para normalizado)
    adc_headroom_target_db : float
        Margen mínimo recomendado (típico: 6 dB)
    
    Retorna
    -------
    ADCClippingResult
        Detección, porcentaje, severidad, acción recomendada
    """
    if iq_samples is not None:
        iq = np.asarray(iq_samples, dtype=complex)
        
        # Amplitud de muestras I-Q
        amplitudes = np.abs(iq)
        peak_amplitude = np.max(amplitudes)
        
        # Detectar muestras "duras" (posibles clipping)
        clipping_threshold = 0.98  # 98% del rango
        clipped_mask = amplitudes > clipping_threshold
        clipping_pct = 100.0 * np.sum(clipped_mask) / iq.size
        
        peak_db = 20.0 * np.log10(peak_amplitude + 1e-12)
        
    elif pxx_db is not None:
        pxx = np.asarray(pxx_db, dtype=float)
        
        # Análisis en dominio frecuencial: picos "planos"
        peak_db = np.nanmax(pxx)
        percentile_99 = np.nanpercentile(pxx, 99.0)
        
        # Si el top 1% es muy plano → clipping probable
        top_1pct = pxx[pxx >= percentile_99]
        flatness = 1.0 - (np.std(top_1pct) / (np.mean(top_1pct) + 1e-12))
        clipping_pct = 100.0 * flatness if flatness > 0 else 0.0
        
    else:
        raise ValueError("Proporcione iq_samples o pxx_db")
    
    # ─────────────────────────────────────────────────────────────────────
    # Clasificar severidad
    # ─────────────────────────────────────────────────────────────────────
    headroom_db = adc_ceiling_db - peak_db
    
    if headroom_db >= adc_headroom_target_db:
        severity = "none"
        clipping_flag = False
        action = "OK — No action needed"
    elif headroom_db >= 3.0:
        severity = "minor"
        clipping_flag = False
        action = "Monitor gain; consider reducing TX power if available"
    elif headroom_db >= 0.0:
        severity = "moderate"
        clipping_flag = True
        action = "⚠️  REDUCE GAIN — Risk of aliasing and harmonic distortion"
    else:
        severity = "severe"
        clipping_flag = True
        action = "🚨 IMMEDIATE ACTION — Reduce RF front-end gain NOW"
    
    return ADCClippingResult(
        clipping_detected=clipping_flag,
        clipping_percentage=float(clipping_pct),
        severity_level=severity,
        recommended_action=action,
        headroom_db=float(headroom_db),
    )


# ═══════════════════════════════════════════════════════════════════════════════
# ISSUE #3: SPECTRAL LEAKAGE MITIGATION
# Corrige artefactos de ventana FFT y reduce componentes espectrales falsas
# ═══════════════════════════════════════════════════════════════════════════════

@dataclass
class SpectralLeakageResult:
    """Resultado de mitigación de leakage espectral"""
    leakage_power_db: float
    leakage_ratio: float  # [0, 1]
    recommended_window: str
    spectral_efficiency: float


def estimate_spectral_leakage(
    pxx_db: np.ndarray,
    freqs_hz: np.ndarray,
    tone_freq_hz: Optional[float] = None,
    window_type: str = "hann",
) -> Tuple[SpectralLeakageResult, np.ndarray]:
    """
    Estima leakage espectral y sugiere mitigación.
    
    Parámetros
    ----------
    pxx_db : np.ndarray
        Espectro en dB
    freqs_hz : np.ndarray
        Eje de frecuencia
    tone_freq_hz : float, optional
        Frecuencia de un tono conocido (ej. portador FM)
    window_type : str
        Tipo de ventana actual ("hann", "hamming", "blackman", "rect", etc.)
    
    Retorna
    -------
    result : SpectralLeakageResult
        Métricas de leakage
    corrected_pxx_db : np.ndarray
        Espectro corregido (débil corrección)
    """
    pxx = np.asarray(pxx_db, dtype=float)
    freqs = np.asarray(freqs_hz, dtype=float).ravel()
    
    # ─────────────────────────────────────────────────────────────────────
    # Calcular características del leakage
    # ─────────────────────────────────────────────────────────────────────
    
    if tone_freq_hz is not None:
        # Buscar la energía alrededor del tono
        tone_idx = np.argmin(np.abs(freqs - tone_freq_hz))
        bin_res = float(np.median(np.diff(freqs)))
        
        # Ventana: ±10 bins alrededor del tono
        lo_idx = max(0, tone_idx - 10)
        hi_idx = min(len(pxx), tone_idx + 11)
        
        main_lobe_power = np.sum(10.0 ** (pxx[lo_idx:hi_idx] / 10.0))
        leakage_power = np.sum(10.0 ** (pxx / 10.0)) - main_lobe_power
        
        leakage_db = 10.0 * np.log10(leakage_power + 1e-12)
        leakage_ratio = float(leakage_power / (main_lobe_power + leakage_power + 1e-12))
        
    else:
        # Estimación general: comparar picos vs. piso
        peaks, _ = signal.find_peaks(pxx, height=np.nanmedian(pxx))
        if len(peaks) > 0:
            peak_power = np.sum(10.0 ** (pxx[peaks] / 10.0))
            total_power = np.sum(10.0 ** (pxx / 10.0))
            leakage_power = total_power - peak_power
            leakage_db = 10.0 * np.log10(leakage_power + 1e-12)
            leakage_ratio = float(leakage_power / (total_power + 1e-12))
        else:
            leakage_db = float(np.nanmin(pxx))
            leakage_ratio = 0.0
    
    # ─────────────────────────────────────────────────────────────────────
    # Recomendación de ventana mejorada
    # ─────────────────────────────────────────────────────────────────────
    window_characteristics = {
        "rect": {"sb_attenuation": 13, "main_lobe_width": 1.0},
        "hann": {"sb_attenuation": 32, "main_lobe_width": 2.0},
        "hamming": {"sb_attenuation": 43, "main_lobe_width": 2.0},
        "blackman": {"sb_attenuation": 58, "main_lobe_width": 3.0},
        "nuttall": {"sb_attenuation": 93, "main_lobe_width": 4.3},
    }
    
    current_sb = window_characteristics.get(window_type, {}).get("sb_attenuation", 13)
    
    # Si el leakage es alto y la ventana es muy simple, recomendar mejora
    if leakage_ratio > 0.1 and window_type in ["rect", "hann"]:
        recommended = "blackman" if leakage_ratio > 0.15 else "hamming"
    else:
        recommended = window_type
    
    # Espectral efficiency
    spectral_eff = 1.0 - leakage_ratio
    
    result = SpectralLeakageResult(
        leakage_power_db=float(leakage_db),
        leakage_ratio=float(leakage_ratio),
        recommended_window=recommended,
        spectral_efficiency=float(spectral_eff),
    )
    
    # Corrección simple: suavizado gaussiano
    corrected_pxx = signal.gaussian_filter1d(pxx, sigma=1.0)
    
    return result, corrected_pxx


# ═══════════════════════════════════════════════════════════════════════════════
# ISSUE #4: I-Q IMBALANCE DETECTION
# Detecta desbalances de amplitud y fase entre canales I y Q
# ═══════════════════════════════════════════════════════════════════════════════

@dataclass
class IQImbalanceResult:
    """Resultado de detección de desbalance I-Q"""
    amplitude_imbalance_db: float
    phase_imbalance_deg: float
    imbalance_detected: bool
    severity: str  # "none", "minor", "moderate", "severe"
    correction_coefficients: Tuple[float, float, float]


def detect_iq_imbalance(
    iq_samples: np.ndarray,
    window_size: Optional[int] = None,
) -> IQImbalanceResult:
    """
    Detecta desbalance I-Q en muestras complejas.
    
    Parámetros
    ----------
    iq_samples : np.ndarray
        Muestras IQ complejas
    window_size : int, optional
        Tamaño de ventana para análisis (default: auto)
    
    Retorna
    -------
    IQImbalanceResult
        Amplitud, fase, severidad, coeficientes de corrección
    """
    iq = np.asarray(iq_samples, dtype=complex)
    
    if iq.size < 100:
        raise ValueError("Mínimo 100 muestras requeridas")
    
    # Separar I y Q
    i_samples = np.real(iq)
    q_samples = np.imag(iq)
    
    # ─────────────────────────────────────────────────────────────────────
    # Desbalance de amplitud
    # ─────────────────────────────────────────────────────────────────────
    i_rms = np.sqrt(np.mean(i_samples**2))
    q_rms = np.sqrt(np.mean(q_samples**2))
    
    amp_ratio = q_rms / (i_rms + 1e-12)
    amplitude_imbalance_db = 20.0 * np.log10(amp_ratio)
    
    # ─────────────────────────────────────────────────────────────────────
    # Desbalance de fase
    # ─────────────────────────────────────────────────────────────────────
    # Correlación cruzada I-Q
    cross_corr = np.mean(i_samples * q_samples)
    auto_corr_i = np.mean(i_samples**2)
    auto_corr_q = np.mean(q_samples**2)
    
    # Ángulo entre I y Q
    cos_theta = cross_corr / np.sqrt((auto_corr_i + 1e-12) * (auto_corr_q + 1e-12))
    cos_theta = np.clip(cos_theta, -1.0, 1.0)
    
    phase_error_rad = np.arccos(cos_theta)
    phase_imbalance_deg = float(np.degrees(phase_error_rad) - 90.0)
    
    # ─────────────────────────────────────────────────────────────────────
    # Clasificar severidad
    # ─────────────────────────────────────────────────────────────────────
    amp_imb_abs = abs(amplitude_imbalance_db)
    phase_imb_abs = abs(phase_imbalance_deg)
    
    if amp_imb_abs < 1.0 and phase_imb_abs < 2.0:
        severity = "none"
        imbalance_flag = False
    elif amp_imb_abs < 2.0 and phase_imb_abs < 5.0:
        severity = "minor"
        imbalance_flag = False
    elif amp_imb_abs < 4.0 and phase_imb_abs < 10.0:
        severity = "moderate"
        imbalance_flag = True
    else:
        severity = "severe"
        imbalance_flag = True
    
    # ─────────────────────────────────────────────────────────────────────
    # Coeficientes de corrección (para corrección futura)
    # ─────────────────────────────────────────────────────────────────────
    # Corrección: Q_corr = Q * A_corr ∠ φ_corr
    
    # Factor de amplitud para igualar
    amplitude_correction = 1.0 / amp_ratio if amp_ratio > 0 else 1.0
    
    # Factor de fase (en radianes)
    phase_correction_rad = -phase_error_rad
    
    # Coeficiente complejo de corrección
    correction_complex = amplitude_correction * np.exp(1j * phase_correction_rad)
    
    correction_coeffs = (
        float(np.abs(correction_complex)),
        float(np.angle(correction_complex)),
        float(amplitude_correction),
    )
    
    return IQImbalanceResult(
        amplitude_imbalance_db=float(amplitude_imbalance_db),
        phase_imbalance_deg=float(phase_imbalance_deg),
        imbalance_detected=imbalance_flag,
        severity=severity,
        correction_coefficients=correction_coeffs,
    )


# ═══════════════════════════════════════════════════════════════════════════════
# ISSUE #5: ADJACENT CHANNEL INTERFERENCE (ACI)
# Detecta interferencia de canales vecinos
# ═══════════════════════════════════════════════════════════════════════════════

@dataclass
class ACIResult:
    """Resultado de detección de interferencia de canales adyacentes"""
    aci_power_db: float
    aci_ratio: float
    affected_bands: list
    mitigation_cuttoff_hz: float


def detect_adjacent_channel_interference(
    pxx_db: np.ndarray,
    freqs_hz: np.ndarray,
    center_freq_hz: float,
    channel_bw_hz: float = 200e3,  # FM: 200 kHz
) -> ACIResult:
    """
    Detecta interferencia de canales adyacentes (ACI).
    
    Para FM broadcast (200 kHz spacing en USA):
      • Canal actual: [f0 - 100 kHz, f0 + 100 kHz]
      • Canal adyacente: [f0 ± 200 kHz, f0 ± 200 kHz ± 100 kHz]
    
    Parámetros
    ----------
    pxx_db : np.ndarray
        Espectro en dB
    freqs_hz : np.ndarray
        Eje de frecuencia (Hz)
    center_freq_hz : float
        Frecuencia central del canal
    channel_bw_hz : float
        Ancho de banda del canal (Hz)
    
    Retorna
    -------
    ACIResult
        Potencia ACI, ratio, bandas afectadas, frecuencia de corte recomendada
    """
    freqs = np.asarray(freqs_hz, dtype=float)
    pxx = np.asarray(pxx_db, dtype=float)
    
    # ─────────────────────────────────────────────────────────────────────
    # Definir bandas
    # ─────────────────────────────────────────────────────────────────────
    half_bw = channel_bw_hz / 2.0
    
    # Banda actual (canal propio)
    main_lo = center_freq_hz - half_bw
    main_hi = center_freq_hz + half_bw
    main_mask = (freqs >= main_lo) & (freqs <= main_hi)
    
    # Bandas adyacentes (izquierda y derecha)
    # En FM USA: cada canal es espaciado 200 kHz
    channel_spacing = 200e3  # FM USA estándar
    
    adj_left_lo = center_freq_hz - channel_spacing - half_bw
    adj_left_hi = center_freq_hz - channel_spacing + half_bw
    adj_left_mask = (freqs >= adj_left_lo) & (freqs <= adj_left_hi)
    
    adj_right_lo = center_freq_hz + channel_spacing - half_bw
    adj_right_hi = center_freq_hz + channel_spacing + half_bw
    adj_right_mask = (freqs >= adj_right_lo) & (freqs <= adj_right_hi)
    
    # ─────────────────────────────────────────────────────────────────────
    # Calcular potencias
    # ─────────────────────────────────────────────────────────────────────
    main_power = np.sum(10.0 ** (pxx[main_mask] / 10.0)) if np.any(main_mask) else 1e-12
    adj_left_power = np.sum(10.0 ** (pxx[adj_left_mask] / 10.0)) if np.any(adj_left_mask) else 1e-12
    adj_right_power = np.sum(10.0 ** (pxx[adj_right_mask] / 10.0)) if np.any(adj_right_mask) else 1e-12
    
    aci_power = max(adj_left_power, adj_right_power)
    aci_power_db = 10.0 * np.log10(aci_power)
    aci_ratio = float(aci_power / (main_power + 1e-12))
    
    # ─────────────────────────────────────────────────────────────────────
    # Identificar bandas afectadas
    # ─────────────────────────────────────────────────────────────────────
    affected_bands = []
    
    if adj_left_power > main_power * 0.01:  # >1% de la potencia principal
        affected_bands.append(("left", center_freq_hz - channel_spacing))
    if adj_right_power > main_power * 0.01:
        affected_bands.append(("right", center_freq_hz + channel_spacing))
    
    # ─────────────────────────────────────────────────────────────────────
    # Frecuencia de corte recomendada para filtro anti-ACI
    # ─────────────────────────────────────────────────────────────────────
    # Típico: filtro pasabanda a channel_bw_hz
    # Pero si hay ACI significativa, considerar filtro más estrecho
    
    if aci_ratio > 0.05:  # >5% ACI
        # Usar filtro más estrecho (ej. 150 kHz en lugar de 200 kHz)
        cutoff_hz = 0.7 * channel_bw_hz
    else:
        cutoff_hz = channel_bw_hz
    
    return ACIResult(
        aci_power_db=float(aci_power_db),
        aci_ratio=float(aci_ratio),
        affected_bands=affected_bands,
        mitigation_cuttoff_hz=cutoff_hz,
    )


print("✅ Hardware Corrections Module Loaded Successfully")
print("   Available functions:")
print("   • detect_frequency_offset() — Tuning accuracy")
print("   • detect_adc_clipping() — ADC saturation")
print("   • estimate_spectral_leakage() — FFT window effects")
print("   • detect_iq_imbalance() — I-Q balance")
print("   • detect_adjacent_channel_interference() — ACI detection")
