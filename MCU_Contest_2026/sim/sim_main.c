/* ============================================================================
 * sim_main.c - Host simulation harness for the SN32F407 digital clock
 * Da Nang MCU Contest 2026
 *
 * Build (from MCU_Contest_2026/):
 *   gcc -std=c99 -Wall -Wextra -DMOCK_SIMULATION -I sim \
 *       main.c clock.c keypad.c display.c buzzer.c eeprom.c \
 *       sim/SN32F400_mock.c sim/sim_main.c -o clock_sim
 *
 * This harness executes the REAL firmware (SysTick_Handler + App_LoopIteration)
 * against RAM-mocked peripherals. It drives the keypad GPIO lines exactly like
 * the hardware would and checks the display/buzzer/LED outputs, covering:
 *   - boot state and blank-EEPROM recovery
 *   - key debounce and single-event-per-press (incl. bounce and hold)
 *   - time/alarm edit FSMs, blink behaviour, wraparound arithmetic
 *   - EEPROM magic + checksum persistence (incl. 00:00-armed case)
 *   - alarm trigger at hh:mm:00, 5s pip-pip ring, cancel on edit
 *   - 30s inactivity timeout rollback + beep
 *
 * Exit code 0 = all checks passed (used as CI gate).
 * ==========================================================================*/

#include <stdio.h>
#include <stdint.h>
#include "system.h"
#include "app.h"
#include "clock.h"
#include "keypad.h"
#include "display.h"
#include "buzzer.h"
#include "eeprom.h"

/* ------------------------------- test state ------------------------------ */
static int g_tests = 0;
static int g_fails = 0;

#define CHECK(cond, msg) do {                                                  \
    g_tests++;                                                                 \
    if (!(cond)) {                                                             \
        g_fails++;                                                             \
        printf("  [FAIL] %s (line %d)\n", msg, __LINE__);                      \
    }                                                                          \
} while (0)

/* ----------------------------- clock tick model --------------------------- */
static uint32_t g_ms = 0;
static uint32_t g_disp_pattern = 0xFF;   /* last segment pattern written     */
static int      g_led = 0;               /* LED D6 pin (1 = high = off)      */
static int      sim_buz_activity = 0;    /* sticky: tone driven this tick    */

/* Overrides of the buzzer/LED hardware abstraction seams. The tone burst
 * alternates on/off inside a single tick, so the harness records tick-level
 * ACTIVITY (any ToneOn) instead of a level; LED writes are level-based. */
void Buzzer_HW_ToneOn(void)  { sim_buz_activity = 1; }
void Buzzer_HW_ToneOff(void) { /* level sampled as per-tick activity */ }
void Display_HW_LedOn(void)  { g_led = 0; }   /* active low: pin low = on  */
void Display_HW_LedOff(void) { g_led = 1; }

/* Return and clear the sticky tone-activity flag for the last tick. */
static int buz_sampled(void)
{
    int a = sim_buz_activity;
    sim_buz_activity = 0;
    return a;
}

static void tick1(void)
{
    uint32_t g0_b = sn_gpio0_obj.BSET;

    SysTick_Handler();
    App_LoopIteration();

    if (sn_gpio0_obj.BSET != g0_b) {
        g_disp_pattern = sn_gpio0_obj.BSET & 0xFF;
    }
    g_ms++;
}

static void sim_tick(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++) tick1();
}

/* Tick and report whether the buzzer tone was driven at any point. */
static int seen_buzzer_on(uint32_t ms)
{
    int seen = 0;
    sim_buz_activity = 0;               /* discard stale activity first   */
    for (uint32_t i = 0; i < ms; i++) {
        tick1();
        if (buz_sampled()) seen = 1;
    }
    return seen;
}

/* ------------------------- display pattern helpers ------------------------ */
typedef struct { uint32_t pat[64]; int n; } PatSet;

static void capture_ms(uint32_t ms, PatSet *ps)
{
    ps->n = 0;
    for (uint32_t i = 0; i < ms; i++) {
        tick1();
        if (ps->n < 64) ps->pat[ps->n++] = g_disp_pattern;
    }
}

