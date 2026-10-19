/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 * This file use for the pin wakeup detect.
 */

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1006b8
#define LOG_MODULE    AMOD_DRV

#include <aic_osa.h>
#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_type.h>
#include <gpio_inc.h>
#include <slp_inc.h>
#include <serial/pin_wakeup_config.h>
#include <eipu.h>
#include <aic_at_resp_handle.h>
#include <aic_at_cmd_handle.h>

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define PIN_WAKEUP_TASK_STACK_SIZE      2048
#define PIN_WAKEUP_TASK_PRIORITY        40
#define PIN_WAKEUP_TASK_QUEUE_CNT       10

enum _pin_wakeup {
    PIN_SLEEP = 0,
    PIN_WAKEUP,
    PIN_INVALID
};

#define PIN_SLEEP_LEVEL                 ((~PIN_WAKEUP_LEVEL) & 0x1) /* pin sleep level */

#define DEFAULT_DELAYSLEEP_TIME         (5) /* unit: s */

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Static Global Variables
 **---------------------------------------------------------------------------*/
static aos_thread_id pin_wakeup_thread_id;
static aos_queue_ptr pin_wakeup_queue;

#ifndef MCU_DEFAULT_SLEEP
#define MCU_DEFAULT_SLEEP 0
#endif

static u32 pin_wakeup_status = PIN_INVALID;
static u32 pin_wakeup_at_status = MCU_DEFAULT_SLEEP;

extern void net_status_light_enable(bool enabled);
extern void slp_set_delaysleep(u32 delaysleep);

/**---------------------------------------------------------------------------*
 ** Function Definitions
 **---------------------------------------------------------------------------*/
#ifdef CONFIG_PIN_WAKEUP_GPIO
/**
 * pin_wakeup_callback - pin wakeup detect by gpio.
 *
 * @param
 *
 * Return:
 *
 */
static void aic_pin_wakeup_callback(void *param)
{
    u32 level, wakeup_status;

    level = aic_gpio_level_get(PIN_WAKEUP_DETECT);
    AIC_ASSERT(level == 0 || level == 1);
    if (level == PIN_WAKEUP_LEVEL) {
        wakeup_status = PIN_WAKEUP;
    } else {
        wakeup_status = PIN_SLEEP;
    }

    if (wakeup_status != pin_wakeup_status) {
        pin_wakeup_status = wakeup_status;
        aos_send_to_queue(pin_wakeup_queue, &wakeup_status, AOS_NO_WAIT);
    }
}
#endif

#ifdef CONFIG_PIN_WAKEUP_EIPU
/**
 * aic_pin_wakeup_eipu_callback - pin wakeup detect by eipu.
 *
 * @param
 *
 * Return:
 *
 */
static void aic_pin_wakeup_eipu_callback(u32 index, u32 value, u32 eipu_pol)
{
    u32 level, wakeup_status;

    if (pin_wakeup_at_status != 1) {
        return;
    }

    level = EIPU_GetIndata(NOSEC_MODE, EIPU_DBNC, PIN_WAKEUP_DETECT);
    AIC_ASSERT(level == 0 || level == 1);
    if (level == PIN_WAKEUP_LEVEL) {
        wakeup_status = PIN_WAKEUP;
        EIPU_SetPol(NOSEC_MODE, EIPU_DBNC, PIN_WAKEUP_DETECT, !level);
    } else {
        wakeup_status = PIN_SLEEP;
        EIPU_SetPol(NOSEC_MODE, EIPU_DBNC, PIN_WAKEUP_DETECT, !level);
    }

    if (wakeup_status != pin_wakeup_status) {
        pin_wakeup_status = wakeup_status;
        aos_send_to_queue(pin_wakeup_queue, &wakeup_status, AOS_NO_WAIT);
    }
    return;
}
#endif

/**
 * pin_wakeup_entry - pin wakeup by pin task.
 *
 * @param
 *
 * Return:
 *
 */
static void aic_pin_wakeup_entry(void *param)
{
    u32 ret, wakeup_status;

    while(1) {
        ret = aos_recv_from_queue(pin_wakeup_queue, &wakeup_status,
                                  AOS_WAIT_FOREVER);
        if (ret)
            continue;

        aicx_info(0x14a67d, "s", "wakeup pin status: %s\n", wakeup_status ? "wakeup" : "sleep");

        if (wakeup_status == PIN_WAKEUP) {
            aic_get_wakelock();
#ifdef PIN_WAKEUP_INDICATE
            aic_gpio_output(PIN_WAKEUP_INDICATE, PIN_WAKEUP_LEVEL);
#endif

#ifdef CONFIG_NET_STATUS_LIGHT_API
            net_status_light_enable(true);
#endif
        } else {
            aic_put_wakelock();
#ifdef PIN_WAKEUP_INDICATE
            aic_gpio_output(PIN_WAKEUP_INDICATE, PIN_SLEEP_LEVEL);
#endif

#ifdef CONFIG_NET_STATUS_LIGHT_API
            net_status_light_enable(false);
#endif
        }
    }
}

