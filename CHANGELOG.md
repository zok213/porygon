# Changelog

All notable changes to the Porygon FPGA & MCU framework are documented here.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)
and this project uses semantic-style versioning.

> **Branch state**: `main` = the hardware-proven **baseline (Quang build,
> verbatim, tag `v1.0.0-quang`)**. The improved candidate lives on
> **`MCU_dev`** (tag `v1.1.0-rc3`) until the hardware matrix in
> [`MCU_Contest_2026/TESTING.md`](MCU_Contest_2026/TESTING.md) passes.

---

## [1.0.0-quang] - 2026-08-08 — main reset to the working baseline

### Changed
- **`main` now contains the hardware-proven baseline, verbatim**: the
  firmware flashed on the board today (`main_clock_skeleton.c` + SONiX
  `I2C0.c`/`I2C.h` + Keil project + RTE), byte-for-byte identical to the
  pristine reference in `MCU_Contest_2026_Quang/` (hash-verified).
- The improved firmware (rc1→rc3: debounce, audible buzzer, race-free
  alarm, EEPROM magic/checksum, modular code, simulation, full CI) moved to
  **`MCU_dev` only**. Nothing is lost — it remains on `MCU_dev` and in
  `main`'s history.
- READMEs rewritten for the baseline-first story; the baseline's known
  defects are documented with their fixes on `MCU_dev`.
- CI on `main` reduced to what the baseline tree supports (FPGA iverilog +
  documentation integrity); the full verification stack (55-check
  simulation, cppcheck, style gate, production syntax) runs on `MCU_dev`.

### Why
Per the branch policy: `main` is the known-working baseline; unproven
improvements only ever land on development branches and merge after
hardware validation. The board can always be restored to the exact working
state by checking out `main` (tag `v1.0.0-quang`).

---

## Baseline — Quang's working firmware (the hardware-proven version)

**Definition.** The firmware that **actually runs on the MCU** today,
preserved in [`MCU_Contest_2026_Quang/`](MCU_Contest_2026_Quang/README.md)
(`main_clock_skeleton.c` + SONiX `I2C0.c`/`I2C.h`). It is the starting point
for everything in this changelog: **every change below is measured against
this baseline**, not against the original contest skeleton.

**Why it is the baseline.** It is the only version with silicon evidence:
the I2C layer works on the board, the pins are right, the display behaves
correctly. Register-level code that boots and runs beats register-level code
that only compiles.

**What the baseline already gets right (kept unchanged in rc3):**

| Area | Baseline behaviour (proven on the board) |
| :--- | :--- |
| I2C driver | SONiX interrupt-driven I2C0 library - handles ACK/NACK/arbitration in the ISR |
| I2C pins | `PFPA` = option 2: SCL0 = P0.10, SDA0 = P0.11 - no collision with the 7-seg G/DP lines |
| I2C speed | 400 kHz (`SCLHT/SCLLT = 14`), ~8x faster than the skeleton's 50 kHz setting |
| Display during I2C | Transfers are interrupt-driven - the 7-seg refresh never stops (the skeleton stopped/restarted SysTick around each access) |
| I2C hang guard | SysTick watchdog: bus busy >50ms forces `Timeout`, so a stall cannot hang the firmware |
| DP indicator | Colon tick-pulses 100ms at each second boundary in NORMAL mode; solid during edits |

**What the baseline gets wrong (fixed by rc3 - see the table below):**

| Defect in baseline | Impact on the board |
| :--- | :--- |
| Raw edge key detection (no debounce) | Contact bounce can double-fire the FSM: one SW3 press can skip an edit state |
| Buzzer tone ~10-15 kHz (80-NOP, 10 bursts) | Inaudible on most piezo elements; ~67% SysTick ISR load during beeps |
| Non-atomic alarm comparison | Torn read can fire the alarm one minute early |
| Alarm 00:00 disarmed on power-off (`if (hour\|\|min)`) | Persistence inconsistency vs. the save path (which arms 00:00) |
| No magic/checksum on the EEPROM record | Corrupted bytes silently accepted |
| Single 312-line file, mojibake comments, stale artifacts | "Code & document" scoring + Q&A risk |

