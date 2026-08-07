# FPGA Track — Gowin GW1N Series (WIP)

> ⚠️ **STATUS: WORK-IN-PROGRESS SKELETON.** The RTL compiles cleanly with
> iverilog, but it does **NOT yet satisfy the FPGA đề bài** (`ĐỀ THI FGPA
> 2026.docx.pdf` at the repo root). See [Gaps vs. đề bài](#gaps-vs-đề-bài)
> and [Roadmap](#roadmap) below. Do not submit this as a finished track.

## Platform

| Item | Value |
| :--- | :--- |
| Target devices | Gowin `GW1N-UV1P5` (Kiwi 1P5, QN48X) or `GW1NSR-LV4C` (Kiwi Nano 4K, MG64P) |
| Onboard oscillator | 24 MHz or 27 MHz (check the board schematic) |
| Target system clock | 50 MHz (Gowin PLL IP core) |
| EDA | GOWIN EDA (IP Core Generator for the PLL) |
| Required protocol | UART TX 115200 bps, 8N1, no parity |

## Module Overview

| Module | File | Status |
| :--- | :--- | :--- |
| Clock | `pll_50mhz.v` | ⚠️ **Behavioral stub** — currently *divides* the input clock; the real Gowin PLL IP core must be generated in GOWIN EDA and instantiated (24/27 MHz → 50 MHz) |
| Debounce | `debouncer.v` | ⚠️ 4-sample shift register (level output, ~80 ns window) — needs a ~10-20 ms window and a **single-cycle pulse** output per press/hold |
| PWM | `breathing_pwm.v` | ⚠️ Has 3+ modes but not the required LOW 25% / HIGH 100% / AUTO breathing with an **exactly 2.0 s** cycle |
| UART TX | `uart_tx.v` | ✅ Standard 115200 8N1 bit-bang core; `CLKS_PER_BIT = 50000000/115200 = 434` (0.006% error, well within ±2%) |
| Top | `top.v` | ⚠️ Minimal 2-state FSM, one button, sends a single byte `'A'` — needs the full supervisor FSM + `"MODE: LOW \r\n"` / `"MODE: HIGH \r\n"` / `"MODE: AUTO \r\n"` strings |

## What the đề bài requires (summary)

1. **Khối 1 (2.0đ)** — Gowin PLL IP: onboard clock → **50 MHz**; debounce for
   both buttons with **one pulse per press/hold**.
2. **Khối 2 (2.5đ)** — PWM LED: Mode 1 LOW = 25% duty, Mode 2 HIGH = 100%,
   Mode 3 AUTO = breathing 0→100→0 in **exactly 2.0 s**, flicker-free rate.
3. **Khối 3 (2.5đ)** — UART TX 115200 8N1 from the 50 MHz PLL output.
4. **Khối 4 (3.0đ)** — FSM: reset → LOW (send `"MODE: LOW \r\n"` once);
   Button 1 toggles LOW↔HIGH (send string each transition); Button 2 →
   AUTO (send string); in AUTO, Button 1 → LOW.

**Deliverables**: source code + `.cst` pin constraints, testbench +
waveform results, technical report PDF (board choice, block diagram, FSM
diagram, baud divider table, serial-terminal proof).

## Build flow (GOWIN EDA)

1. Open GOWIN EDA, create a project for the chosen board (Kiwi 1P5 or
   Kiwi Nano 4K).
2. Add the `.v` files from this folder.
3. Generate the **PLL IP core** (IP Core Generator) and replace the stub.
4. Write the `.cst` physical constraints from the board schematic
   (clock pin, LED pin, button pins, UART_TX via the GWU2 bridge).
5. Synthesize → Place & Route → Generate bitstream → Program.
6. Simulate with a testbench first (iverilog / Gowin Simulator) and capture
   waveforms for the report.

## Gaps vs. đề bài

- [ ] Real PLL IP core (50 MHz output)
- [ ] Debounce: 10-20 ms window + single-cycle pulse
- [ ] PWM modes LOW 25% / HIGH 100% / AUTO 2.0 s breathing
- [ ] UART sends the required `"MODE: ... \r\n"` ASCII strings
- [ ] Full supervisor FSM (2 buttons, exact transition rules)
- [ ] `.cst` constraints file
- [ ] Testbench + waveform evidence
- [ ] Technical report PDF

## Roadmap

Complete the gaps above in order (FSM → PLL → debounce → UART strings →
testbench → CST → report), then merge to `FPGA_main` per the repository
branch policy. The team's chosen board is **Kiwi Nano 4K (GW1NSR-LV4C)**.
