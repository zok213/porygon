#ifndef __SN32F400_I2C_H
#define __SN32F400_I2C_H

/*_____ I N C L U D E S ____________________________________________________*/
#include <SN32F400.h>

/*_____ D E F I N I T I O N S ______________________________________________*/
#define EEPROM_less_than_32K 1 /* 1: EEPROM smaller than 32K bits (e.g. AT24C02)   */
#define Device_ADDRESS 0xA0
#define Lost_Arbitration 0x100
#define SLAVE_ADDRESS_HIT_TX 0x80
#define SLAVE_ADDRESS_HIT_RX 0x40
#define START_DONE 0x10
#define STOP_DONE 0x08
#define NACK_DONE 0x04
#define ACK_DONE 0x02
#define RX_DONE 0x01

/* FIFO sizes */
#define I2C_RX_FIFO_LENGTH 32
#define I2C_TX_FIFO_LENGTH 32

/* I2Cn Control register <I2Cn_CTRL> (0x00) */
/* [1:1] Assert NACK (HIGH level to SDA) flag */
#define I2C_NACK_NOFUNCTION 0 /* No function */
#define I2C_NACK 1 /* An NACK will be returned during the acknowledge clock pulse on SCLn */
#define mskI2C_NACK_NOFUNCTION (I2C_NACK_NOFUNCTION << 1)
#define mskI2C_NACK (I2C_NACK << 1)

/* [2:2] Assert ACK (Low level to SDA) flag */
#define I2C_ACK_NOFUNCTION 0 /* No function */
#define I2C_ACK 1 /* An ACK will be returned during the acknowledge clock pulse on SCLn */
#define mskI2C_ACK_NOFUNCTION (I2C_ACK_NOFUNCTION << 2)
#define mskI2C_ACK (I2C_ACK << 2)

/* [4:4] STOP flag */
#define I2C_STO_IDLE 0 /* Stop condition idle */
#define I2C_STO_STOP 1 /* Transmit a STOP condition in master mode */
#define mskI2C_STO_IDLE (I2C_STO_IDLE << 4)
#define mskI2C_STO_STOP (I2C_STO_STOP << 4)

/* [5:5] START bit */
#define I2C_STA_IDLE 0  /* No START condition will be generated */
#define I2C_STA_START 1 /* Transmit a START or a Repeated START condition */
#define mskI2C_STA_IDLE (I2C_STA_IDLE << 5)
#define mskI2C_STA_START (I2C_STA_START << 5)

/* [8:8] I2C Interface enable bit */
#define I2C_I2CEN_DIS 0 /* Disable */
#define I2C_I2CEN_EN 1  /* Enable */
#define mskI2C_I2CEN_DIS (I2C_I2CEN_DIS << 8)
#define mskI2C_I2CEN_EN (I2C_I2CEN_EN << 8)

/* [20:20] I2C TX_BIT changing timing bit */
#define I2C_SCLL_HF_SEL_EDGE 0 /* Change at SCL negative edge */
#define I2C_SCLL_HF_SEL_HALF 1 /* Change at half of SCL low level */
#define mskI2C_SCLL_HF_SEL_EDGE (I2C_SCLL_HF_SEL_EDGE << 20)
#define mskI2C_SCLL_HF_SEL_HALF (I2C_SCLL_HF_SEL_HALF << 20)

/* [21:21] Master Arbitration enable bit */
#define I2C_ARBITRATION_DIS 0 /* Disable */
#define I2C_ARBITRATION_EN 1  /* Enable */
#define mskI2C_ARBITRATION_DIS (I2C_ARBITRATION_DIS << 21)
#define mskI2C_ARBITRATION_EN (I2C_ARBITRATION_EN << 21)

/* I2Cn Status register <I2Cn_STAT> (0x04) */
/* [0:0] RX done status */
#define I2C_RX_DN_NO_HANDSHAKE 0 /* No RX with ACK/NACK transfer */
#define I2C_RX_DN_HANDSHAKE 1    /* 8-bit RX with ACK/NACK transfer is done */
#define mskI2C_RX_DN_NO_HANDSHAKE (I2C_RX_DN_NO_HANDSHAKE << 0)
#define mskI2C_RX_DN_HANDSHAKE (I2C_RX_DN_HANDSHAKE << 0)

/* [1:1] ACK done status */
#define I2C_ACK_STAT_NO_RECEIVED_ACK 0 /* Not received an ACK */
#define I2C_ACK_STAT_RECEIVED_ACK 1    /* Received an ACK */
#define mskI2C_ACK_STAT_NO_RECEIVED_ACK (I2C_ACK_STAT_NO_RECEIVED_ACK << 1)
#define mskI2C_ACK_STAT_RECEIVED_ACK (I2C_ACK_STAT_RECEIVED_ACK << 1)

