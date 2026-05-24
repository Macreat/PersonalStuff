# LiquidSDR Method 3 Implementation: Methodology and Architecture

## 1. Introduction
This document details the architecture and implementation of the IQ calibration pipeline (Method 3) using the **Liquid-DSP** library. This implementation is optimized for high-performance SDR processing on embedded systems, specifically targeting the Raspberry Pi platform.

## 2. System Architecture
The system is designed as a **Hybrid Professional** streaming pipeline, combining high-speed file I/O with optimized DSP kernels.

### 2.1 Core Modules
- **I/O Manager**: Optimized for SigMF metadata parsing and chunked binary data streaming.
- **DSP Engine (Liquid-DSP)**:
    - `liquid_fft`: Using Liquid-DSP's optimized FFT interfaces (`fft_create_plan`).
    - `liquid_window`: Professional windowing (Hann) applied per segment.
    - `liquid_vector`: Implementation of Method 3 (DC + RMS) with focus on memory locality.
- **Metrics Engine**: Real-time tracking of CPU, RAM, and DSP throughput.
- **Web Dashboard**: Automatic generation of HTML reports (`web/index.html`) for quick validation.

### 2.2 Data Flow
1. `Raw Data (int8)` -> `I/O Manager` -> `Chunked Buffer`.
2. `Chunked Buffer` -> `Conversion` (to `float complex`).
3. `Method 3 Preprocessing` (DC Removal + RMS Norm).
4. `Spectral Analysis` (Welch PSD via Liquid-DSP FFT).
5. `Metric Extraction` (SNR, NF, Center Power).
6. `Result Storage` -> `JSON/CSV`.
7. `Web Reporter` -> `HTML Visualization`.

## 3. Deployment on Raspberry Pi
### 3.1 Dependencies
- `liquid-dsp`: Must be installed on the target system.
- `cmake`: Version 3.10+.
- `gcc/g++`: With support for ARM NEON/VFP if applicable.

### 3.2 Compilation
#### Native Compilation:
```bash
cd src/liquidSDR
mkdir build && cd build
cmake ..
make -j4
```

#### Deployment:
The `scripts/deploy_rpi.sh` script facilitates syncing sources and triggering remote builds via SSH.

## 4. Optimizations Applied
- **FFT Efficiency**: Liquid-DSP chooses the best FFT backend (e.g., FFTW, KissFFT, or native radix-2).
- **Memory Management**: Zero-allocation inner loops with pre-allocated reusable buffers via a custom `mem_tracker`.
- **Chunked Processing**: Balanced I/O and CPU load to prevent throughput jitter.

## 5. Automated Benchmarking
Use the following scripts in `scripts/`:
- `build_and_run.sh`: Automated build, run, and initial metric collection.
- `benchmark.sh`: Stability audit via multiple iterations.
- `generate_report.py`: Converts JSON results into a visual HTML dashboard.

## 6. Structure
```text
liquidSDR/
├── include/       # Public headers (liquid_bench.h)
├── src/           # Implementation logic
├── scripts/       # Automation & Deployment
├── web/           # HTML Visualization
├── results/       # Benchmark data (JSON)
├── tests/         # Unit tests
└── CMakeLists.txt # Build configuration
```
