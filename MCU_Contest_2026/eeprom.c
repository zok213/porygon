/* ============================================================================
 * eeprom.c - AT24C02 EEPROM over I2C0 with magic/checksum persistence
 * SN32F407_EVK Digital Clock - Da Nang MCU Contest 2026
 *
 * The byte-level I2C driver is unchanged from the hardware-verified build
 * (polling on STAT bit 3, write-cycle delay loop). On top of it a persistent
 * record with magic header + XOR checksum guards the alarm settings, so a
 * blank or corrupted EEPROM can never boot the clock into invalid values.
 *
 * When built with -DMOCK_SIMULATION the driver is replaced by a RAM-backed
 * simulation so the whole firmware can be compiled and tested on a host PC.
 * ==========================================================================*/

#include "eeprom.h"
#include "system.h"

/* ------------------------- Persistent record layout ---------------------- */
#define EEPROM_MAGIC        0xA5
#define EEPROM_ADDR_MAGIC   0
#define EEPROM_ADDR_HOUR    1
#define EEPROM_ADDR_MIN     2
#define EEPROM_ADDR_ARMED   3
#define EEPROM_ADDR_CKSUM   4

#define WRITE_CYCLE_DELAY   30000   /* AT24C02 needs ~5ms per page write    */

/* ===========================================================================
 * Host simulation build (no I2C hardware): RAM-backed EEPROM.
 * =========================================================================*/
#ifdef MOCK_SIMULATION

static uint8_t sim_eeprom[8];

void EEPROM_Init(void) { /* nothing to initialise in simulation */ }

uint8_t EEPROM_WriteByte(uint8_t addr, uint8_t dat)
{
    if (addr < sizeof(sim_eeprom)) {
        sim_eeprom[addr] = dat;
    }
    return 1;
}

uint8_t EEPROM_ReadByte(uint8_t addr)
{
    return (addr < sizeof(sim_eeprom)) ? sim_eeprom[addr] : 0xFF;
}

/* ===========================================================================
 * Real hardware build: I2C0 driver (hardware-verified sequence).
 * =========================================================================*/
#else

#define I2C_WAIT() { uint32_t t = 50000; while (!(SN_I2C0->STAT & 8)) if (--t == 0) return 0; }

void EEPROM_Init(void)
{
    SN_SYS1->AHBCLKEN |= (1 << 21);                 /* enable I2C0 clock    */
    SN_SYS1->PRST     |= (1 << 21);                 /* release reset        */
    SN_PFPA->I2C0 = (SN_PFPA->I2C0 & ~0x0F) | 0x0A; /* I2C0 pin function    */
    SN_GPIO0->CFG = (SN_GPIO0->CFG & ~0x500000) | 0x500000; /* open-drain   */
    SN_I2C0->SCLHT = 120;
    SN_I2C0->SCLLT = 120;
    SN_I2C0->CTRL = 1;
}

uint8_t EEPROM_WriteByte(uint8_t addr, uint8_t dat)
{
    SN_I2C0->CTRL |= 2; I2C_WAIT();                 /* start                */
    SN_I2C0->TXDATA = 0xA0; SN_I2C0->STAT = 8; I2C_WAIT();  /* device+W   */
    SN_I2C0->TXDATA = addr; SN_I2C0->STAT = 8; I2C_WAIT();  /* word addr  */
    SN_I2C0->TXDATA = dat;  SN_I2C0->STAT = 8; I2C_WAIT();  /* data       */
    SN_I2C0->CTRL |= 4;     SN_I2C0->STAT = 8;     /* stop                 */
    for (volatile int d = 0; d < WRITE_CYCLE_DELAY; d++) {}  /* tWR wait   */
    return 1;
}

uint8_t EEPROM_ReadByte(uint8_t addr)
{
    uint8_t val = 0;
    SN_I2C0->CTRL |= 2; I2C_WAIT();                 /* start                */
    SN_I2C0->TXDATA = 0xA0; SN_I2C0->STAT = 8; I2C_WAIT();  /* device+W   */
    SN_I2C0->TXDATA = addr; SN_I2C0->STAT = 8; I2C_WAIT();  /* word addr  */
    SN_I2C0->CTRL |= 2;     SN_I2C0->STAT = 8; I2C_WAIT();  /* restart    */
    SN_I2C0->TXDATA = 0xA1; SN_I2C0->STAT = 8; I2C_WAIT();  /* device+R   */
    SN_I2C0->CTRL &= ~8;    SN_I2C0->STAT = 8; I2C_WAIT();  /* no ACK     */
    val = SN_I2C0->RXDATA;
    SN_I2C0->CTRL |= 4;     SN_I2C0->STAT = 8;      /* stop                 */
    return val;
}

#endif /* MOCK_SIMULATION */

/* ===========================================================================
 * Persistent alarm record (common to both builds).
 * =========================================================================*/
static uint8_t RecordValid(uint8_t magic, uint8_t h, uint8_t m, uint8_t a, uint8_t ck)
{
    return (magic == EEPROM_MAGIC) && ((magic ^ h ^ m ^ a) == ck);
}

uint8_t EEPROM_LoadAlarm(uint8_t *hour, uint8_t *min, uint8_t *armed)
{
    uint8_t magic = EEPROM_ReadByte(EEPROM_ADDR_MAGIC);
    uint8_t h     = EEPROM_ReadByte(EEPROM_ADDR_HOUR);
    uint8_t m     = EEPROM_ReadByte(EEPROM_ADDR_MIN);
    uint8_t a     = EEPROM_ReadByte(EEPROM_ADDR_ARMED);
    uint8_t ck    = EEPROM_ReadByte(EEPROM_ADDR_CKSUM);
    uint8_t valid = RecordValid(magic, h, m, a, ck);

    if (!valid) {
        /* Blank (0xFF) or corrupted memory: safe defaults + repair. */
        h = 0; m = 0; a = 0;
        EEPROM_SaveAlarm(h, m, a);
    }

    /* Boundary clamp: never let out-of-range values reach the display. */
    if (h > 23) h = 0;
    if (m > 59) m = 0;
    if (a > 1)  a = 0;

    *hour  = h;
    *min   = m;
    *armed = a;
    return valid;
}

void EEPROM_SaveAlarm(uint8_t hour, uint8_t min, uint8_t armed)
{
    EEPROM_WriteByte(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
    EEPROM_WriteByte(EEPROM_ADDR_HOUR,  hour);
    EEPROM_WriteByte(EEPROM_ADDR_MIN,   min);
    EEPROM_WriteByte(EEPROM_ADDR_ARMED, armed);
    EEPROM_WriteByte(EEPROM_ADDR_CKSUM, (uint8_t)(EEPROM_MAGIC ^ hour ^ min ^ armed));
}
