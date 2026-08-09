/******************** (C) COPYRIGHT 2024 SONiX *******************************
* COMPANY:          SONiX
* DATE:             2024/11
* AUTHOR:           SA1
* IC:               SN32F400
* DESCRIPTION:      I2C0 related functions. (FIXED - NO SYSTICK INTERFERENCE)
*****************************************************************************/

#include <SN32F400_Def.h>
#include "I2C.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/
volatile uint8_t bI2C0_RxFIFO[I2C_RX_FIFO_LENGTH];
volatile uint8_t bI2C0_TxFIFO[I2C_TX_FIFO_LENGTH] = {0};
volatile uint8_t bI2C0_RxFIFO_cnts, bI2C_Rx_cnts;
volatile uint8_t bI2C0_TxFIFO_cnts, bI2C_Tx_cnts;
volatile uint8_t *bTX_ptr;
volatile uint8_t EEPROM_ADR_H, EEPROM_ADR_L;
volatile uint8_t EEPROM_WR;             
volatile uint8_t Busy = 1, Error = 0;
volatile uint8_t Read_Down = 0;
volatile uint8_t Send_Address;
volatile uint8_t Timeout = 0;

typedef enum{
    eI2C0_ISR_NORMAL,
    eI2C0_ISR_MASTER_TX,
    eI2C0_ISR_MASTER_RX,
    eI2C0_ISR_SLAVE_TX,
    eI2C0_ISR_SLAVE_RX,
}I2C0_ISR_Selection_e;

static uint8_t _b_I2C0_DMA_SlvAddr;
static I2C0_DMA_Status_e        _eI2C0_DMA_STATUS = eI2C_DMA_IDLE;
static I2C0_ISR_Selection_e _eI2C0_ISR_SEL = eI2C0_ISR_NORMAL;

static void __i2c0_irq_handler_normal(void);
static void __i2c0_irq_handler_dma_mst_tx(void);
static void __i2c0_irq_handler_dma_mst_rx(void);
static void __i2c0_irq_handler_dma_slv_tx(void);
static void __i2c0_irq_handler_dma_slv_rx(void);

static void (*__fptrI2C0_irq_handler[])(void) = {
    __i2c0_irq_handler_normal,  
    __i2c0_irq_handler_dma_mst_tx,  
    __i2c0_irq_handler_dma_mst_rx,  
    __i2c0_irq_handler_dma_slv_tx,  
    __i2c0_irq_handler_dma_slv_rx,  
};

/*_____ F U N C T I O N S __________________________________________________*/

void I2C0_IRQHandler(void)
{       
    __fptrI2C0_irq_handler[_eI2C0_ISR_SEL]();
}

