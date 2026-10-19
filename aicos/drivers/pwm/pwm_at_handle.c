/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is pwm at cmd handle.
 *
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x00328c
#define LOG_MODULE    AMOD_DRV
#include <aic_io.h>
#include <aic_osa.h>
#include <aic_type.h>
#include <aic_at_process_common.h>
#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_reg_base.h>
#include <aic_at_resp_handle.h>
#include <aic_bitmap.h>
#include <pwm.h>
#include <timer_inc.h>
#include "./pwm_phy.h"
#include <aic_list.h>

struct list_pwm_list {
    int cfg_num;
    int enable;
};

struct pwm_cfg_unit {
    u32 pwm_period_us;
    u32 pwm_duty_perc;
    struct list_head unit_list; /*current pwm config*/
};

struct pwm_start_para {
    u32 pwm_idx;
    u32 pwm_run_cycles;
    int cfg_num;
    struct list_head *pwm_seq_list;
};

#define CUS_PWM_NUM 2
#define CUS_PWM_OFFSET 4
#define PWM_GAP_SCALING_US   5
#define PWM_CM_TASK_PRIO   16
aos_mutex_ptr pwm_at_mutex = AOS_NULL;
struct list_head *s_pwm_chan_cfg_list[8];
struct list_pwm_list pwm_list[8];
static aos_thread_id pwm_cm_task = AOS_INVALID_THREAD_ID;

struct pwm_start_para pwm_task_para[CUS_PWM_NUM];

int pwm_cm_enabled(int pwm_idx, int enabled, u32 cycles);

static aos_status pwm_lock()
{
    int ret = AOS_SUCCESS;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(pwm_at_mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(ret == AOS_SUCCESS);
    }

    return ret;
}

static void pwm_unlock()
{
    if (aos_is_user_thread_context()) {
        aos_put_mutex(pwm_at_mutex);
    }
}

int free_unit_list(struct list_head *list)
{
    struct pwm_cfg_unit *cur_unit;
    list_for_each_entry(cur_unit, list, unit_list, struct pwm_cfg_unit) {
        aos_free(cur_unit);
    }
    return 0;
}

int init_pwn_list(int index)
{

    s_pwm_chan_cfg_list[index] = aos_malloc(sizeof(struct list_head));
    list_head_init(s_pwm_chan_cfg_list[index]);
    pwm_list[index].cfg_num = 0;

    return 0;
}

int pwm_cm_config(int pwm_idx, u32 period_us, u32 duty_perc)
{
    struct pwm_cfg_unit *cur_pwm_cfg;
    int ret = 0;

    if (!period_us || pwm_idx > CUS_PWM_NUM || duty_perc > 100) {
        aicx_warn(0x072baf, "sddd", "%s unsupport idx:%d or period:%d duty_perc:%d\n",
                __func__, pwm_idx, period_us, duty_perc);
        ret = AIC_ENOENT;
        goto out;
    }

    cur_pwm_cfg = aos_zalloc(sizeof(struct pwm_cfg_unit));
    if (!cur_pwm_cfg) {
        aicx_warn(0x02ad96, "s", "%s not enough space!\n", __func__);
        ret = AIC_ENOMEM;
        goto out;
    }
    cur_pwm_cfg->pwm_period_us = period_us;
    cur_pwm_cfg->pwm_duty_perc = duty_perc;

    list_head_init(&cur_pwm_cfg->unit_list);


    ret = aos_get_mutex(pwm_at_mutex, AOS_NO_WAIT);
    if (ret) {
        aicx_warn(0x145a43, "s", "%s pwm alread run, forbid config, stop pwm before config!\n", __func__);
        return ret;
    }
    list_add_tail(&cur_pwm_cfg->unit_list, s_pwm_chan_cfg_list[pwm_idx]);
    pwm_list[pwm_idx].cfg_num ++;
    aos_put_mutex(pwm_at_mutex);

out:
    return ret;
}

