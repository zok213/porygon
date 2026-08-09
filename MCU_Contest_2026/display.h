/* ============================================================================
 * display.h - 4-digit 7-segment multiplexed display + status LED D6
 * ==========================================================================*/
#ifndef DISPLAY_H
#define DISPLAY_H

/* Drive one multiplexing step (called every 1ms from the SysTick ISR).
 * A 3-phase anti-ghosting sequence is enforced:
 *   phase 0: blank all digit lines and segment lines
 *   phase 1: write the segment pattern, then enable the target digit
 * In edit modes the active field blinks at a 1s period (0.5s on / 0.5s off)
 * and the status LED D6 blinks during alarm edit modes only. */
void Display_Tick1ms(void);

/* Width of the DP tick-pulse in NORMAL mode (ms at the start of each second).
 * Exposed so the host simulation can verify the pulse deterministically. */
#define DP_PULSE_MS 100UL

/* Hardware abstraction seam for the status LED D6 pin (GPIO3.8, active low:
 * LedOn = pin low). The host simulation overrides these to observe the LED
 * state directly. */
void Display_HW_LedOn(void);
void Display_HW_LedOff(void);

#endif /* DISPLAY_H */