/* [2:2] NACK done status */
#define I2C_NACK_STAT_NO_RECEIVED_NACK 0 /* Not received a NACK */
#define I2C_NACK_STAT_RECEIVED_NACK 1    /* Received a NACK */
#define mskI2C_NACK_STAT_NO_RECEIVED_NACK (I2C_NACK_STAT_NO_RECEIVED_NACK << 2)
#define mskI2C_NACK_STAT_RECEIVED_NACK (I2C_NACK_STAT_RECEIVED_NACK << 2)

/* [3:3] Stop done status */
#define I2C_STOP_DN_NO_STOP 0 /* No STOP bit */
#define I2C_STOP_DN_STOP 1    /* Master mode: a STOP condition was issued */
#define mskI2C_STOP_DN_NO_STOP (I2C_STOP_DN_NO_STOP << 3)
#define mskI2C_STOP_DN_STOP (I2C_STOP_DN_STOP << 3)

/* [4:4] Start done status */
#define I2C_START_DN_NO_START 0 /* No START bit */
#define I2C_START_DN_START 1    /* Master mode: a START bit was issued */
#define mskI2C_START_DN_NO_START (I2C_START_DN_NO_START << 4)
#define mskI2C_START_DN_START (I2C_START_DN_START << 4)

#define I2C_MST_SLAVE 0 /* [5:5] Master/Slave status */
#define I2C_MST_MASTER 1
#define mskI2C_MST_SLAVE (I2C_MST_SLAVE << 5)
#define mskI2C_MST_MASTER (I2C_MST_MASTER << 5)

#define mskI2C_STA_STA_STO ((I2C_START_DN_START << 4) | (I2C_STOP_DN_STOP << 3))
#define mskI2C_STA_MASTER_STA_STO                                                                  \
    ((I2C_MST_MASTER << 5) | (I2C_START_DN_START << 4) | (I2C_STOP_DN_STOP << 3))

/* [6:6] Slave address check */
#define I2C_SLV_RX_NO_MATCH_ADDR 0 /* No matched slave address */
#define I2C_SLV_RX_MATCH_ADDR 1    /* Slave address hit, called for RX in slave mode */
#define mskI2C_SLV_RX_NO_MATCH_ADDR (I2C_SLV_RX_NO_MATCH_ADDR << 6)
#define mskI2C_SLV_RX_MATCH_ADDR (I2C_SLV_RX_MATCH_ADDR << 6)

/* [7:7] Slave address check */
#define I2C_SLV_TX_NO_MATCH_ADDR 0 /* No matched slave address */
#define I2C_SLV_TX_MATCH_ADDR 1    /* Slave address hit, called for TX in slave mode */
#define mskI2C_SLV_TX_NO_MATCH_ADDR (I2C_SLV_TX_NO_MATCH_ADDR << 7)
#define mskI2C_SLV_TX_MATCH_ADDR (I2C_SLV_TX_MATCH_ADDR << 7)

/* [8:8] Lost arbitration */
#define I2C_LOST_ARB_NO_LOST 0          /* Not lost arbitration */
#define I2C_LOST_ARB_LOST_ARBITRATION 1 /* Lost arbitration */
#define mskI2C_LOST_ARB_NO_LOST (I2C_LOST_ARB_NO_LOST << 8)
#define mskI2C_LOST_ARB_LOST_ARBITRATION (I2C_LOST_ARB_LOST_ARBITRATION << 8)

/* [9:9] Time-out status */
#define I2C_TIMEOUT_NO_TIMEOUT 0 /* No Timeout */
#define I2C_TIMEOUT_TIMEOUT 1    /* Timeout */
#define mskI2C_TIMEOUT_TIMEOUT (I2C_TIMEOUT_TIMEOUT << 9)
#define mskI2C_TIMEOUT_NO_TIMEOUT (I2C_TIMEOUT_NO_TIMEOUT << 9)

/* [15:15] I2C Interrupt flag */
#define I2C_I2CIF_STAUS_NO_CHANGE 0 /* I2C status doesn't change */
#define I2C_I2CIF_INTERRUPT 1       /* Read: I2C status changes; Write: clear this flag */
#define mskI2C_I2CIF_STAUS_NO_CHANGE (I2C_I2CIF_STAUS_NO_CHANGE << 15)
#define mskI2C_I2CIF_INTERRUPT (I2C_I2CIF_INTERRUPT << 15)

/* I2Cn Slave Address 0 register <I2Cn_SLVADDR0> (0x10) */
#define I2C_ADDR_SLAVE_ADDR0 0x07 /* The I2C slave address 0 */