#if 0
static void pwm_cm_task_entry(void *data)
{
    struct pwm_start_para *para = data;
    struct pwm_cfg_unit *cur_unit;
    u32 last_period_us = 0;
    int pwm_gap = 0;
    bool first = true;
    bool need_change = false;
    bool loop = false;

    pwm_lock();

    pwm_list[para->pwm_idx].enable = 1;
    pwm_unlock();

    if (!para->pwm_run_cycles)
        loop = true;

    if(loop) {
        while(1) {//loop forever until disable kill task
            list_for_each_entry(cur_unit, &s_pwm_chan_cfg_list[para->pwm_idx], unit_list, struct pwm_cfg_unit) {

                if (!first && (last_period_us != cur_unit->pwm_period_us)) {
                    pwm_gap = cur_unit->pwm_period_us - PWM_GAP_SCALING_US;
                    need_change = true;
                } else {
                    pwm_gap = cur_unit->pwm_period_us;
                    need_change = false;
                }
                if (!first && (pwm_gap > 0))
                    aos_usleep(pwm_gap);

                if (need_change) {
                    pwm_enabled(para->pwm_idx + CUS_PWM_OFFSET, false);
                }
                if (first || need_change) {
                    pwm_set(para->pwm_idx + CUS_PWM_OFFSET, cur_unit->pwm_duty_perc * cur_unit->pwm_period_us * 10, cur_unit->pwm_period_us * 1000);
                }
                first = false;
                last_period_us = cur_unit->pwm_period_us;
            }
        }
    } else {
        while(para->pwm_run_cycles--){ //loop forever until disable kill task
            list_for_each_entry(cur_unit, &s_pwm_chan_cfg_list[para->pwm_idx], unit_list, struct pwm_cfg_unit) {

                if (!first && (last_period_us != cur_unit->pwm_period_us)) {
                    pwm_gap = cur_unit->pwm_period_us - PWM_GAP_SCALING_US;
                    need_change = true;
                } else {
                    pwm_gap = cur_unit->pwm_period_us;
                    need_change = false;
                }
                if (!first && (pwm_gap > 0))
                    aos_usleep(pwm_gap);

                if (need_change) {
                    pwm_enabled(para->pwm_idx + CUS_PWM_OFFSET, false);
                }
                if (first || need_change) {
                    pwm_set(para->pwm_idx + CUS_PWM_OFFSET, cur_unit->pwm_duty_perc * cur_unit->pwm_period_us * 10, cur_unit->pwm_period_us * 1000);
                }
                first = false;
                last_period_us = cur_unit->pwm_period_us;
            }
        }
    }
    if (last_period_us > 0) {
        aos_usleep(last_period_us);
    }
    pwm_enabled(para->pwm_idx + CUS_PWM_OFFSET, false);

    pwm_lock();
    pwm_list[para->pwm_idx].enable = 0;
    pwm_unlock();
}
#else
static void pwm_cm_task_entry(void *data)
{
    struct pwm_start_para *para = data;
    struct pwm_cfg_unit *cur_unit;
    int ret;
    int pwm_gap;

    bool loop = false;

    pwm_lock();
    pwm_list[para->pwm_idx].enable = 1;
    pwm_unlock();

    if (!para->pwm_run_cycles)
        loop = true;

    if (para->cfg_num == 1 && loop) {

        pwm_lock();
        cur_unit = list_last_entry(para->pwm_seq_list,
                                    struct pwm_cfg_unit, unit_list);
        pwm_unlock();
        ret = pwm_set(para->pwm_idx + CUS_PWM_OFFSET, cur_unit->pwm_duty_perc * cur_unit->pwm_period_us * 10, cur_unit->pwm_period_us * 1000);
        goto out;
    } else if (para->cfg_num == 1) {
        pwm_lock();
        cur_unit = list_last_entry(para->pwm_seq_list,
                                    struct pwm_cfg_unit, unit_list);
        pwm_unlock();
        pwm_set(para->pwm_idx + CUS_PWM_OFFSET, cur_unit->pwm_duty_perc * cur_unit->pwm_period_us * 10, cur_unit->pwm_period_us * 1000);

        pwm_gap = cur_unit->pwm_period_us * para->pwm_run_cycles - PWM_GAP_SCALING_US;
        if (pwm_gap > 0)
            aos_usleep(cur_unit->pwm_period_us * para->pwm_run_cycles - PWM_GAP_SCALING_US);
        pwm_enabled(para->pwm_idx + CUS_PWM_OFFSET, false);
    } else if(loop) {
        while(1) {//loop forever until disable kill task
            list_for_each_entry(cur_unit, para->pwm_seq_list, unit_list, struct pwm_cfg_unit) {
                pwm_set(para->pwm_idx + CUS_PWM_OFFSET, cur_unit->pwm_duty_perc * cur_unit->pwm_period_us * 10, cur_unit->pwm_period_us * 1000);
                pwm_gap = cur_unit->pwm_period_us - PWM_GAP_SCALING_US;
                if (pwm_gap > 0)
                    aos_usleep(pwm_gap);
                pwm_enabled(para->pwm_idx  + CUS_PWM_OFFSET, false);
            }
        }

    } else {
        while(para->pwm_run_cycles--) //loop forever until disable kill task
        list_for_each_entry(cur_unit, para->pwm_seq_list, unit_list, struct pwm_cfg_unit) {
            pwm_set(para->pwm_idx + CUS_PWM_OFFSET, cur_unit->pwm_duty_perc * cur_unit->pwm_period_us * 10, cur_unit->pwm_period_us * 1000);
            pwm_gap = cur_unit->pwm_period_us - PWM_GAP_SCALING_US;
            if (pwm_gap > 0)
                aos_usleep(pwm_gap);
            pwm_enabled(para->pwm_idx + CUS_PWM_OFFSET, false);
        }
    }
    pwm_lock();
    pwm_list[para->pwm_idx].enable = 0;
    pwm_unlock();
out:
    return;
}
#endif

