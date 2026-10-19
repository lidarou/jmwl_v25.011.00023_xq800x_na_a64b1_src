/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file tele_cfg.c
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x053500
#define LOG_MODULE    AMOD_SYS

/*********************
 *      INCLUDES
 *********************/
#include "tele_cfg.h"
#include "gpio_inc.h"

/*********************
 *      DEFINES
 *********************/

/*************** START the macros which customer can config  *********************/

#define SIM_SLOT_COUNT                      (1)/* range 1, 2 */

#define SIM_SLOT_SWITCHED_BY_GPIO           (0)/* 0: disable switch slot by gpio. 1: enable switch slot by gpio */

#define SIM_SLOT_SWITCHED_GPIO_INDEX        (GPIO_9)/* the index of gpio which switch sim slot when SIM_SLOT_SWITCHED_BY_GPIO enabled */

#define SIM_SLOT_SWITCHED_SLOT0_LEVEL       (GPIO_OUTPUT_LVL_LOW)/* range GPIO_OUTPUT_LVL_LOW,GPIO_OUTPUT_LVL_HIGH.
                                                                   gpio level switch sim slot 0 when SIM_SLOT_SWITCHED_BY_GPIO enabled */

#define TELE_SRV_TYPE                       (TELE_SRV_DISABLE_AND_VOICE_DISABLE)/* enum of tele_srv_type_e */

#define DEFAULT_VOICE_CHANNEL_TYPE          (TELE_VOICE_CHANNEL_TYPE_END)/* enum of tele_voice_channel_type_e */

#define DEFAULT_VOICE_LOCAL_RINGBACK_TYPE   (TELE_VOICE_LOCAL_RINGBACK_TYPE_DEFAULT)/* enum of tele_voice_local_ringback_type_e */

#define DEFAULT_VOICE_VOLUME                (10)/* currently support 0~15 */

/*************** END the macros which customer can config  *********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
uint8_t tele_cfg_get_sim_slot_count(void)
{
    return SIM_SLOT_COUNT;
}

/* TODO: Support the configuration of slot 1 or slot 2 for single card. */

tele_srv_type_e tele_cfg_get_tele_srv_type(void)
{
    return TELE_SRV_TYPE;
}

tele_voice_channel_type_e tele_cfg_get_voice_channel_type(void)
{
    return DEFAULT_VOICE_CHANNEL_TYPE;
}

tele_voice_local_ringback_type_e tele_cfg_get_voice_local_ringback_type(void)
{
    return DEFAULT_VOICE_LOCAL_RINGBACK_TYPE;
}

uint32_t tele_cfg_get_voice_volume(void)
{
    return DEFAULT_VOICE_VOLUME;
}

void tele_cfg_switch_sim_slot_by_gpio(int32_t slot_id)
{
#if SIM_SLOT_SWITCHED_BY_GPIO
    GPIO_MODE level = 0;

    if (2 == SIM_SLOT_COUNT) {
        if (0 == slot_id) {

            level = SIM_SLOT_SWITCHED_SLOT0_LEVEL;
        } else if (1 == slot_id) {

            if (GPIO_OUTPUT_LVL_LOW == SIM_SLOT_SWITCHED_SLOT0_LEVEL) {
                level = GPIO_OUTPUT_LVL_HIGH;
            } else {
                level = GPIO_OUTPUT_LVL_LOW;
            }
        } else {
            aicx_error(0x089998, "sd", "[%s] error slot id %d .", __func__, slot_id);
            return;
        }
        aic_gpio_output(SIM_SLOT_SWITCHED_GPIO_INDEX, level);
        aicx_info(0x03f71d, "sddd", "[%s] Switch slot %d, gpio %d output level %d.", __func__, slot_id, SIM_SLOT_SWITCHED_GPIO_INDEX, level);

    } else {
        aicx_error(0x06c065, "s", "[%s] Single card ,please do not switched .", __func__);
    }

#else
    aicx_info(0x0b22db, "s", "[%s] not support switch sim slot by gpio.", __func__);
#endif
}