/**
 * pin_wakeup_csat_cmd - response AT+CSCLK=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 pin_wakeup_csat_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+CSCLK: (0-2)";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * pin_wakeup_csat_query_cmd - response AT+CSCLK?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 pin_wakeup_csat_query_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    u8 result_line[AT_MAX_AP_RESP_LENGTH];

    snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
            "+CSCLK: %d", pin_wakeup_at_status);
    line_pointers[0] = (ulong)result_line;
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * pin_wakeup_csat_set_cmd - response AT+CSCLK= <status>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 pin_wakeup_csat_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *para;
    u32 wakeup, delaysleep;
    int errcode, status;
    boolean valid;
    u16 count;
    AOS_INTERRUPT_DECLARE;

    do {
        errcode = AIC_SUCCESS;
        count = 0;
        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+CSCLK=",
                                                 &count);
        if ((count != 0) && (!paras_arr || (count != 1 && count != 2))) {
            errcode = AIC_EINVAL;
            break;
        }

        if (count == 0) {
            status = 0;
        } else {
            para = (u8 *)paras_arr[0];
            valid = aic_is_valid_integer_para(para, strlen(para));
            if (!valid) {
                errcode = AIC_EINVAL;
                break;
            }
            status = atoi((u8 *)paras_arr[0]);
            if (status < 0 || status > 2) {
                errcode = AIC_EINVAL;
                break;
            }
        }

        if (status == 2) {
            if (count == 2) {
                para = (u8 *)paras_arr[1];
                valid = aic_is_valid_integer_para(para, strlen(para));
                if (!valid) {
                    errcode = AIC_EINVAL;
                    break;
                }
                delaysleep = atoi((u8 *)paras_arr[1]);
                if (delaysleep < 1 || delaysleep > 255) {
                    errcode = AIC_EINVAL;
                    break;
                }
            } else {
                delaysleep = DEFAULT_DELAYSLEEP_TIME;
            }
            slp_set_delaysleep(delaysleep);
        } else {
            slp_set_delaysleep(0);
        }

        if (status != pin_wakeup_at_status) {
            AOS_INTERRUPT_DISABLE;
            pin_wakeup_at_status = status;
            AOS_INTERRUPT_RESTORE;

            if (pin_wakeup_at_status == 0) {
                wakeup = PIN_WAKEUP;
                aos_send_to_queue(pin_wakeup_queue, &wakeup, AOS_NO_WAIT);
            } else {
                wakeup = PIN_SLEEP;
                aos_send_to_queue(pin_wakeup_queue, &wakeup, AOS_NO_WAIT);
            }
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", -errcode);
        line_pointers[0] = (ulong)result_line;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    if (count)
        aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/**
 * pin_wakeup_at_cmd - response AT+QSCLK=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 pin_wakeup_qsat_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+QSCLK: (0-2)";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * pin_wakeup_qsat_query_cmd - response AT+QSCLK?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 pin_wakeup_qsat_query_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    u8 result_line[AT_MAX_AP_RESP_LENGTH];

    snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
            "+QSCLK: %d", pin_wakeup_at_status);
    line_pointers[0] = (ulong)result_line;
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * pin_wakeup_qsat_set_cmd - response AT+QSCLK= <status>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 pin_wakeup_qsat_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *para;
    u32 wakeup, delaysleep;
    int errcode, status;
    boolean valid;
    u16 count;
    AOS_INTERRUPT_DECLARE;

    do {
        errcode = AIC_SUCCESS;
        count = 0;
        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+QSCLK=",
                                                 &count);
        if ((count != 0) && (!paras_arr || (count != 1 && count != 2))) {
            errcode = AIC_EINVAL;
            break;
        }

        if (count == 0) {
            status = 0;
        } else {
            para = (u8 *)paras_arr[0];
            valid = aic_is_valid_integer_para(para, strlen(para));
            if (!valid) {
                errcode = AIC_EINVAL;
                break;
            }
            status = atoi((u8 *)paras_arr[0]);
            if (status < 0 || status > 2) {
                errcode = AIC_EINVAL;
                break;
            }
        }

        if (status == 2) {
            if (count == 2) {
                para = (u8 *)paras_arr[1];
                valid = aic_is_valid_integer_para(para, strlen(para));
                if (!valid) {
                    errcode = AIC_EINVAL;
                    break;
                }
                delaysleep = atoi((u8 *)paras_arr[1]);
                if (delaysleep < 1 || delaysleep > 255) {
                    errcode = AIC_EINVAL;
                    break;
                }
            } else {
                delaysleep = DEFAULT_DELAYSLEEP_TIME;
            }
            slp_set_delaysleep(delaysleep);
        } else {
            slp_set_delaysleep(0);
        }

        if (status != pin_wakeup_at_status) {
            AOS_INTERRUPT_DISABLE;
            pin_wakeup_at_status = status;
            AOS_INTERRUPT_RESTORE;

            if (pin_wakeup_at_status == 0) {
                wakeup = PIN_WAKEUP;
                aos_send_to_queue(pin_wakeup_queue, &wakeup, AOS_NO_WAIT);
            } else {
                wakeup = PIN_SLEEP;
                aos_send_to_queue(pin_wakeup_queue, &wakeup, AOS_NO_WAIT);
            }
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", -errcode);
        line_pointers[0] = (ulong)result_line;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    if (count)
        aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/*
 *  Add fs AT handle here.
 */
