# Contribution and Git Flow Guidelines

## Branch Management Strategy

This repository utilizes a multi-track Git Flow branch hierarchy:

| Branch Name | Role & Purpose | Direct Pushes Allowed? |
| :--- | :--- | :--- |
| `main` | Production-ready stable release branch. | No (Requires PR review) |
| `release` | Release candidate staging branch for QA. | No (Requires PR review) |
| `MCU_main` | Stable production branch for MCU (SN32F407) track. | No (Requires PR review) |
| `MCU_dev` | Active development branch for MCU firmware features. | Yes (Developers) |
| `FPGA_main` | Stable production branch for FPGA (Gowin GW1N) track. | No (Requires PR review) |
| `FPGA_dev` | Active development branch for Verilog/VHDL RTL logic. | Yes (Developers) |

---

## Commit Message Convention

All commit messages must follow Conventional Commits formatting:

- `feat(mcu)`: Add new feature to MCU clock system
- `feat(fpga)`: Add new module to Gowin FPGA RTL
- `fix(display)`: Resolve 7-segment digit ghosting issue
- `docs`: Update technical documentation or README
- `refactor`: Structural code cleanup without logic change

---

## Pull Request Guidelines

1. Target the appropriate development branch (`MCU_dev` or `FPGA_dev`).
2. Ensure Keil MDK-ARM project builds with **0 Error(s), 0 Warning(s)**.
3. Verify all Mermaid diagrams in documentation render cleanly.
4. Obtain at least 1 peer review approval prior to merging into `MCU_main` or `main`.
