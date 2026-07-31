# Porygon: FPGA and MCU Embedded System Design Framework (Da Nang Contest 2026)

## Repository Metadata

- **Repository**: `zok213/porygon`
- **Target Microcontroller**: SN32F407 (ARM Cortex-M0 Core @ 48MHz)
- **Target FPGA Devices**: Gowin GW1N Series (GW1N-UV1P5 / GW1NSR-LV4C)
- **IDE & Toolchain**: Keil MDK-ARM uVision v5.x / Gowin EDA
- **Continuous Integration**: GitHub Actions Workflows (`.github/workflows/ci.yml`)

---

## Document Overview

This repository contains the complete engineering specification, source code implementations, hardware abstraction drivers, continuous integration configuration, and reference documentation for both the **Microcontroller (MCU)** and **Field-Programmable Gate Array (FPGA)** competition tracks of the **Da Nang FPGA & MCU Design Competition 2026**.

The codebase enforces **Defensive Embedded Programming**, deterministic Real-Time Clock execution, zero-flicker 7-segment display multiplexing, non-blocking I2C memory access, and generic Verilog/VHDL logic design.

---

## Repository Structure and Directory Layout

```
porygon/
├── .github/
│   └── workflows/
│       └── ci.yml                                        # GitHub Actions Automated CI/CD Pipeline
├── .gitignore                                            # Root exclusion rules for large archives & build artifacts
├── README.md                                             # Root Architectural & System Specification
├── ĐỀ THI MCU 2026.pdf                                   # MCU Competition Track Specification Document
├── ĐỀ THI FGPA 2026.docx.pdf                             # FPGA Competition Track Specification Document
├── Gowin-FPGA-Vietnamese-Book-ACG525-Basic-part-Print-v (1).pdf # Gowin FPGA Design Handbook Reference
└── MCU_Contest_2026/                                     # Keil uVision Firmware Sub-Repository
    ├── .gitignore                                        # Toolchain build output exclusion rules
    ├── README.md                                         # Sub-repository technical specification
    ├── main_clock_skeleton.c                             # Production C firmware source code
    ├── Clock_Simulation.uvprojx                          # Keil MDK-ARM Project Configuration
    ├── Clock_Simulation.uvoptx                           # Keil MDK-ARM Option Configuration
    ├── debug.ini                                         # Hardware Debug Initialization Script
    ├── EventRecorderStub.scvd                            # Keil Event Recorder System View
    ├── Docs/                                             # Embedded PDF documentation archive
    │   └── ĐỀ THI MCU 2026.pdf                           # Embedded copy of MCU specification
    └── RTE/                                              # Run-Time Environment CMSIS drivers
```

---

## Continuous Integration and Automated Pipeline (CI/CD)

The repository integrates a GitHub Actions CI/CD workflow defined in `.github/workflows/ci.yml`. Every `push` and `pull_request` to `main` executes automated validation:

```mermaid
flowchart LR
    PushEvent[Git Push / Pull Request] --> CI[GitHub Actions Pipeline]
    
    subgraph Job1 [Firmware Static Analysis Job]
        CI --> Checkout1[Checkout Code]
        Checkout1 --> Toolchain[Install gcc-arm-none-eabi & cppcheck]
        Toolchain --> StructCheck[Verify MCU Project Files]
        StructCheck --> StaticCheck[Run Cppcheck Static Analysis]
        StaticCheck --> SyntaxCheck[ARM Cortex-M0 Syntax Compile Check]
    end

    subgraph Job2 [Documentation Check Job]
        CI --> Checkout2[Checkout Code]
        Checkout2 --> DocCheck[Verify Mandatory README & PDF Files]
    end

    Job1 --> BuildResult[Pipeline Status: Success / Failure]
    Job2 --> BuildResult
```

---

## Hardware Architecture Block Diagram

```mermaid
graph TD
    subgraph MCU_System_SN32F407 [MCU System Track: ARM Cortex-M0 SN32F407]
        MCU_Core[SN32F407 Core @ 48MHz]
        SysTick[SysTick Timer ISR 1ms]
        KeyMatrix[4x4 Keypad Matrix]
        Display7Seg[4-Digit 7-Segment LED Display]
        EEPROM_I2C[AT24C02 EEPROM via I2C0]
        Buzzer[Piezo Buzzer GPIO3_0]

        MCU_Core --> SysTick
        SysTick --> Display7Seg
        KeyMatrix --> MCU_Core
        MCU_Core <--> EEPROM_I2C
        MCU_Core --> Buzzer
    end

    subgraph FPGA_System_Gowin [FPGA System Track: Gowin GW1N]
        FPGA_Clock[Onboard Clock 24MHz / 27MHz]
        PLL_Module[Gowin IP Core PLL -> 50MHz]
        Debounce_Mod[Key Debounce Module]
        FSM_Control[System Supervisor FSM]
        PWM_Mod[3-Mode Breathing PWM Generator]
        UART_TX[UART Transmitter @ 115200 bps]
        LED_Output[Board LED Output]

        FPGA_Clock --> PLL_Module
        PLL_Module --> FSM_Control
        PLL_Module --> PWM_Mod
        PLL_Module --> UART_TX
        Debounce_Mod --> FSM_Control
        FSM_Control --> PWM_Mod
        FSM_Control --> UART_TX
        PWM_Mod --> LED_Output
    end
```

