/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    IRQ definitions related with hardware.
*/

#ifndef __IRQ_CHIP_H__
#define __IRQ_CHIP_H__

/* -------------------------  Interrupt Number Definition  ------------------------ */

typedef enum IRQn
{
/* -------------------  Processor Exceptions Numbers  ----------------------------- */
    NonMaskableInt_IRQn           = -14,     /*  2 Non Maskable Interrupt */
    HardFault_IRQn                = -13,     /*  3 HardFault Interrupt */
    MemoryManagement_IRQn         = -12,     /*  4 Memory Management Interrupt */
    BusFault_IRQn                 = -11,     /*  5 Bus Fault Interrupt */
    UsageFault_IRQn               = -10,     /*  6 Usage Fault Interrupt */
    SecureFault_IRQn              =  -9,     /*  7 Secure Fault Interrupt */
    SVCall_IRQn                   =  -5,     /* 11 SV Call Interrupt */
    DebugMonitor_IRQn             =  -4,     /* 12 Debug Monitor Interrupt */
    PendSV_IRQn                   =  -2,     /* 14 Pend SV Interrupt */
    SysTick_IRQn                  =  -1,     /* 15 System Tick Interrupt */

/* -------------------  Processor Interrupt Numbers  ------------------------------ */
    IRQ_EIP130                    =   0,
    IRQ_EIP130_SEC                =   1,
    IRQ_USB                       =   2,
    IRQ_USBWU                     =   3,
    IRQ_SDIOH                     =   4,
    IRQ_SSP0                      =   5,
    IRQ_SSP1                      =   6,
    IRQ_CSI                       =   7,
    IRQ_LCDC                      =   8,
    IRQ_I2C0                      =   9,
    IRQ_I2C1                      =   10,
    IRQ_UART0                     =   11,
    IRQ_UART1                     =   12,
    IRQ_UART2                     =   13,
    IRQ_TMR                       =   14,
    IRQ_DMA                       =   15,
    IRQ_DMA_SEC                   =   16,
    IRQ_PPPD                      =   17,
    IRQ_PPPE                      =   18,
    IRQ_MCU_BM                    =   19,
    IRQ_AON_TMR0                  =   20,
    IRQ_WDG                       =   21,
    IRQ_AON_UART                  =   22,
    IRQ_AON_GPIO                  =   23,

    IRQ_EIPU                      =   25,
    IRQ_KPD                       =   26,
    IRQ_GPD                       =   27,
    IRQ_EIPU_SEC                  =   28,
    IRQ_TMR_SEC                   =   29,
    IRQ_WDG_SEC                   =   30,
    IRQ_AON_GPIO_SEC              =   31,
    IRQ_I2C_PMIC                  =   32,
    IRQ_AON_I2C0                  =   33,
    IRQ_AON_I2C1                  =   34,
    IRQ_THM                       =   35,
    IRQ_TOP_UART0                 =   36,
    IRQ_TOP_UART1                 =   37,
    IRQ_TOP_I2S0                  =   38,
    IRQ_TOP_I2S1                  =   39,
    IRQ_EFUSE                     =   40,
    IRQ_TOP_GPIO                  =   41,
    IRQ_MB_OUT_DIR                =   42,
    IRQ_MB_OUT                    =   43,
    IRQ_MB_IN                     =   44,
    IRQ_LZMA                      =   45,
    IRQ_CPU_BM                    =   46,


    IRQ_IPU_SEC                   =   49,
    IRQ_AUD_RST_REQ               =   50,
    IRQ_CP_RST_REQ                =   51,
    IRQ_QSPI                      =   52,
    IRQ_PMIC                      =   53,
    IRQ_PSRM                      =   54,
    IRQ_BM_CP                     =   55,
    IRQ_BM_CPU                    =   56,
    IRQ_BM_ACC                    =   57,
    IRQ_MPU_VIO                   =   58,
    INT_MAXIUM                    =   59,
} IRQn_Type;

typedef u32 INT_PRIORITY_E;

typedef enum {
    NONE_WORKER  = 0x0,
    CALL_HWORKER = 0x1,
    CALL_LWORKER = 0x2,
} ISR_RET_E;

#endif /* __IRQ_CHIP_H__ */
