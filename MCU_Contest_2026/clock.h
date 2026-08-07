/* ============================================================================
 * clock.h - Master timekeeping, alarm settings and UI shadow buffers
 * ==========================================================================*/
#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>
#include "system.h"

/* Restore alarm settings from EEPROM (magic + checksum validated inside). */
void Clock_LoadAlarmFromEEPROM(void);

/* Copy master clock into the time edit shadow buffers. */
void Clock_BeginTimeEdit(void);

/* Commit the time edit shadow buffers into the master clock, seconds = 0. */
void Clock_CommitTimeEdit(void);

/* Copy alarm settings into the alarm edit shadow buffers. */
void Clock_BeginAlarmEdit(void);

/* Commit alarm edit and persist it to EEPROM, arming the alarm. */
void Clock_CommitAlarmEdit(void);

/* Increment/decrement the active edit field (hour or minute) with wraparound.
 * inc = +1 (KEY_PLUS) or -1 (KEY_MINUS). */
void Clock_AdjustEdit(int8_t inc, SystemMode_t mode);

/* Advance the master clock by one 1ms tick (seconds/minutes/hours rollover).
 * The clock is intentionally paused while the user edits time
 * (MODE_EDIT_HOUR / MODE_EDIT_MIN) so the value on screen is stable. */
void Clock_Tick1ms(void);

/* IRQ-safe snapshot: returns 1 when the master clock exactly matches the
 * alarm time at second 0 (hh:mm:00). Reads all fields with interrupts
 * disabled so a SysTick rollover cannot tear the comparison. */
uint8_t Clock_AlarmMatchNow(void);

#endif /* CLOCK_H */
