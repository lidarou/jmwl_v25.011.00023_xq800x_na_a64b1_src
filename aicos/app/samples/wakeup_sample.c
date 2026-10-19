/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file wakeup_sample.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <aic_list.h>
#include <strings.h>
#include <aic_init.h>
#include <aic_errno.h>
#include <aic_osa_ex.h>
#include "aic_at_cmd_handle.h"
#include "aic_at_interact_cp.h"
#include "aic_at_parse_utils.h"
#include "gpio_inc.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/
static int wakeup_sample_init(void)
{
    int ret = -1;

    ret = aic_gpio_request(GPIO_AON_10);
    if (AIC_SUCCESS != ret) {
        app_loge("[%s]aic_gpio_request fail.(ret = %d)", __func__, ret);
        return -1;
    }
    aic_gpio_output(GPIO_AON_10, GPIO_OUTPUT_LVL_LOW);
    ret = aic_gpio_request(GPIO_AON_11);
    if (AIC_SUCCESS != ret) {
        app_loge("[%s]aic_gpio_request fail.(ret = %d)", __func__, ret);
        return -1;
    }
    aic_gpio_input(GPIO_AON_11, GPIO_INPUT_NOINT, NULL, NULL);

    return AIC_SUCCESS;
}

AIC_NORMAL_APP_P2_INIT_EXPORT(wakeup_sample_init);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
boolean wakeup_sample_is_urc_need_wakeup(u8 *urc_str)
{
    u8 *p_dst_str = NULL;
    u16 string_len = 0;
    ulong *p_paras_arr = NULL;
    u16 paras_cnt = 0;
    bool is_need_wakeup = false;

    if (strcasestr(urc_str, "+GMQTTURC:") || strcasestr(urc_str, "+GHTTPURC:") ||
        strcasestr(urc_str, "+GIPURC:")) {
        return true;
    }

    string_len = strlen(urc_str) - 4;
    p_dst_str = (u8 *)aic_zalloc(string_len + 1);
    if (NULL == p_dst_str) {
        app_loge("[%s] malloc fail.\r\n", __func__);
        return is_need_wakeup;
    }
    aic_delete_start_end_symbols(p_dst_str, urc_str, string_len);

    if (0 == strlen(p_dst_str)) {
        app_logi("[%s] urc length is 0.\r\n", __func__, strlen(p_dst_str));
        aic_free(p_dst_str);
        return is_need_wakeup;
    }

    if (strcasestr(p_dst_str, "+CMCCSI: ")) {/* the urc for call */
        /* get the parameter afer +CMCCSI: . */
        p_paras_arr = aic_split_paras_to_str_array(p_dst_str, "+CMCCSI: ", &paras_cnt);
        app_logi("[%s] paras_cnt = %d.\r\n", __func__, paras_cnt);

        if (p_paras_arr && (11 <= paras_cnt)) {
            u32 cc_status = 0;

            /* parse cc status parameter. */
            cc_status = (u32)atoi((u8 *)p_paras_arr[6]);
            app_logi("[%s] ccstatus = %d. \r\n", __func__, cc_status);
            if (5 == cc_status) {
                /* incoming call, need wake up JL */
                app_logi("[%s] incoming call, need wake up JL. \r\n", __func__);
                is_need_wakeup = true;
            }
        }

        /* Free paras. */
        aic_free_paras_mem(p_paras_arr, paras_cnt);
    } else if (strcasestr(p_dst_str, "+CMTI: ")) {/* the urc for receive sms */
        /* receive sms, need wake up JL */
        app_logi("[%s] receive sms, need wake up JL. \r\n", __func__);
        is_need_wakeup = true;
    } else if (strcasestr(p_dst_str, "^CARDSTATUS: ")) {/* the urc for card status updated */
        /* receive card status updated, need wake up JL */
        app_logi("[%s] receive sms, need wake up JL. \r\n", __func__);
        is_need_wakeup = true;
    }

    aic_free(p_dst_str);

    return is_need_wakeup;
}

void wakeup_sample_enable_mcu_sleep(void)
{
    aic_gpio_output(GPIO_AON_10, GPIO_OUTPUT_LVL_LOW);
}

bool wakeup_is_mcu_sleeping(void)
{
    return (GPIO_OUTPUT_LVL_HIGH != aic_gpio_level_get(GPIO_AON_11));
}

boolean wakeup_sample_awaken_mcu(void)
{
    u8 count = 0;

    aic_gpio_output(GPIO_AON_10, GPIO_OUTPUT_LVL_HIGH);
    aos_sleep(1);
    while (GPIO_OUTPUT_LVL_HIGH != aic_gpio_level_get(GPIO_AON_11)) {
        /* wait for GPIO_AON_11 to high */
        count++;
        if (count <= 500) {
            aos_sleep(1);
            continue;
        } else
            break;
    }

    return (GPIO_OUTPUT_LVL_HIGH == aic_gpio_level_get(GPIO_AON_11));
}

