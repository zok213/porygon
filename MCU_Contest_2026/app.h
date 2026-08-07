/* ============================================================================
 * app.h - Application boot sequence and super-loop iteration
 *
 * Exposing these two entry points lets the host simulation harness drive the
 * exact same logic the hardware runs: App_Init() performs the power-on boot
 * sequence and App_LoopIteration() is one pass of the main super-loop.
 * ==========================================================================*/
#ifndef APP_H
#define APP_H

#include <stdint.h>

/* Power-on boot: hardware init + alarm restore from EEPROM. */
void App_Init(void);

/* One super-loop pass: key scan/process, edit timeout, alarm trigger. */
void App_LoopIteration(void);

/* 1ms heartbeat ISR (SysTick). Exposed so the host simulation can drive
 * the exact interrupt the hardware generates every millisecond. */
void SysTick_Handler(void);

#endif /* APP_H */