void stop_pwm_working(int pwm_idx)
{
    if (aos_is_thread_created(pwm_cm_task)) {
        aos_terminate_thread(pwm_cm_task);
        aos_delete_thread(pwm_cm_task);
        /* clear working list */
        pwm_task_para[pwm_idx].pwm_run_cycles = 0;
        pwm_task_para[pwm_idx].cfg_num = 0;
        if (pwm_task_para[pwm_idx].pwm_seq_list) {
            free_unit_list(pwm_task_para[pwm_idx].pwm_seq_list);
            pwm_task_para[pwm_idx].pwm_seq_list = NULL;
        }
    }
    pwm_enabled(pwm_idx + CUS_PWM_OFFSET, false);
    pwm_list[pwm_idx].enable = 0;
}

int pwm_cm_enabled(int pwm_idx, int enabled, u32 cycles)
{
    int ret = 0;

    if (enabled) {
        if (!pwm_list[pwm_idx].cfg_num) {
            aicx_error(0x15dfdf, "s", "%s pwm cfg list is NULL!\n", __func__);
            ret = AIC_ECHILD;
            goto out;
        }

        if (aos_is_thread_created(pwm_cm_task)) {
            stop_pwm_working(pwm_idx);
        }
 
        pwm_task_para[pwm_idx].pwm_idx = pwm_idx;
        pwm_task_para[pwm_idx].pwm_run_cycles = cycles;
        pwm_task_para[pwm_idx].cfg_num = pwm_list[pwm_idx].cfg_num;
        pwm_task_para[pwm_idx].pwm_seq_list = s_pwm_chan_cfg_list[pwm_idx];
        pwm_list[pwm_idx].cfg_num = 0;
        s_pwm_chan_cfg_list[pwm_idx] = NULL;
        pwm_cm_task = aos_create_thread_simply("pwm_cm_task",
                                                     pwm_cm_task_entry,
                                                     &pwm_task_para[pwm_idx], 1024, PWM_CM_TASK_PRIO);
        if (pwm_cm_task == AOS_INVALID_THREAD_ID) {
            aicx_error(0x146d13, "s", "%s create task fail!\n", __func__);
            ret =  AIC_ENXIO;
            goto out;
        }
        //get lock make sure system wakeup
    } else {

        stop_pwm_working(pwm_idx);
        //put lock make sure system can enter sleep
    }

out:
    return ret;
}

