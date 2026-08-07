# Hardware Testing Guide — SN32F407 Digital Clock (Da Nang MCU Contest 2026)

This guide takes the firmware from source to a fully tested board, verifies
**every requirement of the đề bài**, and doubles as the demo-video script
(20% of the score).

---

## 1. Prerequisites

| Item | Detail |
| :--- | :--- |
| Board | SN32F407_EVK (SN32F407F, 12 MHz IHRC) |
| Debugger | SN-Link (USB) |
| Toolchain | Keil MDK v5.43+ (ArmClang v6), SONiX SN32F4_DFP **1.1.1** pack, CMSIS 6.3.0 |
| Source | `MCU_Contest_2026/` (this folder) |
| Optional | Host gcc + `make` (for the simulation, `sim/` folder) |

> The SONiX pack is no longer downloadable from the vendor CDN (404). Keep
> your existing local install — `C:\Users\<user>\AppData\Local\Arm\Packs\SONiX\SN32F4_DFP\1.1.1`.

## 2. Build (Keil)

1. Open `Clock_Simulation.uvprojx` in Keil MDK.
2. Select target `Target_1`.
3. `Project → Rebuild all target files`.
4. **Expected**: `0 Error(s), 0 Warning(s)`, code ≈ 2.4–2.9 KB.

If the build cannot find `SN32F400.h`, the DFP pack is missing (see above).

## 3. Flash

1. Connect the SN-Link to the board, power the board.
2. `Flash → Download` (F8). Expected: "Application running…" and the display
   shows **`00.00`** (fresh EEPROM) with the colon DP lit.

## 4. Pre-test sanity (60 seconds)

| Step | Action | Expected |
| :--- | :--- | :--- |
| 1 | Power on | Display `00.00`, LED D6 **off**, buzzer **silent** |
| 2 | Press SW3 once | HH blinks at 1s period (0.5s on/0.5s off) |
| 3 | Wait 31s without pressing | Returns to `00.00` normal + **0.3s beep** |
| 4 | Press any key | Short **0.3s pip** from the buzzer (audible!) |

If the beep is inaudible → see Troubleshooting §8 (buzzer tuning).

## 5. Requirement-by-requirement test matrix

### R1 — Basic clock (35% bucket)
| # | Test | Action | Expected |
| :- | :--- | :--- | :--- |
| 1.1 | Boot state | Power on | `00.00` |
| 1.2 | Minute counter | Wait ~1 min | Minute increments by 1 |
| 1.3 | Minute rollover | Set time 12:59 (see R2/R4), wait 1 min | 13:00, hour incremented |
| 1.4 | Hour rollover | Set 23:59, wait 1 min | `00.00` |
| 1.5 | Display format | Look at digits | HH.MM with DP separator between HH and MM |

### R2 — SW3 time edit (35% bucket)
| # | Test | Action | Expected |
| :- | :--- | :--- | :--- |
| 2.1 | Enter edit | Press SW3 once | HH digits blink (1s period) |
| 2.2 | Hour → minute | Press SW3 again | MM digits blink, HH steady |
| 2.3 | Commit | Press SW3 again | Normal mode, seconds reset to 0 |
| 2.4 | Blink exactness | Watch during edit | Exactly 0.5s on / 0.5s off |
| 2.5 | No double-step | Press SW3 firmly once | Advances exactly ONE state (no skip) |

### R3 — SW16 alarm edit + EEPROM (15% bucket)
| # | Test | Action | Expected |
| :- | :--- | :--- | :--- |
| 3.1 | Enter alarm edit | Press SW16 once | HH blinks **and** LED D6 blinks (1s) |
| 3.2 | Alarm hour → min | Press SW16 again | MM blinks, LED D6 still blinking |
| 3.3 | Save | Press SW16 again | Normal mode; LED D6 off; alarm armed |

### R4 — SW6 (+) (35% bucket)
| # | Test | Action | Expected |
| :- | :--- | :--- | :--- |
| 4.1 | Hour increment | In hour-edit, press SW6 | Hour +1 |
| 4.2 | Hour wrap 23→0 | Set hour 23, press SW6 | Hour becomes 0 |
| 4.3 | Minute wrap 59→0 | In minute-edit, set 59, press SW6 | Minute becomes 0 |

### R5 — SW10 (−) (35% bucket)
| # | Test | Action | Expected |
| :- | :--- | :--- | :--- |
| 5.1 | Hour decrement | In hour-edit, press SW10 | Hour −1 |
| 5.2 | Hour wrap 0→23 | Set hour 0, press SW10 | Hour becomes 23 |
| 5.3 | Minute wrap 0→59 | Set minute 0, press SW10 | Minute becomes 59 |

### R6 — Buzzer (35% bucket)
| # | Test | Action | Expected |
| :- | :--- | :--- | :--- |
| 6.1 | Key pip | Press any of SW3/SW6/SW10/SW16 | 0.3s pip **immediately** |
| 6.2 | Alarm ring | Set alarm = current time +1 min, wait | At that minute: pip-pip 0.5s on/0.5s off for **exactly 5s**, then stops |
| 6.3 | Timeout pip | Enter edit, wait 30s | Exits to normal with a 0.3s pip |

