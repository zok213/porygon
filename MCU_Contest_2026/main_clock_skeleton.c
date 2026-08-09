#include "SN32F400.h"
#include "I2C.h"                      // thu vien I2C (da fix PFPA_I2C0 -> P0.10/P0.11)

/* Cac bien toan cuc cua thu vien I2C0.c, can truy cap tu main.c */
extern volatile uint8_t Busy, Timeout, Error;
extern volatile uint8_t bI2C0_TxFIFO[];
extern volatile uint8_t bI2C0_RxFIFO[];

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

#define DP_PULSE_MS 100   // do rong xung DP sang moi khi sang giay moi (ms)

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
volatile uint16_t ms_in_this_sec = 1;   // 0..999, reset dung luc time_sec tang len 1

/* =======================================================================
 * GIAO TIEP EEPROM QUA THU VIEN I2C0 CUA SONiX
 * (Da fix I2C0_Init() trong I2C.c: set PFPA_I2C0 -> SCL0=P0.10, SDA0=P0.11
 *  de khong dam vao chan Segment G/DP cua LED 7 doan.
 *  Thu vien I2C0.c hien tai KHONG con tu tat/bat SysTick ben trong ham
 *  I2C0_Read/Write nua, nen khong can goi lai SysTick->CTRL o day.)
 * ===================================================================== */
uint8_t EEPROM_SaveAlarm(uint8_t hour, uint8_t min) {
    uint8_t ok;

    // 1. Ghi byte Gio tai dia chi 0
    Timeout = 0;
    bI2C0_TxFIFO[0] = hour;
    ok = I2C0_Write(0, 1);
    if (!ok) return 0;

    // 2. ACK POLLING: Thu lai cho toi khi EEPROM ghi xong tWR va tra ACK
    uint32_t poll_retry = 0;
    do {
        Timeout = 0;
        bI2C0_TxFIFO[0] = min;
        ok = I2C0_Write(1, 1);
        if (ok) break;
    } while (++poll_retry < 50);

    return ok;
}

uint8_t EEPROM_LoadAlarm(uint8_t *hour, uint8_t *min) {
    uint8_t ok1, ok2;

    Timeout = 0;
    ok1 = I2C0_Read(0, 1);
    if (ok1) *hour = bI2C0_RxFIFO[0];

    Timeout = 0;
    ok2 = I2C0_Read(1, 1);
    if (ok2) *min = bI2C0_RxFIFO[0];

    return ok1 && ok2;
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
    I2C0_Init();

    SysTick->LOAD = 11999; SysTick->VAL = 0; SysTick->CTRL = 7;
}

#define HOLD_INITIAL_DELAY_MS   500UL   // Tre 500ms ban dau truoc khi bat dau cuon so
#define HOLD_REPEAT_RATE_MS     250UL   // Toc do cuon 250ms/nac (4 so / giay) de LED 7 doan hien thi day du, khong bi miss/nhay koc
#define KEY_REPEAT_FLAG         0x80

uint8_t Scan_Key(void) {
    static uint8_t  last = 0;
    static uint32_t hold_time_ms = 0;
    static uint32_t last_scan_time = 0;
    uint8_t curr = 0;

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
    uint32_t now = system_ms_counter;
    uint32_t delta = now - last_scan_time;
    last_scan_time = now;

    if (curr) {
        if (curr != last) {
            last = curr;
            hold_time_ms = 0;
            return curr; // Nhan lan dau: Tra ve ma phim chuan (co keu coi 0.3s)
        } else {
            hold_time_ms += delta;
            if (curr == KEY_PLUS || curr == KEY_MINUS) {
                if (hold_time_ms >= HOLD_INITIAL_DELAY_MS) {
                    hold_time_ms -= HOLD_REPEAT_RATE_MS;
                    return (curr | KEY_REPEAT_FLAG); // Nhan giu: Tra ve ma phim kem flag Lặp (coi KHONG keu them)
                }
            }
        }
    } else {
        last = 0;
        hold_time_ms = 0;
    }

    return 0;
}

/* =======================================================================
 * 3. FSM XU LY PHIM
 * ===================================================================== */
void Process_Key(uint8_t k) {
    if (!k) return;

    uint8_t is_repeat = (k & KEY_REPEAT_FLAG) ? 1 : 0;
    uint8_t raw_key   = k & ~KEY_REPEAT_FLAG;

    // Chi keu coi 0.3s o lan nhan phim dau tien; khi nhan giu cuon so thi coi KHONG keu nua
    if (!is_repeat) {
        buzzer_beep_ms = BEEP_KEY_MS;
    }
    inactivity_ms = 0;

    if (raw_key == KEY_SETUP && system_mode < MODE_EDIT_AL_HOUR) {
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
    else if (raw_key == KEY_ALARM && (system_mode == MODE_NORMAL || system_mode >= MODE_EDIT_AL_HOUR)) {
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
            if (!EEPROM_SaveAlarm(alarm_hour, alarm_min)) {
                buzzer_beep_ms = BEEP_KEY_MS * 3;
                return;
            }
            system_mode = MODE_NORMAL;
        }
    }
    else if ((raw_key == KEY_PLUS || raw_key == KEY_MINUS) && system_mode != MODE_NORMAL) {
        volatile uint8_t *h = (system_mode >= MODE_EDIT_AL_HOUR) ? &edit_alarm_hour : &edit_time_hour;
        volatile uint8_t *m = (system_mode >= MODE_EDIT_AL_HOUR) ? &edit_alarm_min  : &edit_time_min;
        int inc = (raw_key == KEY_PLUS) ? 1 : -1;

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
    SystemInit();            // Cau hinh Clock he thong chuan
    SystemCoreClockUpdate(); // Cap nhat Flash Wait-State & SystemCoreClock theo HCLK
    
    HW_Init();
    WDT_Init();

    if (!EEPROM_LoadAlarm((uint8_t*)&alarm_hour, (uint8_t*)&alarm_min)) {
        alarm_hour = 0;
        alarm_min  = 0;
    }
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
    /* Watchdog bao ve I2C treo, vi thu vien khong tu set Timeout */
    static uint16_t i2c_wd = 0;
    if (Busy) {
        if (++i2c_wd > 50) { Timeout = 1; i2c_wd = 0; }  // qua 50ms coi nhu loi
    } else {
        i2c_wd = 0;
    }
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

        /* --- DP (dau cham) nhay 1 xung ngan dong bo dung luc time_sec tang ---
         * Chi ap dung o che do NORMAL: DP sang trong DP_PULSE_MS dau tien
         * cua moi giay (giong hieu ung "tick" dong ho that), roi tat cho
         * den khi time_sec tang tiep. O cac che do chinh gio/phut, giu DP
         * sang co dinh de lam moc phan cach, tranh xung dot voi hieu ung
         * nhap nhay cua blink_on (dung de bao hieu digit dang duoc chinh).
         */
        uint8_t dp_bit = (system_mode == MODE_NORMAL)
                          ? ((ms_in_this_sec < DP_PULSE_MS) ? 0x80 : 0x00)
                          : 0x80;

        uint8_t out[4] = {
            off_h ? 0 : seg7[h / 10],
            off_h ? 0 : (seg7[h % 10] | dp_bit),
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
        if (++ms_in_this_sec >= 1000) {
            ms_in_this_sec = 0;
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

/* =======================================================================
 * 7. HARDFAULT HANDLER (DEFENSIVE RECOVERY)
 * ===================================================================== */
void HardFault_Handler(void) {
    __disable_irq();
    NVIC_SystemReset();
    while (1);
}