const static AT_CMD_HANDLE pin_wakeup_at_handles[] =
{
    {"AT+CSCLK", pin_wakeup_csat_cmd, pin_wakeup_csat_query_cmd, pin_wakeup_csat_set_cmd, NULL},
    {"AT+QSCLK", pin_wakeup_qsat_cmd, pin_wakeup_qsat_query_cmd, pin_wakeup_qsat_set_cmd, NULL},
    {AIC_NULL},
};

/**
 * aic_pin_wakeup_init - wakeup by pin.
 *
 * Return:
 *
 */
int aic_pin_wakeup_init(void)
{
    int ret, wakeup;

    pin_wakeup_thread_id = aos_create_thread("pin_wakeup_task",
                                             aic_pin_wakeup_entry, AOS_NULL,
                                             PIN_WAKEUP_TASK_STACK_SIZE, 0,
                                             AIC_NULL, PIN_WAKEUP_TASK_PRIORITY,
                                             AOS_AUTO_START);
    AIC_ASSERT(AOS_INVALID_THREAD_ID != pin_wakeup_thread_id);

    pin_wakeup_queue = aos_create_queue("pin_wakeup_queue", sizeof(u32),
                                        PIN_WAKEUP_TASK_QUEUE_CNT);
    AIC_ASSERT(AOS_NULL != pin_wakeup_queue);

#ifdef PIN_WAKEUP_INDICATE
    ret = aic_gpio_request(PIN_WAKEUP_INDICATE);
    AIC_ASSERT(ret == AIC_SUCCESS);

    aic_gpio_output(PIN_WAKEUP_INDICATE, PIN_SLEEP_LEVEL);
#endif

#if defined (CONFIG_PIN_WAKEUP_GPIO)
    ret = aic_gpio_request(PIN_WAKEUP_DETECT);
    AIC_ASSERT(ret == AIC_SUCCESS);

    aic_gpio_input(PIN_WAKEUP_DETECT, GPIO_TRIGGER_EDGE_ANY,
                   aic_pin_wakeup_callback, NULL);
#elif defined (CONFIG_PIN_WAKEUP_EIPU)
    if (PIN_WAKEUP_LEVEL == 0) {
        ret = EIPU_RegCallback(PIN_WAKEUP_DETECT, EIPU_DBNC,
                                aic_pin_wakeup_eipu_callback,
                                LOW_LEVEL, 0x20,
                                NOSEC_MODE);
    } else {
        ret = EIPU_RegCallback(PIN_WAKEUP_DETECT, EIPU_DBNC,
                                aic_pin_wakeup_eipu_callback,
                                HIGH_LEVEL, 0x20,
                                NOSEC_MODE);
    }
    AIC_ASSERT(ret == AIC_SUCCESS);
#endif

    aic_add_at_cmd_handle((AT_CMD_HANDLE *)pin_wakeup_at_handles);

    if (pin_wakeup_at_status == 0) {
        wakeup = PIN_WAKEUP;
        aos_send_to_queue(pin_wakeup_queue, &wakeup, AOS_NO_WAIT);
    }

    return AIC_SUCCESS;
}

AIC_COMP_P2_INIT_EXPORT(aic_pin_wakeup_init);