static void __i2c0_irq_handler_normal(void)
{       
    if (((SN_I2C0->STAT) & (mskI2C_LOST_ARB_LOST_ARBITRATION)) == Lost_Arbitration)
    {
        SN_I2C0->STAT_b.I2CIF = 1;      
        SN_I2C0->CTRL_b.I2CEN = 0;                      
        SN_I2C0->CTRL_b.I2CEN = 1;                  
        I2C0_Start();                                                   
    }
    else if (((SN_I2C0->STAT) & (mskI2C_STA_MASTER_STA_STO)) == STOP_DONE)
    {   
        Busy = 0;   
        SN_I2C0->STAT_b.I2CIF = 1;          
        if (EEPROM_WR == 1)
        {
            Read_Down = 1;
        }
    }   
    else
    {   
        SN_I2C0->STAT_b.I2CIF = 1;  

        switch (SN_I2C0->STAT)
        {           
            case (Lost_Arbitration | mskI2C_MST_MASTER):
                I2C0_Start();   
            break;
                
            case (RX_DONE | mskI2C_MST_MASTER):
                bI2C0_RxFIFO[bI2C_Rx_cnts++] = SN_I2C0->RXDATA;                 
                if (bI2C_Rx_cnts < (bI2C0_RxFIFO_cnts - 1))
                {
                    SN_I2C0->CTRL_b.ACK =   1;
                }
                else if (bI2C_Rx_cnts == (bI2C0_RxFIFO_cnts - 1))
                {
                    SN_I2C0->CTRL_b.NACK = 1;           
                }                   
                else if (bI2C_Rx_cnts == bI2C0_RxFIFO_cnts)
                {
                    I2C0_Stop(); 
                }
                Busy = 0;               
            break;
                
            case (ACK_DONE | mskI2C_MST_MASTER):
                if (EEPROM_WR == 1)
                {
                    Busy = 0;                   
                    if(bI2C0_RxFIFO_cnts == 1)
                    {
                        SN_I2C0->CTRL_b.NACK = 1;
                    }                       
                    else
                    {
                        SN_I2C0->CTRL_b.ACK = 1;
                    }
                }           
                if (EEPROM_WR == 0)
                {
                    if (Send_Address == 0)  
                    {
                        bI2C_Tx_cnts++;
                        if (bI2C_Tx_cnts <  bI2C0_TxFIFO_cnts)
                        {
                            SN_I2C0->TXDATA = *bTX_ptr++;
                        }
                        else if (bI2C_Tx_cnts == bI2C0_TxFIFO_cnts)
                        {
                            Busy = 0;
                        }                           
                    }
                    else
                        Busy = 0;
                }
            break;
                
            case (NACK_DONE | mskI2C_MST_MASTER):
                SN_I2C0->CTRL_b.STO = 1;
                Error  = 1;
            break;
                
            case (START_DONE | mskI2C_MST_MASTER):
                #if (EEPROM_less_than_32K == 1)
                    SN_I2C0->TXDATA = Device_ADDRESS | (EEPROM_ADR_H << 1) | EEPROM_WR;
                #else
                    SN_I2C0->TXDATA = Device_ADDRESS | EEPROM_WR;
                #endif
            break;
            
            default:
                SN_I2C0->CTRL_b.I2CEN = 0;
                SN_I2C0->CTRL_b.I2CEN = 1;
                SN_I2C0->CTRL_b.STA = 1;
            break;
        }
    }   
}

void I2C0_Init(void)
{
    NVIC_ClearPendingIRQ(I2C0_IRQn);    
    NVIC_EnableIRQ(I2C0_IRQn);
    NVIC_SetPriority(I2C0_IRQn, 0);

    SN_SYS1->AHBCLKEN_b.I2C0CLKEN = 1;

    /* QUAN TR?NG: ch?n SCL0 = P0.10, SDA0 = P0.11 theo dúng schematic board */
    SN_PFPA->I2C0 = (2 << 2) | (2 << 0);   // SCL0=P0.10 (option2), SDA0=P0.11 (option2)

    SN_I2C0->SCLHT = I2C0_SCLHT;
    SN_I2C0->SCLLT = I2C0_SCLLT;
    
    SN_I2C0->CTRL_b.I2CEN = I2C_I2CEN_EN;
}

void I2C0_Start(void)
{
    SN_I2C0->CTRL_b.STA = 1;
}

void I2C0_Stop(void)
{
    SN_I2C0->CTRL_b.STO = 1;
}

