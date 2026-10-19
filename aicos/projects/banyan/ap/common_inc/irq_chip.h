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
    IRQ_USB                     = 0,
    IRQ_USB_WAKEUP              = 1,
    IRQ_SSPC0                   = 2,
    IRQ_CSI                     = 3,
    IRQ_LCDC                    = 4,
    IRQ_I2C0                    = 5,
    IRQ_I2C1                    = 6,
    IRQ_UART0                   = 7,
    IRQ_UART1                   = 8,
    IRQ_TMR                     = 9,
    IRQ_DMA                     = 10,
    IRQ_PPPD                    = 11,
    IRQ_PPPE                    = 12,
    IRQ_BM                      = 13,
    IRQ_TOP_WDT_MCU             = 14,
    IRQ_AON_EIPU                = 15,
    IRQ_TOP_I2CC_PMIC_MCU       = 16,
    IRQ_TOP_I2CC0               = 17,
    IRQ_TOP_THMC                = 18,
    IRQ_TOP_UART0               = 19,
    IRQ_TOP_IIS0                = 20,
    IRQ_MCU_CAN                 = 21,
    IRQ_TOP_EFUSE               = 22,
    IRQ_TOP_GPIO                = 23,
    IRQ_TOP_MBOX_OUTBOX_D_MCU   = 24,
    IRQ_MB_OUT                  = 25,
    IRQ_MB_IN                   = 26,
    IRQ_MCU_LZMA                = 27,
    IRQ_TOP_WDT_CP_RST_REQ      = 28,
    IRQ_MEM_FLASH               = 29,
    IRQ_PMIC                    = 30,
    IRQ_MEM_AXI_PERF            = 31,
    IRQ_MPU_VIO                 = 32,
    IRQ_TOP_TRNG                = 33,
    IRQ_CP_PWR_UP               = 34,
    IRQ_CP_PWR_DOWN             = 35,
    IRQ_ND_LP_UART              = 36,
    IRQ_ND_TIMER                = 37,
    IRQ_ND_EIPU                 = 38,
    IRQ_TOP_RCCALI              = 39,

    INT_MAXIUM                  = 40,
} IRQn_Type;

typedef u32 INT_PRIORITY_E;

typedef enum {
    NONE_WORKER  = 0x0,
    CALL_HWORKER = 0x1,
    CALL_LWORKER = 0x2,
} ISR_RET_E;

#endif /* __IRQ_CHIP_H__ */
