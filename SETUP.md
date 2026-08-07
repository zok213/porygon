# Project Setup Guide

## Prerequisites

### MCU Track (SN32F407)
| Tool | Version | Purpose |
| :--- | :--- | :--- |
| Keil MDK-ARM | v5.x | IDE, compiler, debugger |
| Keil STM32/NUC400 Device Pack | latest | Device support for SN32F407 |
| ARM GCC (optional) | arm-none-eabi-gcc | Syntax-only CI checks |

### FPGA Track (Gowin GW1N)
| Tool | Version | Purpose |
| :--- | :--- | :--- |
| Gowin EDA (Diamond) | v3.x | Synthesis, P&R, programming |
| Gowin Programmer | v3.x | Flash bitstream to board |
| Icarus Verilog (optional) | v11+ | RTL syntax check in CI |

### CI/CD
| Tool | Purpose |
| :--- | :--- |
| GitHub Actions | Automated firmware static analysis & FPGA RTL verification |
| Cppcheck | C firmware static analysis |
| arm-none-eabi-gcc | ARM Cortex-M0 syntax compilation check |

## Clone & Branch Setup

```bash
git clone https://github.com/zok213/porygon.git
cd porygon

# Create development branches per Git Flow policy
git checkout -b MCU_dev main
git checkout -b FPGA_dev main
git checkout -b MCU_main main
git checkout -b FPGA_main main
git checkout -b release main
```

## MCU Build

1. Open `MCU_Contest_2026/Clock_Simulation.uvprojx` in Keil MDK-ARM.
2. Ensure the device is set to **SN32F407F**.
3. Build the project — target **0 Errors, 0 Warnings**.
4. The build output (`.axf`, `.hex`, `.bin`) is excluded from Git by `.gitignore`.

## FPGA Build

1. Open Gowin EDA (Diamond).
2. Create a new project targeting **GW1N-UV1P5**.
3. Add all `.v` files from `FPGA/` directory.
4. Run Synthesis → Place & Route → Generate bitstream.
5. Use Gowin Programmer to flash the `.fs` file to the board.

## CI/CD Pipeline

The GitHub Actions workflow (`.github/workflows/ci.yml`) runs on push/PR to `main`:

| Job | Checks |
| :--- | :--- |
| `firmware-static-analysis` | Project structure, Cppcheck, ARM syntax compilation |
| `fpga-rtl-verification` | FPGA directory presence, documentation integrity, iverilog syntax (if available) |
| `documentation-verification` | Mandatory files present, PDF archives verified |

## File Conventions

- **Commit messages**: Conventional Commits (`feat(mcu)`, `feat(fpga)`, `fix(display)`, `docs`, `refactor`)
- **Branch naming**: `MCU_dev`, `MCU_main`, `FPGA_dev`, `FPGA_main`, `release`, `main`
- **Build artifacts**: Excluded via `.gitignore` at root and `MCU_Contest_2026/`