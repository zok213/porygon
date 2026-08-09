# MCU Contest Documentation & Rule References

This directory contains official competition documentation, rules, and reference materials for the **Da Nang MCU Contest 2026**.

---

## Directory Contents

| Document | Format | Description |
| :--- | :--- | :--- |
| **`ĐỀ THI MCU 2026.pdf`** | PDF | Official Da Nang MCU Contest 2026 Problem Statement & Evaluation Criteria for SN32F407_EVK. |
| **`ISSUE_ANALYSIS_DEEP_DIVE.md`** | Markdown | Technical audit, real-hardware verification, and deep-dive resolution report for all 6 MCU edge-case issues. |

---

## Reference Specs Summary

- **MCU Microcontroller**: SONiX SN32F407F (ARM Cortex-M0, 64KB Flash, 8KB SRAM)
- **Evaluation Board**: SN8F5708_EVK / SN32F407_EVK
- **I2C EEPROM Chip**: AT24C02 ($2\text{Kb}$ I2C EEPROM, address `0xA0`, SCL = `P0.10`, SDA = `P0.11`)
- **Keypad Matrix**: 4x4 Row/Column Matrix (Rows: `P1.4..P1.7`, Cols: `P2.4..P2.7`)
- **Display Driver**: 4-Digit Cathode 7-Segment LED (Segment: `P0.0..P0.7`, Digits: `P1.9..P1.12`)
- **Buzzer Output**: Piezoelectric Buzzer (`P3.0`)
- **Status LED**: Board D6 LED (`P3.8`, Active Low)
