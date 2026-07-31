# SN32F407 Smart Digital Clock Firmware Specification (Da Nang MCU Contest 2026)

## System Abstract

This document details the production-grade firmware architecture for the **Smart Digital Clock and Alarm System** running on the **SN32F407_EVK** evaluation board (ARM Cortex-M0 core). 

The implementation applies **Defensive Embedded Programming** principles to guarantee zero-flicker display multiplexing, non-blocking I2C EEPROM storage, deterministic matrix key debouncing, and fault-tolerant finite state machine (FSM) execution.

---

## Competition Evaluation Criteria Breakdown

| Criteria Component | Weight | Implementation Strategy in Source Code |
| :--- | :--- | :--- |
| **Basic Digital Clock Functions** | 35% | SysTick $1\text{ms}$ ISR, 7-Segment driver $HH.MM$, $00..23$ hour rollover, $00..59$ minute rollover. |
| **Alarm Setting & EEPROM** | 15% | I2C0 driver, AT24C02 EEPROM read/write routines, persistent memory across resets. |
| **Bonus Features** | 10% | 30-second button inactivity timeout auto-rollback, status LED D6 blinking indicator. |
| **Demo Video Presentation** | 20% | System test flow, edge-case verification, team presentation skills. |
| **Code & Documentation** | 10% | Defensive programming style, modular file organization, zero compilation warnings. |
| **Technical Defense Q&A** | 10% | Peripheral register control, race condition prevention, hardware timing defense. |

---

## Hardware Peripheral Mapping (SN32F407_EVK)

| System Block | Target Hardware | MCU Pin Assignment | Electrical Configuration |
| :--- | :--- | :--- | :--- |
| **Display Bus** | 7-Segment Segment Lines (A..G, DP) | `GPIO0 [0..7]` | Push-Pull Output, Active High |
| **Display Driver** | 7-Segment Digit Select (D1..D4) | `GPIO1 [9..12]` | Push-Pull Output, Active High |
| **Key Matrix** | Matrix Key Row Drivers | `GPIO1 [4..7]` | Open-Drain / Push-Pull Scan Out |
| **Key Matrix** | Matrix Key Column Inputs | `GPIO2 [4..7]` | Input with Internal Pull-Up Resistors |
| **Non-Volatile Memory** | I2C EEPROM (AT24C02) SCL/SDA | `I2C0` (`GPIO0_10` / `GPIO0_11`) | Open-Drain, External $4.7\text{k}\Omega$ Pull-up |
| **Audio Output** | Piezo Electric Buzzer | `GPIO3 [0]` | NPN Transistor Driver, Active High |
| **Status Indicator** | Mode Status LED (Board D6) | `GPIO3 [8]` | Push-Pull Output, Active Low |

---

## Memory Architecture Map

```
SN32F407 Internal Flash Memory Map (64 KB):
+-----------------------+ 0x0000_0000
| Vector Table & Reset  | 
+-----------------------+ 0x0000_00C0
| Program Code (.text)  |
+-----------------------+ 0x0000_FFFF

SN32F407 Internal SRAM Memory Map (8 KB):
+-----------------------+ 0x2000_0000
| Globals & Static RAM  |
+-----------------------+ 0x2000_0400
| Call Stack & Heap     |
+-----------------------+ 0x2000_1FFF

External I2C EEPROM (AT24C02) Memory Map (Address 0xA0):
+------+-----------------------+---------------------------------------+
| Byte | Field Name            | Functional Description                |
+------+-----------------------+---------------------------------------+
| 0x00 | Magic Header          | Signature byte (Must equal 0xA5)      |
| 0x01 | Alarm Hour            | Stored Alarm Hour (Range: 0..23)      |
| 0x02 | Alarm Minute          | Stored Alarm Minute (Range: 0..59)    |
| 0x03 | Checksum              | Validation XOR Checksum               |
+------+-----------------------+---------------------------------------+
```

---

## System Finite State Machine (FSM) Diagram

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

## Core Engineering Principles and Implementation Design

### Principle 1: Memory Signature Validation and Boundary Clamping
Uninitialized or corrupted EEPROM cells return `0xFF` ($255$). If referenced directly in array indexing (`seg7[255/10]`), memory corruption or out-of-bounds exceptions occur. The system implements a **Magic Header Signature** check (`0xA5`) combined with strict numerical boundary clamping.

```mermaid
flowchart TD
    A["EEPROM Read Byte 0x00"] --> B{"Magic Signature == 0xA5?"}
    B -- Valid --> C["Read Alarm Hour Byte 0x01 & Min Byte 0x02"]
    B -- Invalid --> D["Initialize Default Values: 00:00"]
    C --> E{"Hour <= 23 AND Min <= 59?"}
    E -- Valid --> F["Load Values into Active System Memory"]
    E -- Invalid --> D
    D --> G["Write Magic 0xA5 & Default 00:00 to EEPROM"]
    G --> F
```

---

### Principle 2: Isolated Shadow Editing Buffers and Continuous Real-Time Ticking
Background time calculation must remain decoupled from user interface operations. Stopping the seconds counter during edit mode induces severe time drift. The firmware maintains an uninterrupted Real-Time Clock (RTC) counter in the SysTick interrupt while UI editing operates on isolated shadow variables (`edit_h`, `edit_m`).

