# Changelog

All notable changes to the Porygon FPGA & MCU framework are documented here.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)
and this project uses semantic-style versioning.

> **Latest test candidate**: `v1.1.0-rc3` on branch **`MCU_dev`**.
> Flash this version and run the hardware matrix in
> [`MCU_Contest_2026/TESTING.md`](MCU_Contest_2026/TESTING.md) before any
> merge to `MCU_main` / `release` / `main`.

---

## [1.1.0-rc3] - 2026-08-08

### Added (from the hardware-working board build)
- **SONiX interrupt-driven I2C0 library** (`I2C0.c`/`I2C.h`, vendor
  reference driver) adopted as the EEPROM byte-level driver. This is the
  driver proven on the real board (folder `MCU_Contest_2026_Quang`):
  - **Pin fix**: `PFPA` selects SCL0 = P0.10, SDA0 = P0.11 (option 2) so I2C
    no longer collides with the 7-segment G/DP lines (the previous polling
    driver's pin setup was suspect - its `GPIO0 CFG` write was the likely
    conflict).
  - Runs fully interrupt-driven: the display keeps refreshing during EEPROM
    access (the old driver's SysTick stop/start dance was removed).
  - I2C speed raised to 400 kHz (`SCLHT/SCLLT = 14`), ~8x faster than the
    old 50 kHz setting.
- **I2C hang watchdog** (`EEPROM_I2CWatchdog()` fed from the SysTick ISR):
  if the bus stays busy >50ms the library's `Timeout` flag is forced, so a
  stalled transaction can never hang the firmware.
- **DP tick-pulse** (from the working board build): in NORMAL mode the
  HH.MM separator pulses 100ms at the start of every second (real-clock
  "tick" effect); it stays solid during edits. Verified by the simulation
  (new check - harness is now 55 checks).
- Keil project updated: `I2C0.c` added (7 files total).

### Changed
- `eeprom.c` real path rewritten on top of the vendor driver; the
  magic/checksum/armed-flag persistence layer is unchanged.
- `clock.c`: `ms_in_this_sec` counter (replaces the internal `t1s`) drives
  the DP pulse and wraps exactly at the second boundary.
- `display.c`: DP driven by the tick-pulse in NORMAL mode, solid in edits.

### Fixed
- None in rc3 (rc2 logic preserved; this release adopts the hardware-proven
  I2C layer and adds the DP tick).

### Verification
- Host simulation: **55/55 checks pass** (one new DP-pulse check).
- Production path (`-Wall -Wextra -Werror -pedantic`, non-MOCK): 6 modules
  clean. `I2C0.c` is vendor code requiring `SN32F400_Def.h` from the SONiX
  pack - verified on hardware, excluded from host checks by design.

---

## [1.1.0-rc2] - 2026-08-07

### Added
- **`.clang-format` coding standard** (repo root) - 4-space indent, Allman
  braces, 100-column limit, pointers-left. Applies to all firmware sources.
- **CI `code-style` job** - runs the pinned `clang-format 22.1.8`
  (`--dry-run --Werror`) over every firmware source, so the format standard
  is enforced on every push, not just a convention.
- **`-pedantic` on the production-path CI check** - C99-strict compilation
  on top of `-Wall -Wextra -Werror`.
- `SYSTICK_RELOAD_1MS` named constant in `main.c` (derivation documented:
  12,000,000 / 1,000 - 1 = 11999 for a 1ms interrupt at the 12 MHz IHRC).
- **Coding Standards section** in `MCU_Contest_2026/README.md`
  (formatter, compiler discipline, static analysis, behavioural gate,
  naming conventions).

### Changed
- **Full formatting pass** over all MCU sources with the new `.clang-format`
  - whitespace/layout only. Verified by re-running the 54-check simulation
  (54/54 pass) and the production `-Werror -pedantic` build after the pass.
- `Keypad_ReadRaw()`: replaced the nested ternary with explicit early
  returns - clearer, and silences a cppcheck `knownConditionTrueFalse`
  diagnostic.
- `Display_Tick1ms()`: `out[]` declared `const` (cppcheck `constVariable`).

### Fixed
- None - rc2 is rc1's logic, provably identical (no logic changes in this
  release; it is a standards/documentation pass only).

---

## [1.1.0-rc1] - 2026-08-07

### Added
- **Modular firmware split** - the single `main_clock_skeleton.c` (289
  lines) was refactored into six focused modules with clean interfaces:
  - `main.c` - hardware init, key FSM, super-loop, SysTick ISR orchestration
  - `clock.c` - master timekeeping, alarm settings, UI shadow edit buffers
  - `keypad.c` - 4x4 matrix scan with time-based debounce
  - `display.c` - anti-ghosting 7SEG multiplex, blink generator, LED D6
  - `buzzer.c` - key beep, 5s alarm ring pattern, tone generation
  - `eeprom.c` - I2C0 driver + persistent alarm record
  - shared state bus in `system.h`, app entry points in `app.h`
- **Host simulation harness** (`sim/`) - the firmware logic (SysTick ISR +
  super-loop) runs unmodified on a PC against a RAM-mocked SN32F400 device
  layer. 54 assertions cover boot, debounce, edit FSMs, blink phases,
  wraparound, timeout, EEPROM persistence/corruption recovery, alarm
  firing, 5s pip-pip pattern, cancel-on-edit and midnight rollover.
  Build with `make run`.
