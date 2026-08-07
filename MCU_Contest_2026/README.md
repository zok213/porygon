# SN32F407 Smart Digital Clock Firmware (Da Nang MCU Contest 2026)

## System Abstract

Smart 24-hour digital clock and alarm on the **SN32F407_EVK** evaluation board
(ARM Cortex-M0, 12 MHz IHRC). The firmware is organised into small modules with
explicit hardware-abstraction seams, and ships with a **host simulation
harness** (the "Clock_Simulation" in the project name) that executes the real
FSM on a PC and verifies all contest behaviours.

> **Documents**
> - [`TESTING.md`](TESTING.md) — hardware test guide: build/flash steps,
>   requirement-by-requirement matrix, edge cases, demo-video script,
>   troubleshooting.
> - [`CHANGES_VS_ORIGINAL.md`](CHANGES_VS_ORIGINAL.md) — deep comparison of
>   this version with the original `main_clock_skeleton.c`: what changed,
>   why, risk, and how each change was verified.

```
+-----------------+      +----------+      +-----------------+
| SysTick 1ms ISR | ---> | Clock    | ---> | master hh:mm:ss |
|  (main.c)       |      | display  |      | edit buffers    |
|                 | ---> | Buzzer   | ---> | beep / 5s ring  |
+-----------------+      +----------+      +-----------------+
        ^                                            |
        | 1ms heartbeat                               v
+-----------------+      +----------+      +-----------------+
| main super-loop | <->  | Keypad   | <->  | 4x4 matrix key  |
| FSM + timeout   |      | (20ms    |      | SW3/SW6/SW10/   |
| + alarm trigger |      | debounce)|      | SW16             |
+-----------------+      +----------+      +-----------------+
        |
        v
+-----------------+      +----------+
| EEPROM (I2C0)   | <->  | AT24C02  |
| magic+checksum  |      | 8Kbit    |
+-----------------+      +----------+
```

## Repository Layout

```
MCU_Contest_2026/
├── main.c                 # HW init, key FSM, main loop, SysTick ISR
├── app.h                  # App_Init / App_LoopIteration entry points
├── system.h               # shared volatile state bus + FSM enum
├── clock.h/.c             # master timekeeping, alarm settings, edit buffers
├── keypad.h/.c            # 4x4 matrix scan + 20ms time-based debounce
├── display.h/.c           # 7SEG multiplex (anti-ghosting) + LED D6
├── buzzer.h/.c            # key beep, 5s alarm ring, ~4kHz tone
├── eeprom.h/.c            # I2C0 driver + magic/checksum persistent record
├── sim/
│   ├── SN32F400.h         # mock device header (host builds only)
│   ├── SN32F400_mock.c    # RAM-backed register objects
│   ├── sim_main.c         # 54-check verification harness
│   └── Makefile           # make run  -> build + run all checks
├── TESTING.md             # hardware test guide + demo script
├── CHANGES_VS_ORIGINAL.md # change analysis vs. the original skeleton
├── Clock_Simulation.uvprojx  # Keil MDK project
└── RTE/                   # CMSIS + SONiX device support
```

## Competition Requirements Coverage (Đề thi MCU 2026)
| # | Requirement | Implementation | Status |
| :- | :--- | :--- | :--- |
| 1 | 4x7SEG HH.MM, boot 00:00, min 0-59 -> hour 0-23 | `Clock_Tick1ms` rollover + `Display_Tick1ms` multiplex | ✅ |
| 2 | SW3: NORMAL -> edit hour (blink) -> edit min (blink) -> NORMAL | FSM in `Process_Key`, 1s blink in `display.c` | ✅ |
| 3 | SW16: alarm hour -> alarm min -> save EEPROM -> NORMAL | FSM + `EEPROM_SaveAlarm` | ✅ |
| 4 | SW6 (+): hour 23->0, min 59->0 | `Clock_AdjustEdit` modular arithmetic | ✅ |
| 5 | SW10 (-): hour 0->23, min 0->59 | `Clock_AdjustEdit` modular arithmetic | ✅ |
| 6 | Buzzer: key pip 0.3s; alarm 5s pip-pip 0.5/0.5; timeout pip 0.3s | `Buzzer_BeepKey` / `Buzzer_StartAlarm` | ✅ |
| 7 | LED D6 blinks 1s in alarm edit only, else off | `Display_HW_LedOn/Off` (active low) | ✅ |
| 8 | EEPROM persists alarm hh:mm (sec = 0) | magic 0xA5 + checksum + armed flag | ✅ |
| 9 | 30s no-key timeout in all edit modes -> NORMAL | `inactivity_ms` + main-loop check | ✅ |