### R7 — LED D6 (bonus, 10% bucket)
| # | Test | Action | Expected |
| :- | :--- | :--- | :--- |
| 7.1 | Off in normal | Any normal mode | LED off |
| 7.2 | Blinks in alarm edit | Enter SW16 edit | Blinks 1s period |
| 7.3 | Off in time edit | Enter SW3 edit | LED off (only display blinks) |

### R8 — EEPROM persistence (15% bucket)
| # | Test | Action | Expected |
| :- | :--- | :--- | :--- |
| 8.1 | Save + reboot | Set alarm 07:15 (SW16 flow), **power off**, power on | Alarm restored to 07:15 and armed |
| 8.2 | 00:00 armed survives | Set alarm 00:00, power cycle | Still armed (rings at 00:00) — fixed behaviour |
| 8.3 | Blank EEPROM | First ever boot (or wiped chip) | Safe defaults 00:00, no crash, no garbage digits |
| 8.4 | Corrupted EEPROM (optional) | Program 0xFF/random into addr 0..4 | Boots to safe defaults, record repaired |

### R9 — Timeout (bonus, 10% bucket)
| # | Test | Action | Expected |
| :- | :--- | :--- | :--- |
| 9.1 | Time edit timeout | Enter SW3 edit, wait 30s | Back to normal + 0.3s pip |
| 9.2 | Alarm edit timeout | Enter SW16 edit, wait 30s | Back to normal + 0.3s pip |
| 9.3 | Activity keeps it open | Press SW6 every 10s for 60s | Never times out |

## 6. Edge-case / robustness tests (impress the judges)

| # | Test | Expected |
| :- | :--- | :--- |
| E1 | **Firm key press with bounce** (press hard / fast) | Exactly one state change, never two |
| E2 | **Hold a key 3 seconds** | One event, no repeat flood |
| E3 | Press SW3 and SW16 "almost together" | One key wins per scan; no crash, no stuck mode |
| E4 | Alarm fires while you press a key | Key pip plays; ring resumes pattern after |
| E5 | Enter edit **while alarm is ringing** | Ring stops immediately (silenced by edit) |
| E6 | Set alarm to the **current time** and save | Rings within the minute (sec = 0) |
| E7 | Watch the alarm fire exactly at `hh:mm:00` | Starts precisely at the minute boundary (race-free check) |
| E8 | Edit time, wait 31s, verify **clock did not drift** | Time frozen during edit, resumes correctly after |
| E9 | Display stability | No flicker/ghosting during I2C writes or beeps (ISR-driven) |

## 7. Demo video script (20% of score)

Suggested 3–4 minute structure (record in one take, good lighting):

1. **Intro** (20s) — board, team, task.
2. **Boot** (15s) — power on → `00.00`.
3. **Basic clock** (30s) — show minute tick and hour rollover (pre-set 23:59).
4. **Time edit** (40s) — SW3 flow with blink demo, SW6/SW10, commit.
5. **Alarm edit + persistence** (45s) — SW16 flow, power cycle, alarm restored.
6. **Alarm fires** (30s) — 5s pip-pip on camera (phone close to buzzer!).
7. **Bonus** (30s) — LED D6 blink, 30s timeout + beep.
8. **Outro** (15s) — summary + thank you.

Tip: before recording, run the whole flow twice — judges value a clean take over a long one.

## 8. Troubleshooting

| Symptom | Likely cause | Fix |
| :--- | :--- | :--- |
| Buzzer silent / very quiet | Tone not in piezo resonance range | Increase `BUZZ_HALF_PERIOD_NOP` in `buzzer.c` (250 → 400 lowers to ~3kHz). Rebuild. |
| Keys skip states (one press = two steps) | (shouldn't happen) — debounce active | Check keypad wiring/pull-ups; ensure `GPIO2` CFG keeps pull-ups enabled (compare with EVK schematic) |
| Clock runs 2x/4x fast | SysTick LOAD wrong for actual clock | Confirm device runs 12 MHz (`SYS0_CLKCFG_VAL=0`); LOAD must be 11999 |
| Alarm lost after power-off | I2C write failing | Scope SCL/SDA; verify EEPROM at 0xA0; check write-cycle delay; LED indicator: record valid → magic 0xA5 |
| Display ghosting | (shouldn't happen) — 3-phase blanking active | Check digit/segment wiring vs `GPIO0[0..7]` / `GPIO1[9..12]` |
| Build error: `SN32F400.h` not found | DFP pack not installed on this PC | Install SN32F4_DFP 1.1.1 (see §1 note) |
| First boot shows garbage | EEPROM mid-write from previous session | Power-cycle; record self-repairs (magic/checksum) |

## 9. Post-test checklist

- [ ] 0 errors / 0 warnings build
- [ ] R1–R9 all pass (use the matrices above)
- [ ] E1–E9 edge cases pass
- [ ] Power-cycle persistence verified
- [ ] Demo video recorded per §7
- [ ] `git status` clean; commit pushed