---

## MCU Subsystem Architecture (SN32F407)

### Hardware Pinout Mapping (SN32F407_EVK)

| Function | MCU Register / Pin | Hardware Interface | Electrical Signal |
| :--- | :--- | :--- | :--- |
| Segment Bus (A..G, DP) | `GPIO0 [0..7]` | 7-Segment Segment Lines | Push-Pull Output, Active High |
| Digit Drivers (D1..D4) | `GPIO1 [9..12]` | Multiplexing Select Pins | Push-Pull Output, Active High |
| Key Matrix Rows | `GPIO1 [4..7]` | Keypad Scan Drivers | Open-Drain / Push-Pull Scan Out |
| Key Matrix Columns | `GPIO2 [4..7]` | Keypad Input Lines | Input with Internal Pull-Up Resistors |
| I2C Bus SCL | `GPIO0 [10]` | AT24C02 EEPROM Clock | Open-Drain, External $4.7\text{k}\Omega$ Pull-up |
| I2C Bus SDA | `GPIO0 [11]` | AT24C02 EEPROM Data | Open-Drain, External $4.7\text{k}\Omega$ Pull-up |
| Buzzer Signal | `GPIO3 [0]` | Piezo Electric Buzzer | NPN Transistor Switch Active High |
| Mode LED | `GPIO3 [8]` | Board LED D6 | Push-Pull Output, Active Low |

---

### System Finite State Machine (FSM) Diagram

```mermaid
stateDiagram-v2
    [*] --> MODE_NORMAL : System Power On / Restore Memory
    
    state MODE_NORMAL {
        [*] --> Clock_Running
        Clock_Running --> Check_Alarm : SysTick 1ms
        Check_Alarm --> Ring_Buzzer : Hour & Min Match (Sec == 0)
    }

    MODE_NORMAL --> MODE_EDIT_HOUR : SW3 Key Press
    MODE_EDIT_HOUR --> MODE_EDIT_MIN : SW3 Key Press
    MODE_EDIT_MIN --> MODE_NORMAL : SW3 Key Press (Commit Time & Reset Sec)

    MODE_NORMAL --> MODE_EDIT_AL_HOUR : SW16 Key Press
    MODE_EDIT_AL_HOUR --> MODE_EDIT_AL_MIN : SW16 Key Press
    MODE_EDIT_AL_MIN --> MODE_NORMAL : SW16 Key Press (Save Alarm to EEPROM)

    MODE_EDIT_HOUR --> MODE_NORMAL : Inactivity Timeout 30s (Rollback)
    MODE_EDIT_MIN --> MODE_NORMAL : Inactivity Timeout 30s (Rollback)
    MODE_EDIT_AL_HOUR --> MODE_NORMAL : Inactivity Timeout 30s (Rollback)
    MODE_EDIT_AL_MIN --> MODE_NORMAL : Inactivity Timeout 30s (Rollback)
```

---

### Anti-Ghosting 7-Segment Display Timing Diagram

```mermaid
sequenceDiagram
    autonumber
    participant ISR as SysTick Interrupt (1ms)
    participant Digit as GPIO1 Digit Select Pins
    participant Seg as GPIO0 Segment Data Pins

    ISR->>Digit: Phase 1: Blanking (De-assert GPIO1 Pins 9..12 to 0x00)
    ISR->>Seg: Phase 2: Load Data (Write GPIO0 Pins 0..7 with Seg Pattern)
    ISR->>Digit: Phase 3: Enable Target Digit (Assert Active GPIO1 Pin High)
```

---

### Memory Validation and Data Integrity Flowchart

```mermaid
flowchart TD
    Start([System Power On / Reset]) --> ReadHeader[Read Byte 0x00: EEPROM Magic Signature]
    ReadHeader --> CheckMagic{Magic Byte == 0xA5?}
    
    CheckMagic -- Yes --> ReadData[Read Byte 0x01: Alarm Hour, Byte 0x02: Alarm Min]
    CheckMagic -- No --> InitDefaults[Corrupted / Fresh EEPROM Detected]
    
    ReadData --> ValidateRange{Hour <= 23 AND Min <= 59?}
    ValidateRange -- Yes --> ApplySettings[Load Alarm Configuration into Master RAM]
    ValidateRange -- No --> InitDefaults
    
    InitDefaults --> SetZero[Set Alarm Hour = 0, Alarm Min = 0]
    SetZero --> WriteEEPROM[Write Magic 0xA5, Hour 0, Min 0 to EEPROM]
    WriteEEPROM --> ApplySettings
    ApplySettings --> SystemReady([System Operates in MODE_NORMAL])
```