All nine requirements are covered and continuously verified by the host
simulation (see below).

## Key Design Decisions

### 1. Key debounce (20ms, time-based, single-event per press)
`keypad.c` reports a key only after the raw input has been stable for 20ms,
then latches until the release has also been stable for 20ms. A bouncy press
or a long hold can never produce more than one event. Debounce timing uses the
system 1ms counter, so the main-loop call rate does not affect behaviour.

### 2. Isolated shadow edit buffers, clock frozen during time edit
Editing operates on `edit_*` shadow buffers. The master clock keeps running
during alarm edits but is intentionally paused during time edits
(`MODE_EDIT_HOUR/MIN`), so the value on screen cannot change under the user.
Committing copies the buffer atomically and resets seconds to 0.

### 3. IRQ-safe alarm trigger (no torn reads)
`Clock_AlarmMatchNow()` reads hour/minute/second and the alarm settings with
interrupts disabled. A SysTick rollover between the reads can therefore never
fire the alarm one minute early (or re-ring it). The ring is edge-triggered
single-shot: `!Buzzer_IsAlarmRinging()` prevents re-triggering during the 5s
ring, and entering any edit mode silences it.

### 4. Persistent EEPROM record with magic + checksum
```
addr 0 : 0xA5 magic header
addr 1 : alarm hour    (0..23)
addr 2 : alarm minute  (0..59)
addr 3 : armed flag    (0/1)
addr 4 : XOR checksum of addr 0..3
```
`EEPROM_LoadAlarm()` validates magic and checksum, clamps ranges, and repairs
a blank/corrupted record to safe defaults (00:00, disarmed). The armed flag is
persisted, so an alarm set to 00:00 stays armed across power cycles.

### 5. Bounded, audible buzzer tone
The tone is a ~4-5 kHz square wave generated by a short NOP-loop burst
(`BUZZ_HALF_PERIOD_NOP = 250`, `BUZZ_BURST_CYCLES = 2`) - chosen for the piezo
resonance range. The burst is kept short so the SysTick ISR stays bounded
(~0.5ms worst case during beeps only).

### 6. Hardware-abstraction seams for testability
`Keypad_HW_DriveRow/ReadColumnBits`, `Buzzer_HW_ToneOn/Off` and
`Display_HW_LedOn/Off` touch the registers on the real board and are
overridden by the simulation. This keeps all logic identical between
hardware and host builds - nothing is duplicated.

## Host Simulation (Clock_Simulation)

The firmware logic (SysTick ISR + super-loop FSM) runs unmodified on a PC
against RAM-mocked peripherals. The harness drives the keypad matrix, samples
the display/buzzer/LED outputs and checks 54 assertions:

boot state / blank-EEPROM recovery, display 00.00, idle silence,
**debounce (bouncy press = one event, hold = no repeat)**, time/alarm edit
FSMs, blink on/off phases, wraparound arithmetic (0-1 -> 23/59, 23+1 -> 0),
commit semantics, 30s timeout rollback + beep, EEPROM persistence
(incl. 00:00-armed and corruption recovery), alarm firing at hh:mm:00,
5s pip-pip pattern, silence after ring, cancel-on-edit, 23:59:59 -> 00:00:00.

```bash
# requires gcc (any host)
make run        # builds ./clock_sim and runs all checks (exit code 0 = pass)
make clean
```

The same build is executed by the repository CI pipeline.

## Building for Hardware (Keil MDK)

1. Open `Clock_Simulation.uvprojx` in Keil MDK v5.43+ (SONiX SN32F4_DFP 1.1.1
   pack, CMSIS 6.3.0).
2. Rebuild target `Target_1` (ArmClang v6, `-O2`). Expected: 0 errors,
   0 warnings, ~2.4 KB code.
3. Flash via SN-Link and verify with the demo script in the video section.

> Note: `SN32F400.h` ships with the SONiX SN32F4_DFP pack - it is intentionally
> not vendored in this repository. The mock header in `sim/` is only used for
> host builds (never by the Keil build).

## Watchdog Note

The hardware watchdog is not enabled in this build (not required by the
assignment). The firmware's super-loop is short and bounded; if a watchdog is
desired for defence-in-depth, it can be added behind `WDT_Feed()` call sites
in `main.c` - the design already isolates all feed points to the main loop.