```mermaid
sequenceDiagram
    autonumber
    participant ISR as "SysTick ISR (1ms)"
    participant Master as "Master Clock RAM (time_sec, time_min, time_hour)"
    participant UI as "UI Shadow Buffer (edit_h, edit_m)"
    
    ISR->>Master: Increment Master RTC continuous (Sec -> Min -> Hour)
    Note over UI: User presses SW6 / SW10 to modify Edit Buffer
    ISR->>UI: Read Edit Buffer for 7-Segment Blinking Display
    Note over Master,UI: User presses SW3 / SW16 to Commit
    UI->>Master: Atomic Copy Edit Buffer to Master Clock RAM
```

---

### Principle 3: Edge-Triggered Single-Shot Alarm Activation
Comparing target time equality inside high-speed main execution loops causes thousands of re-triggering events during a single minute. The system utilizes an **Edge-Triggered Latch Flag** (`alarm_triggered_this_minute`) to ensure single-shot execution per minute boundary.

---

### Principle 4: Anti-Ghosting 7-Segment Multiplexing
Multiplexed displays suffer from crosstalk ghosting if segment lines transition while digit enable pins remain high. The driver enforces a 3-phase blanking sequence inside the 1ms SysTick ISR:

```mermaid
sequenceDiagram
    autonumber
    participant ISR as "SysTick Interrupt (1ms)"
    participant DigitBus as "GPIO1 Digit Lines (Pins 9..12)"
    participant SegBus as "GPIO0 Segment Lines (Pins 0..7)"

    ISR->>DigitBus: Phase 1: Clear All Digit Enable Lines (Blanking)
    ISR->>SegBus: Phase 2: Write Target Digit Segment Data to Bus
    ISR->>DigitBus: Phase 3: Assert Target Digit Enable Line High
```

---

### Principle 5: Matrix Key Debouncing Flowchart

```mermaid
flowchart TD
    ScanStart(["Matrix Key Scan Triggered (Every 1ms)"]) --> DriveRow["Assert Active Row Low (GPIO1 Pins 4..7)"]
    DriveRow --> ReadCol["Read Column Data Bus (GPIO2 Pins 4..7)"]
    ReadCol --> KeyCheck{"Key Detected?"}

    KeyCheck -- Yes --> FilterIntegrator{"5 Consecutive Samples Equal?"}
    KeyCheck -- No --> ResetIntegrator["Reset Debounce Counter to 0"]

    FilterIntegrator -- Yes --> SingleShotLock{"Key Previously Released?"}
    FilterIntegrator -- No --> SampleWait["Increment Debounce Counter"]

    SingleShotLock -- Yes --> RegisterKey["Emit Debounced Key Code (SW3, SW6, SW10, SW16)"]
    SingleShotLock -- No --> IgnoreHold["Lockout Repeat Key Pulse"]

    RegisterKey --> LockState["Set Key Pressed Latch = True"]
    ResetIntegrator --> UnlockState["Set Key Pressed Latch = False"]
```

---

### Principle 6: Watchdog Supervisor Sequence Diagram

```mermaid
sequenceDiagram
    autonumber
    participant MainLoop as "Super-Loop (main)"
    participant FSM as "System State Machine"
    participant WDT as "Hardware Watchdog (WDT)"

    loop Every Main Iteration
        MainLoop->>FSM: Execute Task Health Checks
        alt All Subsystems Healthy
            FSM-->>MainLoop: Health Check Passed
            MainLoop->>WDT: Issue WDT_Feed()
        else Subsystem Locked / Invalid State
            FSM-->>MainLoop: Health Check Failed
            Note over MainLoop,WDT: WDT Feed Withheld
            WDT->>WDT: Timeout Period Expires (2.0s)
            WDT->>MainLoop: Issue Hardware System Reset
        end
    end
```

---

## Technical Comparison Matrix

| Functional Module | Naive Implementation | Production Firmware Architecture |
| :--- | :--- | :--- |
| **EEPROM Load** | Unchecked read; system crashes on unformatted `0xFF` memory. | Magic Byte validation ($0xA5$) + Range clamping ($0..23$, $0..59$). |
| **Clock Maintenance** | Seconds counter halted during user edit mode. | Continuous SysTick RTC background ticker; UI shadow edit buffer. |
| **Alarm Trigger** | Polling equality check; multiple re-triggers per minute. | Edge-triggered single-shot latch (`alarm_triggered_this_minute`). |
| **7-Segment Display** | Direct pin mutation; severe digit ghosting bleed. | 3-Phase timing sequence (Blanking $\rightarrow$ Data Load $\rightarrow$ Enable Digit). |
| **Key Processing** | Delay loop polling (`delay_ms(20)`); causes display jitter. | Integrator filter with consecutive sample validation (5ms window). |
| **Watchdog Supervision** | Fed inside timer ISR; fails to reset if main loop freezes. | Super-loop health check; fed exclusively in main loop cycle. |

---

## License

This software is released under the **MIT License**. Created for the **Da Nang FPGA & MCU Design Competition 2026**.