static int pat_has(const PatSet *ps, uint32_t v)
{
    for (int i = 0; i < ps->n; i++) {
        if (ps->pat[i] == v) return 1;
    }
    return 0;
}

/* Expected 7-segment codes (must match display.c seg7 table). */
static const uint8_t SEG[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

/* ------------------------------ key helpers ------------------------------- */
/* Matrix model: bit (row*4+col) set = that key is physically pressed. The
 * simulated column read returns a column low only when the key's row is the
 * one currently driven low - exactly like the real keypad wiring. */
static uint16_t sim_pressed = 0;
static uint8_t  sim_active_row = 0;

/* Overrides of the keypad hardware abstraction seam (see keypad.h). */
void Keypad_HW_DriveRow(uint8_t row)
{
    sim_active_row = row;
}

uint8_t Keypad_HW_ReadColumnBits(void)
{
    uint8_t cols = 0x0F;            /* all columns high = nothing pressed */
    for (int c = 0; c < 4; c++) {
        if (sim_pressed & (1u << (sim_active_row * 4 + c))) {
            cols &= ~(uint8_t)(1u << c);   /* column low = pressed        */
        }
    }
    return cols;
}

static void key_pos(uint8_t key, int *r, int *c)
{
    if (key == 16) { *r = 3; *c = 0; return; }
    if (key == 15) { *r = 3; *c = 1; return; }
    *r = (key - 3) / 4;
    *c = (key - 3) % 4;
}

static void key_set(uint8_t key, int pressed)
{
    int r, c;
    key_pos(key, &r, &c);
    if (pressed) {
        sim_pressed |=  (uint16_t)(1u << (r * 4 + c));
    } else {
        sim_pressed &= ~(uint16_t)(1u << (r * 4 + c));
    }
}

/* Clean press: 30ms hold (20ms debounce + margin), 30ms release. */
static void press(uint8_t key)
{
    key_set(key, 1); sim_tick(30);
    key_set(key, 0); sim_tick(30);
}

/* ------------------------- hardware state reset --------------------------- */
/* The real board powers up with pull-ups on the keypad columns (released
 * keys read high). Zero-initialised mock registers would read as "all keys
 * pressed", so the harness must set hardware-like defaults before boot. */
static void sim_reset_hw(void)
{
    sn_gpio0_obj  = (SN_GPIO_Type){0};
    sn_gpio1_obj  = (SN_GPIO_Type){0};
    sn_gpio2_obj  = (SN_GPIO_Type){0};
    sn_gpio3_obj  = (SN_GPIO_Type){0};
    sn_sys1_obj   = (SN_SYS1_Type){0};
    sn_pfpa_obj   = (SN_PFPA_Type){0};
    sn_i2c0_obj   = (SN_I2C_Type){0};
    sn_systick_obj = (SysTick_Type){0};
    sn_gpio2_obj.DATA = 0xFFFFFFFF;      /* all keypad columns released   */
    sim_pressed = 0;                     /* no keys held                   */
    sim_active_row = 0;
}

/* ------------------------------- main ------------------------------------- */
int main(void)
{
    printf("=== SN32F407 Digital Clock - Host Simulation ===\n");

    /* ---------- 1. Fresh boot on a blank EEPROM (0xFF cells) ---------- */
    sim_reset_hw();
    for (uint8_t i = 0; i < 8; i++) EEPROM_WriteByte(i, 0xFF);
    App_Init();

    CHECK(time_hour == 0 && time_min == 0 && time_sec == 0,
          "boot: time = 00:00:00");
    CHECK(alarm_hour == 0 && alarm_min == 0 && alarm_armed == 0,
          "boot: alarm defaults disarmed");
    CHECK(system_mode == MODE_NORMAL, "boot: mode = NORMAL");
    CHECK(EEPROM_ReadByte(0) == 0xA5, "boot: EEPROM magic repaired (0xA5)");
    CHECK(EEPROM_ReadByte(3) == 0,    "boot: EEPROM armed flag = 0");

    /* ---------- 2. Idle: display shows 00.00, buzzer silent, LED off -- */
    PatSet ps;
    capture_ms(20, &ps);
    CHECK(pat_has(&ps, SEG[0]) && pat_has(&ps, SEG[0] | 0x80),
          "idle: display shows 00.00");
    CHECK(!seen_buzzer_on(20), "idle: buzzer silent");
    CHECK(g_led == 1, "idle: LED D6 off (pin high = active-low off)");

    /* ---------- 3+4. One physical press: bounce + long hold = one event - */
    for (int i = 0; i < 3; i++) {           /* contact bounce dips          */
        key_set(KEY_SETUP, 1); sim_tick(3);
        key_set(KEY_SETUP, 0); sim_tick(2);
    }
    key_set(KEY_SETUP, 1); sim_tick(40);    /* settle -> emit after 20ms    */
    CHECK(system_mode == MODE_EDIT_HOUR,
          "debounce: bouncy SW3 press enters EDIT_HOUR exactly once");
    sim_tick(800);                          /* keep holding                 */
    CHECK(system_mode == MODE_EDIT_HOUR,
          "debounce: 800ms hold produces no second event");
    key_set(KEY_SETUP, 0); sim_tick(40);    /* release                      */

    /* ---------- 5. Time edit: set minute to 56, then verify blink ------- */
    press(KEY_SETUP);                       /* EDIT_HOUR -> EDIT_MIN       */
    CHECK(system_mode == MODE_EDIT_MIN, "edit: SW3 advances to EDIT_MIN");
    for (int i = 0; i < 4; i++) press(KEY_MINUS);
    CHECK(edit_time_min == 56, "edit: SW10 x4 wraps minute 0 -> 56");

    /* Blink-on half period (500ms on): minute digits 5,6 visible. */
    while ((g_ms / 500) % 2 == 1) sim_tick(1);
    capture_ms(50, &ps);
    CHECK(pat_has(&ps, SEG[5]) && pat_has(&ps, SEG[6]),
          "blink: minute digits visible in blink-on phase");
    /* Blink-off half period: minute digits blanked, hour digits shown. */
    while ((g_ms / 500) % 2 == 0) sim_tick(1);
    capture_ms(50, &ps);
    CHECK(pat_has(&ps, 0x00), "blink: digits blanked in blink-off phase");
    CHECK(!pat_has(&ps, SEG[5]) && !pat_has(&ps, SEG[6]),
          "blink: minute digits hidden in blink-off phase");
    CHECK(pat_has(&ps, SEG[0]), "blink: hour digits still shown");

    /* ---------- 6. Commit time edit ------------------------------------- */
    press(KEY_SETUP);                       /* commit -> NORMAL            */
    CHECK(system_mode == MODE_NORMAL && time_hour == 0 && time_min == 56
          && time_sec == 0, "edit: commit writes 00:56, seconds reset");

    /* ---------- 7. Time edit with plus: hour 0 -> 5 --------------------- */
    press(KEY_SETUP);                       /* -> EDIT_HOUR                */
    for (int i = 0; i < 5; i++) press(KEY_PLUS);
    CHECK(edit_time_hour == 5, "edit: SW6 x5 sets hour 5");
    press(KEY_SETUP);                       /* -> EDIT_MIN (56 kept)       */
    press(KEY_SETUP);                       /* commit -> NORMAL            */
    CHECK(time_hour == 5 && time_min == 56 && time_sec == 0,
          "edit: commit writes 05:56");
    capture_ms(20, &ps);
    CHECK(pat_has(&ps, SEG[5]) && pat_has(&ps, SEG[5] | 0x80)
          && pat_has(&ps, SEG[5]) && pat_has(&ps, SEG[6]),
          "display: 05.56 rendered (HH.MM with DP)");

    /* ---------- 8. 30s inactivity timeout rollback ---------------------- */
    press(KEY_SETUP);                       /* -> EDIT_HOUR                */
    sim_tick(29000);
    CHECK(system_mode == MODE_EDIT_HOUR, "timeout: still editing after 29s");
    CHECK(time_hour == 5 && time_min == 56,
          "timeout: clock frozen during time edit");
    sim_tick(1100);                         /* cross the 30s boundary      */
    CHECK(system_mode == MODE_NORMAL, "timeout: rollback to NORMAL after 30s");
    CHECK(seen_buzzer_on(50), "timeout: exit beep active");
    sim_tick(400);
    CHECK(!seen_buzzer_on(200), "timeout: exit beep ends after ~0.3s");

    /* ---------- 9. Alarm edit + EEPROM save ------------------------------ */
    press(KEY_ALARM);                       /* -> EDIT_AL_HOUR             */
    CHECK(system_mode == MODE_EDIT_AL_HOUR, "alarm: SW16 enters alarm hour edit");
    for (int i = 0; i < 7; i++) press(KEY_PLUS);
    CHECK(edit_alarm_hour == 7, "alarm: hour set to 7");
    press(KEY_ALARM);                       /* -> EDIT_AL_MIN              */
    press(KEY_MINUS);                       /* minute 0 -> 59              */
    CHECK(edit_alarm_min == 59, "alarm: minute wrap 0 -> 59");
    /* LED blink: both on and off phases seen over 1.1s. */
    uint8_t led_on = 0, led_off = 0;
    for (uint32_t i = 0; i < 1100; i++) {
        tick1();
        if (g_led) led_on = 1; else led_off = 1;
    }
    CHECK(led_on && led_off, "alarm: LED D6 blinks at 1s period");
    press(KEY_ALARM);                       /* commit -> save + NORMAL     */
    CHECK(system_mode == MODE_NORMAL, "alarm: commit returns to NORMAL");
    CHECK(alarm_hour == 7 && alarm_min == 59 && alarm_armed == 1,
          "alarm: settings 07:59 armed");
    CHECK(EEPROM_ReadByte(0) == 0xA5 && EEPROM_ReadByte(1) == 7
          && EEPROM_ReadByte(2) == 59 && EEPROM_ReadByte(3) == 1
          && EEPROM_ReadByte(4) == (uint8_t)(0xA5 ^ 7 ^ 59 ^ 1),
          "alarm: EEPROM record (magic,h,m,armed,checksum) written");
    CHECK(g_led == 1, "alarm: LED D6 off again in NORMAL");

    /* ---------- 10. Persistence across power cycles ---------------------- */
    Clock_LoadAlarmFromEEPROM();
    CHECK(alarm_hour == 7 && alarm_min == 59 && alarm_armed == 1,
          "persist: alarm restored after power cycle");
    /* 00:00 armed must survive a power cycle (previously disarmed). */
    EEPROM_SaveAlarm(0, 0, 1);
    Clock_LoadAlarmFromEEPROM();
    CHECK(alarm_hour == 0 && alarm_min == 0 && alarm_armed == 1,
          "persist: 00:00 armed alarm survives power cycle");
    EEPROM_SaveAlarm(7, 59, 1);
    Clock_LoadAlarmFromEEPROM();

    /* ---------- 11. Corrupted EEPROM recovery ---------------------------- */
    EEPROM_WriteByte(2, 0x00);              /* corrupt minute -> bad cksum */
    Clock_LoadAlarmFromEEPROM();
    CHECK(alarm_hour == 0 && alarm_min == 0 && alarm_armed == 0,
          "corrupt: bad checksum falls back to safe defaults");
    CHECK(EEPROM_ReadByte(0) == 0xA5, "corrupt: record repaired");
    EEPROM_SaveAlarm(7, 59, 1);
    Clock_LoadAlarmFromEEPROM();
    CHECK(alarm_armed == 1 && alarm_hour == 7, "corrupt: alarm re-armed");

    /* ---------- 12. Alarm fires at hh:mm:00, 5s pip-pip, then stops ------ */
    time_hour = 7; time_min = 58; time_sec = 59;   /* 1s before 07:59:00  */
    sim_tick(1500);
    CHECK(Buzzer_IsAlarmRinging() == 1, "alarm: rings at 07:59:00");
    uint8_t buzz_on = 0, buzz_off = 0;
    for (uint32_t i = 0; i < 1200; i++) {
        tick1();
        if (buz_sampled()) buzz_on = 1; else buzz_off = 1;
    }
    CHECK(buzz_on && buzz_off, "alarm: pip-pip pattern 0.5s on / 0.5s off");
    sim_tick(4200);
    CHECK(Buzzer_IsAlarmRinging() == 0, "alarm: ring stops after 5s");
    CHECK(!seen_buzzer_on(200), "alarm: buzzer silent after ring");

    /* ---------- 13. Entering edit mode silences an active ring ----------- */
    time_hour = 7; time_min = 58; time_sec = 59;
    sim_tick(1500);
    CHECK(Buzzer_IsAlarmRinging() == 1, "alarm: second ring starts");
    press(KEY_ALARM);                       /* enter alarm edit            */
    CHECK(Buzzer_IsAlarmRinging() == 0, "alarm: silenced by entering edit");
    press(KEY_ALARM);                       /* EDIT_AL_MIN                 */
    press(KEY_ALARM);                       /* commit (re-save 07:59)      */
    CHECK(system_mode == MODE_NORMAL, "alarm: back to NORMAL after cancel");

    /* ---------- 14. Master clock wraparound 23:59:59 -> 00:00:00 --------- */
    time_hour = 23; time_min = 59; time_sec = 58;
    sim_tick(3000);
    CHECK(time_hour == 0 && time_min == 0 && time_sec == 1,
          "clock: 23:59:59 wraps to 00:00:00");

    /* ---------- 15. Edit wraparound: hour 23+1=0, hour 0-1=23 ------------ */
    press(KEY_SETUP);                       /* -> EDIT_HOUR (time 00:01)   */
    press(KEY_MINUS);
    CHECK(edit_time_hour == 23, "edit: hour 0 - 1 wraps to 23");
    press(KEY_PLUS);
    CHECK(edit_time_hour == 0, "edit: hour 23 + 1 wraps to 0");
    press(KEY_SETUP);                       /* -> EDIT_MIN                 */
    press(KEY_MINUS);
    CHECK(edit_time_min == 59, "edit: minute 0 - 1 wraps to 59");
    press(KEY_SETUP);                       /* commit                      */
    CHECK(system_mode == MODE_NORMAL && time_min == 59,
          "edit: commit after wrap works");

    /* ---------- 16. Alarm edit wraparound checks ------------------------- */
    press(KEY_ALARM);                       /* -> EDIT_AL_HOUR (alarm 07:59)*/
    for (int i = 0; i < 7; i++) press(KEY_MINUS);   /* 7 -> 0              */
    CHECK(edit_alarm_hour == 0, "alarm edit: decrement reaches 0");
    press(KEY_MINUS);
    CHECK(edit_alarm_hour == 23, "alarm edit: 0 - 1 wraps to 23");
    press(KEY_PLUS);
    CHECK(edit_alarm_hour == 0, "alarm edit: 23 + 1 wraps to 0");
    press(KEY_ALARM);                       /* -> EDIT_AL_MIN              */
    press(KEY_ALARM);                       /* commit (alarm 00:59 armed)  */
    CHECK(system_mode == MODE_NORMAL && alarm_hour == 0 && alarm_min == 59
          && alarm_armed == 1, "alarm edit: commit persists wrapped alarm");

    /* ------------------------------- summary ------------------------------ */
    printf("=== Result: %d checks, %d failures ===\n", g_tests, g_fails);
    if (g_fails == 0) {
        printf("ALL TESTS PASSED\n");
        return 0;
    }
    printf("TESTS FAILED\n");
    return 1;
}
