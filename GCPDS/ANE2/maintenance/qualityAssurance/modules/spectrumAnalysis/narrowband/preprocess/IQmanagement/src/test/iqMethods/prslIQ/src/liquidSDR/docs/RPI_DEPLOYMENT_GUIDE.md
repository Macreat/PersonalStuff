# Raspberry Pi Deployment Guide (SSH Workflow)

This document outlines the professional workflow for deploying, building, and executing the **Liquid-DSP IQ Calibration Pipeline** on a Raspberry Pi sensor node from a host development machine.

## 1. Prerequisites

### 1.1 Hardware & Network
- **Target**: Raspberry Pi 3/4/5 (running Raspberry Pi OS / Debian).
- **Network**: Both host and Pi must be on the same network or reachable via SSH.
- **SSH Access**: Public key authentication is highly recommended to avoid password prompts during automation.

### 1.2 Remote Environment
The Raspberry Pi must have the following dependencies installed:
```bash
sudo apt update
sudo apt install -y git cmake build-essential libfftw3-dev
# Install Liquid-DSP (Native build on Pi)
git clone https://github.com/jgaeddert/liquid-dsp
cd liquid-dsp
./bootstrap.sh && ./configure && make -j4
sudo make install
sudo ldconfig
```

---

## 2. Automated Deployment Workflow

The project includes a specialized script `scripts/deployRaspi.sh` that automates the entire sync-and-build cycle.

### 2.1 Configuration
Edit the top of `scripts/deployRaspi.sh` to match your Pi's credentials:
```bash
RPI_USER="pi"
RPI_HOST="192.168.1.100" # or "raspberrypi.local"
RPI_DIR="/home/pi/liquid_sdr_bench"
```

### 2.2 Execution
From your host machine, run:
```bash
chmod +x scripts/deployRaspi.sh
./scripts/deployRaspi.sh
```

**What this script does:**
1.  **Workspace Sync**: Uses `rsync` to push the source code, headers, and scripts to the Pi. It automatically ignores local `build/` and `results/` folders to save bandwidth.
2.  **Remote Build**: Connects via SSH and triggers `cmake` and `make` using the Pi's native compiler.

---

## 3. Remote Execution & Benchmarking

Once deployed, you can trigger the full benchmark suite remotely without leaving your host terminal:

```bash
ssh pi@raspberrypi.local "cd /home/pi/liquid_sdr_bench && ./scripts/build_and_run.sh"
```

This will:
- Run 5 iterations of the IQ calibration algorithm.
- Collect resource metrics (CPU/RAM/Throughput) natively on the ARM architecture.
- Generate the HTML report inside the Pi's `web/` folder.

---

## 4. Retrieving Results

To pull the generated metrics and visual report back to your host machine for analysis:

```bash
rsync -avz pi@raspberrypi.local:/home/pi/liquid_sdr_bench/results/ ./results_rpi/
rsync -avz pi@raspberrypi.local:/home/pi/liquid_sdr_bench/web/ ./web_rpi/
```

---

## 5. Troubleshooting

- **Connection Timeout**: Verify the IP address and ensure SSH service is enabled on the Pi (`sudo raspi-config`).
- **Missing Library**: If the binary fails to run on the Pi with "libliquid.so not found", ensure you ran `sudo ldconfig` after installing Liquid-DSP.
- **Permission Denied**: Ensure the target directory `/home/pi/` is writable by the specified user.
