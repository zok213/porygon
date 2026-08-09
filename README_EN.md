# Porygon: FPGA & MCU Multi-Track System Design Framework (Da Nang Contest 2026)

## 📍 OFFICIAL GITHUB REPOSITORY BRANCH NAVIGATION MATRIX

The `main` branch serves as the **Central Architecture Routing & Navigation Hub**. Below is the official multi-track branch management structure:

| Technical Sub-Track | Baseline Production Branch | Active Development Branch | Primary Workspace Directory |
| :--- | :--- | :--- | :--- |
| **MCU Track (ARM Cortex-M0 SN32F407)** | 🟢 [`MCU_main`](https://github.com/zok213/porygon/tree/MCU_main) | 🛠️ [`MCU_dev`](https://github.com/zok213/porygon/tree/MCU_dev) | [`MCU_Contest_2026/`](MCU_Contest_2026/) |
| **FPGA Track (Gowin GW1N Verilog RTL)**| 🟢 [`FPGA_main`](https://github.com/zok213/porygon/tree/FPGA_main) | 🛠️ [`FPGA_dev`](https://github.com/zok213/porygon/tree/FPGA_dev) | [`FPGA/`](FPGA/) |
| **Final Official Release** | 🏆 [`release`](https://github.com/zok213/porygon/tree/release) | — | Complete Repository |

---

## 1. System Abstract & Multi-Track Sub-System Overview

This document presents the master architectural solution for the **Da Nang FPGA & MCU Design Competition 2026**.

The repository integrates two hardware tracks:

### 1.1 Microcontroller (MCU) Track (ARM Cortex-M0 SN32F407)
- **Source Architecture**: Production C99 firmware targeted for the **SN32F407_EVK** evaluation platform. Implements a 24-hour real-time clock, zero-flicker 7-segment multiplexing, hardware interrupt-driven I2C0 EEPROM persistence, ACK Polling, SysTick 1ms RTC, non-blocking key matrix scanning, and HardFault self-healing reset.
- **Hardware Verification**: 100% verified on physical hardware.
- **Source Workspace**: [`MCU_Contest_2026/`](MCU_Contest_2026/)
- **Branch Links**: Official Baseline [`MCU_main`](https://github.com/zok213/porygon/tree/MCU_main) and Active Sandbox [`MCU_dev`](https://github.com/zok213/porygon/tree/MCU_dev).

### 1.2 Field-Programmable Gate Array (FPGA) Track (Gowin GW1N)
- **Source Architecture**: Register-Transfer Level (RTL) Verilog targeted for **Gowin GW1N** FPGAs (Kiwi 1P5 / Kiwi Nano 4K). Incorporates a 50MHz rPLL IP Core, 4-stage button debouncer, 3-mode breathing PWM generator, and hardware RS-232 UART TX telemetry ($115200\text{ bps}$).
- **Source Workspace**: [`FPGA/`](FPGA/)
- **Branch Links**: Official Baseline [`FPGA_main`](https://github.com/zok213/porygon/tree/FPGA_main) and Active Sandbox [`FPGA_dev`](https://github.com/zok213/porygon/tree/FPGA_dev).

### 1.3 Final Official Release (`release`)
- The [`release`](https://github.com/zok213/porygon/tree/release) branch serves as the **Final Submission Package** containing verified deliverables ready for competition jury evaluation.

---

## 2. Competition Evaluation Criteria Breakdown

| Criteria Component | Weight | Implementation Strategy in Source Code (C / Verilog) |
| :--- | :--- | :--- |
| **Digital Clock & RTL Core** | 35% | SysTick $1\text{ms}$ ISR, 7-Segment multiplexing $HH.MM$; FPGA Supervisor FSM controlling UART & PWM. |
| **Alarm, EEPROM & UART Telemetry**| 15% | Interrupt-driven I2C0 driver for AT24C02; Hardware UART TX $115200\text{ bps}$ telemetry output. |
| **Bonus Features** | 10% | 30s inactivity auto-rollback (MCU); 3-mode breathing/blink/static PWM LED brightness controller (FPGA). |
| **Demo Video Presentation** | 20% | Real-hardware verification flow on SN32F407_EVK MCU board and Gowin GW1N FPGA board. |
| **Code & Document Architecture** | 10% | Defensive C99 HAL & Verilog IEEE 1364-2001 compliance with 0 compilation and synthesis warnings. |
| **Q&A Technical Defense** | 10% | Register-level AHB/APB analysis, mathematical proofs for FPGA rPLL synthesis and UART baudrate generation. |

---

## 3. System Repository File and Directory Map (`main` Branch)

```
porygon/
├── .github/
│   ├── ISSUE_TEMPLATE/
│   │   ├── bug_report.md                                 # Issue template for MCU & FPGA bugs
│   │   └── feature_request.md                            # Issue template for feature requests
│   ├── pull_request_template.md                          # Consolidated PR review checklist
│   └── workflows/
│       └── ci.yml                                        # Automated CI/CD pipeline for MCU & FPGA RTL
├── .gitignore                                            # Root exclusion rules for Keil MDK & Gowin EDA build artifacts
├── CODE_OF_CONDUCT.md                                    # Contributor Covenant Code of Conduct
├── CONTRIBUTING.md                                       # Multi-track Git Flow governance rules
├── LICENSE                                               # MIT License
├── README.md                                             # Master System Specification (Master Index)
├── README_VI.md                                          # Master Vietnamese Technical Specification (MCU + FPGA)
├── README_EN.md                                          # Master English Technical Specification (MCU + FPGA)
├── SECURITY.md                                           # Vulnerability reporting & RDP flash security policy
├── SETUP.md                                              # Multi-track toolchain setup guide (Keil MDK & Gowin EDA)
├── ĐỀ THI MCU 2026.pdf                                   # Official MCU Competition Specification
├── ĐỀ THI FGPA 2026.docx.pdf                             # Official FPGA Competition Specification
├── Gowin-FPGA-Vietnamese-Book-ACG525-Basic-part-Print-v (1).pdf # Gowin FPGA Reference Handbook
├── MCU_Contest_2026/                                     # MCU Track Sub-Repository (from MCU_main)
│   ├── README.md                                         # MCU Track Sub-Specification
│   ├── main_clock_skeleton.c                             # Production C firmware source code (FSM & SysTick 1ms)
│   ├── I2C0.c                                            # SONiX I2C hardware interrupt driver
│   ├── I2C.h                                             # I2C driver header interface
│   ├── ISSUE_ANALYSIS_DEEP_DIVE.md                       # Deep-dive issue resolution & audit report
│   └── Clock_Simulation.uvprojx                          # Keil MDK-ARM Project Configuration
└── FPGA/                                                 # FPGA Track Sub-Repository (from FPGA_main)
    ├── README.md                                         # FPGA Track Sub-Specification
    ├── top.v                                             # Supervisor Top-Level FSM Module
    ├── pll_50mhz.v                                       # Gowin rPLL 24MHz -> 50MHz Clock IP Core
    ├── debouncer.v                                       # Button Debouncer Module (4-Stage Shift Register)
    ├── breathing_pwm.v                                   # 3-Mode Breathing PWM Generator
    └── uart_tx.v                                         # Hardware UART TX Telemetry Module (115200 bps 8N1)
```