int pwm_cm_init(void)
{
    int i;

    pwm_at_mutex = aos_create_mutex("pwm_at_mutex");
    AIC_ASSERT(pwm_at_mutex);

    for (i = 0; i < CUS_PWM_NUM; i ++) {
        s_pwm_chan_cfg_list[i] = aos_malloc(sizeof(struct list_head));
        list_head_init(s_pwm_chan_cfg_list[i]);
        pwm_list[i].cfg_num = 0;
        pwm_list[i].enable = 0;
    }

    return 0;
}
AIC_DRV_L1_INIT_EXPORT(pwm_cm_init);

static u32 at_pwm_ctrl_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];

    sprintf(rsp_str, "+MPWMCTRL: AT+MPWMCTRL=<channel>,<onoff>[,<cycle>]");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 at_pwm_ctrl_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    ulong *paras_arr;
    u16 count;
    int ret, channel, onoff, cycles;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+MPWMCTRL=", &count);
    if (!paras_arr || count > 3)
        goto CMD_ERROR;

    if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0]))
        || !aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
        goto CMD_ERROR;

    if (count == 3) {
        if(!aic_is_valid_integer_para((u8 *)paras_arr[2], strlen((u8 *)paras_arr[2])))
            goto CMD_ERROR;

        cycles = atoi((u8 *)paras_arr[2]);
    } else if (count == 2) {
        cycles = 0;
    } else
        goto CMD_ERROR;

    channel = atoi((u8 *)paras_arr[0]);
    onoff = atoi((u8 *)paras_arr[1]);

    if (channel >= CUS_PWM_NUM || channel < 0)
        goto CMD_ERROR;

    ret = pwm_cm_enabled(channel, onoff, cycles);
    if (ret)
        goto CMD_ERROR;

    strcpy(result_line0, "OK");
    /* fill response,one lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    aic_free_paras_mem(paras_arr, count);
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;

CMD_ERROR:
    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "+CME ERROR: %d", 3);
    line_pointers[0] = (ulong)&result_line0[0];
    aic_free_paras_mem(paras_arr, count);
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

static u32 at_pwm_data_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    struct pwm_cfg_unit *cur_pwm_cfg;
    int i;

    for ( i = 0; i < CUS_PWM_NUM; i ++) {
        memset(result_line0, 0, sizeof(result_line0));
        if (!pwm_list[i].cfg_num) {
            //snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "+MPWMDATA: %d,None", i);
            continue;
        } else {
            snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "+MPWMDATA: %d", i);
            list_for_each_entry(cur_pwm_cfg, s_pwm_chan_cfg_list[i], unit_list, struct pwm_cfg_unit) {
                snprintf(result_line0 + strlen(result_line0), AT_MAX_AP_RESP_LENGTH, ",%d,%d",
                                        cur_pwm_cfg->pwm_period_us, cur_pwm_cfg->pwm_duty_perc);
            }
        }
        /* Fill response,one line. */
        line_pointers[0] = (ulong)&result_line0[0];
        aos_usleep(1000);//add for trigger at flush buf to send.
        aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    }

    aic_reply_ok_resp(at_req_res_ptr);

    return AIC_SUCCESS;
}

