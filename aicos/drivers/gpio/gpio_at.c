/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file for gpio at cmd handle.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x15a1e7
#define LOG_MODULE    AMOD_GPIO

#include <aic_osa.h>
#include <aic_assert.h>
#include <aic_at_process_common.h>
#include <aic_at_resp_handle.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aic_type.h>
#include <gpio_inc.h>

#ifdef CONFIG_CUSTOMER_CM_AT
/**
 * aic_gpio_m_cmd Get GPIO at command info
 * AT+MGPIO=?
 * output info: [gpio pin][mode]
 */
static u32 aic_gpio_m_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];

    rsp[0] = (ulong)"+MGPIO: (0,4,9-11,19,20,33,34),(0-2)";
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

/**
 * aic_gpio_m_cmd_cfg Get GPIO input value at command
 * AT+MGPIO=<pin><control>
 * pin: gpio pin
 * control: 0 - input; 1 - output low; 2 - output high
 */
static u32 aic_gpio_m_cmd_cfg(AT_REQ_RESP *at_req_res_ptr)
{
    ulong line_pointers[5];
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong *paras_arr;
    GPIO_MODE mode;
    u16 paras_count = 0;
    u32 error_code, gpio_index, gpio_level;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+MGPIO=", &paras_count);
    if (!paras_arr || !paras_count || paras_count > 2) {
        error_code = AT_ERROR_INVALID_PARAS;
        goto PARA_ERROR;
    }

    gpio_index = atoi((u8 *)paras_arr[0]);
    if (gpio_index >= GPIO_PIN_MAX) {
        error_code = AT_ERROR_INVALID_PARAS;
        goto PARA_ERROR;
    }

    /* AT+MGPIO=<pin> */
    if (paras_count == 1) {
        mode = aic_gpio_get_mode(gpio_index);
        switch (mode) {
        case GPIO_OUTPUT_LVL_LOW:
            gpio_level = 1;
            break;
        case GPIO_OUTPUT_LVL_HIGH:
            gpio_level = 2;
            break;
        default:
            gpio_level = 0;
        }
        /* fill response,one lines. */
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "+MGPIO: %d,%d", gpio_index, gpio_level);
        line_pointers[0] = (ulong)&result_line[0];
        aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
        return AIC_SUCCESS;
    }

    /* AT+MGPIO=<pin><control> */
    gpio_level = atoi((u8 *)paras_arr[1]);
    if (gpio_level > 2) {
        error_code = AT_ERROR_INVALID_PARAS;
        goto PARA_ERROR;
    }

    if (!gpio_level) {
        aic_gpio_input(gpio_index, GPIO_INPUT_NOINT, NULL, NULL);
        gpio_level = aic_gpio_level_get(gpio_index);
        switch (gpio_level) {
        case GPIO_OUTPUT_LVL_LOW:
            gpio_level = 1;
            break;
        case GPIO_OUTPUT_LVL_HIGH:
            gpio_level = 2;
            break;
        default:
            gpio_level = 1;
        }

        /* fill response,one lines. */
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "+MGPIO: %d,%d", gpio_index, gpio_level);
        line_pointers[0] = (ulong)&result_line[0];
        aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    } else {
        switch (gpio_level) {
        case 1:
            gpio_level = GPIO_OUTPUT_LVL_LOW;
            break;
        case 2:
            gpio_level = GPIO_OUTPUT_LVL_HIGH;
            break;
        default:
            gpio_level = GPIO_OUTPUT_LVL_LOW;
        }
        aic_gpio_output(gpio_index, gpio_level);
        /* fill response,one lines. */
        strcpy(result_line, "OK");
        line_pointers[0] = (ulong)&result_line[0];
        aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    }

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "+CME ERROR: %d", error_code);
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}
#endif

AT_CMD_HANDLE gpio_at_handles[] = {
#ifdef CONFIG_CUSTOMER_CM_AT
    {"AT+MGPIO", aic_gpio_m_cmd, AIC_NULL, aic_gpio_m_cmd_cfg, AIC_NULL},
#endif
    {AIC_NULL},
};

int aic_gpio_at_cmd_init(void)
{
    aic_add_at_cmd_handle(gpio_at_handles);
    return 0;
}
AIC_COMP_L2_INIT_EXPORT(aic_gpio_at_cmd_init);
