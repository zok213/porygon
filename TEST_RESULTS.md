# Hardware Test Results — SN32F407 Digital Clock (v1.1.0-rc3)

> Fill this in while running the matrix in `MCU_Contest_2026/TESTING.md`.
> This file is the **evidence that the improved firmware passed hardware
> validation** — required before merging `MCU_dev → MCU_main → release → main`.

## Test session

| Field | Value |
| :--- | :--- |
| Date | |
| Tester | |
| Board / serial | |
| Branch + tag | `MCU_dev` @ `v1.1.0-rc3` (`git describe --tags` output: ) |
| Build result (Keil) | errors: __ / warnings: __ |
| Buzzer audible? (tone ok?) | ☐ yes / ☐ no (if no: tuned `BUZZ_HALF_PERIOD_NOP` to __, retested ☐) |

## Requirement matrix (TESTING.md §5)

| Req | Result | Notes |
| :--- | :--- | :--- |
| R1 boot / rollovers | ☐ pass / ☐ fail | |
| R2 SW3 edit + blink | ☐ pass / ☐ fail | |
| R3 SW16 alarm edit + save | ☐ pass / ☐ fail | |
| R4 SW6 + | ☐ pass / ☐ fail | |
| R5 SW10 − | ☐ pass / ☐ fail | |
| R6 buzzer (pip / 5s ring / timeout pip) | ☐ pass / ☐ fail | |
| R7 LED D6 | ☐ pass / ☐ fail | |
| R8 EEPROM persistence (incl. power cycle) | ☐ pass / ☐ fail | |
| R9 30s timeout | ☐ pass / ☐ fail | |

## Edge cases (TESTING.md §6)

| Test | Result | Notes |
| :--- | :--- | :--- |
| E1 bouncy press = one event | ☐ pass / ☐ fail | |
| E2 hold 3s = one event | ☐ pass / ☐ fail | |
| E3 simultaneous keys | ☐ pass / ☐ fail | |
| E4 key pip during alarm ring | ☐ pass / ☐ fail | |
| E5 edit silences ring | ☐ pass / ☐ fail | |
| E6 alarm = current time | ☐ pass / ☐ fail | |
| E7 ring starts at hh:mm:00 | ☐ pass / ☐ fail | |
| E8 clock frozen during time edit | ☐ pass / ☐ fail | |
| E9 display stability | ☐ pass / ☐ fail | |

## Persistence & corruption checks

- [ ] Alarm 07:15 survives power cycle
- [ ] Alarm 00:00 armed survives power cycle
- [ ] Blank/corrupt EEPROM boots to safe defaults (no garbage digits)

## Verdict

- [ ] **ALL PASS → merge `MCU_dev → MCU_main → release → main`**
- [ ] Failures found → log them below, fix on `MCU_dev`, re-test

### Failure log

| Date | Symptom | Root cause (if known) | Fix | Re-tested |
| :--- | :--- | :--- | :--- | :--- |
| | | | | ☐ |
