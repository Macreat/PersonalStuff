# Canonical Dictionary — Signal Quality Validation

**Version:** 1.0  
**Last Updated:** 2026-03-14  
**Purpose:** Single source of truth for all signal QA variables, units, and types.

---

## Variable Definitions

| Field Name | Type | Unit | Description | Source | Importance |
|---|---|---|---|---|---|
| `pxx` / `PSD` | array/float | dB or linear | Power Spectral Density array for a capture window (e.g., length 4096). Used for noise floor and signal detection. | DataAcq.ipynb, per-node CSV | High |
| `pxx_per_node` | array/float | dB or linear | PSD computed per node for distributed comparison. | per-node CSVs | High |
| `timestamp` | ISO8601 string | UTC | Capture timestamp. Must use ISO8601 format with UTC timezone. | CSVs | **High** |
| `node_id` | string/int | — | Unique identifier for the sensor node. Matches filename and MAC if available. | CSVs (id, mac columns) | **High** |
| `mac` | string | — | Hardware MAC address. Optional, but when available should be stored. | CSVs | Medium |
| `start_freq_hz` | int/float | Hz | Lower bound of PSD frequency range. | CSVs, capture config | **High** |
| `end_freq_hz` | int/float | Hz | Upper bound of PSD frequency range. | CSVs, capture config | **High** |
| `sample_rate_hz` | float | Hz | Acquisition sample rate. Critical for DC-blocker alpha and FFT scaling calculations. | DataAcq.ipynb, capture config | **High** |
| `raw_iq` | bytes / int8 array | int8 interleaved | Raw interleaved I/Q samples from HackRF: I₀,Q₀,I₁,Q₁,... | DataAcq.ipynb | High |
| `iq` | complex64 array | normalized units | Complex IQ samples after conversion from `raw_iq`. | DataAcq.ipynb | High |
| `iq_clean` | complex64 array | normalized units | Complex IQ after DC blocking and optional normalization. | DataAcq.ipynb (iq_clean output) | High |
| `alpha` | float | unitless | DC-blocker pole coefficient (0 ≤ alpha < 1). Computed from `sample_rate_hz` and `cutoff_hz`. | DataAcq.ipynb (dc_block_alpha) | Medium |
| `cutoff_hz` | float | Hz | DC-blocker cutoff frequency. Typical range: 10–50 Hz. | DataAcq.ipynb | Medium |
| `noise_floor` | float | dB | Estimated noise floor for a capture window. | processing output | **High** |
| `gain_dB` | float | dB | Applied gain (LNA, VGA, or post-processing). If available, include in metadata. | capture metadata | Medium |
| `temperature_c` | float | °C | Node temperature at capture time. Important for hardware-related QA and anomaly detection. | node telemetry | Medium |
| `ram_percent` | float | % | Node RAM usage (0–100) at capture time. Useful for correlating processing artifacts. | node telemetry | Low/Medium |
| `cpu_percent` | float | % | Node CPU usage (0–100) at capture time. Use alongside `ram_percent` for resource tracking. | node telemetry | Low/Medium |
| `lat` | float | decimal degrees | Node latitude. GPS/GNSS coordinates (where available). | CSVs, node config | Medium |
| `lng` | float | decimal degrees | Node longitude. GPS/GNSS coordinates (where available). | CSVs, node config | Medium |
| `rbw_hz` | float | Hz | Resolution bandwidth used for PSD computation. | capture metadata | Medium |
| `span_hz` | float | Hz | Frequency span used for PSD computation. | capture metadata | Medium |
| `excursion_peak_to_peak_hz` | float | Hz | Peak-to-peak frequency excursion (noise metric). | per-node CSVs | Medium |
| `excursion_deviation` | float | Hz | Standard deviation of frequency excursion. | per-node CSVs | Medium |
| `excursion_rms_deviation` | float | Hz | RMS deviation of frequency excursion. | per-node CSVs | Medium |
| `created_at` | ISO8601 string | UTC | Ingestion timestamp for this DB record. | CSVs or system | Low |

---

## cannonical array 

