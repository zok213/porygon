/* ============================================================================
 * display.c - 4-digit 7-segment multiplexed display + status LED D6
 * SN32F407_EVK Digital Clock - Da Nang MCU Contest 2026
 *
 * Hardware:
 *   Segment bus A..G,DP : GPIO0 pins 0..7   (active high)
 *   Digit select D1..D4  : GPIO1 pins 9..12  (active high)
 *   Status LED D6        : GPIO3 pin 8       (active low)
 *
 * Layout: D1 = hour tens, D2 = hour units + DP (HH.MM separator),
 *         D3 = minute tens, D4 = minute units.
 * ==========================================================================*/

#include "display.h"
#include "system.h"

static const uint8_t seg7[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

#define SEG_DP 0x80
#define SEGMENT_MASK 0x00FF   /* GPIO0 bits 0..7                    */
#define DIGIT_MASK 0x1E00     /* GPIO1 bits 9..12                   */
#define LED_D6 0x100          /* GPIO3 bit 8, active low            */
#define BLINK_PERIOD_MS 500UL /* 500ms on / 500ms off = 1s period */
#define DP_PULSE_MS 100UL     /* DP tick-pulse width in NORMAL mode */

/* ------------------------------------------------------------------------- */
/* Hardware abstraction: real-board implementation.                          */
/* The host simulation build (MOCK_SIMULATION) overrides these two functions */
/* so the harness can observe the LED state deterministically.               */
#ifndef MOCK_SIMULATION

void Display_HW_LedOn(void)
{
    SN_GPIO3->BCLR = LED_D6; /* active low: on  */
}

void Display_HW_LedOff(void)
{
    SN_GPIO3->BSET = LED_D6; /* off             */
}

#endif /* !MOCK_SIMULATION */

/* ------------------------------------------------------------------------- */
void Display_Tick1ms(void)
{
    static uint8_t scan_phase = 0;
    static uint8_t dig = 0;
    static uint32_t blink_ms = 0;
    static uint8_t blink_on = 1;

    /* --- Phase 0: blank (kill old digit + segments before new data) ----- */
    if (scan_phase == 0)
    {
        SN_GPIO1->BCLR = DIGIT_MASK;
        SN_GPIO0->BCLR = SEGMENT_MASK;
    }
    /* --- Phase 1: load pattern for the active digit, then enable it ----- */
    else
    {
        uint8_t h = (system_mode >= MODE_EDIT_AL_HOUR) ? edit_alarm_hour
                    : (system_mode != MODE_NORMAL)     ? edit_time_hour
                                                       : time_hour;
        uint8_t m = (system_mode >= MODE_EDIT_AL_HOUR) ? edit_alarm_min
                    : (system_mode != MODE_NORMAL)     ? edit_time_min
                                                       : time_min;

        uint8_t off_h =
            (system_mode == MODE_EDIT_HOUR || system_mode == MODE_EDIT_AL_HOUR) && !blink_on;
        uint8_t off_m =
            (system_mode == MODE_EDIT_MIN || system_mode == MODE_EDIT_AL_MIN) && !blink_on;

        /* DP behaviour (hardware-verified on the board):
         *  - NORMAL : short tick-pulse at the start of every second
         *             (real-clock "tick" effect), off for the rest.
         *  - EDIT   : solid DP as the HH.MM separator (never conflicts with
         *             the blink effect - blink only blanks digits). */
        uint8_t dp_bit = (system_mode == MODE_NORMAL)
                             ? ((ms_in_this_sec < DP_PULSE_MS) ? SEG_DP : 0x00)
                             : SEG_DP;

        const uint8_t out[4] = {off_h ? 0 : seg7[h / 10], off_h ? 0 : (seg7[h % 10] | dp_bit),
                                off_m ? 0 : seg7[m / 10], off_m ? 0 : seg7[m % 10]};

        SN_GPIO0->BSET = out[dig];
        SN_GPIO1->BSET = (uint32_t)(1 << (9 + dig));
        dig = (dig + 1) & 3;
    }
    scan_phase ^= 1;

    /* --- Blink generator (shared by digit blink and LED blink) ----------- */
    if (++blink_ms >= BLINK_PERIOD_MS)
    {
        blink_ms = 0;
        blink_on ^= 1;
    }

    /* --- Status LED D6: blink (1s period) only in alarm edit modes ------- */
    if (system_mode >= MODE_EDIT_AL_HOUR)
    {
        if (blink_on)
        {
            Display_HW_LedOn();
        }
        else
        {
            Display_HW_LedOff();
        }
    }
    else
    {
        Display_HW_LedOff();
    }
}