---

## FPGA Subsystem Architecture (Gowin GW1N)

### FPGA Platform Hardware Specifications

| Hardware Parameter | Platform A (Kiwi 1P5) | Platform B (Kiwi Nano 4K) |
| :--- | :--- | :--- |
| FPGA Device | Gowin GW1N-UV1P5 | Gowin GW1NSR-LV4C |
| Package Type | QN48X | MG64P |
| Logic Elements (LUTs) | 1,152 | 4,608 |
| Target System Clock | 50.0 MHz via Gowin PLL Core | 50.0 MHz via Gowin PLL Core |
| Telemetry Interface | GWU2 USB-to-UART Bridge | GWU2 USB-to-UART Bridge |
| UART Parameters | 115200 bps, 8N1 | 115200 bps, 8N1 |

---

### FPGA Signal Processing Flowchart

```mermaid
flowchart LR
    CLK_IN[Onboard Oscillator] --> PLL[Gowin PLL Core]
    PLL -->|50MHz Clock| SYS_CLK[Global Clock Network]

    BTN1[Button 1 Input] --> DEB1[Debounce Module 1]
    BTN2[Button 2 Input] --> DEB2[Debounce Module 2]

    DEB1 -->|Pulse Output| FSM[Supervisor FSM]
    DEB2 -->|Pulse Output| FSM

    FSM -->|Select Mode: LOW / HIGH / AUTO| PWM[PWM Generator]
    FSM -->|Trigger Message| UART[UART TX Module 115200bps]

    PWM -->|Breathing Signal| LED[Board LED Output]
    UART -->|Serial Telemetry| TX[UART_TX Pin]
```

---

## Technical Comparison Matrix

| Subsystem Module | Naive Implementation | Production-Grade Engineering Architecture |
| :--- | :--- | :--- |
| **EEPROM Read** | Unchecked read; crashes when processing unformatted `0xFF` memory. | Magic Byte validation ($0xA5$) + Range clamping ($0..23$, $0..59$). |
| **Time Maintenance** | Seconds counter paused during edit mode, causing clock drift. | Continuous background SysTick RTC counter; UI shadow edit buffer. |
| **Alarm Trigger** | Polling equality check; multiple re-triggers per minute. | Edge-triggered single-shot latch (`alarm_triggered_this_minute`). |
| **Display Queting** | Direct pin mutation; severe digit ghosting bleed. | 3-Phase timing sequence (Blanking $\rightarrow$ Data Load $\rightarrow$ Enable Digit). |
| **Key Debouncing** | Blocking software delay `delay_ms(20)`; causes display jitter. | Integrator filter with consecutive sample validation (5ms window). |
| **Watchdog Supervision** | Fed inside timer ISR; fails to reset if main loop freezes. | Super-loop health check; fed exclusively in main loop cycle. |

---

## Git Workflow & Remote Repository Setup

### Initializing Local Git Repository and Committing

```bash
# Navigate to workspace root
cd /d/FPGA&MCU

# Initialize Git
git init

# Add all files (excluding large zip archives handled by .gitignore)
git add .

# Create initial commit
git commit -m "feat: Initial release of Porygon FPGA and MCU framework (Da Nang Contest 2026)"

# Rename default branch to main
git branch -M main

# Add remote origin for repository zok213/porygon
git remote add origin https://github.com/zok213/porygon.git

# Push to GitHub main branch
git push -u origin main
```

---

## Build and Deployment Protocols

### Microcontroller Target (Keil MDK-ARM)
1. Open `MCU_Contest_2026/Clock_Simulation.uvprojx` in **Keil MDK-ARM uVision v5.x**.
2. Select target device `SN32F407`.
3. Press `F7` to rebuild target. Ensure build completes with `0 Error(s), 0 Warning(s)`.
4. Connect **SN32F407_EVK** board via CMSIS-DAP debugger and press `F8` to flash.

### FPGA Target (Gowin EDA)
1. Launch **Gowin EDA**.
2. Open target project for `GW1N-UV1P5` or `GW1NSR-LV4C`.
3. Synthesize RTL and run Floorplanner for physical pin constraint assignment (`.cst`).
4. Generate Bitstream (`.fs`) and program target SRAM/Flash using Gowin Programmer.

---

## License

Distributed under the **MIT License**. Maintained for **zok213/porygon**.
