# .github — CI Pipeline & Repository Tooling

## Continuous Integration

Every push / pull request to `main` runs the **Firmware & RTL Verification
Pipeline** ([workflows/ci.yml](workflows/ci.yml)). All jobs must pass before
a merge — this is the machine that keeps the "zero warnings / zero broken
behaviour" promise of the repo.

| Job | What it does | Why it matters |
| :--- | :--- | :--- |
| `simulation` | Builds the MCU host simulation with gcc (`-Werror`) and runs the **54-check harness**; fails on any failed check | Proves firmware *behaviour* on every push, not just compilation |
| `static-analysis` | cppcheck (`--enable=warning,style,performance,portability --error-exitcode=1`) over all six firmware modules | Catches uninitialised vars, dead code, style defects |
| `production-path` | `gcc -fsyntax-only -Wall -Wextra -Werror -pedantic` on every module **without** `MOCK_SIMULATION` | Verifies the exact code that ships to the MCU (real I2C driver, real HW seams, `main()`) |
| `code-style` | `clang-format 22.1.8` (pinned via pip — identical to local tooling) `--dry-run --Werror` on all sources | Enforces the `.clang-format` standard mechanically |
| `fpga-rtl-verification` | Installs iverilog, syntax-checks every `FPGA/*.v`, verifies sources exist | Guards the FPGA RTL (currently WIP) against regressions |
| `documentation` | Verifies READMEs, changelog, TESTING, project files and contest PDFs exist | Repo integrity for the "Code & document" criterion |

> Note: the jobs use plain `gcc` on Linux for the host builds. The real ARM
> build happens in Keil MDK on the development machine (the SONiX DFP pack is
> not vendored in the repo).

## How to run the same checks locally

```bash
# simulation
gcc -std=c99 -Wall -Wextra -Werror -DMOCK_SIMULATION \
    -I MCU_Contest_2026 -I MCU_Contest_2026/sim \
    MCU_Contest_2026/{main,clock,keypad,display,buzzer,eeprom}.c \
    MCU_Contest_2026/sim/{SN32F400_mock,sim_main}.c -o clock_sim && ./clock_sim

# style gate
clang-format --dry-run --Werror MCU_Contest_2026/*.c MCU_Contest_2026/*.h

# RTL syntax
iverilog -t null FPGA/*.v
```

## Issue & Pull Request Templates

- `ISSUE_TEMPLATE/bug_report.md` — hardware/firmware bug report
  (board, repro steps, expected vs actual).
- `ISSUE_TEMPLATE/feature_request.md` — enhancement request.
- `pull_request_template.md` — PR checklist: build status, simulation
  result, docs updated, tests run on hardware.
