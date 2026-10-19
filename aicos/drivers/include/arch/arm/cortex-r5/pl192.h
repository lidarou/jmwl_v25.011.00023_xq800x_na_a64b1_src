/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    PL192 interface declaration.
*/

#ifndef __PL192_H__
#define __PL192_H__

/**
 * pl192_en() - Enable/Disable VIC
 *
 * Enable/Disable VIC.
 *
 */
void pl192_en(BOOL en);

/**
 * pl192_clear_sgi() - Clear SGI
 *
 * Clear software generated interrupts.
 *
 */
void pl192_clear_sgi(u32 param);

/**
 * pl192_clren() - Disable int
 *
 * Disable interrupt.
 *
 */
void pl192_clren(u32 param);

/**
 * pl192_seten() - Enable int
 *
 * Enable interrupt.
 *
 */
void pl192_seten(u32 param);

/**
 * pl192_intsel() - Select fiq/irq for single source
 *
 * param: set bitN to 1 means int N via fiq, 0 for irq.
 *
 * Set all int to irq default.
 *
 * Return:none
 */
void pl192_intsel(u32 param);

/**
 * pl192_setvec() - Set vector address
 *
 * Set interrupt vector address.
 *
 */
void pl192_setvec(u32 int_num, u32 addr);

/**
 * pl192_setpri() - Set int priority
 *
 * Set interrupt priority.
 *
 */
void pl192_setpri(u32 int_num, u32 pri);

/**
 * pl192_readvec() - Read vector address reg
 *
 * Read vector address reg to signal that the interrupt is being serviced.
 *
 */
u32 pl192_readvec(void);

/**
 * pl192_writevec() - Write vector address
 *
 * Write something to vector address reg to signal that the inerrupt has been
 * serviced.
 *
 */
void pl192_writevec(void);

/**
 * pl192_get_fiqsts() - Get fiq status
 *
 * Get fiq interrupt status.
 *
 */
u32 pl192_get_fiqsts(void);

/**
 * pl192_get_irqsts() - Get irq status
 *
 * Get irq interupt status.
 *
 */
u32 pl192_get_irqsts(void);

/**
 * pl192_get_rawsts() - Get irq raw status
 *
 * Get irq interupt status.
 *
 */
u32 pl192_get_rawsts(u32 n);

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
void pl192_prot(boolean);

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
void pl192_swpri_mask(u32);

/**
 * pl192_testmode() - Test mode enanble control
 *
 * @val: true, enter test mode;false, not test mode
 *
 * It must be assured that the VIC is not in test mode when initialization.
 *
 * Return: none
 */
void pl192_testmode_ctrl(boolean);

/**
 * pl192_suspend() - Process suspend.
 *
 * Do suspend processing.
 *
 */
void pl192_suspend(void);

/**
 * pl192_resume() - Prcoess resume.
 *
 * Do resume processing.
 *
 */
void pl192_resume(void);
#endif /* __PL192_H__ */
