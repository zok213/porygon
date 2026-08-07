/* ============================================================================
 * keypad.c - 4x4 matrix keypad scanning with time-based debounce
 * SN32F407_EVK Digital Clock - Da Nang MCU Contest 2026
 *
 * Keypad wiring:
 *   Rows  (scan output) : GPIO1 pins 4..7, driven low one at a time
 *   Cols  (read input)  : GPIO2 pins 4..7, active low with pull-ups
 *
 * Debounce design (replaces the previous raw edge detector that could fire
 * twice on contact bounce):
 *   1. Any change of the raw input restarts a 20ms "stable" window.
 *   2. A key is reported only after it has been read stable for 20ms.
 *   3. After one event the scan latches until the key has been released
 *      stably for 20ms, so a single mechanical press can never produce
 *      more than one event, even with severe bounce.
 * ==========================================================================*/

#include "keypad.h"
#include "system.h"

#define KEY_DEBOUNCE_MS 20UL /* stable press window before emission  */
#define KEY_RELEASE_MS 20UL  /* stable release window before re-arm  */

/* ------------------------------------------------------------------------- */
/* Hardware abstraction: real-board implementation.                          */
/* The host simulation build (MOCK_SIMULATION) replaces these two functions  */
/* with a matrix model so the scan logic runs on a PC.                       */
#ifndef MOCK_SIMULATION

void Keypad_HW_DriveRow(uint8_t row)
{
    SN_GPIO1->BSET = (0x0F << 4);      /* all rows high                 */
    SN_GPIO1->BCLR = (1 << (4 + row)); /* drive active row low          */
}

uint8_t Keypad_HW_ReadColumnBits(void)
{
    return (uint8_t)((SN_GPIO2->DATA >> 4) & 0x0F); /* cols 4..7, low=on */
}

#endif /* !MOCK_SIMULATION */

/* ------------------------------------------------------------------------- */
/* Raw matrix read: returns the first pressed key code or 0.                */
static uint8_t Keypad_ReadRaw(void)
{
    for (int r = 0; r < 4; r++)
    {
        Keypad_HW_DriveRow((uint8_t)r);
        uint8_t cols = Keypad_HW_ReadColumnBits();
        for (int c = 0; c < 4; c++)
        {
            if (!(cols & (1u << c)))
            {
                /* Row 3 carries SW16 (column 0) and SW15 (column 1). */
                if (r == 3 && c == 0)
                    return 16;
                if (r == 3 && c == 1)
                    return 15;
                return (uint8_t)(r * 4 + c + 3);
            }
        }
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
uint8_t Keypad_Scan(void)
{
    static uint8_t raw_prev = 0;    /* last raw sample                 */
    static uint8_t emit_latch = 0;  /* event emitted, waiting release  */
    static uint8_t release_run = 0; /* stable-release window active    */
    static uint32_t change_ms = 0;  /* ms of last raw change           */
    static uint32_t release_ms = 0; /* ms of release window start      */

    uint8_t raw = Keypad_ReadRaw();
    uint32_t now = system_ms_counter;

    /* --- Waiting for a confirmed release after an emitted event --------- */
    if (emit_latch)
    {
        if (raw)
        {
            release_run = 0; /* bounced back down: restart */
        }
        else if (!release_run)
        {
            release_run = 1; /* release candidate          */
            release_ms = now;
        }
        else if ((uint32_t)(now - release_ms) >= KEY_RELEASE_MS)
        {
            emit_latch = 0; /* fully released, re-arm     */
            release_run = 0;
            raw_prev = 0;
        }
        return 0;
    }

    /* --- Input changed: restart the debounce window --------------------- */
    if (raw != raw_prev)
    {
        raw_prev = raw;
        change_ms = now;
        return 0;
    }

    /* --- Stable for the debounce window: emit one edge ------------------ */
    if (raw && (uint32_t)(now - change_ms) >= KEY_DEBOUNCE_MS)
    {
        emit_latch = 1;
        return raw;
    }

    return 0;
}
