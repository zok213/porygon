# MCU_Contest_2026_Quang — Hardware-Proven Reference Build

> **What this is**: the firmware that **actually ran on the MCU** during
> development (built by teammate Quang). It is the **pristine original** of
> the repository's `main` branch baseline: `MCU_Contest_2026/` on `main`
> contains byte-for-byte identical copies of these files (hash-verified),
> so the board can always be restored to this known-working state via
> `git checkout main` (tag `v1.0.0-quang`).

> **What this is NOT**: the improved development baseline. The improved
> firmware (debounce, audible buzzer, race-free alarm, EEPROM
> magic/checksum, modular code, 55-check simulation) lives on branch
> **`MCU_dev`** (tag `v1.1.0-rc3`) and merges to `main` only after the
> hardware test matrix passes.

## What `main` inherits from this build (verbatim)

| File in this folder | Role on `main` |
| :--- | :--- |
| `main_clock_skeleton.c` | Application (single file, pre-modularization) |
| `I2C0.c` / `I2C.h` | SONiX interrupt-driven I2C0 driver (hardware-verified) |
| `Clock_Simulation.uvprojx` | Keil project (builds the two files above) |
| `RTE/` | CMSIS + SONiX device support |

## Known defects in THIS build (fixed on `MCU_dev`, not on `main`)

- No key debounce (raw edge detection — bounce can double-fire the FSM)
- Buzzer tone ~10-15 kHz (inaudible on most piezo elements) + heavy ISR load
- Alarm trigger has a torn-read race (can fire one minute early)
- Alarm 00:00 is disarmed on power-off (no armed flag / magic / checksum)
- Mojibake Vietnamese comments (mixed ANSI/UTF-8 encoding)

Build artifacts and Keil user-state files are gitignored.