uint8_t I2C0_Read(uint16_t eeprom_adr, uint8_t read_num)
{   
    bI2C_Rx_cnts = 0;
    EEPROM_ADR_H =  eeprom_adr >> 8;                
    EEPROM_ADR_L =  eeprom_adr & 0x00ff;        
    bI2C0_RxFIFO_cnts = read_num;                       
    Busy = 1;
    EEPROM_WR = 0;                                                  
    
    I2C0_Start();                                                       
    // SysTick->CTRL = 0x7; (REMOVED)
    
    Send_Address = 1;                                               
    while (Busy == 1 && Timeout == 0);
    // SysTick->CTRL = 0x0; (REMOVED)
    
    if (Error == 1 || Timeout == 1) return FALSE;
        
    #if (EEPROM_less_than_32K == 0) 
        SN_I2C0->TXDATA = EEPROM_ADR_H;             
        // SysTick->CTRL = 0x7; (REMOVED)
        Busy = 1;
        while (Busy == 1 && Timeout == 0);
        // SysTick->CTRL = 0x0; (REMOVED)
        if (Error == 1 || Timeout == 1) return FALSE;
    #endif
    
    SN_I2C0->TXDATA = EEPROM_ADR_L;                 
    // SysTick->CTRL = 0x7; (REMOVED)
    Busy = 1;
    while (Busy == 1 && Timeout == 0);
    // SysTick->CTRL = 0x0; (REMOVED)
    
    if (Error == 1 || Timeout == 1) return FALSE;

    Read_Down = 0;                                                    
    Send_Address = 0;
    EEPROM_WR = 1;                                                  
    
    I2C0_Start();                                                       
    // SysTick->CTRL = 0x7; (REMOVED)
    
    while (Read_Down == 0 && Timeout == 0);
    // SysTick->CTRL = 0x0; (REMOVED)
    
    Read_Down = 0;  
    
    if (Error == 1 || Timeout == 1) return FALSE;
    return TRUE;
}

uint8_t I2C0_Write(uint16_t eeprom_adr, uint8_t write_num)
{
    Timeout = 0;
    bI2C_Tx_cnts = 0;
    bTX_ptr = &bI2C0_TxFIFO[0];                             
    EEPROM_ADR_H =  eeprom_adr >> 8;                
    EEPROM_ADR_L =  eeprom_adr & 0x00ff;        
    bI2C0_TxFIFO_cnts = write_num;                      
    Busy = 1;
    EEPROM_WR = 0;                                                  
    
    // SysTick->CTRL = 0x7; (REMOVED)
    I2C0_Start();                                                       

    Send_Address = 1;                                               
    while (Busy == 1 && Timeout == 0);
    // SysTick->CTRL = 0x0; (REMOVED)
    
    if (Error == 1 || Timeout == 1) return FALSE;

    #if (EEPROM_less_than_32K == 0)
        SN_I2C0->TXDATA = EEPROM_ADR_H;                 
        // SysTick->CTRL = 0x7; (REMOVED)
        Busy = 1;
        while (Busy == 1 && Timeout == 0);
        // SysTick->CTRL = 0x0; (REMOVED)
        if (Error == 1 || Timeout == 1) return FALSE;
    #endif
    
    SN_I2C0->TXDATA = EEPROM_ADR_L;                 
    // SysTick->CTRL = 0x7; (REMOVED)
    Busy = 1;   
    while (Busy == 1 && Timeout == 0);
    if (Error == 1 || Timeout == 1 ) return FALSE;
    // SysTick->CTRL = 0x0; (REMOVED)
    
    Send_Address = 0;
    SN_I2C0->TXDATA = *bTX_ptr++;                       
    
    // SysTick->CTRL = 0x7; (REMOVED)
    Busy = 1;   
    while (Busy == 1 && Timeout == 0);
    if (Error == 1 || Timeout == 1) return FALSE;
    // SysTick->CTRL = 0x0; (REMOVED)
    
    Busy = 1;   
    I2C0_Stop();
    
    // SysTick->CTRL = 0x7; (REMOVED)
    while (Busy == 1 && Timeout == 0);
    if (Error == 1 || Timeout == 1) return FALSE;
    // SysTick->CTRL = 0x0; (REMOVED)
    
    return TRUE;
}

