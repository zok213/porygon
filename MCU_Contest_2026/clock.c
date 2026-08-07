/* ============================================================================
 * clock.c - Master timekeeping, alarm settings and UI shadow buffers
 * SN32F407_EVK Digital Clock - Da Nang MCU Contest 2026
 *
 * The master clock (time_hour/min/sec) advances in the 1ms SysTick ISR via
 * Clock_Tick1ms(). Editing operates on isolated shadow buffers so the master
 * clock can never be corrupted by UI activity.
 * ==========================================================================*/

#include "clock.h"
#include "eeprom.h"

/* ----------------------------- Master clock ------------------------------ */
volatile uint8_t time_sec = 0;
volatile uint8_t time_min = 0;
volatile uint8_t time_hour = 0;

/* ----------------------------- Alarm settings ---------------------------- */
volatile uint8_t alarm_hour = 0;
volatile uint8_t alarm_min = 0;
volatile uint8_t alarm_armed = 0;

/* ------------------------ UI shadow edit buffers ------------------------- */
volatile uint8_t edit_time_hour = 0;
volatile uint8_t edit_time_min = 0;
volatile uint8_t edit_alarm_hour = 0;
volatile uint8_t edit_alarm_min = 0;

/* ------------------------------------------------------------------------- */
void Clock_LoadAlarmFromEEPROM(void)
{
    uint8_t h = 0, m = 0, armed = 0;

    EEPROM_LoadAlarm(&h, &m, &armed); /* validates magic + checksum      */
    alarm_hour = h;
    alarm_min = m;
    alarm_armed = armed;
}

void Clock_BeginTimeEdit(void)
{
    edit_time_hour = time_hour;
    edit_time_min = time_min;
}

void Clock_CommitTimeEdit(void)
{
    time_hour = edit_time_hour;
    time_min = edit_time_min;
    time_sec = 0;
}

void Clock_BeginAlarmEdit(void)
{
    edit_alarm_hour = alarm_hour;
    edit_alarm_min = alarm_min;
}

void Clock_CommitAlarmEdit(void)
{
    alarm_hour = edit_alarm_hour;
    alarm_min = edit_alarm_min;
    alarm_armed = 1;
    EEPROM_SaveAlarm(alarm_hour, alarm_min, alarm_armed);
}

void Clock_AdjustEdit(int8_t inc, SystemMode_t mode)
{
    volatile uint8_t* h;
    volatile uint8_t* m;

    if (mode >= MODE_EDIT_AL_HOUR)
    {
        h = &edit_alarm_hour;
        m = &edit_alarm_min;
    }
    else
    {
        h = &edit_time_hour;
        m = &edit_time_min;
    }

    if (mode == MODE_EDIT_HOUR || mode == MODE_EDIT_AL_HOUR)
    {
        *h = (uint8_t)((*h + inc + 24) % 24);
    }
    else
    {
        *m = (uint8_t)((*m + inc + 60) % 60);
    }
}

void Clock_Tick1ms(void)
{
    /* Time is frozen while the user edits the time itself. */
    if (system_mode == MODE_EDIT_HOUR || system_mode == MODE_EDIT_MIN)
    {
        return;
    }

    static uint16_t t1s = 0;
    if (++t1s < 1000)
    {
        return;
    }
    t1s = 0;

    if (++time_sec >= 60)
    {
        time_sec = 0;
        if (++time_min >= 60)
        {
            time_min = 0;
            time_hour = (uint8_t)((time_hour + 1) % 24);
        }
    }
}

uint8_t Clock_AlarmMatchNow(void)
{
    uint8_t h, m, s, ah, am;

    /* Atomic snapshot: a rollover between the reads could otherwise fire the
     * alarm one minute early (tear between min-read and sec-read). */
    __disable_irq();
    h = time_hour;
    m = time_min;
    s = time_sec;
    ah = alarm_hour;
    am = alarm_min;
    __enable_irq();

    return (h == ah && m == am && s == 0) ? 1 : 0;
}
