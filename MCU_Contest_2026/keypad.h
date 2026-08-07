/* ============================================================================
 * keypad.h - 4x4 matrix keypad scanning with time-based debounce
 * ==========================================================================*/
#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>

/* Debounced, edge-triggered key scan. Returns the key code of a newly
 * confirmed press (KEY_SETUP/KEY_PLUS/KEY_MINUS/KEY_ALARM) or 0 when no
 * new event exists. Holds produce exactly one event per physical press.
 *
 * Must be called regularly from the main loop; debounce timing is based on
 * the system 1ms counter so the call rate does not affect behaviour. */

/* Hardware abstraction seam: drive one keypad row low, then read the four
 * column input bits (bit set = column low = key pressed in that row).
 * On the real board these touch GPIO1/GPIO2 directly; the host simulation
 * overrides them with a matrix model so the scan logic is fully testable. */
void Keypad_HW_DriveRow(uint8_t row);
uint8_t Keypad_HW_ReadColumnBits(void);

uint8_t Keypad_Scan(void);

#endif /* KEYPAD_H */