/* --- (CAC HAM I2C DMA DUOC GIU NGUYEN DE KHONG ANH HUONG DE BAI) --- */
void I2C0_SlaveAddressSet(uint8_t b_AddMode, uint8_t b_GCEN) {
    uint8_t b_addr_shift;
    SN_I2C0->SLVADDR0_b.ADD_MODE = b_AddMode;
    SN_I2C0->SLVADDR0_b.GCEN = b_GCEN;
    b_addr_shift = (b_AddMode == I2C_ADD_MODE_7BIT)? 1 : 0;
    SN_I2C0->SLVADDR0_b.ADDR    = I2C_ADDR_SLAVE_ADDR0 << b_addr_shift;
    SN_I2C0->SLVADDR1_b.ADDR1 = I2C_ADDR_SLAVE_ADDR1 << b_addr_shift;
    SN_I2C0->SLVADDR1_b.ADDR2 = I2C_ADDR_SLAVE_ADDR2 << b_addr_shift;
    SN_I2C0->SLVADDR1_b.ADDR3 = I2C_ADDR_SLAVE_ADDR3 << b_addr_shift;
    SN_I2C0->SLVADDR2_b.ADDR4 = I2C_ADDR_SLAVE_ADDR4 << b_addr_shift;
    SN_I2C0->SLVADDR2_b.ADDR5 = I2C_ADDR_SLAVE_ADDR5 << b_addr_shift;
    SN_I2C0->SLVADDR2_b.ADDR6 = I2C_ADDR_SLAVE_ADDR6 << b_addr_shift;
    SN_I2C0->SLVADDR3_b.ADDR7 = I2C_ADDR_SLAVE_ADDR7 << b_addr_shift;
    SN_I2C0->SLVADDR3_b.ADDR8 = I2C_ADDR_SLAVE_ADDR8 << b_addr_shift;
    SN_I2C0->SLVADDR3_b.ADDR9 = I2C_ADDR_SLAVE_ADDR9 << b_addr_shift;
}

uint32_t I2C0_Get_DMA_Status(void) { return _eI2C0_DMA_STATUS; }
void I2C0_DMA_Master_TX_Start(uint8_t b_SlvAddr, uint32_t w_Size) {
    _b_I2C0_DMA_SlvAddr = b_SlvAddr;
    SN_I2C0->DMA = mskI2C_TX_DMA_DIS | mskI2C_RX_DMA_DIS | w_Size;
    _eI2C0_ISR_SEL = eI2C0_ISR_MASTER_TX; _eI2C0_DMA_STATUS = eI2C_DMA_START; SN_I2C0->CTRL_b.STA = 1;
}
void I2C0_DMA_Master_RX_Start(uint8_t b_SlvAddr, uint32_t w_Size, uint8_t b_Last_NACK) {
    _b_I2C0_DMA_SlvAddr = b_SlvAddr;
    SN_I2C0->DMA = mskI2C_TX_DMA_DIS | mskI2C_RX_DMA_DIS | w_Size;
    SN_I2C0->DMA_b.RX_DMA_LAST_NACK = b_Last_NACK;
    _eI2C0_ISR_SEL = eI2C0_ISR_MASTER_RX; _eI2C0_DMA_STATUS = eI2C_DMA_START; SN_I2C0->CTRL_b.STA = 1;
}
void I2C0_DMA_Slave_TX_Start(uint32_t w_Size) {
    SN_I2C0->DMA = mskI2C_TX_DMA_DIS | mskI2C_RX_DMA_DIS | w_Size;
    _eI2C0_ISR_SEL = eI2C0_ISR_SLAVE_TX; _eI2C0_DMA_STATUS = eI2C_DMA_START;
}
void I2C0_DMA_Slave_RX_Start(uint32_t w_Size, uint8_t b_Last_NACK) {
    SN_I2C0->DMA = mskI2C_TX_DMA_DIS | mskI2C_RX_DMA_DIS | w_Size;
    SN_I2C0->DMA_b.RX_DMA_LAST_NACK = b_Last_NACK;
    _eI2C0_ISR_SEL = eI2C0_ISR_SLAVE_RX; _eI2C0_DMA_STATUS = eI2C_DMA_START;
}

static void __i2c0_irq_handler_dma_mst_tx(void) {}
static void __i2c0_irq_handler_dma_mst_rx(void) {}
static void __i2c0_irq_handler_dma_slv_tx(void) {}
static void __i2c0_irq_handler_dma_slv_rx(void) {}