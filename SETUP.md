# Project Setup Guide — MCU Track (SN32F407)

## Prerequisites

### MCU Toolchain Requirements
| Tool | Version | Purpose |
| :--- | :--- | :--- |
| **Keil MDK-ARM** | v5.3x / v5.4x | IDE, C compiler (ArmClang V6), debugger |
| **SONiX SN32F4 DFP Pack** | v1.1.1 | Device support pack for SN32F407 |
| **SN-Link Driver** | v2.00.323 | USB JTAG/SWD Debugger driver for Keil |

---

## Clone & Branch Setup

```bash
git clone https://github.com/zok213/porygon.git
cd porygon

# Checkout MCU active development branch
git checkout MCU_dev
```

---

## MCU Firmware Build Flow

1. Open project `MCU_Contest_2026/Clock_Simulation.uvprojx` in **Keil MDK-ARM**.
2. Verify target device is set to **SN32F407F**.
3. Press **F7 (Rebuild All)** — verify target **0 Error(s), 0 Warning(s)**.
4. Connect **SN-Link Debugger** to board `SN32F407_EVK`.
5. Press **F8 (Download)** to flash `.hex` binary to MCU Flash.

---

## CI/CD Pipeline Integration

GitHub Actions workflow (`.github/workflows/ci.yml`) runs automated MCU firmware checks:
- Project structure verification
- Cppcheck C static analysis
- ARM GCC Cortex-M0 syntax checks