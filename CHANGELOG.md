# Changelog

All notable changes to the Porygon FPGA & MCU framework are documented here.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)
and this project uses semantic-style versioning.

## [1.1.0] - 2026-08-07

### Added (new logic)
- **Modular firmware split** - the single `main_clock_skeleton.c` (289 lines)
  was refactored into six focused modules with clean interfaces:
  - `main.c` - hardware init, key FSM, super-loop, SysTick ISR orchestration
  - `clock.c` - master timekeeping, alarm settings, UI shadow edit buffers
  - `keypad.c` - 4x4 matrix scan with time-based debounce
  - `display.c` - anti-ghosting 7SEG multiplex, blink generator, LED D6
  - `buzzer.c` - key beep, 5s alarm ring pattern, tone generation
  - `eeprom.c` - I2C0 driver + persistent alarm record
  - shared state bus in `system.h`, app entry points in `app.h`
- **Key debounce (fix)** - `keypad.c` now requires 20ms of stable input
  before reporting a key and latches until a 20ms-stable release. A bouncy
  or long-held press produces exactly one event (previously a raw edge
  detector could double-fire the FSM on contact bounce).
- **Audible buzzer tone (fix)** - tone retuned from an inaudible ~10-15 kHz
  to ~4-5 kHz (piezo resonance range) and burst-limited so the SysTick ISR
  stays bounded (~0.5ms worst case instead of ~0.7ms+).
- **Race-free alarm trigger (fix)** - `Clock_AlarmMatchNow()` snapshots
  time + alarm settings with interrupts disabled, eliminating a torn-read
  window that could fire the alarm one minute early.
- **EEPROM magic + checksum + armed flag (fix)** - persistent record now
  stores magic 0xA5, hour, minute, armed flag and an XOR checksum. Blank or
  corrupted cells are detected and repaired to safe defaults; an alarm set
  to 00:00 now stays armed across power cycles (previously disarmed).
- **Host simulation harness** (`sim/`) - the firmware logic runs unmodified
  on a PC against a RAM-mocked SN32F400 device layer. 54 assertions cover
  boot, debounce, edit FSMs, blink phases, wraparound, timeout, EEPROM
  persistence/corruption recovery, alarm firing, 5s pip-pip pattern,
  cancel-on-edit and midnight rollover. Build with `make run`.
- **Hardware-abstraction seams** - `Keypad_HW_*`, `Buzzer_HW_*`,
  `Display_HW_*` isolate register access so simulation observes real logic
  without duplication.
- **Real CI gates** - pipeline now builds + runs the simulation (fails on
  any failed check), runs cppcheck with error exit code, and verifies the
  repository structure. (Previously the pipeline swallowed all failures.)
- **Keil project updated** - `Clock_Simulation.uvprojx` now lists the six
  application modules; the dead `MOCK_SIMULATION` define was removed.

### Changed
- System clock documentation corrected to **12 MHz** IHRC (was claimed 48 MHz
  in the abstract; the SysTick proof already used 12 MHz).
- Flash size documentation corrected to **32 KB** IROM (was claimed 64 KB).
- Documentation now matches the code: debounce description, EEPROM layout,
  clock-pause-during-time-edit behaviour, watchdog status.
- All comments rewritten in clean English (previous file contained mojibake
  Vietnamese comment remnants from the original skeleton).

### Removed
- Stale build artifacts from the repository: `main_clock_skeleton.err` (a
  failed-build log), `.esym`/`.xsym`, Keil user-state `uvoptx`, `.base@`
  backup files, `__history`.
- Dead code: unused `system_ms_counter` consumers, empty `WDT_Init/Feed`
  stubs (watchdog documented as optional, not claimed as implemented).

### Fixed
- `main_clock_skeleton.c` -> modular sources; behavior preserved and
  verified by the new simulation harness (54/54 checks pass).
- EEPROM `LoadAlarm` return value corrected for the checksum-failure path.

## [1.0.0] - 2026-07-31

### Added
- Initial release: `main_clock_skeleton.c` firmware for the SN32F407_EVK
  (24h clock + alarm, matrix keypad, I2C EEPROM, 7SEG multiplexing, buzzer,
  LED D6), Keil MDK project, contest specifications, repository governance
  (branch strategy, issue/PR templates, CI skeleton, security policy),
  mathematical timing proofs and architecture documentation.
