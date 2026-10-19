/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the AIC system parameter source file.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x14ed14
#define LOG_MODULE    AMOD_INIT

#include <aic_cmdline.h>
#include <aic_sys_param.h>
#include <alog_api.h>
#include <stdlib.h>

static int aic_sys_get_key_int_value(cl_key_t key)
{
    int v = 0, i;
    char str[10] = {0};
    char *value = aic_cmdline_get_value(key);

    if (value) {
        for (i = 0; i < sizeof(str) && *value; i++, value++ ) {
            if (*value == '\n')
                break;

            str[i] = *value;
        }

        v = atoi(str);
    } else {
        aicx_debug(0x1b5802, "d", "can't find key(%d), return 0\n", key);
    }
    return v;
}

bool aic_sys_is_32kless(void)
{
    return (aic_sys_get_key_int_value(cl_key_32kless) > 0);
}

int aic_sys_get_crystal_type(void)
{
    return aic_sys_get_key_int_value(cl_key_crystal);
}

int aic_sys_get_board_id(void)
{
    return aic_sys_get_key_int_value(cl_key_boardid);
}

int aic_sys_get_boot_cnt(void)
{
    return aic_sys_get_key_int_value(cl_key_bootcnt);
}

