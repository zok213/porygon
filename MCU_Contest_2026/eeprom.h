/* ============================================================================
 * eeprom.h - AT24C02 EEPROM over I2C0 with magic/checksum persistence
 * ==========================================================================*/
#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>

/* I2C0 peripheral + GPIO0 pins 10/11 (SCL/SDA) configuration. */
void EEPROM_Init(void);

/* Low-level single byte access (blocking, interrupt-driven I2C0). */
uint8_t EEPROM_WriteByte(uint8_t addr, uint8_t dat);
uint8_t EEPROM_ReadByte(uint8_t addr);

/* I2C hang watchdog: call every 1ms from the SysTick ISR. Forces the
 * library's Timeout flag if the bus stays busy for more than 50ms, so a
 * stalled transaction can never hang the firmware. */
void EEPROM_I2CWatchdog(void);

/* Persistent alarm storage layout:
 *   addr 0 : magic header 0xA5
 *   addr 1 : alarm hour  (0..23)
 *   addr 2 : alarm minute (0..59)
 *   addr 3 : armed flag  (0/1)
 *   addr 4 : XOR checksum of addr 0..3
 *
 * EEPROM_LoadAlarm() validates magic + checksum and clamps ranges. When the
 * memory is blank or corrupted it initialises defaults (00:00, disarmed) and
 * rewrites the record, so the system can never boot into out-of-range values.
 * Returns 1 when valid data was restored, 0 when defaults were used. */
uint8_t EEPROM_LoadAlarm(uint8_t* hour, uint8_t* min, uint8_t* armed);
void EEPROM_SaveAlarm(uint8_t hour, uint8_t min, uint8_t armed);

#endif /* EEPROM_H */
