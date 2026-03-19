"""
Hardware Signal Corrections Application Module
===============================================

Aplicar correcciones detectadas a datos reales del pipeline

Functions:
  - apply_frequency_offset_correction() — Corregir offset de sintonización
  - apply_adc_gain_correction() — Ajustar ganancia por clipping
  - apply_window_correction() — Reaplicar PSD con ventana optimizada
  - apply_ici_filter() — Filtro anti-interferencia entre canales
  - CorrectionPipeline — Clase para aplicar todas las correcciones
"""

import numpy as np
from dataclasses import dataclass
from typing import Tuple, Optional
from scipy import signal


@dataclass
class CorrectedSignalResult:
    """Resultado de aplicación de correcciones"""
    original_pxx: np.ndarray
    corrected_pxx: np.ndarray
    corrections_applied: dict
    snr_improvement_db: float  # SNR after vs before
    thd_improvement_pct: float  # THD reduction %
    quality_score: float  # 0-1, higher is better


class CorrectionPipeline:
    """
    Pipeline de correcciones para señales RF
    
    Aplica secuencialmente todas las correcciones detectadas.
    """
    
    def __init__(self, freqs_hz: np.ndarray):
        """
        Init
        
        Args:
            freqs_hz: Eje de frecuencias en Hz
        """
        self.freqs_hz = freqs_hz
        self.freqs_mhz = freqs_hz / 1e6
        self.resolution_hz = freqs_hz[1] - freqs_hz[0] if len(freqs_hz) > 1 else 1.0
        
    def apply_corrections(self,
                          pxx_original: np.ndarray,
                          freq_offset_hz: Optional[float] = None,
                          adc_headroom_db: Optional[float] = None,
                          recommended_window: Optional[str] = None,
                          aci_filter_cutoff_hz: Optional[float] = None) -> CorrectedSignalResult:
        """
        Aplicar cadena completa de correcciones
        
        Args:
            pxx_original: PSD original (lineal o dB)
            freq_offset_hz: Offset de frecuencia detectado
            adc_headroom_db: Headroom del ADC (si negativo = necesita corrección)
            recommended_window: Ventana FFT recomendada ('hann', 'hamming', etc)
            aci_filter_cutoff_hz: Cutoff de filtro anti-ACI
        
        Returns:
            CorrectedSignalResult con PSD corregida y métricas
        """
        
        pxx = pxx_original.copy()
        corrections_applied = {}
        
        # 1️⃣ Corrección de frequency offset
        if freq_offset_hz is not None and abs(freq_offset_hz) > 100:  # > 100 Hz es significativo
            pxx = self._correct_frequency_offset(pxx, freq_offset_hz)
            corrections_applied['frequency_offset'] = freq_offset_hz
        
        # 2️⃣ Corrección de ADC clipping
        if adc_headroom_db is not None and adc_headroom_db < 3.0:
            pxx = self._correct_adc_clipping(pxx, adc_headroom_db)
            corrections_applied['adc_clipping'] = adc_headroom_db
        
        # 3️⃣ Ventana FFT optimizada
        if recommended_window is not None:
            pxx = self._correct_windowing(pxx, recommended_window)
            corrections_applied['windowing'] = recommended_window
        
        # 4️⃣ Filtro anti-ACI
        if aci_filter_cutoff_hz is not None:
            pxx = self._apply_aci_filter(pxx, aci_filter_cutoff_hz)
            corrections_applied['aci_filter'] = aci_filter_cutoff_hz
        
        # Calcular mejoras de calidad
        snr_improvement = self._estimate_snr_improvement(pxx_original, pxx)
        thd_improvement = self._estimate_thd_improvement(pxx_original, pxx)
        quality_score = self._calculate_quality_score(pxx, corrections_applied)
        
        return CorrectedSignalResult(
            original_pxx=pxx_original,
            corrected_pxx=pxx,
            corrections_applied=corrections_applied,
            snr_improvement_db=snr_improvement,
            thd_improvement_pct=thd_improvement,
            quality_score=quality_score,
        )
    
    def _correct_frequency_offset(self,
                                  pxx: np.ndarray,
                                  freq_offset_hz: float) -> np.ndarray:
        """
        Corrección de offset de frecuencia: desplazar PSD
        
        Método: Rotación de índices (FFT shift)
        """
        
        # Desplazamiento en muestras
        shift_samples = int(np.round(freq_offset_hz / self.resolution_hz))
        
        if abs(shift_samples) > 0:
            # Rotación cíclica del espectro
            corrected = np.roll(pxx, shift_samples)
            
            # Suavizar bordes (transición cíclica)
            window = signal.windows.hamming(len(pxx))
            corrected = corrected * window + pxx * (1 - window + 1e-10)
        else:
            corrected = pxx
        
        return corrected
    
    def _correct_adc_clipping(self,
                              pxx: np.ndarray,
                              headroom_db: float) -> np.ndarray:
        """
        Corrección de clipping: reducir banda de paso
        
        Si headroom < 3 dB: comprimir dinámicamente espectro de banda estrecha
        Método: Non-linear compression (similar a AGC)
        """
        
        if headroom_db > 0:
            return pxx
        
        # Factor de compresión: mayor compresión si headroom es más negativo
        compression_factor = 1.0 - (headroom_db / -10.0)
        compression_factor = np.clip(compression_factor, 0.1, 0.8)
        
        # Normalizador soft: log-linear compression
        pxx_linear = 10 ** (pxx / 10.0)
        compressed = np.log10(pxx_linear ** compression_factor) * 10.0
        
        return compressed
    
    def _correct_windowing(self,
                          pxx: np.ndarray,
                          window_type: str) -> np.ndarray:
        """
        Corrección de leakage: aplicar ventana FFT a PSD
        
        Método: Convolve con respuesta de ventana
        """
        
        valid_windows = ['hann', 'hamming', 'blackman', 'nuttall']
        if window_type not in valid_windows:
            return pxx
        
        # Crear respuesta de ventana normalizada
        window_resp = signal.get_window(window_type, len(pxx))
        window_resp = window_resp / np.sum(window_resp)
        
        # Convolve para suavizar
        corrected = signal.convolve(pxx, window_resp, mode='same')
        
        # Preservar peak power
        if np.max(pxx) > 0:
            corrected = corrected * (np.max(pxx) / np.max(corrected))
        
        return corrected
    
    def _apply_aci_filter(self,
                         pxx: np.ndarray,
                         cutoff_hz: float) -> np.ndarray:
        """
        Filtro anti-interferencia entre canales
        
        Método: FIR bandpass alrededor de frecuencia de sintonización
        """
        
        # Crear filtro paso-banda
        nyquist = self.resolution_hz / 2.0
        if cutoff_hz > nyquist:
            return pxx
        
        normalized_cutoff = cutoff_hz / (self.resolution_hz * len(pxx) * 0.5)
        normalized_cutoff = np.clip(normalized_cutoff, 0.01, 0.99)
        
        # FIR de 51 taps centrado
        num_taps = min(51, len(pxx) // 4)
        if num_taps < 5:
            return pxx
        
        # Diseño simple: Hann window + sinc kernel
        b = signal.firwin(num_taps, normalized_cutoff, window='hann')
        
        # Aplicar (padding para evitar transientes)
        pad_size = num_taps * 2
        padded = np.pad(pxx, (pad_size, pad_size), mode='edge')
        filtered = signal.lfilter(b, 1.0, padded)
        result = filtered[pad_size:-pad_size]
        
        return result
    
    def _estimate_snr_improvement(self,
                                  pxx_original: np.ndarray,
                                  pxx_corrected: np.ndarray) -> float:
        """Estimar mejora de SNR (dB) después de correcciones"""
        
        # Aproximación: ratio de varianza de ruido estimado
        noise_original = np.median(pxx_original)  # Estimador robusto de piso de ruido
        noise_corrected = np.median(pxx_corrected)
        
        # SNR improvement ~= 20*log10(original_noise / corrected_noise)
        if noise_corrected > 0:
            snr_improvement = 20 * np.log10(max(noise_original, 1e-10) / noise_corrected)
        else:
            snr_improvement = 0.0
        
        return float(np.clip(snr_improvement, -10, 20))
    
    def _estimate_thd_improvement(self,
                                  pxx_original: np.ndarray,
                                  pxx_corrected: np.ndarray) -> float:
        """Estimación de reducción THD (%)"""
        
        # Simplificado: ratio de potencia total antes/después
        if np.sum(pxx_original) > 0:
            ratio = np.sum(pxx_corrected) / np.sum(pxx_original)
        else:
            ratio = 1.0
        
        # Convertir a reducción en %
        thd_reduction_pct = (1.0 - ratio) * 100.0
        
        return float(np.clip(thd_reduction_pct, -50, 50))
    
    def _calculate_quality_score(self,
                                pxx: np.ndarray,
                                corrections: dict) -> float:
        """
        Score de calidad 0-1
        
        Basado en:
        - Flatness del espectro (bajo ∼ menos distorsión)
        - Número de correcciones aplicadas
        - Energía normalizada
        """
        
        # Flatness: std de la derivada
        if len(pxx) > 2:
            pxx_diff = np.diff(pxx)
            flatness = 1.0 / (1.0 + np.std(pxx_diff))
        else:
            flatness = 0.5
        
        # Correction count (más correcciones ∼ más mejoramiento)
        correction_bonus = min(0.2, len(corrections) * 0.05)
        
        # Energía normalizada
        if np.max(pxx) > 0:
            energy_ratio = np.mean(pxx) / np.max(pxx)
        else:
            energy_ratio = 0.5
        
        # Score final
        score = (flatness * 0.6 + energy_ratio * 0.3 + correction_bonus * 0.1)
        
        return float(np.clip(score, 0.0, 1.0))


def apply_frequency_offset_correction(signal_time: np.ndarray,
                                      sample_rate_hz: float,
                                      freq_offset_hz: float) -> np.ndarray:
    """
    Aplicar corrección de offset de frecuencia en dominio del tiempo
    
    Args:
        signal_time: Señal temporal (IQ o real)
        sample_rate_hz: Frecuencia de muestreo en Hz
        freq_offset_hz: Offset de frecuencia en Hz (puede ser negativo)
    
    Returns:
        Señal con offset corregido
    """
    
    t = np.arange(len(signal_time)) / sample_rate_hz
    
    # Fase de corrección: -2π * offset * t
    correction_phase = -2 * np.pi * freq_offset_hz * t
    correction_phasor = np.exp(1j * correction_phase)
    
    # Aplicar: multiplicar señal por phasor de corrección
    corrected = signal_time * correction_phasor
    
    return corrected


def apply_iq_imbalance_correction(iq_samples: np.ndarray,
                                  amplitude_imbalance_db: float,
                                  phase_imbalance_deg: float) -> np.ndarray:
    """
    Aplicar corrección de imbalance I-Q
    
    Args:
        iq_samples: Muestras IQ complejas
        amplitude_imbalance_db: Imbalance de amplitud detectado (dB)
        phase_imbalance_deg: Imbalance de fase detectado (grados)
    
    Returns:
        Muestras IQ corregidas
    """
    
    # Extraer I y Q
    I = np.real(iq_samples)
    Q = np.imag(iq_samples)
    
    # Corrección de amplitud: amplificar Q si Q es debil
    if amplitude_imbalance_db > 0.1:
        amplitude_correction = 10 ** (amplitude_imbalance_db / 20.0)
        Q = Q * amplitude_correction
    
    # Corrección de fase: rotar Q respecto a I
    phase_correction_rad = np.radians(phase_imbalance_deg)
    Q_corrected = Q * np.cos(phase_correction_rad) + I * np.sin(phase_correction_rad)
    I_corrected = I * np.cos(phase_correction_rad) - Q * np.sin(phase_correction_rad)
    
    # Recombinar
    iq_corrected = I_corrected + 1j * Q_corrected
    
    return iq_corrected


# Test/Demo
if __name__ == "__main__":
    # Crear datos de prueba
    freqs_hz = np.linspace(88e6, 108e6, 512)
    t = np.arange(1000) / 2e6
    
    # PSD sintética con leakage
    window = signal.get_window('rect', 1000)  # Rectangular (máximo leakage)
    signal_time = np.sin(2*np.pi*101.1e6*t) + 0.1*np.sin(2*np.pi*88e6*t + np.pi/4)
    windowed = signal_time * window
    
    pxx = 10 * np.log10(np.abs(np.fft.fftshift(np.fft.fft(windowed)))**2 + 1e-10)
    pxx = pxx[len(pxx)//4:3*len(pxx)//4]
    freqs_hz = freqs_hz[:len(pxx)]
    
    # Inicializar pipeline
    pipeline = CorrectionPipeline(freqs_hz)
    
    # Aplicar correcciones
    result = pipeline.apply_corrections(
        pxx,
        freq_offset_hz=2500,  # 2.5 kHz offset
        adc_headroom_db=-2,  # Clipping ligero
        recommended_window='hann',  # Mejorar leakage
        aci_filter_cutoff_hz=100e3,
    )
    
    print("✅ Correction Pipeline Test")
    print(f"   Original SNR: baseline")
    print(f"   Corrected SNR improvement: {result.snr_improvement_db:+.2f} dB")
    print(f"   THD reduction: {result.thd_improvement_pct:.1f}%")
    print(f"   Quality score: {result.quality_score:.3f}")
    print(f"   Corrections applied: {list(result.corrections_applied.keys())}")
