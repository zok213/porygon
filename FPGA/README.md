# FPGA Track — Gowin GW1N Series

## Platform
- **Target Device**: Gowin GW1N-UV1P5 / GW1NSR-LV4C (Kiwi 1P5 / Kiwi Nano 4K)
- **Onboard Oscillator**: 24 MHz / 27 MHz
- **Target System Clock**: 50 MHz (via PLL)

## Module Overview

| Module | File | Description |
| :--- | :--- | :--- |
| PLL | `pll_50mhz.v` | Clock synthesis: 24 MHz → 50 MHz |
| Debouncer | `debouncer.v` | 4-sample shift register debouncer (1-cycle pulse output) |
| Breathing PWM | `breathing_pwm.v` | 3-mode PWM: off, static, breathing, blink |
| UART TX | `uart_tx.v` | 115200 bps, 8N1, polled transmit |
| Top-Level | `top.v` | Supervisor FSM integrating all modules |

## Build Flow

1. Open Gowin EDA (Diamond / Programmer)
2. Create new project targeting GW1N-UV1P5
3. Add all `.v` files from this directory
4. Run Synthesis → Place & Route → Generate bitstream
5. Flash via Gowin Programmer

## Timing Constraints

- System clock: 50 MHz (20 ns period)
- UART baud: 115200 bps (86.8 µs per bit at 50 MHz)
- PWM period: 1000 clock ticks (20 µs, 50 kHz)
- Debounce window: 4 clock ticks (80 ns, software-filtered)