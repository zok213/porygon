/* ============================================================================
 * buzzer.c - Key beep, alarm ring pattern and tone generation
 * SN32F407_EVK Digital Clock - Da Nang MCU Contest 2026
 *
 * Hardware: piezo buzzer on GPIO3 pin 0 (NPN transistor driver, active high).
 *
 * Tone generation:
 *   A 4-5kHz square wave is produced by a short NOP-loop burst inside the
 *   1ms ISR. The loop length is tuned for 12MHz: ~250 iterations per half
 *   period (about 100-125us, i.e. roughly 4-5kHz), which sits in the
 *   audible range and near the piezo resonance - the previous 80-iteration
 *   value produced an inaudible ~10-15kHz tone. The burst is limited to 2
 *   full cycles so the ISR stays bounded (~0.5ms worst case during beeps).
 *
 * Patterns:
 *   key pip      : 300ms continuous tone
 *   alarm ring   : 5s of pip-pip, 0.5s ON / 0.5s OFF
 * ==========================================================================*/

#include "buzzer.h"
#include "system.h"

#define BEEP_KEY_MS       300UL
#define ALARM_RING_MS     5000UL
#define ALARM_TOGGLE_MS   1000UL

#define BUZZ_HALF_PERIOD_NOP  250   /* ~4-5kHz square wave @ 12MHz          */
#define BUZZ_BURST_CYCLES     2     /* full cycles per 1ms tick (bounded)   */

#define BUZZER_PIN   1               /* GPIO3 bit 0                          */

static volatile uint16_t buzzer_beep_ms   = 0;
static volatile uint8_t  alarm_ringing    = 0;
static volatile uint32_t alarm_ring_ms    = 0;
static volatile uint8_t  buzzer_active    = 0;

/* ------------------------------------------------------------------------- */
void Buzzer_BeepKey(void)
{
    buzzer_beep_ms = BEEP_KEY_MS;
}

void Buzzer_StartAlarm(void)
{
    if (!alarm_ringing) {
        alarm_ringing = 1;
        alarm_ring_ms = 0;
    }
}

void Buzzer_CancelAlarm(void)
{
    alarm_ringing = 0;
}

uint8_t Buzzer_IsAlarmRinging(void)
{
    return alarm_ringing;
}

/* ------------------------------------------------------------------------- */
/* Hardware abstraction: real-board implementation.                          */
/* The host simulation build (MOCK_SIMULATION) overrides these two functions */
/* so the harness can observe tone activity deterministically.               */
#ifndef MOCK_SIMULATION

void Buzzer_HW_ToneOn(void)
{
    SN_GPIO3->BSET = BUZZER_PIN;
}

void Buzzer_HW_ToneOff(void)
{
    SN_GPIO3->BCLR = BUZZER_PIN;
}

#endif /* !MOCK_SIMULATION */

/* ------------------------------------------------------------------------- */
void Buzzer_Tick1ms(void)
{
    /* --- Pattern selection: key beep has priority over alarm ring ------- */
    if (buzzer_beep_ms) {
        buzzer_beep_ms--;
        buzzer_active = 1;
    } else if (alarm_ringing) {
        buzzer_active = (++alarm_ring_ms < ALARM_RING_MS)
                      ? ((alarm_ring_ms % ALARM_TOGGLE_MS) < (ALARM_TOGGLE_MS / 2))
                      : (alarm_ringing = 0);
    } else {
        buzzer_active = 0;
    }

    /* --- Tone generation (short burst, bounded ISR time) ---------------- */
    if (buzzer_active) {
        for (int i = 0; i < BUZZ_BURST_CYCLES; i++) {
            Buzzer_HW_ToneOn();
            for (volatile int d = 0; d < BUZZ_HALF_PERIOD_NOP; d++) {}
            Buzzer_HW_ToneOff();
            for (volatile int d = 0; d < BUZZ_HALF_PERIOD_NOP; d++) {}
        }
    } else {
        Buzzer_HW_ToneOff();
    }
}
