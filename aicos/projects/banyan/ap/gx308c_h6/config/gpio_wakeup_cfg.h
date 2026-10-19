/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __GPIO_WAKEUP_CFG_H__
#define __GPIO_WAKEUP_CFG_H__

#include <aic_osa.h>
#include <gpio_inc.h>
#include <gpo_inc.h>

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define GPIO_WAKEUP_TASK_STACK_SIZE      2048
#define GPIO_WAKEUP_TASK_PRIORITY        1
#define GPIO_WAKEUP_TASK_QUEUE_CNT       10

#define GPIO_MCU_WAKEUP_TIMEOUT     5
#define WAKEUP_MCU_TRY_TIMES        5

//#define GPIO_WAKEUP_DEBUG

#ifdef GPIO_WAKEUP_DEBUG
#define DEBUG_PRINTF    alog_info
#else
#define DEBUG_PRINTF(...)
#endif

enum target_mcu{
    MCU_SLEEP,
    MCU_WAKEUP,
};

enum ext_modem_status{
    MODEM_SLEEP,
    MODEM_WAKEUP,
};

enum request_mode {
    ACK_TRANSFER,
    REQ_TRANSFER,
};

void pin_output(u32 id, u32 level)
{
    aic_gpo_output(id, level);
}

struct mcu_control {
    u32 wakeup_gpio;
    u32 indicate_gpio;
    enum target_mcu mcu_slp;
    enum ext_modem_status modem_slp;
    u32 initial;
    u32 count;
    enum request_mode request;
    aos_thread_id wakeup_mcu;
    aos_queue_ptr wakeup_mcu_q;
    aos_mutex_ptr mutex;
    u32 eipu_mode;
    u32 eipu;
    u32 after_release_mcu_delay_us;
    u32 before_release_mcu_delay_ms;
    u32 retry;
    void(*pin_output)(u32, u32);
};

struct mcu_control mctrl = {
    .count = 0,
    .after_release_mcu_delay_us = 300,
    .before_release_mcu_delay_ms = 2,
    .retry = WAKEUP_MCU_TRY_TIMES,
};

#endif /* __GPIO_WAKEUP_CFG_H__ */