**Lineage note.** rc1/rc2 below were developed from the *original contest
skeleton* (logic-verified, no silicon proof). rc3 is the **merge point**:
the baseline's proven hardware layer + the rc1/rc2 robustness work. Both
lineages now live in one firmware.

---

## Baseline → rc3: complete change table (what / why)

| Area | Baseline (Quang, working) | rc3 | Why this change |
| :--- | :--- | :--- | :--- |
| I2C driver | SONiX IRQ library | **Kept unchanged** | Proven on silicon; the rc2 polling driver's `GPIO0 CFG` pin setup was the prime suspect for the G/DP pin collision |
| I2C watchdog | In `SysTick_Handler` | **Kept**, moved to `EEPROM_I2CWatchdog()` in `eeprom.c` | Same 50ms rule; moved so the ISR stays a thin dispatcher |
| DP tick-pulse | In ISR | **Kept**, driven by `ms_in_this_sec` in `clock.c` | Same visible behaviour; counter now wraps exactly at the second boundary and is simulation-verified |
| Key debounce | None (raw edge) | 20ms stable-input + 20ms release lockout (`keypad.c`) | One physical press = exactly one FSM event, even with bounce or a long hold - a bouncy press can no longer skip an edit state in the demo |
| Buzzer tone | 80-NOP / 10 bursts → ~10-15 kHz | 250-NOP / 2 bursts → ~4-5 kHz (`buzzer.c`) | Piezo resonance range, audible to judges; ISR worst case ~0.5ms instead of ~0.7ms+ |
| Alarm trigger | 3 separate volatile reads | IRQ-safe snapshot in `Clock_AlarmMatchNow()` | A SysTick rollover between the reads can no longer fire the alarm one minute early (stopwatch-proof) |
| EEPROM record | hour@0, min@1, no validation | magic 0xA5 + hour + min + armed flag + XOR checksum (`eeprom.c`) | Blank/corrupt cells self-repair to safe defaults; a 00:00 alarm stays armed across power cycles |
| Boot alarm restore | `if (hour\|\|min) armed=1` | armed flag persisted and restored | 00:00 alarm no longer silently disarms on power-off |
| I2C write-cycle wait | `for (d<20000)` fixed delay | Same delay, named constant | Behaviour unchanged; self-documenting |
| Code organisation | 2 files (312-line app + driver) | 7 modules + headers + `sim/` (`main/clock/keypad/display/buzzer/eeprom/I2C0`) | Modular structure = the "Code & document" criterion; each peripheral has one home |
| Comments/encoding | Mojibake Vietnamese | Clean English, clang-format enforced | Judges read the code; a broken-encoding comment reads as carelessness |
| Testability | None (board only) | Host simulation, 55 checks, CI gates | Every fix in this changelog is proven by execution, not inspection |
| Repo hygiene | `.err` failed-build log, `.uvoptx`, `.base@` tracked | All removed/gitignored; Quang folder preserved as reference | A failed-build log in the repo is the worst first impression |

---

## [1.1.0-rc3] - 2026-08-08 (merge point: baseline + robustness)

### Added (from baseline - hardware-proven, kept)
- **SONiX interrupt-driven I2C0 library** (`I2C0.c`/`I2C.h`, vendor reference
  driver). Pin fix: `PFPA` option 2 → SCL0 = P0.10, SDA0 = P0.11, so I2C
  cannot collide with the 7-segment G/DP lines. Bus runs at 400 kHz and the
  display keeps refreshing during transfers.
- **I2C hang watchdog** - `EEPROM_I2CWatchdog()` fed from the SysTick ISR:
  bus busy >50ms forces the library's `Timeout` flag so a stalled
  transaction can never hang the firmware.
- **DP tick-pulse** - in NORMAL mode the HH.MM separator pulses 100ms at the
  start of every second (real-clock tick effect); solid during edits.
  Driven by `ms_in_this_sec` (clock.c), verified by a new simulation check.
- `MCU_Contest_2026_Quang/` committed as the **hardware-proven reference**
  (sources + project + RTE + README; artifacts gitignored).

### Added (vs baseline - robustness)
- **Key debounce** - 20ms stable-input + 20ms release lockout. One event per
  physical press (baseline double-fired on bounce).