/* [30:30] General call address enable bit */
#define I2C_GCEN_DIS 0 /* Disable */
#define I2C_GCEN_EN 1  /* Enable general call address (0x0) */
#define mskI2C_GCEN_DIS (I2C_GCEN_DIS << 30)
#define mskI2C_GCEN_EN (I2C_GCEN_EN << 30)

/* [31:31] Slave address mode */
#define I2C_ADD_MODE_7BIT 0  /* 7-bit address mode */
#define I2C_ADD_MODE_10BIT 1 /* 10-bit address mode */
#define mskI2C_ADD_MODE_7BIT (I2C_ADD_MODE_7BIT << 31)
#define mskI2C_ADD_MODE_10BIT (I2C_ADD_MODE_10BIT << 31)

/* I2Cn Slave Address 1~3 registers (0x14/0x18/0x1C) */
#define I2C_ADDR_SLAVE_ADDR1 0
#define I2C_ADDR_SLAVE_ADDR2 0
#define I2C_ADDR_SLAVE_ADDR3 0
#define I2C_ADDR_SLAVE_ADDR4 0
#define I2C_ADDR_SLAVE_ADDR5 0
#define I2C_ADDR_SLAVE_ADDR6 0
#define I2C_ADDR_SLAVE_ADDR7 0
#define I2C_ADDR_SLAVE_ADDR8 0
#define I2C_ADDR_SLAVE_ADDR9 0

/* I2Cn SCL High Time register <I2Cn_SCLHT> (0x20) */
#define I2C0_SCLHT 14 /* [7:0] SCL High Period = (SCLH+1) * I2C0_PCLK cycle */
#define I2C1_SCLHT 4

/* I2Cn SCL Low Time register <I2Cn_SCLLT> (0x24) */
#define I2C0_SCLLT 14 /* [7:0] SCL Low Period = (SCLL+1) * I2C0_PCLK cycle */
#define I2C1_SCLLT 4

/* I2Cn Timeout Control register <I2Cn_TOCTRL> (0x2C) */
#define I2C_TO_DIS 0 /* [15:0] Count for checking Timeout */
#define I2C_TO_PERIOD_TIME 0

#define I2C_ERROR 0x00001

/* I2Cn DMA Mode register <I2Cn_DMA> (0x50) */
/* [28:28] I2C issues NACK/ACK for last data in RX DMA mode */
#define I2C_RX_DMA_LAST_ACK 0  /* I2C issues ACK for last data in RX DMA mode */
#define I2C_RX_DMA_LAST_NACK 1 /* I2C issues NACK for last data in RX DMA mode */
#define mskI2C_RX_DMA_LAST_ACK (I2C_RX_DMA_LAST_ACK << 28)
#define mskI2C_RX_DMA_LAST_NACK (I2C_RX_DMA_LAST_NACK << 28)

/* [30:30] RX DMA mode enable */
#define I2C_RX_DMA_DIS 0 /* Disable */
#define I2C_RX_DMA_EN 1  /* Enable */
#define mskI2C_RX_DMA_DIS (I2C_RX_DMA_DIS << 30)
#define mskI2C_RX_DMA_EN (I2C_RX_DMA_EN << 30)

/* [31:31] TX DMA mode enable */
#define I2C_TX_DMA_DIS 0U /* Disable */
#define I2C_TX_DMA_EN 1U  /* Enable */
#define mskI2C_TX_DMA_DIS (I2C_TX_DMA_DIS << 31)
#define mskI2C_TX_DMA_EN (I2C_TX_DMA_EN << 31)

typedef enum
{
    eI2C_DMA_IDLE,
    eI2C_DMA_START,
    eI2C_DMA_WORKING,
    eI2C_DMA_DONE,
    eI2C_DMA_FAIL_RECEIVE_NACK,
    eI2C_DMA_FAIL_LOST_ARBITRATION,
    eI2C_DMA_FAIL_OTHERS
} I2C0_DMA_Status_e;

/*_____ D E C L A R A T I O N S ____________________________________________*/
void I2C0_Init(void);
void I2C0_Start(void);
void I2C0_Stop(void);
uint8_t I2C0_Read(uint16_t, uint8_t);
uint8_t I2C0_Write(uint16_t, uint8_t);
void I2C0_SlaveAddressSet(uint8_t, uint8_t);
uint32_t I2C0_Get_DMA_Status(void);
void I2C0_DMA_Master_TX_Start(uint8_t, uint32_t);
void I2C0_DMA_Master_RX_Start(uint8_t, uint32_t, uint8_t);
void I2C0_DMA_Slave_TX_Start(uint32_t);
void I2C0_DMA_Slave_RX_Start(uint32_t, uint8_t);

#endif /*__SN32F400_I2C_H*/
