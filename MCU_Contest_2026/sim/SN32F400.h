/* ============================================================================
 * SN32F400.h - HOST SIMULATION MOCK of the SONiX SN32F400 device header
 *
 * THIS FILE IS ONLY USED FOR HOST-PC SIMULATION BUILDS
 * (compile with -DMOCK_SIMULATION and -I sim/).
 *
 * It replaces the CMSIS device header (which lives in the SONiX SN32F4_DFP
 * Keil pack) with RAM-backed register objects so the firmware logic can be
 * compiled and executed on a normal PC. Register semantics are simplified:
 * BSET/BCLR are plain volatile stores (the simulation only needs the values
 * written), and interrupts are no-ops because the simulation is inherently
 * single-threaded.
 *
 * The REAL Keil build never sees this file - it uses the official pack
 * header from the SONiX SN32F4_DFP 1.1.1 package.
 * ==========================================================================*/
#ifndef SN32F400_H
#define SN32F400_H

#include <stdint.h>

/* ------------------------- interrupt control mocks ----------------------- */
#define __disable_irq()                                                                            \
    do                                                                                             \
    {                                                                                              \
    } while (0)
#define __enable_irq()                                                                             \
    do                                                                                             \
    {                                                                                              \
    } while (0)

/* ------------------------------ register types --------------------------- */
typedef struct
{
    volatile uint32_t MODE;
    volatile uint32_t CFG;
    volatile uint32_t DATA;
    volatile uint32_t BSET;
    volatile uint32_t BCLR;
} SN_GPIO_Type;

typedef struct
{
    volatile uint32_t AHBCLKEN;
    volatile uint32_t PRST;
} SN_SYS1_Type;

typedef struct
{
    volatile uint32_t I2C0;
} SN_PFPA_Type;

typedef struct
{
    volatile uint32_t SCLHT;
    volatile uint32_t SCLLT;
    volatile uint32_t CTRL;
    volatile uint32_t STAT;
    volatile uint32_t TXDATA;
    volatile uint32_t RXDATA;
} SN_I2C_Type;

typedef struct
{
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CTRL;
} SysTick_Type;

/* ------------------------- object declarations --------------------------- */
extern SN_GPIO_Type sn_gpio0_obj;
extern SN_GPIO_Type sn_gpio1_obj;
extern SN_GPIO_Type sn_gpio2_obj;
extern SN_GPIO_Type sn_gpio3_obj;
extern SN_SYS1_Type sn_sys1_obj;
extern SN_PFPA_Type sn_pfpa_obj;
extern SN_I2C_Type sn_i2c0_obj;
extern SysTick_Type sn_systick_obj;

/* ------------------------- register aliases ------------------------------ */
#define SN_GPIO0 (&sn_gpio0_obj)
#define SN_GPIO1 (&sn_gpio1_obj)
#define SN_GPIO2 (&sn_gpio2_obj)
#define SN_GPIO3 (&sn_gpio3_obj)
#define SN_SYS1 (&sn_sys1_obj)
#define SN_PFPA (&sn_pfpa_obj)
#define SN_I2C0 (&sn_i2c0_obj)
#define SysTick (&sn_systick_obj)

#endif /* SN32F400_H */