```bash
{
  "node_metadata": {
    "node_id": string,
    "mac": string,
    "lat": float (decimal degrees),
    "lng": float (decimal degrees)
  },
  
  "signal_measurement": {
    "timestamp": ISO8601 UTC,
    "pxx": array (shape: 4096, dtype: float64),
    "sample_rate_hz": float,
    "start_freq_hz": float,
    "end_freq_hz": float,
    
    "pxx_measurements": {
      "noise_floor": float (dB),
      "peak_power": float (dB),
      "snr": float (dB),
      "gain_dB": float
    },
    
    "monitoring_metrics": {
      "temperature_c": float,
      "ram_percent": float,
      "cpu_percent": float,
      "latency_ms": float
    },
    
    "spatio_temporal_params": {
      "rbw_hz": float,
      "span_hz": float,
      "correlation_to_neighbors": dict,
      "excursion_peak_to_peak_hz": float
    }
  }
}

``` 
## Naming Conventions

### Primary Keys (for time-series alignment)
- Use `node_id` + `timestamp` as composite primary key for per-node observations
- Ensure both are always present in ingested data

### Unit Consistency
- **Frequencies:** All in Hz (never MHz, kHz without explicit context)
- **Power:** PSD in dB or linear units (specify clearly in code)
- **Temperature:** Celsius (°C), never Kelvin or Fahrenheit
- **Percentages:** 0–100, not 0–1.0
- **Time:** ISO8601 UTC (e.g., `2026-03-14T12:34:56Z`)
- **Coordinates:** Decimal degrees (not DMS)

### Naming Patterns
- **Frequency-related:** suffix with `_hz` (e.g., `sample_rate_hz`, `cutoff_hz`)
- **Power/Gain:** suffix with `_dB` (e.g., `gain_dB`, `noise_floor` if in dB)
- **Resource metrics:** suffix with `_percent` (e.g., `ram_percent`, `cpu_percent`)
- **Time:** use `timestamp` (ISO8601) or `created_at` (ingestion time)

### Multi-Node Data
- **Per-node arrays:** suffix with `_per_node` (e.g., `pxx_per_node`)
- **Aggregated:** no suffix; use in scalar context

---

## Data Type Mappings

| Type | Python | NumPy | Notes |
|------|--------|-------|-------|
| Frequency (Hz) | `float` | `np.float64` | Use float; precision required |
| Power/Gain (dB) | `float` | `np.float64` | dB is logarithmic; preserve precision |
| Sample rate | `float` | `np.float32` or `np.float64` | Prefer float64 for FFT scaling |
| IQ samples | `complex64` or `complex128` | `np.complex64` | complex128 for high precision; complex64 for storage |
| Raw IQ | `int8` | `np.int8` | Interleaved: I,Q,I,Q,... |
| Timestamp | `str` (ISO8601) | N/A | Store as string; parse to datetime when needed |
| Node ID | `str` or `int` | N/A | Use string if includes non-numeric chars (e.g., MAC) |
| Percentage | `float` (0–100) | `np.float64` | Not 0–1.0 |

---


## Validation Rules

### Required Fields (must always be present)
- `timestamp` (ISO8601 UTC)
- `node_id`
- `pxx` (or `pxx_per_node`)
- `sample_rate_hz`
- `start_freq_hz`, `end_freq_hz`

### Conditional Fields (required in specific routes)
- **Wideband:** `pxx`, `noise_floor`, `gain_dB`
- **Narrowband:** `iq_clean`, `start_freq_hz`, `end_freq_hz`
- **Voice:** `timestamp`, `node_id`, temperature, latency data

### Quality Assurance
- All numeric fields must be non-NaN or explicitly marked as null
- Timestamps must be valid ISO8601 strings; parser should reject invalid formats
- Frequencies must be positive
- Power (dB) should be ≤ 0 dB for noise; extreme values (< −200 dB) suggest data errors

---

## Notes & Best Practices

- **Units:** Always prefer SI base units (Hz, seconds, Celsius, degrees). When using dB, clearly specify reference level in code comments.
- **Precision:** Use `float64` for PSD and gain calculations; `float32` for storage if space is critical.
- **Time-series alignment:** Use `(node_id, timestamp)` pair for merging data across routes.
- **CSV ingestion:** Validate header against this dictionary on load; fail loudly if mismatch.
- **Version control:** When adding new fields, increment DICTIONARY version and update plan.md
- **Documentation:** Reference this file in code docstrings when using these variables.

---

## Update History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-03-14 | Initial canonical dictionary; 25 core variables |

---
