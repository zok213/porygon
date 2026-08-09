/* ============================================================================
 * eeprom.c - AT24C02 EEPROM over I2C0 with magic/checksum persistence
 * SN32F407_EVK Digital Clock - Da Nang MCU Contest 2026
 *
 * The byte-level driver is the SONiX interrupt-driven I2C0 library
 * (I2C0.c/I2C.h) - the hardware-verified version running on the board
 * (SCL0=P0.10, SDA0=P0.11, option 2). On top of it a persistent record with
 * magic header + XOR checksum guards the alarm settings, so a blank or
 * corrupted EEPROM can never boot the clock into invalid values.
 *
 * When built with -DMOCK_SIMULATION the driver is replaced by a RAM-backed
 * simulation so the whole firmware can be compiled and tested on a host PC.
 * ==========================================================================*/

#include "eeprom.h"
#include "I2C.h" /* SONiX interrupt-driven I2C0 driver (real hardware path) */
#include "system.h"

/* ------------------------- Persistent record layout ---------------------- */
#define EEPROM_MAGIC 0xA5
#define EEPROM_ADDR_MAGIC 0
#define EEPROM_ADDR_HOUR 1
#define EEPROM_ADDR_MIN 2
#define EEPROM_ADDR_ARMED 3
#define EEPROM_ADDR_CKSUM 4

/* ===========================================================================
 * Host simulation build (no I2C hardware): RAM-backed EEPROM.
 * =========================================================================*/
#ifdef MOCK_SIMULATION

static uint8_t sim_eeprom[8];

void EEPROM_Init(void)
{ /* nothing to initialise in simulation */
}

uint8_t EEPROM_WriteByte(uint8_t addr, uint8_t dat)
{
    if (addr < sizeof(sim_eeprom))
    {
        sim_eeprom[addr] = dat;
    }
    return 1;
}

uint8_t EEPROM_ReadByte(uint8_t addr)
{
    return (addr < sizeof(sim_eeprom)) ? sim_eeprom[addr] : 0xFF;
}

/* ===========================================================================
 * Real hardware build: SONiX interrupt-driven I2C0 library (I2C0.c).
 * This driver is hardware-verified on the board (SCL0 = P0.10, SDA0 = P0.11
 * option 2 - the polling driver's pin setup collided with the 7-segment
 * G/DP lines). The library blocks with Busy/Timeout handshakes, so the
 * application must feed EEPROM_I2CWatchdog() from the 1ms SysTick ISR.
 * =========================================================================*/
#else

/* FIFOs and handshake flags, defined in I2C0.c. */
extern volatile uint8_t bI2C0_TxFIFO[];
extern volatile uint8_t bI2C0_RxFIFO[];
extern volatile uint8_t Busy, Timeout;

#define I2C_WRITE_CYCLE_DELAY 20000UL /* EEPROM internal write ~5ms        */

void EEPROM_Init(void)
{
    I2C0_Init(); /* vendor driver init: clock, pins (P0.10/P0.11), NVIC    */
}

uint8_t EEPROM_WriteByte(uint8_t addr, uint8_t dat)
{
    Timeout = 0;
    bI2C0_TxFIFO[0] = dat;
    if (!I2C0_Write(addr, 1))
    {
        return 0;
    }
    for (volatile uint32_t d = 0; d < I2C_WRITE_CYCLE_DELAY; d++)
    {
    } /* wait for the internal write cycle */
    return 1;
}

uint8_t EEPROM_ReadByte(uint8_t addr)
{
    uint8_t val = 0;
    Timeout = 0;
    if (I2C0_Read(addr, 1))
    {
        val = bI2C0_RxFIFO[0];
    }
    return val;
}

#endif /* MOCK_SIMULATION */

/* ===========================================================================
 * I2C hang watchdog - call every 1ms from the SysTick ISR.
 * The vendor library waits forever on Busy if a transaction stalls (it does
 * not self-set Timeout). If the bus stays busy for more than 50ms, force
 * Timeout so the blocking loops in I2C0_Read/Write exit with an error.
 * =========================================================================*/
void EEPROM_I2CWatchdog(void)
{
#ifdef MOCK_SIMULATION
    /* nothing to guard in simulation */
#else
    static uint16_t i2c_wd = 0;
    if (Busy)
    {
        if (++i2c_wd > 50)
        {
            Timeout = 1;
            i2c_wd = 0;
        }
    }
    else
    {
        i2c_wd = 0;
    }
#endif
}

/* ===========================================================================
 * Persistent alarm record (common to both builds).
 * =========================================================================*/
static uint8_t RecordValid(uint8_t magic, uint8_t h, uint8_t m, uint8_t a, uint8_t ck)
{
    return (magic == EEPROM_MAGIC) && ((magic ^ h ^ m ^ a) == ck);
}

uint8_t EEPROM_LoadAlarm(uint8_t* hour, uint8_t* min, uint8_t* armed)
{
    uint8_t magic = EEPROM_ReadByte(EEPROM_ADDR_MAGIC);
    uint8_t h = EEPROM_ReadByte(EEPROM_ADDR_HOUR);
    uint8_t m = EEPROM_ReadByte(EEPROM_ADDR_MIN);
    uint8_t a = EEPROM_ReadByte(EEPROM_ADDR_ARMED);
    uint8_t ck = EEPROM_ReadByte(EEPROM_ADDR_CKSUM);
    uint8_t valid = RecordValid(magic, h, m, a, ck);

    if (!valid)
    {
        /* Blank (0xFF) or corrupted memory: safe defaults + repair. */
        h = 0;
        m = 0;
        a = 0;
        EEPROM_SaveAlarm(h, m, a);
    }

    /* Boundary clamp: never let out-of-range values reach the display. */
    if (h > 23)
        h = 0;
    if (m > 59)
        m = 0;
    if (a > 1)
        a = 0;

    *hour = h;
    *min = m;
    *armed = a;
    return valid;
}

void EEPROM_SaveAlarm(uint8_t hour, uint8_t min, uint8_t armed)
{
    EEPROM_WriteByte(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
    EEPROM_WriteByte(EEPROM_ADDR_HOUR, hour);
    EEPROM_WriteByte(EEPROM_ADDR_MIN, min);
    EEPROM_WriteByte(EEPROM_ADDR_ARMED, armed);
    EEPROM_WriteByte(EEPROM_ADDR_CKSUM, (uint8_t)(EEPROM_MAGIC ^ hour ^ min ^ armed));
}
