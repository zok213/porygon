/* ============================================================================
 * main.c - Application glue: hardware init, key FSM, main loop, SysTick ISR
 * SN32F407_EVK Digital Clock - Da Nang MCU Contest 2026
 *
 * Architecture:
 *   SysTick (1ms)   -> Clock_Tick1ms()  timekeeping
 *                      Display_Tick1ms() 7SEG multiplex + LED D6
 *                      Buzzer_Tick1ms()  beep / alarm pattern + tone
 *   main super-loop -> Keypad_Scan() (debounced) -> Process_Key() FSM
 *                      edit-mode inactivity timeout (30s rollback)
 *                      alarm trigger check (IRQ-safe snapshot)
 * ==========================================================================*/

#include "app.h"
#include "buzzer.h"
#include "clock.h"
#include "display.h"
#include "eeprom.h"
#include "keypad.h"
#include "system.h"

/* ------------------------- Shared volatile state -------------------------- */
volatile SystemMode_t system_mode = MODE_NORMAL;
volatile uint32_t system_ms_counter = 0;
volatile uint32_t inactivity_ms = 0;

/* SysTick reload for a 1ms interrupt at the 12 MHz IHRC system clock. */
#define SYSTICK_RELOAD_1MS 11999UL /* 12,000,000 / 1,000 - 1 */

/* ===========================================================================
 * 1. Hardware initialisation
 * =========================================================================*/
static void HW_Init(void)
{
    /* Peripheral clocks + GPIO mode configuration */
    SN_SYS1->AHBCLKEN |= 0x78;
    SN_GPIO0->MODE |= 0x00FF;  /* segments A..G,DP : outputs   */
    SN_GPIO1->MODE |= 0x1EF0;  /* keypad rows + digit selects  */
    SN_GPIO2->MODE &= ~0x00F0; /* keypad columns : inputs      */
    SN_GPIO2->CFG &= ~0xFF00;
    SN_GPIO3->MODE |= 0x0101; /* buzzer (pin 0) + LED D6 (8)  */

    /* Initial pin states: digits off, buzzer off, LED D6 off (active low)  */
    SN_GPIO1->BCLR = 0x1E00;
    SN_GPIO3->BCLR = 1;
    SN_GPIO3->BSET = 0x100;

    EEPROM_Init();

    /* SysTick: 12MHz / 1000 - 1 = 1ms period (f_HCLK = 12 MHz IHRC) */
    SysTick->LOAD = SYSTICK_RELOAD_1MS;
    SysTick->VAL = 0;
    SysTick->CTRL = 7; /* enable + tickint + hclk */
}

/* ===========================================================================
 * 2. Key event FSM (SW3 time edit, SW16 alarm edit, SW6/SW10 adjust)
 * =========================================================================*/
static void Process_Key(uint8_t k)
{
    if (!k)
        return;

    Buzzer_BeepKey(); /* 300ms pip on any key press   */
    inactivity_ms = 0;

    /* --- SW3 : time edit state machine --------------------------------- */
    if (k == KEY_SETUP && system_mode < MODE_EDIT_AL_HOUR)
    {
        if (system_mode == MODE_NORMAL)
        {
            Clock_BeginTimeEdit();
            system_mode = MODE_EDIT_HOUR;
        }
        else if (system_mode == MODE_EDIT_HOUR)
        {
            system_mode = MODE_EDIT_MIN;
        }
        else if (system_mode == MODE_EDIT_MIN)
        {
            Clock_CommitTimeEdit();
            system_mode = MODE_NORMAL;
        }
    }
    /* --- SW16 : alarm edit state machine (commit persists to EEPROM) --- */
    else if (k == KEY_ALARM && (system_mode == MODE_NORMAL || system_mode >= MODE_EDIT_AL_HOUR))
    {
        if (system_mode == MODE_NORMAL)
        {
            Clock_BeginAlarmEdit();
            system_mode = MODE_EDIT_AL_HOUR;
        }
        else if (system_mode == MODE_EDIT_AL_HOUR)
        {
            system_mode = MODE_EDIT_AL_MIN;
        }
        else
        {
            Clock_CommitAlarmEdit();
            system_mode = MODE_NORMAL;
        }
    }
    /* --- SW6 / SW10 : increment / decrement the active edit field ------ */
    else if ((k == KEY_PLUS || k == KEY_MINUS) && system_mode != MODE_NORMAL)
    {
        Clock_AdjustEdit((k == KEY_PLUS) ? 1 : -1, system_mode);
    }
}

/* ===========================================================================
 * 3. Application boot sequence + main super-loop
 * =========================================================================*/
void App_Init(void)
{
    HW_Init();
    Clock_LoadAlarmFromEEPROM(); /* restore + validate alarm     */
}

void App_LoopIteration(void)
{
    uint8_t k = Keypad_Scan();
    if (k)
        Process_Key(k);

    /* 30s inactivity in any edit mode: rollback to NORMAL + beep.        */
    if (system_mode != MODE_NORMAL && inactivity_ms >= TIMEOUT_MS)
    {
        system_mode = MODE_NORMAL;
        Buzzer_BeepKey();
        inactivity_ms = 0;
    }

    /* Edge-triggered single-shot alarm at hh:mm:00 (IRQ-safe match).     */
    if (system_mode == MODE_NORMAL && alarm_armed && !Buzzer_IsAlarmRinging() &&
        Clock_AlarmMatchNow())
    {
        Buzzer_StartAlarm();
    }

    /* Entering any edit mode silences an active alarm ring.              */
    if (system_mode != MODE_NORMAL && Buzzer_IsAlarmRinging())
    {
        Buzzer_CancelAlarm();
    }
}

#ifndef MOCK_SIMULATION
int main(void)
{
    App_Init();

    while (1)
    {
        App_LoopIteration();
    }
}
#endif

/* ===========================================================================
 * 4. SysTick interrupt: 1ms heartbeat
 * =========================================================================*/
void SysTick_Handler(void)
{
    system_ms_counter++;

    EEPROM_I2CWatchdog(); /* protects the blocking I2C waits from hanging  */
    Clock_Tick1ms();
    Display_Tick1ms();
    Buzzer_Tick1ms();

    inactivity_ms++;
}
