# sim/ — Host Simulation of the SN32F407 Digital Clock

This folder gives the project its name its due: **Clock_Simulation**. The
firmware's real logic (`SysTick_Handler` + `App_LoopIteration`) is compiled
for the host PC and executed against RAM-mocked peripherals, so behaviour is
verified **without hardware** — on any laptop and in CI.

## Why this exists

- Every firmware change is behaviour-proven (54 checks) before it ever
  reaches a board.
- The simulation uses the **same code that ships** — nothing is
  re-implemented for testing. Hardware access goes through abstraction seams
  (`Keypad_HW_*`, `Buzzer_HW_*`, `Display_HW_*`) that this harness overrides.
- The CI pipeline runs the same build on every push.

## Layout

| File | Role |
| :--- | :--- |
| `SN32F400.h` | **Mock device header** — replaces the SONiX CMSIS pack header for host builds. Provides the register objects, `SysTick`, and no-op `__disable_irq/__enable_irq`. **Never used by the Keil build.** |
| `SN32F400_mock.c` | RAM-backed register object definitions (one instance shared by all translation units). |
| `sim_main.c` | The harness: boots the firmware, drives the keypad matrix model, samples display/buzzer/LED outputs, runs **54 assertions**, prints a summary, exits 0/1. |
| `Makefile` | `make run` — build + run; `make clean`. |

## Build & run

```bash
cd MCU_Contest_2026
make run            # gcc required (any host)

# or manually:
gcc -std=c99 -Wall -Wextra -Werror -DMOCK_SIMULATION -I . -I sim \
    main.c clock.c keypad.c display.c buzzer.c eeprom.c \
    sim/SN32F400_mock.c sim/sim_main.c -o clock_sim && ./clock_sim
```

Expected output:

```
=== SN32F407 Digital Clock - Host Simulation ===
=== Result: 54 checks, 0 failures ===
ALL TESTS PASSED
```

## What the 54 checks cover

| Area | Checks |
| :--- | :--- |
| Boot | state, time 00:00:00, blank-EEPROM recovery (magic repair), display 00.00, idle silence |
| Debounce | bouncy press = exactly one event; 800ms hold = no repeat |
| Time edit | SW3 FSM, SW6/SW10 wraparound (0-1→23/59, 23+1→0), blink on/off phases, commit semantics |
| Display | 05.56 rendered with DP separator, hour digits kept during minute blink |
| Timeout | still editing at 29s, rollback at 30s, clock frozen during edit, exit beep duration |
| Alarm edit | SW16 FSM, hour/minute wrap, LED D6 blink pattern, EEPROM record bytes (magic/h/m/armed/checksum) |
| EEPROM | persistence across power cycle, **00:00-armed survives**, checksum-corruption falls back to safe defaults and repairs |
| Alarm fire | rings exactly at hh:mm:00, pip-pip 0.5s/0.5s, stops after 5s, silenced by entering edit mode |
| Clock | 23:59:59 → 00:00:00 rollover |

## How the hardware seams are overridden

```c
/* in sim_main.c — replaces the register writes of the real build */
void Keypad_HW_DriveRow(uint8_t row)   { sim_active_row = row; }
uint8_t Keypad_HW_ReadColumnBits(void) { /* matrix model from sim_pressed */ }
void Buzzer_HW_ToneOn(void)            { sim_buz_activity = 1; }
void Display_HW_LedOn(void)            { g_led = 0; }
...
```

The keypad is modelled as a true 4x4 **matrix** (a column reads low only
when its row is the one being driven low), which is exactly how the wiring
behaves on the board.

## Extending the harness

Add a `CHECK(condition, "message")` plus a `press(KEY_*)` / `sim_tick(ms)`
sequence and rebuild with `make run`. The harness runs a single continuous
timeline (like a real demo), so new checks should continue that timeline.