- **Audible buzzer tone** - ~4-5 kHz, burst-limited ISR (baseline ~10-15 kHz,
  inaudible on most piezo elements).
- **Race-free alarm trigger** - IRQ-safe snapshot (baseline could fire one
  minute early on a torn read).
- **EEPROM magic + checksum + armed flag** (baseline had none; 00:00 alarm
  disarmed on power-off).
- Modular firmware, host simulation (55 checks), real CI gates, coding
  standard (.clang-format), documentation suite.

### Changed
- `eeprom.c` real path rewritten on top of the vendor driver; the
  magic/checksum layer sits above it unchanged.
- `clock.c` / `display.c` / `main.c` restructured to host the baseline
  behaviours cleanly (watchdog in the ISR dispatcher, DP counter in clock).

### Fixed
- The five baseline defects listed in the table above (debounce, buzzer,
  alarm race, 00:00 disarm, EEPROM validation) plus mojibake comments and
  repo hygiene.

### Verification
- Host simulation: **55/55 checks pass** (one new DP-pulse check).
- Production path (`-Wall -Wextra -Werror -pedantic`, non-MOCK): 6 modules
  clean. `I2C0.c` is vendor code requiring `SN32F400_Def.h` from the SONiX
  pack - verified on hardware, excluded from host checks by design.

---

## [1.1.0-rc2] - 2026-08-07

### Added
- **`.clang-format` coding standard** (repo root) - 4-space indent, Allman
  braces, 100-column limit, pointers-left.
- **CI `code-style` job** - pinned `clang-format 22.1.8` (`--dry-run
  --Werror`) so the format standard is enforced on every push.
- **`-pedantic` on the production-path CI check** - C99-strict.
- `SYSTICK_RELOAD_1MS` named constant in `main.c` (12,000,000/1,000-1).
- **Coding Standards section** in the MCU README.

### Changed
- Full formatting pass over all MCU sources (layout only; simulation still
  54/54, production build clean).
- `Keypad_ReadRaw()` nested ternary → explicit early returns (clarity;
  silences cppcheck `knownConditionTrueFalse`).
- `Display_Tick1ms()` `out[]` declared `const` (cppcheck `constVariable`).

### Fixed
- None - rc2 is rc1's logic plus standards; no logic changes.

---

## [1.1.0-rc1] - 2026-08-07 (first robustness release, pre-merge)

### Added
- **Modular firmware split** - single `main_clock_skeleton.c` (289 lines)
  refactored into `main/clock/keypad/display/buzzer/eeprom` + `system.h` +
  `app.h`.
- **Host simulation harness** (`sim/`) - firmware logic runs unmodified on a
  PC against a RAM-mocked SN32F400; 54 assertions (boot, debounce, FSMs,
  blink, wraparound, timeout, EEPROM, alarm fire, midnight rollover).
- **Hardware-abstraction seams** (`Keypad_HW_*`, `Buzzer_HW_*`,
  `Display_HW_*`) - simulation observes the real logic without duplication.
- **Real CI gates** (simulation run, cppcheck, production syntax) - the
  previous pipeline swallowed all failures with `|| echo`.
- Keil project updated (dead `MOCK_SIMULATION` define removed).
- `TESTING.md` (hardware matrix + demo script) and `CHANGES_VS_ORIGINAL.md`.

### Fixed (same defect list the baseline inherits - see baseline table)
- Key debounce, audible buzzer tone, race-free alarm, EEPROM
  magic/checksum/armed flag, `LoadAlarm` return value on corrupt records.

### Removed
- Stale artifacts (`.err` failed-build log, `.esym/.xsym`, `.uvoptx`,
  `.base@`), dead `WDT_Init/Feed` stubs, mojibake comments.

### FPGA track (merged into this lineage)
- Teammate's FPGA skeleton (`FPGA/`, `SETUP.md`) merged with compile fixes
  (wire→reg, invalid sensitivity list, `output reg locked`, BOM). **Status:
  WIP - does not yet satisfy the FPGA đề bài** (see FPGA README gaps list).

---

## [1.0.0] - 2026-07-31

### Added
- Initial release: original contest skeleton firmware, Keil project, contest
  specifications, repository governance (branch strategy, templates, CI
  skeleton, security policy), timing proofs and architecture documentation.
