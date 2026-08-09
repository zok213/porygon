# RTE/ — Keil Run-Time Environment (CMSIS + SONiX Device Support)

This folder is **managed by Keil MDK's Run-Time Environment manager** — it
should not be hand-edited. It contains the device support files that the
SONiX SN32F4_DFP 1.1.1 pack installs into the project.

```
RTE/
├── Device/SN32F407F/
│   ├── startup_SN32F400.s      # reset vector table, stack (512 B), heap (0),
│   │                           # weak default SysTick_Handler
│   └── system_SN32F400.c       # clock config: SYS0_CLKCFG_VAL=0 → IHRC 12 MHz
└── _Target_1/
    └── RTE_Components.h        # generated component defines (CMSIS CORE)
```

## What the firmware needs from here

- `startup_SN32F400.s` provides the **weak** `SysTick_Handler` symbol; the
  firmware's strong definition in `main.c` overrides it — this is how the
  1ms ISR gets wired to the vector table.
- `system_SN32F400.c` runs before `main()` and leaves the system at
  **12 MHz IHRC** (no PLL), which is the clock the SysTick reload constant
  (`SYSTICK_RELOAD_1MS = 11999`) is derived from.

## Important

- `SN32F400.h` / `SN32F400_Def.h` (the real device headers) live **inside
  the installed pack** (`...\SONiX\SN32F4_DFP\1.1.1\Device\Include\`), not
  in this folder.
- The SONiX SN32F4_DFP 1.1.1 pack is no longer on the vendor's download
  server — keep your local copy safe; a fresh machine needs it to build.
- This is the `main` branch: the hardware-proven baseline. The improved
  firmware on `MCU_dev` has its own `RTE/` (identical) plus a host
  simulation layer (`sim/`) that is not present here.
