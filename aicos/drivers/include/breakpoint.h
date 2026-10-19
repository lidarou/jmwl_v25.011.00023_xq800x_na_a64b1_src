/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of breakpoint module.
 *   It includes all the breakpoint function api.
 */

#ifndef __BP_INC_H__
#define __BP_INC_H__

typedef enum {
    WP_COMP_BYTE = 0,
    WP_COMP_HWORD,
    WP_COMP_WORD
} WP_COMP_TYPE;

/*
 * gx_bp_addr_set
 * This api used to configure breakpoint address
 * @addr: the breakpoint address
 */
int gx_bp_addr_set(u32 addr);

/*
 * gx_bp_addr_clr
 * This api used to clear breakpoint address
 * @addr: the breakpoint address
 */
int gx_bp_addr_clr(u32 addr);

/*
 * gx_bp_clr_all
 * This api used to clear all the breakpoints
 */
void gx_bp_clr_all(void);

/*
 * gx_wp_addr_set
 * This api used to configure watchpoint address
 * @addr: the watchpoint address
 * @type: the watchpoint address type
 */
int gx_wp_addr_set(u32 addr, WP_COMP_TYPE type);

/*
 * gx_wp_data_set
 * This api used to configure watchpoint data value
 * @data: the watchpoint data value
 * @type: the watchpoint data type
 */
int gx_wp_data_set(u32 data, WP_COMP_TYPE type);

/*
 * gx_wp_clr_all
 * This api used to clear all the watchpoints
 */
void gx_wp_clr_all(void);

#endif /* __BP_INC_H__ */

