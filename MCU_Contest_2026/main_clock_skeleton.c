/* =========================================================================
 * DONG HO SO - SN32F407_EVK (FIXED COMPILE ERROR)
 * ========================================================================= */

#include "SN32F400.h"   

const uint8_t seg7[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

typedef enum {
    MODE_NORMAL       = 0,
    MODE_EDIT_HOUR    = 1,
    MODE_EDIT_MIN     = 2,
    MODE_EDIT_AL_HOUR = 3,
    MODE_EDIT_AL_MIN  = 4
} SystemMode_t;

#define KEY_SETUP   3
#define KEY_PLUS    6
#define KEY_MINUS   10
#define KEY_ALARM   16

#define TIMEOUT_MS      30000UL
#define BEEP_KEY_MS     300
#define ALARM_RING_MS   5000UL
#define ALARM_TOGGLE_MS 1000UL

#define BUZZ_HALF_PERIOD_NOP 80
#define BUZZ_BURST_CYCLES    10

/* ------------------------------------------------------------------- */
volatile SystemMode_t system_mode = MODE_NORMAL;
volatile uint8_t  time_sec = 0, time_min = 0, time_hour = 0;
volatile uint8_t  alarm_hour = 0, alarm_min = 0;
volatile uint8_t  alarm_armed = 0;

volatile uint8_t  edit_time_hour = 0, edit_time_min = 0; 
volatile uint8_t  edit_alarm_hour = 0, edit_alarm_min = 0; 

volatile uint32_t inactivity_ms = 0;
volatile uint32_t blink_ms = 0;
volatile uint8_t  blink_on = 1;
volatile uint16_t buzzer_beep_ms = 0;
volatile uint8_t  alarm_ringing = 0;
volatile uint32_t alarm_ring_ms = 0;
volatile uint8_t  buzzer_active = 0;
volatile uint32_t system_ms_counter = 0; 

/* =======================================================================
 * 1. GIAO TIEP EEPROM
 * ===================================================================== */
#define I2C_WAIT() { uint32_t t = 50000; while(!(SN_I2C0->STAT & 8)) if(--t==0) return 0; }

void I2C_Init(void) {
    SN_SYS1->AHBCLKEN |= (1 << 21);
    SN_SYS1->PRST |= (1 << 21);
    SN_PFPA->I2C0 = (SN_PFPA->I2C0 & ~0x0F) | 0x0A;
    SN_GPIO0->CFG = (SN_GPIO0->CFG & ~0x500000) | 0x500000;
    SN_I2C0->SCLHT = 120; SN_I2C0->SCLLT = 120;
    SN_I2C0->CTRL = 1;
}

// Ðã s?a t? void thành uint8_t d? tuong thích v?i I2C_WAIT()
uint8_t EEPROM_Write(uint8_t addr, uint8_t dat) {
    SN_I2C0->CTRL |= 2; I2C_WAIT();
    SN_I2C0->TXDATA = 0xA0; SN_I2C0->STAT = 8; I2C_WAIT();
    SN_I2C0->TXDATA = addr; SN_I2C0->STAT = 8; I2C_WAIT();
    SN_I2C0->TXDATA = dat;  SN_I2C0->STAT = 8; I2C_WAIT();
    SN_I2C0->CTRL |= 4;     SN_I2C0->STAT = 8;
    for (volatile int d = 0; d < 30000; d++); 
    return 1;
}

uint8_t EEPROM_Read(uint8_t addr) {
    uint8_t val = 0;
    SN_I2C0->CTRL |= 2; I2C_WAIT();
    SN_I2C0->TXDATA = 0xA0; SN_I2C0->STAT = 8; I2C_WAIT();
    SN_I2C0->TXDATA = addr; SN_I2C0->STAT = 8; I2C_WAIT();
    SN_I2C0->CTRL |= 2;     SN_I2C0->STAT = 8; I2C_WAIT();
    SN_I2C0->TXDATA = 0xA1; SN_I2C0->STAT = 8; I2C_WAIT();
    SN_I2C0->CTRL &= ~8;    SN_I2C0->STAT = 8; I2C_WAIT();
    val = SN_I2C0->RXDATA;
    SN_I2C0->CTRL |= 4;     SN_I2C0->STAT = 8;
    return val;
}

/* =======================================================================
 * 2. KHOI TAO PHAN CUNG & QUET PHIM
 * ===================================================================== */
void HW_Init(void) {
    SN_SYS1->AHBCLKEN |= 0x78;
    SN_GPIO0->MODE |= 0x00FF;
    SN_GPIO1->MODE |= 0x1EF0;
    SN_GPIO2->MODE &= ~0x00F0; SN_GPIO2->CFG &= ~0xFF00;
    SN_GPIO3->MODE |= 0x0101;

    SN_GPIO1->BCLR = 0x1E00; SN_GPIO3->BCLR = 1; SN_GPIO3->BSET = 0x100;
    I2C_Init();

    SysTick->LOAD = 11999; SysTick->VAL = 0; SysTick->CTRL = 7;
}

uint8_t Scan_Key(void) {
    static uint8_t last = 0; uint8_t curr = 0;
    for (int r = 0; r < 4; r++) {
        SN_GPIO1->BSET = (0x0F << 4);
        SN_GPIO1->BCLR = (1 << (4 + r));
        for (int c = 0; c < 4; c++) {
            if (!(SN_GPIO2->DATA & (1 << (4 + c)))) {
                curr = (r == 3 && c == 0) ? 16 : (r == 3 && c == 1) ? 15 : (r * 4 + c + 3);
                goto key_detect;
            }
        }
    }
key_detect:
    if (curr && curr != last) { last = curr; return curr; }
    if (!curr) last = 0;
    return 0;
}

/* =======================================================================
 * 3. FSM XU LY PHIM
 * ===================================================================== */
void Process_Key(uint8_t k) {
    if (!k) return;

    buzzer_beep_ms = BEEP_KEY_MS;
    inactivity_ms = 0;

    if (k == KEY_SETUP && system_mode < MODE_EDIT_AL_HOUR) {
        if (system_mode == MODE_NORMAL) {
            edit_time_hour = time_hour;
            edit_time_min = time_min;
            system_mode = MODE_EDIT_HOUR;
        } else if (system_mode == MODE_EDIT_HOUR) {
            system_mode = MODE_EDIT_MIN;
        } else if (system_mode == MODE_EDIT_MIN) {
            time_hour = edit_time_hour;
            time_min = edit_time_min;
            time_sec = 0;
            system_mode = MODE_NORMAL;
        }
    }
    else if (k == KEY_ALARM && (system_mode == MODE_NORMAL || system_mode >= MODE_EDIT_AL_HOUR)) {
        if (system_mode == MODE_NORMAL) {
            edit_alarm_hour = alarm_hour;
            edit_alarm_min  = alarm_min;
            system_mode = MODE_EDIT_AL_HOUR;
        } else if (system_mode == MODE_EDIT_AL_HOUR) {
            system_mode = MODE_EDIT_AL_MIN;
        } else {
            alarm_hour = edit_alarm_hour;
            alarm_min  = edit_alarm_min;
            alarm_armed = 1;
            EEPROM_Write(0, alarm_hour);
            EEPROM_Write(1, alarm_min);
            system_mode = MODE_NORMAL;
        }
    }
    else if ((k == KEY_PLUS || k == KEY_MINUS) && system_mode != MODE_NORMAL) {
        volatile uint8_t *h = (system_mode >= MODE_EDIT_AL_HOUR) ? &edit_alarm_hour : &edit_time_hour;
        volatile uint8_t *m = (system_mode >= MODE_EDIT_AL_HOUR) ? &edit_alarm_min  : &edit_time_min;
        int inc = (k == KEY_PLUS) ? 1 : -1;

        if (system_mode == MODE_EDIT_HOUR || system_mode == MODE_EDIT_AL_HOUR) {
            *h = (*h + inc + 24) % 24;
        } else {
            *m = (*m + inc + 60) % 60;
        }
    }
}

/* =======================================================================
 * 4. WATCHDOG
 * ===================================================================== */
void WDT_Init(void) {}
void WDT_Feed(void) {}

/* =======================================================================
 * 5. MAIN
 * ===================================================================== */
int main(void) {
    HW_Init();
    WDT_Init();

    alarm_hour = EEPROM_Read(0);
    alarm_min  = EEPROM_Read(1);
    if (alarm_hour > 23) alarm_hour = 0;
    if (alarm_min > 59)  alarm_min  = 0;
    if (alarm_hour || alarm_min) alarm_armed = 1;

    while (1) {
        WDT_Feed();
        Process_Key(Scan_Key());
        
        if (system_mode != MODE_NORMAL && inactivity_ms >= TIMEOUT_MS) {
            system_mode = MODE_NORMAL; 
            buzzer_beep_ms = BEEP_KEY_MS;
            inactivity_ms = 0;
        }

        if (system_mode == MODE_NORMAL && alarm_armed && !alarm_ringing &&
            time_hour == alarm_hour && time_min == alarm_min && time_sec == 0) {
            alarm_ringing = 1;
            alarm_ring_ms = 0;
        }

        if (system_mode != MODE_NORMAL && alarm_ringing) {
            alarm_ringing = 0;
        }
    }
}

/* =======================================================================
 * 6. NGAT SYSTICK (1ms)
 * ===================================================================== */
void SysTick_Handler(void) {
    system_ms_counter++;

    static uint8_t scan_phase = 0;
    static uint8_t dig = 0;

    if (scan_phase == 0) {
        SN_GPIO1->BCLR = 0x1E00;
        SN_GPIO0->BCLR = 0x00FF;
    } else {
        uint8_t h = (system_mode >= MODE_EDIT_AL_HOUR) ? edit_alarm_hour : ((system_mode != MODE_NORMAL) ? edit_time_hour : time_hour);
        uint8_t m = (system_mode >= MODE_EDIT_AL_HOUR) ? edit_alarm_min  : ((system_mode != MODE_NORMAL) ? edit_time_min  : time_min);

        uint8_t off_h = (system_mode == MODE_EDIT_HOUR || system_mode == MODE_EDIT_AL_HOUR) && !blink_on;
        uint8_t off_m = (system_mode == MODE_EDIT_MIN  || system_mode == MODE_EDIT_AL_MIN)  && !blink_on;

        uint8_t out[4] = {
            off_h ? 0 : seg7[h / 10],
            off_h ? 0 : (seg7[h % 10] | 0x80),
            off_m ? 0 : seg7[m / 10],
            off_m ? 0 : seg7[m % 10]
        };

        SN_GPIO0->BSET = out[dig];
        SN_GPIO1->BSET = (1 << (9 + dig));
        dig = (dig + 1) & 3;
    }
    scan_phase ^= 1;

    inactivity_ms++;
    if (system_mode != MODE_EDIT_HOUR && system_mode != MODE_EDIT_MIN) {
        static uint16_t t1s = 0;
        if (++t1s >= 1000) {
            t1s = 0;
            if (++time_sec >= 60) {
                time_sec = 0;
                if (++time_min >= 60) {
                    time_min = 0;
                    time_hour = (time_hour + 1) % 24;
                }
            }
        }
    }

    if (++blink_ms >= 500) { blink_ms = 0; blink_on ^= 1; }

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

    if (buzzer_active) {
        for (int i = 0; i < BUZZ_BURST_CYCLES; i++) {
            SN_GPIO3->BSET = 1;
            for (volatile int d = 0; d < BUZZ_HALF_PERIOD_NOP; d++) {}
            SN_GPIO3->BCLR = 1;
            for (volatile int d = 0; d < BUZZ_HALF_PERIOD_NOP; d++) {}
        }
    } else {
        SN_GPIO3->BCLR = 1;
    }

    if (system_mode >= MODE_EDIT_AL_HOUR) {
        if (blink_on) SN_GPIO3->BCLR = 0x100; else SN_GPIO3->BSET = 0x100;
    } else {
        SN_GPIO3->BSET = 0x100;
    }
}