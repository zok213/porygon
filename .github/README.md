# .github — CI Pipeline & Repository Tooling

## Continuous Integration on `main`

Every push / pull request to `main` runs the **Repository Integrity
Pipeline** ([workflows/ci.yml](workflows/ci.yml)).

> **`main` carries the hardware-proven baseline (Quang build, verbatim).**
> The pipeline therefore guards the baseline tree (files + docs + FPGA
> syntax) rather than compiling firmware — the full verification stack
> runs on `MCU_dev`, where the improved firmware lives.

| Job | What it does |
| :--- | :--- |
| `fpga-rtl-verification` | Installs iverilog, syntax-checks every `FPGA/*.v`, verifies the FPGA sources exist |
| `documentation` | Verifies the baseline firmware files are present (7 files), mandatory documents, and the contest PDFs |

## The full verification stack (on `MCU_dev`)

The improved firmware on `MCU_dev` runs a richer pipeline: 55-check host
simulation (gcc build + run, exit-code gated), cppcheck, production-path
syntax (`-Werror -pedantic`), clang-format style gate, FPGA iverilog, and
documentation integrity. See that branch's `.github/workflows/ci.yml`.

## Issue & Pull Request Templates

- `ISSUE_TEMPLATE/bug_report.md` — hardware/firmware bug report
  (board, repro steps, expected vs actual).
- `ISSUE_TEMPLATE/feature_request.md` — enhancement request.
- `pull_request_template.md` — PR checklist: build status, tests on
  hardware, docs updated.
