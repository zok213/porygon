# MCU_Contest_2026_Quang — Hardware-Proven Reference Build

> **What this is**: the firmware that **actually ran on the MCU** during
> development (built by teammate Quang). It is kept in the repository as the
> hardware-verified reference for the I2C layer and display behaviour.

> **What this is NOT**: the current development baseline. The maintained
> firmware lives in [`../MCU_Contest_2026/`](../MCU_Contest_2026/) (branch
> `MCU_dev`, tag `v1.1.0-rc3`), which merged the proven parts of this build
> (I2C driver, pin fix, DP tick-pulse, I2C watchdog) with the robustness
> work (debounce, audible buzzer, race-free alarm, EEPROM magic/checksum).

## Proven contributions adopted into the main firmware (rc3)

| Proven here | Adopted as |
| :--- | :--- |
| SONiX interrupt-driven I2C0 library (`I2C0.c`/`I2C.h`) | `MCU_Contest_2026/I2C0.c`, `I2C.h` |
| Pin fix: SCL0=P0.10, SDA0=P0.11 (option 2) — no collision with 7-seg G/DP | `I2C0_Init()` |
| I2C hang watchdog in SysTick (50ms Busy → Timeout) | `EEPROM_I2CWatchdog()` |
| DP tick-pulse (100ms at each second boundary in NORMAL mode) | `display.c` / `clock.c` |

## Known defects in THIS build (fixed in the main firmware)

- No key debounce (raw edge detection — bounce can double-fire the FSM)
- Buzzer tone ~10-15 kHz (inaudible on most piezo elements) + heavy ISR load
- Alarm trigger has a torn-read race (can fire one minute early)
- Alarm 00:00 is disarmed on power-off (no armed flag / magic / checksum)
- Mojibake Vietnamese comments (mixed ANSI/UTF-8 encoding)

## Files

| File | Note |
| :--- | :--- |
| `main_clock_skeleton.c` | Application (single file, pre-modularization) |
| `I2C0.c` / `I2C.h` | SONiX I2C0 reference driver (hardware-verified) |
| `Clock_Simulation.uvprojx` | Keil project (builds the two files above) |
| `RTE/` | CMSIS + SONiX device support |

Build artifacts and Keil user-state files are gitignored.