- **Hardware-abstraction seams** - `Keypad_HW_*`, `Buzzer_HW_*`,
  `Display_HW_*` isolate register access so the simulation observes the
  real logic without duplication.
- **Real CI gates** - the pipeline now builds + runs the simulation (fails
  on any failed check), runs cppcheck with an error exit code, syntax-checks
  the production path, and verifies repository structure. (Previously the
  pipeline swallowed every failure with `|| echo`.)
- **Keil project updated** - `Clock_Simulation.uvprojx` lists the six
  application modules; the dead `MOCK_SIMULATION` define was removed.
- **Branch discipline for testing** - the firmware now lives on `MCU_dev`
  (tag `v1.1.0-rc1`) for hardware validation; `MCU_main` / `release` keep
  the previous known-good firmware as fallback until tests pass. Documented
  in `TESTING.md` §0.
- **`TESTING.md`** - hardware test guide: build/flash steps,
  requirement-by-requirement matrix (R1-R9), edge cases, demo-video script,
  troubleshooting.
- **`CHANGES_VS_ORIGINAL.md`** - deep comparison with the original
  `main_clock_skeleton.c`: what changed, why, risk, and verification.

### Changed
- System clock documentation corrected to **12 MHz** IHRC (the abstract
  claimed 48 MHz; the SysTick proof already used 12 MHz and the silicon
  boots to 12 MHz via `SYS0_CLKCFG_VAL=0`).
- Flash size documentation corrected to **32 KB** IROM (was claimed 64 KB;
  the Keil project configures `IROM 0x7FFC`).
- Documentation now matches the code: debounce description, EEPROM layout,
  clock-pause-during-time-edit behaviour, watchdog status (watchdog is
  documented as optional, not claimed as implemented).
- All comments rewritten in clean English (the original file contained
  mojibake Vietnamese comment remnants).

### Removed
- Stale build artifacts from the repository: `main_clock_skeleton.err` (a
  failed-build log from an old toolchain), `.esym`/`.xsym`, Keil user-state
  `uvoptx`, `.base@` backup files, `__history`.
- Dead code: the empty `WDT_Init`/`WDT_Feed` stubs (the README previously
  claimed a working watchdog; the claim was removed and the design note
  explains how to add one).

### Fixed
- **Key debounce** - `keypad.c` now requires 20ms of stable input before
  reporting a key and latches until a 20ms-stable release. A bouncy or
  long-held press produces exactly one FSM event; the original raw edge
  detector could double-fire on contact bounce and skip edit states.
- **Audible buzzer tone** - tone retuned from an inaudible ~10-15 kHz to
  ~4-5 kHz (piezo resonance range) and burst-limited so the SysTick ISR
  stays bounded (~0.5ms worst case instead of ~0.7ms+).
- **Race-free alarm trigger** - `Clock_AlarmMatchNow()` snapshots time and
  alarm settings with interrupts disabled, eliminating a torn-read window
  that could fire the alarm one minute early (or re-ring at the next
  boundary).
- **EEPROM magic + checksum + armed flag** - the persistent record now
  stores magic 0xA5, hour, minute, armed flag and an XOR checksum. Blank or
  corrupted cells are detected and repaired to safe defaults; an alarm set
  to 00:00 stays armed across power cycles (the original disarmed it on
  boot because arming was inferred from `hour || minute`).
- `EEPROM_LoadAlarm()` return value corrected for the checksum-failure path
  (previously reported "valid" for a corrupt-but-magic-matching record).

### FPGA track (merged into this lineage)
- Merged teammate's FPGA track (`e8e10cf`): `FPGA/` with `top.v`,
  `pll_50mhz.v`, `debouncer.v`, `breathing_pwm.v`, `uart_tx.v`, `README.md`,
  plus `SETUP.md`.
- **Compile fixes applied during the merge** so the iverilog CI job passes:
  `top.v` `wire` -> `reg` for `uart_start`/`uart_data`, invalid
  `posedge !pll_locked` sensitivity list removed, `pll_50mhz.v` `locked`
  declared `output reg`, UTF-8 BOM removed.
- **Status: WIP skeleton - does not yet satisfy the FPGA đề bài.** Known
  gaps: no LOW/HIGH/AUTO mode FSM (mode is hardwired), UART sends `'A'`
  instead of `"MODE: LOW \r\n"` strings, the "PLL" module divides 24 MHz to
  ~480 kHz (a real Gowin IP core is required), the debouncer window is
  80 ns (needs ~10-20 ms) and emits a level instead of a single-cycle
  pulse, and there is no `.cst` constraint file or testbench yet.

---

## [1.0.0] - 2026-07-31

### Added
- Initial release: `main_clock_skeleton.c` firmware for the SN32F407_EVK
  (24h clock + alarm, matrix keypad, I2C EEPROM, 7SEG multiplexing, buzzer,
  LED D6), Keil MDK project, contest specifications (MCU + FPGA PDFs),
  repository governance (branch strategy, issue/PR templates, CI skeleton,
  security policy), mathematical timing proofs and architecture
  documentation.
