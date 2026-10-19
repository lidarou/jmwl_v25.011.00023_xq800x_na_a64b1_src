/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * Multi hardware auto adapt.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0736e6
#define LOG_MODULE    AMOD_DRV

#include <aic_type.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aic_cmdline.h>
#include <mha.h>
#include <mha_cfg.h>

#define VALID_BOARD (g_mha_cfg_id != 0xFFFFFFFF)

unsigned int g_mha_cfg_id;

#ifdef CONFIG_MHA_PIN
void *mha_get_dig_pin_cfg(void)
{
    if (VALID_BOARD)
        return mha_cfg[g_mha_cfg_id].dig_pin.cfg;

    return NULL;
}

void *mha_get_pmu_pin_cfg(void)
{
    if (VALID_BOARD)
        return mha_cfg[g_mha_cfg_id].pmu_pin.cfg;

    return NULL;
}
#endif

#ifdef CONFIG_MHA_LCD_CFG
void *mha_get_lcd_cfg(void)
{
    if (VALID_BOARD)
        return &mha_cfg[g_mha_cfg_id].lcd;

    return NULL;
}
#endif

#ifdef CONFIG_MHA_CAMERA
void *mha_get_camera_cfg(void)
{
    if (VALID_BOARD)
        return &mha_cfg[g_mha_cfg_id].camera;

    return NULL;
}
#endif


#ifdef CONFIG_MHA_SOUND
void *mha_get_sound_cfg(void)
{
    if (VALID_BOARD)
        return &mha_cfg[g_mha_cfg_id].sound;

    return NULL;
}
#endif

static u32 get_board_cfg_id(char * board_id)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(mha_cfg); i++)
        if (!strcmp(board_id, mha_cfg[i].board_id))
            return i;

    return (u32)(-1);
}

void mha_init(void)
{
    char *boardid = aic_cmdline_get_value(cl_key_boardid);

    g_mha_cfg_id = get_board_cfg_id(boardid);
    aicx_info(0x041f21, "sd", "mha init, board id is: %s, index: %d\n", boardid, g_mha_cfg_id);

    return;
}

