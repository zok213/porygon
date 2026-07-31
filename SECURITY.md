# Security Policy and Vulnerability Management

## Security Architecture Overview

This project implements hardware and software security controls for the **SN32F407** ARM Cortex-M0 microcontroller and **Gowin GW1N** FPGA platforms.

---

## Microcontroller Flash Security & Readout Protection (RDP)

To prevent unauthorized EEPROM modification or Flash firmware extraction:
1. **Flash Readout Protection (RDP Level 1)**: Recommended configuration for production deployments to lock SWD/JTAG debug access.
2. **EEPROM Magic Signature Validation**: Prevents execution on corrupted memory layouts (`0xA5` magic header required).
3. **Watchdog Supervisor Guarding**: Ensures hardware reset upon software execution lockup or stack corruption.

---

## Reporting Vulnerabilities

If you discover a potential security flaw in the firmware drivers or FPGA hardware constraints:
1. Do **NOT** open a public GitHub issue.
2. Send a detailed report to the project security maintainers via private channels.
3. Include target hardware details, reproduction steps, and potential exploit vectors.
