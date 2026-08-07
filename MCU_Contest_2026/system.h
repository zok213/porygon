/* ============================================================================
 * system.h - Shared types and system-wide state bus
 * SN32F407_EVK Digital Clock - Da Nang MCU Contest 2026
 *
 * This header is the single point where all modules exchange state.
 * Everything declared here is volatile because it is shared between the
 * 1ms SysTick ISR (producer) and the main super-loop (consumer).
 * ----------------------------------------------------------------------------
 * State ownership map:
 *   system_mode        -> main.c      (FSM)
 *   time_*             -> clock.c     (master clock, written by ISR path)
 *   alarm_*            -> clock.c     (alarm settings, written by FSM)
 *   edit_*             -> clock.c     (UI shadow buffers, written by FSM)
 *   system_ms_counter  -> main.c      (monotonic ms, written by ISR)
 *   inactivity_ms      -> main.c      (key inactivity, written by ISR)
 * ==========================================================================*/
#ifndef SYSTEM_H
#define SYSTEM_H

#include "SN32F400.h"
#include <stdint.h>

/* ------------------------------- FSM states ------------------------------ */
typedef enum
{
    MODE_NORMAL = 0,       /* clock running, no edit */
    MODE_EDIT_HOUR = 1,    /* time hour edit (HH blinks)   */
    MODE_EDIT_MIN = 2,     /* time minute edit (MM blinks) */
    MODE_EDIT_AL_HOUR = 3, /* alarm hour edit (HH blinks)  */
    MODE_EDIT_AL_MIN = 4   /* alarm minute edit (MM blinks)*/
} SystemMode_t;

/* ------------------------------ Key codes -------------------------------- */
#define KEY_SETUP 3  /* SW3  : time edit state machine        */
#define KEY_PLUS 6   /* SW6  : increment hour / minute        */
#define KEY_MINUS 10 /* SW10 : decrement hour / minute        */
#define KEY_ALARM 16 /* SW16 : alarm edit state machine       */

/* --------------------------- Timing constants ---------------------------- */
#define TIMEOUT_MS 30000UL /* edit-mode inactivity rollback   */

/* ------------------------ Shared volatile state bus ---------------------- */
extern volatile SystemMode_t system_mode;

extern volatile uint8_t time_sec;
extern volatile uint8_t time_min;
extern volatile uint8_t time_hour;

extern volatile uint8_t alarm_hour;
extern volatile uint8_t alarm_min;
extern volatile uint8_t alarm_armed;

extern volatile uint8_t edit_time_hour;
extern volatile uint8_t edit_time_min;
extern volatile uint8_t edit_alarm_hour;
extern volatile uint8_t edit_alarm_min;

extern volatile uint32_t system_ms_counter; /* monotonic ms tick      */
extern volatile uint32_t inactivity_ms;     /* ms since last key      */

#endif /* SYSTEM_H */
