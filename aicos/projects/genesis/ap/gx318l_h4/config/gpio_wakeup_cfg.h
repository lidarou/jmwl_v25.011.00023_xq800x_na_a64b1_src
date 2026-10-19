/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __GPIO_WAKEUP_CFG_H__
#define __GPIO_WAKEUP_CFG_H__

#include <aic_osa.h>
#include <gpio_inc.h>

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define GPIO_WAKEUP_TASK_STACK_SIZE      2048
#define GPIO_WAKEUP_TASK_PRIORITY        1
#define GPIO_WAKEUP_TASK_QUEUE_CNT       10

#define GPIO_MCU_WAKEUP_TIMEOUT     5
#define GPIO_WAKEUP                GPIO_TOP_13  /* Detect wakeup MCU request */
#define GPIO_INDICATE              GPIO_TOP_12  /* Indicate MCU enter wakeup status */

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
    aic_gpio_output(id, level);
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
};

struct mcu_control mctrl = {
    .wakeup_gpio = GPIO_WAKEUP,
    .indicate_gpio = GPIO_INDICATE,
    .count = 0,
    .eipu_mode = 1,
    .eipu = 2,
};

#endif /* __GPIO_WAKEUP_CFG_H__ */
