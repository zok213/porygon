## Description of Changes

Provide a summary of the changes proposed in this Pull Request.

- Target Track: [ ] MCU Track (SN32F407)  [ ] FPGA Track (Gowin GW1N)
- Target Branch: `MCU_dev` / `FPGA_dev`

## Technical Verification Checklist

- [ ] Firmware compiles with 0 Errors, 0 Warnings in Keil MDK-ARM uVision.
- [ ] Checked for EEPROM memory boundary safety (`0xA5` magic byte & range clamping).
- [ ] Verified non-blocking 7-segment display multiplexing (zero ghosting).
- [ ] Confirmed Watchdog supervisor feeding in super-loop.
- [ ] All Mermaid diagrams render cleanly without syntax errors.

## Testing Evidence

Attach oscilloscope waveforms, serial terminal logs, or video demonstration links.
