/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __QUAD_PIN_CFG_H__
#define __QUAD_PIN_CFG_H__

#include <aic_osa.h>
#include <gpio_inc.h>

#define QUAD_PIN_TASK_STACK_SIZE      256
#define QUAD_PIN_TASK_PRIORITY        1
#define QUAD_PIN_TASK_QUEUE_CNT       10

#define HIGH_ACTIVE                   1
#define LOW_ACTIVE                    0
#define WAKEUP_MCU_GPO5               5
#define ACK_WAKEUP_MODEM_GPO3         3

enum power_status{
    M_SLEEP,
    M_WAKEUP,
};

struct modem_ctrl {
    u32 wakeup_mcu;
    u32 ack_wakeup_mcu;
    u32 wakeup_modem;
    u32 ack_wakeup_modem;
    aos_event_ptr event;
    aos_mutex_ptr mutex;
    u32 ack_wakeup_mcu_level;
    u32 wakeup_modem_level;
    aos_thread_id modem_thread;
    aos_thread_id mcu_thread;
    aos_queue_ptr wakeup_modem_q;
    aos_queue_ptr wakeup_mcu_q;
    enum power_status modem_status;
    enum power_status mcu_status;
    aos_timer_ptr release_timer;
    u32 release_delay_time;
};

struct modem_ctrl mdc = {
    .wakeup_mcu = WAKEUP_MCU_GPO5,
    .ack_wakeup_mcu = 3,
    .wakeup_modem = 2,
    .ack_wakeup_modem = ACK_WAKEUP_MODEM_GPO3,
    .ack_wakeup_mcu_level = HIGH_ACTIVE,
    .wakeup_modem_level = HIGH_ACTIVE,
    .release_delay_time = 100,
};


#endif /* __QUAD_PIN_CFG_H__ */
