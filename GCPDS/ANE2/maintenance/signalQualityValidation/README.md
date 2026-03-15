# signal quality validation

ANE2-Calibration-SDR — Python toolkit for SDR sensor network calibration and RF spectrum monitoring.

Version: 0.1.0 

Objective: Improve RF signal quality through systematic calibration and anomaly detection across distributed sensors.

---

## Quick start (role-based)

### Banda Ancha (Wideband Spectrum)
Focus: multi-frequency analysis, bandwidth optimization, spectrum mapping.
Entry points:
- Notebook: content/src/example-campaign_nodes.ipynb
- Data source: content/signalQualityValidation/DB/Database-FM-10Nodes/DataAcq.ipynb
Key outputs: PSD aggregation, noise-floor estimates, multi-node comparisons.
Be kind.
Explain your reasoning.
Balance giving explicit directions with just pointing out problems and letting the developer decide.
Encourage developers to simplify code or add code comments instead of just explaining the complexity to you.# Banda Angosta (Narrowband / Voice validation)
Focus: channel-level IQ calibration, spectral mask checks, PSD per-node.
Entry points:
- Notebook: content/src/example-realtime.ipynb
- Processing: DC-block, IQ calibration, PSD estimation (see DataAcq.ipynb)

### Servicio de Voz (Voice services)
Focus: synchronization, per-service quality metrics, anomaly detection.
Entry points: workflow protocols and testing notebooks (see docs).

---

## Project objective
Improve measurement quality of captured RF signals by standardizing acquisition rules, canonicalizing variable names and units, and providing minimal, reproducible validation steps anchored on the DB folder.

## Architecture & data flow (concise)

- Data acquisition: HackRF captures -> raw_iq (int8 interleaved) -> iq (complex) -> iq_clean (DC-block) -> PSD (pxx)
- Storage: per-node CSVs in content/signalQualityValidation/DB/Database-FM-10Nodes/DataBase-RF-FM-88MHz-108MHz-Bogota-Funza
- Consumers: processing pipelines (noise floor, IQ calibration), dashboards, validators

## DB sources (canonical)
- DataAcq notebook: content/signalQualityValidation/DB/Database-FM-10Nodes/DataAcq.ipynb
- Per-node CSVs: content/signalQualityValidation/DB/Database-FM-10Nodes/DataBase-RF-FM-88MHz-108MHz-Bogota-Funza/

## Canonical variables (short)
Refer to DICTIONARY.md for full table. Key fields: node_id, timestamp (ISO8601 UTC), pxx (PSD), start_freq_hz, end_freq_hz, sample_rate_hz, raw_iq, iq_clean, noise_floor, gain_dB.

## Contribution & version control
- Keep README and DICTIONARY.md canonical and minimal.
- Commit changes with clear message referencing DB folder and DataAcq notebook.
- Suggested commit message: "docs(signalQualityValidation): canonical README and DICTIONARY — aligns with DataAcq.ipynb"

## Next steps
1. Review DICTIONARY.md and confirm units/names
2. Approve to finalize and commit
3. Optional: add small validator script to validate CSVs against DICTIONARY