static u32 at_pwm_data_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u8 result_line1[5] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    struct pwm_cfg_unit *cur_pwm_cfg;
    ulong *paras_arr;
    u16 count = 0;
    int i, rsp_cnt = 2;
    int pwm_chan;


    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+MPWMDATA=", &count);
    if (!paras_arr)
        goto CMD_ERROR;

    pwm_chan = atoi((u8 *)paras_arr[0]);
    if (pwm_chan > CUS_PWM_NUM - 1)
        goto CMD_ERROR;

    if (count == 1) {
        if (!pwm_list[pwm_chan].cfg_num)
            goto CMD_ERROR;

        pwm_lock();
        snprintf(result_line0, AT_MAX_AP_RESP_LENGTH, "+MPWMDATA: %d", pwm_chan);
        list_for_each_entry(cur_pwm_cfg, s_pwm_chan_cfg_list[pwm_chan], unit_list, struct pwm_cfg_unit) {
            snprintf(result_line0 + strlen(result_line0), AT_MAX_AP_RESP_LENGTH, ",%d,%d",
                                    cur_pwm_cfg->pwm_period_us, cur_pwm_cfg->pwm_duty_perc);
        }
        pwm_unlock();
    } else if (count % 2 == 1) {
        /* do config */
        if (s_pwm_chan_cfg_list[pwm_chan]) {
            free_unit_list(s_pwm_chan_cfg_list[pwm_chan]);
            s_pwm_chan_cfg_list[pwm_chan] = NULL;
        }
        init_pwn_list(pwm_chan);
        if (!s_pwm_chan_cfg_list[pwm_chan]) {
            s_pwm_chan_cfg_list[pwm_chan] = aos_malloc(sizeof(struct list_head));
            list_head_init(s_pwm_chan_cfg_list[pwm_chan]);
        }
        for ( i = 1; i < count; i += 2)
            if (pwm_cm_config(pwm_chan, atoi((u8 *)paras_arr[i]), atoi((u8 *)paras_arr[i + 1])))
                goto CMD_ERROR;

        rsp_cnt = 1;
    } else {
        goto CMD_ERROR;
    }

    strncpy(result_line1, AT_RESULT_OK, (sizeof(result_line1) - 1));
    /* Fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_free_paras_mem(paras_arr, count);
    aic_fill_transmit_cmd_resp(at_req_res_ptr, &line_pointers[2 - rsp_cnt], rsp_cnt);

    return AIC_SUCCESS;

CMD_ERROR:
    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "+CME ERROR: %d", AT_ERROR_INVALID_CMD);
    line_pointers[0] = (ulong)&result_line0[0];
    aic_free_paras_mem(paras_arr, count);
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

#ifdef CONFIG_PWM_BASE_AT
static u32 at_pwm_test_cmd(AT_REQ_RESP *at_req)
{
    char rsp_str[128] = {0};
    ulong rsp[2];

    sprintf(rsp_str, "AT+PWM=<channel>,<duty_perc_ns>,<period_ns>");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 at_pwm_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    ulong *paras_arr;
    u16 count;
    int ret;
    u32 channel, duty_perc_ns, period_ns;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+PWM=", &count);
    if (!paras_arr || count != 3)
        goto CMD_ERROR;

    channel = atoi((u8 *)paras_arr[0]);
    duty_perc_ns = atoi((u8 *)paras_arr[1]);
    period_ns = atoi((u8 *)paras_arr[2]);

    ret = pwm_set(channel, duty_perc_ns, period_ns);

    strcpy(result_line0, "OK");
    /* fill response,one lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    aic_free_paras_mem(paras_arr, count);
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;

CMD_ERROR:
    snprintf(result_line0, AT_MAX_AP_RESP_LENGTH,
             "+CME ERROR: %d", AT_ERROR_INVALID_CMD);
    line_pointers[0] = (ulong)&result_line0[0];
    aic_free_paras_mem(paras_arr, count);
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}
#endif

AT_CMD_HANDLE pwm_at_handles[] = {
#ifdef CONFIG_PWM_BASE_AT
    {
        "AT+PWM",
        at_pwm_test_cmd,
        AIC_NULL,
        at_pwm_cmd,
        AIC_NULL
    },
#endif
    {
        "AT+MPWMCTRL",
        at_pwm_ctrl_test_cmd,
        AIC_NULL,
        at_pwm_ctrl_cmd,
        AIC_NULL
    },
    {
        "AT+MPWMDATA",
        AIC_NULL,
        at_pwm_data_query_cmd,
        at_pwm_data_cmd,
        AIC_NULL
    },
    {AIC_NULL},
};

int pwm_at_cmd_init(void)
{
    aic_add_at_cmd_handle(&pwm_at_handles[0]);

    return 0;
}
AIC_NORMAL_COMP_L2_INIT_EXPORT(pwm_at_cmd_init);
