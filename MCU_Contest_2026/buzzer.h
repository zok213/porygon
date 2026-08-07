/* ============================================================================
 * buzzer.h - Key beep, alarm ring pattern and tone generation
 * ==========================================================================*/
#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

/* 300ms "pip" on every accepted key press. */
void Buzzer_BeepKey(void);

/* Start the 5s alarm ring (0.5s ON / 0.5s OFF pip-pip pattern). */
void Buzzer_StartAlarm(void);

/* Stop the alarm ring immediately (e.g. user enters an edit mode). */
void Buzzer_CancelAlarm(void);

/* 1 when the alarm ring is currently running. */
uint8_t Buzzer_IsAlarmRinging(void);

/* Advance beep/ring timing one 1ms tick and drive the buzzer pin.
 * Called from the SysTick ISR. */
void Buzzer_Tick1ms(void);

/* Hardware abstraction seam for the tone pin (GPIO3.0, active high).
 * The host simulation overrides these to observe tone activity directly. */
void Buzzer_HW_ToneOn(void);
void Buzzer_HW_ToneOff(void);

#endif /* BUZZER_H */
