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

## Resolved Issues & Defensive Programming Enhancements

The codebase has undergone full real-hardware verification and hardening against the 6 identified competition edge cases:

1. **Boot Alarm Guard (`alarm_armed`)**: Preserves the boot check `if (alarm_hour || alarm_min) alarm_armed = 1;` to prevent accidental 5-second alarm bursts during power-on when `time` and `alarm` both default to `00:00`.
2. **Checked EEPROM Save Return**: `Process_Key()` validates `EEPROM_SaveAlarm(...)`. If write fails, the UI issues 3 long error beeps (`buzzer_beep_ms = 900`) and remains in edit mode instead of silently returning to `MODE_NORMAL`.
3. **Hardware Interrupt I2C Driver (`I2C0.c`)**: Uses SONiX DFP interrupt-driven state machine with FIFO buffers mapped to `P0.10` (SCL0) and `P0.11` (SDA0), avoiding pin collisions with 7-segment lines.
4. **CMSIS Clock & Flash Initialization**: Explicitly invokes `SystemInit()` and `SystemCoreClockUpdate()` at the start of `main()`, ensuring correct Flash wait-states (`SN_FLASH->LPCTRL`) across all CPU clock speeds (12MHz / 48MHz).
5. **Fault Recovery (`HardFault_Handler`)**: Implements an explicit C handler calling `__disable_irq()` and `NVIC_SystemReset()`, providing automatic self-healing reboot instead of permanent `B .` deadlocks.
6. **ACK Polling Protocol**: Replaces fixed NOP loops in `EEPROM_SaveAlarm` with hardware **ACK Polling** retries (`do { ... } while (++poll_retry < 50)`), dynamically matching EEPROM internal write cycle $t_{WR}$ without CPU cycle wastage.
7. **Sticky Error Reset**: Clears `Error = 0;` at the beginning of `I2C0_Read()` and `I2C0_Write()` to prevent permanent I2C bus lockup after transient NACKs.

---

## Finite State Machine (FSM) Execution Flow

```
+-------------------------------------------------------------------------------+
|                                MODE_NORMAL (0)                                |
|  - Displays time HH.MM                                                        |
|  - DP dot blinks for 100ms on second tick                                     |
|  - Checks Alarm match (rings 5s if armed)                                     |
+-------------------------------------------------------------------------------+
       |                                                 |
       | Press SW3 (KEY_SETUP)                           | Press SW16 (KEY_ALARM)
       v                                                 v
+-----------------------------+                   +-----------------------------+
|     MODE_EDIT_HOUR (1)      |                   |   MODE_EDIT_AL_HOUR (3)     |
|  - Hours blink (500ms)      |                   |  - Hours blink (500ms)      |
|  - SW6 (+), SW10 (-) modify |                   |  - LED D6 blinks (500ms)    |
+-----------------------------+                   +-----------------------------+
       |                                                 |
       | Press SW3                                       | Press SW16
       v                                                 v
+-----------------------------+                   +-----------------------------+
|     MODE_EDIT_MIN (2)       |                   |    MODE_EDIT_AL_MIN (4)     |
|  - Minutes blink (500ms)    |                   |  - Minutes blink (500ms)    |
|  - SW6 (+), SW10 (-) modify |                   |  - LED D6 blinks (500ms)    |
+-----------------------------+                   +-----------------------------+
       |                                                 |
       | Press SW3 (Save & Exit)                         | Press SW16 (Commit EEPROM)
       +----------------------->  Return to  <-----------+
                                MODE_NORMAL (0)
```

---

## Build and Flashing Instructions (Keil MDK)

1. Open project file `MCU_Contest_2026/Clock_Simulation.uvprojx` in **Keil MDK 5.3x / 5.4x**.
2. Select target `Target_1` (ArmClang V6 compiler).
3. Press **F7 (Rebuild All)** — verify output is **0 Error(s), 0 Warning(s)**.
4. Connect **SN-Link Debugger** to board `SN32F407_EVK`.
5. Press **F8 (Download)** to flash hex binary to target MCU.
