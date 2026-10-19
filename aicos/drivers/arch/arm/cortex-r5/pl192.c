/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    PL192 interface.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1da258
#define LOG_MODULE    AMOD_DRV

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <core_op.h>
#include <cp_glb.h>
#include <irq.h>

#ifndef WIN32
#include <aic_reg_base.h>
#endif

/**
 * Notes:
 * 1. There are 32 interrupt source totally,every FIQ,IRQ,SGI use a dependent
 * int line.
 * 2. Although no constraint, it is better to use only ONE fiq int line. Too
 * many fiq int line need to query FIQSTATUS to check status.
 * 3. All interrupts configure with handler are enabled in pl192_init.
 * 4. Software generated interrupt(SGI) is not supported.
 */

#ifndef PL192_BASE
#define PL192_BASE   0xDEADDEAD
#endif
#define IRQSTATUS    0x0
#define FIQSTATUS    0x4
#define RAWINTR      0x8
#define INTSELECT    0xC
#define INTENABLE    0x10
#define INTENCLR     0x14
#define SOFTINT      0x18
#define SOFTINTCLR   0x1C
/* Protection vic by unprivilege mode */
#define PROT         0x20
/* Mask each pri level by 1 bit */
#define SWPRIMASK    0x24
#define VECTADDRn    0x100
#define VECTPRIn     0x200
/* Make sure vic is not in test mode */
#define VICITCR      0x300
#define VICADDRESS   0xF00
/* Regs number to be backup */
#define BK_REGS_NUM  (34)

/**
 * pl192_en() - Enable/Disable VIC
 *
 * Enable/Disable VIC.
 *
 */
void pl192_en(BOOL en)
{
#ifdef BIT_CP_GLB_VIC_EB
    if(en)
        AIC_WRITE32(REG_CP_GLB_VIC_EB_SET, BIT_CP_GLB_VIC_EB);
    else
        AIC_WRITE32(REG_CP_GLB_VIC_EB_CLR, BIT_CP_GLB_VIC_EB);
#endif
}

/**
 * pl192_set_sgi() - Trigger SGI
 *
 * Trigger software generated interrupts.
 *
 */
void pl192_set_sgi(u32 param)
{
    AIC_WRITE32(PL192_BASE + SOFTINT, param);
}

/**
 * pl192_clear_sgi() - Clear SGI
 *
 * Clear software generated interrupts.
 *
 */
void pl192_clear_sgi(u32 param)
{
    AIC_WRITE32(PL192_BASE + SOFTINTCLR, param);
}

/**
 * pl192_clren() - Disable int
 *
 * Disable interrupt.
 *
 */
void pl192_clren(u32 param)
{
    AIC_WRITE32(PL192_BASE + INTENCLR, param);
}

/**
 * pl192_seten() - Enable int
 *
 * Enable interrupt.
 *
 */
void pl192_seten(u32 param)
{
    AIC_WRITE32(PL192_BASE + INTENABLE, param);
}

/**
 * pl192_intsel() - Select fiq/irq for single source
 *
 * param: set bitN to 1 means int N via fiq, 0 for irq.
 *
 * Set all int to irq default.
 *
 * Return:none
 */
void pl192_intsel(u32 param)
{
    AIC_WRITE32(PL192_BASE + INTSELECT, param);
}

/**
 * pl192_setvec() - Set vector address
 *
 * Set interrupt vector address.
 *
 */
void pl192_setvec(u32 int_num, u32 addr)
{
    AIC_WRITE32(PL192_BASE + VECTADDRn + 4*int_num, addr);
}

/**
 * pl192_setpri() - Set int priority
 *
 * Set interrupt priority.
 *
 */
void pl192_setpri(u32 int_num, u32 pri)
{
    AIC_WRITE32(PL192_BASE + VECTPRIn + 4*int_num, pri);
}

/**
 * pl192_readvec() - Read vector address reg
 *
 * Read vector address reg to signal that the interrupt is being serviced.
 *
 */
u32 pl192_readvec(void)
{
    u32 vectaddr;

    vectaddr = AIC_READ32(PL192_BASE + VICADDRESS);
    return vectaddr;
}

/**
 * pl192_writevec() - Write vector address
 *
 * Write something to vector address reg to signal that the inerrupt has been
 * serviced.
 *
 */
void pl192_writevec(void)
{
    AIC_WRITE32(PL192_BASE + VICADDRESS, 0xFFFFFFFF);
}

/**
 * pl192_get_fiqsts() - Get fiq status
 *
 * Get fiq interrupt status.
 *
 */
u32 pl192_get_fiqsts(void)
{
    u32 sts;

    sts = AIC_READ32(PL192_BASE + FIQSTATUS);
    return sts;
}

/**
 * pl192_get_irqsts() - Get irq status
 *
 * Get irq interupt status.
 *
 */
