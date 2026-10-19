/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines bcd driver api.
 *    The bcd api will be called by aicos user functions.
 */

#ifndef __BCD_CORE_H__
#define __BCD_CORE_H__
#include "aic_list.h"

#define AIC_BC_DETECT_DELAY 100 //ms
#define USB_PLUG_OUT_MONITOR_CYCLE_MS   100
#define VBUS_VOL_THOLD_MV     4360
#define USB_PLUG_OUT_PRIO   0x3
#define VBUS_STATUS_FINISH_WAIT_CNT 10 //10*10 ms
#define USB_ANTI_SHAKE_TIME_MS          100

struct bcd_list {
    int bc_type;
    struct list_head bcd_list_head;
};

struct bcd_dev {
    char bc_type[10];
    char bcd_name[30];
    void (*bcd_f)(void *param);
    struct list_head link_list;
    void *para;
};

struct bc_type_info {
        int bc_type;
        char *type_name;
};

int aic_bcd_result(void);
int aic_chg_int_rise(void);

#endif /* __BCD_CORE_H__ */
