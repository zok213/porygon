# SN32F407 Smart Digital Clock — Hardware-Proven Baseline (Quang build)

> **This is the version that runs on the MCU today** — the hardware-proven
> baseline. It is kept **verbatim** (byte-for-byte identical to the pristine
> reference in [`MCU_Contest_2026_Quang/`](../MCU_Contest_2026_Quang/)) so
> the board can always be restored to a known-working state.
>
> **Improvements live on `MCU_dev`** (tag `v1.1.0-rc3`): debounce, audible
> buzzer, race-free alarm, EEPROM magic/checksum, modular structure and a
> host simulation. They are NOT merged to `main` until the hardware test
> matrix passes. See the [branch policy](../README.md#branch-governance).

## What this firmware is

A 24-hour digital clock + alarm for the **SN32F407_EVK** (ARM Cortex-M0 @
12 MHz IHRC), built from the contest skeleton plus the SONiX reference I2C
library. Two source files:

| File | Role |
| :--- | :--- |
| `main_clock_skeleton.c` | Application: keypad FSM, display multiplexing, buzzer, timekeeping, EEPROM access |
| `I2C0.c` / `I2C.h` | SONiX interrupt-driven I2C0 driver (AT24C02 EEPROM) |

## Hardware-proven characteristics

| Area | Behaviour (verified on the board) |
| :--- | :--- |
| I2C pins | SCL0 = P0.10, SDA0 = P0.11 (PFPA option 2) — no collision with the 7-seg G/DP lines |
| I2C speed | 400 kHz (`SCLHT/SCLLT = 14`) |
| I2C robustness | Interrupt-driven transfers (display never stops); SysTick watchdog forces `Timeout` if the bus stays busy >50ms |
| Display | 4x7SEG HH.MM; colon **tick-pulses 100ms at each second boundary** in NORMAL mode, solid during edits |
| Alarm | Single-shot 5s pip-pip (0.5s on/0.5s off); silenced by entering any edit mode |
| EEPROM | Alarm hour@addr 0, minute@addr 1; restored at boot with range clamping |

## Requirement coverage (Đề thi MCU 2026)

All nine requirements are functionally implemented by this baseline: clock
HH.MM + rollovers (1), SW3 edit with blink (2), SW16 alarm edit + EEPROM
(3), SW6/SW10 wraparound (4, 5), buzzer patterns (6), LED D6 blink in alarm
edit (7), EEPROM persistence (8), 30s timeout rollback (9).

## Known defects in this baseline (fixed on MCU_dev)

| # | Defect | Impact | Fixed in |
| :- | :--- | :--- | :--- |
| 1 | No key debounce (raw edge detection) | Bounce can double-fire the FSM | `MCU_dev@v1.1.0-rc3` (20ms debounce) |
| 2 | Buzzer ~10-15 kHz (80-NOP, 10 bursts) | Inaudible on most piezo elements | `MCU_dev` (~4-5 kHz, bounded ISR) |
| 3 | Non-atomic alarm comparison | Can fire one minute early (torn read) | `MCU_dev` (IRQ-safe snapshot) |
| 4 | Alarm 00:00 disarmed on power-off | Persistence inconsistency | `MCU_dev` (armed flag + checksum) |
| 5 | No EEPROM magic/checksum | Corrupt bytes silently accepted | `MCU_dev` |
| 6 | Mojibake comments / single 312-line file | Code & document scoring | `MCU_dev` (modular, clean) |

## Build (Keil MDK)

1. Open `Clock_Simulation.uvprojx` (project lists `main_clock_skeleton.c`
   and `I2C0.c`).
2. Rebuild target `Target_1` — expected 0 errors / 0 warnings.
3. Flash via SN-Link.

> `SN32F400.h` / `SN32F400_Def.h` come from the SONiX SN32F4_DFP 1.1.1 pack
> (keep your local copy — the vendor CDN no longer serves it).

## Testing

Use [`TESTING.md`](TESTING.md) for the requirement matrix, edge cases and
demo-video script. It applies to this baseline and to the improved
candidate on `MCU_dev` alike.