u32 pl192_get_irqsts(void)
{
    u32 sts;

    sts = AIC_READ32(PL192_BASE + IRQSTATUS);
    return sts;
}

/**
 * pl192_get_rawsts() - Get irq raw status
 *
 * Get irq interupt status.
 *
 */
u32 pl192_get_rawsts(u32 n)
{
    u32 sts;

    sts = readl_relaxed((u32 *)(PL192_BASE + RAWINTR));

    return (sts >> n) & 1;
}

/**
 * pl192_prot() - Protect vic write by unprivilege mode
 *
 * @prot: true,user mode can access vic; false, user mode can't
 * access vic
 *
 * Protect vic register operated by user mode, default disable
 *
 * Return: none
 */
void pl192_prot(boolean prot)
{
    AIC_WRITE32(PL192_BASE + PROT, prot);
}

/**
 * pl192_prot() - Protect vic write by unprivilege mode
 *
 * @mask: the priority levels will be masked, bitN set to 1
 * will mask priority level N.
 *
 * It must be assured that all priority levels are not masked when
 * initialization.
 *
 * Return:none
 */
void pl192_swpri_mask(u32 mask)
{
    AIC_WRITE32(PL192_BASE + SWPRIMASK, mask);
}

/**
 * pl192_testmode() - Test mode enanble control
 *
 * @val: true, enter test mode;false, not test mode
 *
 * It must be assured that the VIC is not in test mode when initialization.
 *
 * Return: none
 */
void pl192_testmode_ctrl(boolean val)
{
    AIC_WRITE32(PL192_BASE + VICITCR, val);
}
#if 0
__attribute__ ((section(".idle_data")))
u32 g_pl192_bkregs[] = {
    0xC,    /* INTSELECT */
    0x10,   /* INTENABLE */
    0x100,  /* VECTADDR0 */
    0x104,  /* VECTADDR1 */
    0x108,  /* VECTADDR2 */
    0x10C,  /* VECTADDR3 */
    0x110,  /* VECTADDR4 */
    0x114,  /* VECTADDR5 */
    0x118,  /* VECTADDR6 */
    0x11c,  /* VECTADDR7 */
    0x120,  /* VECTADDR8 */
    0x124,  /* VECTADDR9 */
    0x128,  /* VECTADDR10 */
    0x12C,  /* VECTADDR11 */
    0x130,  /* VECTADDR12 */
    0x134,  /* VECTADDR13 */
    0x138,  /* VECTADDR14 */
    0x13C,  /* VECTADDR15 */
    0x140,  /* VECTADDR16 */
    0x144,  /* VECTADDR17 */
    0x148,  /* VECTADDR18 */
    0x14C,  /* VECTADDR19 */
    0x150,  /* VECTADDR20 */
    0x154,  /* VECTADDR21 */
    0x158,  /* VECTADDR22 */
    0x15C,  /* VECTADDR23 */
    0x160,  /* VECTADDR24 */
    0x164,  /* VECTADDR25 */
    0x168,  /* VECTADDR26 */
    0x16C,  /* VECTADDR27 */
    0x170,  /* VECTADDR28 */
    0x174,  /* VECTADDR29 */
    0x178,  /* VECTADDR30 */
    0x17C,  /* VECTADDR31 */
};
#endif
__attribute__ ((section(".idle_data")))
u32 g_pl192_bkarea[BK_REGS_NUM] = {0};

/**
 * pl192_suspend() - Process suspend.
 *
 * Do suspend processing.
 *
 */
void pl192_suspend(void)
{
    u32 i;

    g_pl192_bkarea[0] = AIC_READ32(PL192_BASE + 0xc);
    g_pl192_bkarea[1] = AIC_READ32(PL192_BASE + 0x10);
    for (i = 0; i < BK_REGS_NUM - 2; i++)
        g_pl192_bkarea[i+2] = AIC_READ32(PL192_BASE + 0x100 + i*4);

    return;
}

__attribute__ ((section(".idle_text")))
/**
 * pl192_resume() - Prcoess resume.
 *
 * Do resume processing.
 *
 */
void pl192_resume(void)
{
    u32 i;

#ifdef BIT_CP_GLB_VIC_EB
    /* Enable pl192 */
    AIC_WRITE32(REG_CP_GLB_VIC_EB_SET, BIT_CP_GLB_VIC_EB);
#endif
    /* Disable test mode */
    AIC_WRITE32(PL192_BASE + VICITCR, 0);
    AIC_WRITE32(PL192_BASE + 0xc, g_pl192_bkarea[0]);
    AIC_WRITE32(PL192_BASE + 0x10, g_pl192_bkarea[1]);
    for (i = 0; i < BK_REGS_NUM - 2; i++)
        AIC_WRITE32(PL192_BASE + 0x100 + i*4, g_pl192_bkarea[i+2]);

    return;
}
