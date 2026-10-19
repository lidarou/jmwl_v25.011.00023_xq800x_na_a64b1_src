/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file includes chip interrupt list file.
 */

#ifndef __AIC_INTERRUPT_H__
#define __AIC_INTERRUPT_H__

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
    Interrupt0_IRQn               =   0,
    Interrupt1_IRQn               =   1,
    Interrupt2_IRQn               =   2,
    Interrupt3_IRQn               =   3,
    Interrupt4_IRQn               =   4,
    Interrupt5_IRQn               =   5,
    Interrupt6_IRQn               =   6,
    Interrupt7_IRQn               =   7,
    Interrupt8_IRQn               =   8,
    Interrupt9_IRQn               =   9,
    Interrupt10_IRQn              =   10,
    Interrupt11_IRQn              =   11,
    Interrupt12_IRQn              =   12,
    Interrupt13_IRQn              =   13,
    Interrupt14_IRQn              =   14,
    Interrupt15_IRQn              =   15,
    Interrupt16_IRQn              =   16,
    Interrupt17_IRQn              =   17,
    Interrupt18_IRQn              =   18,
    Interrupt19_IRQn              =   19,
    Interrupt20_IRQn              =   20,
    Interrupt21_IRQn              =   21,
    Interrupt22_IRQn              =   22,
    Interrupt23_IRQn              =   23,
    Interrupt24_IRQn              =   24,
    Interrupt25_IRQn              =   25,
    Interrupt26_IRQn              =   26,
    Interrupt27_IRQn              =   27,
    Interrupt28_IRQn              =   28,
    Interrupt29_IRQn              =   29,
    Interrupt30_IRQn              =   30,
    Interrupt31_IRQn              =   31,

    INT_MAXIUM                    =   64
} IRQn_Type;

#endif /* __AIC_INTERRUPT_H__ */
