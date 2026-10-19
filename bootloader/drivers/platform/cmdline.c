/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the AIC parse cmdline file.
 */

#include "boot_mode.h"
#include "platform/aic_cmdline.h"
#include "initcall.h"
#include "platform/platform.h"
#include "psram_mem_config.h"
#include "console/console.h"
#include "aic_errno.h"
#include "stdio.h"
#include "stdlib.h"

#define CMD_INIT_VALUE  0xfe

#define CMD_WAIT_KEY_END    0
#define CMD_WAIT_VALUE_END  1

const static char *s_cmd_key[cl_key_cnt] = {
    KEY_GXEM,
    KEY_32KLESS,
    KEY_BOARDID,
    KEY_CRYSTAL,
    KEY_BOOT_CNT,
    KEY_PSRAMID,
    KEY_BOOTMODE,
    KEY_USERMODE,
    KEY_PWR_SW,
    KEY_PWR_ON,
    KEY_PWR_OFF,
    KEY_DEV_TYPE,
    KEY_DEV_COM,
    KEY_DEV_BAUD,
    KEY_ASRT_INFO,
    KEY_NV_INFO,
    KEY_NVCUST_INFO,
    KEY_NVDIFF_INFO,
    KEY_NVCALI_INFO,
    KEY_NVNORM_INFO,
    KEY_SNOR_CS_NUM,
    KEY_SNOR0_MINCLK,
    KEY_SNOR0_MAXCLK,
    KEY_SNOR1_MINCLK,
    KEY_SNOR1_MAXCLK,
    KEY_PMIC_ID,
    KEY_PLL_VCO,
    KEY_DCXO_TYPE
};

#define MAX_ASSERT_INFO_LEN     255
static char s_assert_info[MAX_ASSERT_INFO_LEN + 1];

/* the s_cmd_value[index] save the value string of the key */
static char *s_cmd_value[cl_key_cnt];
static u16 s_cmd_offset[cl_key_cnt];

static u32 cmd_key_find_index(const char *key)
{
    u32 i;

    for (i = 0; i < cl_key_cnt; i++) {
        if (!s_cmd_key[i])
            continue;

        if (0 == strncmp(s_cmd_key[i], key, strlen(s_cmd_key[i])))
            break;
    }

    return i;
}

void aic_cmdline_add_assert_info(void)
{
    if (s_assert_info[0] != 0)
        aic_cmdline_add_key(cl_key_asrt_info, s_assert_info);
}

/*
 * every key must end with '\n',
 * the whole key section must end with '\0'
 * cmdline format like below:
 * key0=value0
 * key1=value1
 * ...........
 * keyn=valuen
 */

void aic_cmdline_add_key(cl_key_t key, char *value)
{
    static char *s_pos = (char *)SHM_CMDLINE_BASE;
    const char *p;

    ASSERT(key < cl_key_cnt);
    p = s_cmd_key[key];

    /* add key */
    while(*p)
        *s_pos++ = *p++;

    *s_pos++ = '=';
    /* save the key value pos offset. */
    s_cmd_offset[key] = (u16)((u32)s_pos - SHM_CMDLINE_BASE);

    /* add value */
    p = value;
    while(*p)
        *s_pos++ = *p++;

    /* end this key. */
    *s_pos++ = 0;
}

void aic_cmdline_update_key(cl_key_t key, char *value)
{
    char *dst;

    if (!s_cmd_offset[key]) {
        aic_cmdline_add_key(key, value);
        return;
    }

    dst = (char *)SHM_CMDLINE_BASE + s_cmd_offset[key];

    while (*dst != 0 && *value)
     *dst++ = *value++;

    *dst = 0;
}

void aic_cmdline_parse(void)
{
    char *p = (char *)SHM_CMDLINE_BASE;
    char *end = p + SHM_CMDLINE_SIZE;

    u32 index = 0, state = CMD_WAIT_KEY_END;
    const char *key = p;

    while (p < end) {
        /* 0xfefefe, cmdline end. */
        if (*p == CMD_INIT_VALUE &&
            (*(p + 1) == CMD_INIT_VALUE) &&
            (*(p + 2) == CMD_INIT_VALUE) &&
            (*(p + 3) == CMD_INIT_VALUE))
            break;

        if (state == CMD_WAIT_KEY_END) {
            if (*p != '=') {
                p++;
                continue;
            }

            state = CMD_WAIT_VALUE_END;
            /* skip the '='. */
            p++;

            /* find key index, and save the key, key must skip 0. */
            while (*key == 0)
                key++;

            index = cmd_key_find_index(key);
            log_print("parse cmdline: key[%d]=%s\n", index, key);
            if (index < cl_key_cnt)
                s_cmd_value[index] = p;
        }

        if (state == CMD_WAIT_VALUE_END) {
            if (*p != 0) {
                p++;
                continue;
            }

            /* reset state */
            state = CMD_WAIT_KEY_END;
            key = p + 1;
        }

        p++;
    }
}

char *aic_cmdline_get_value(cl_key_t key)
{
    return s_cmd_value[key];
}

void aic_cmdline_add_key_int(cl_key_t key, int value)
{
    char str[21];

    if (get_boot_mode() == SYSDUMP_BOOT)
        return;

    __itoa(value, str, 10);
    aic_cmdline_add_key(key, str);
}

int aic_cmdline_get_value_int(cl_key_t key)
{
    if (!s_cmd_value[key])
        return 0;

    return atoi(s_cmd_value[key]);
}

int aic_cmdline_init(void)
{
    if (get_boot_mode() == SYSDUMP_BOOT)
        return AIC_SUCCESS;

    /* ap crash, pass.it to big sys for save last ap assert info to fs.*/
    if (is_ap_crash_mode()) {
        char *str, *info_addr;

        /* pass cmd line. for get assert info. */
        aic_cmdline_parse();

        /* get assert info and save to s_assert_info. */
        info_addr = aic_cmdline_get_value(cl_key_asrt_info);
        if (info_addr) {
            str = (char *)(strtoul(info_addr, NULL, 16));
            strncpy(s_assert_info, str, MAX_ASSERT_INFO_LEN);
        }

        /* reset s_cmd_value. */
        memset(s_cmd_value, 0, sizeof(s_cmd_value));
    }

    /* reset cmdline. */
    memset((u8 *)SHM_CMDLINE_BASE, CMD_INIT_VALUE, SHM_CMDLINE_SIZE);

    return AIC_SUCCESS;
}
early_init(aic_cmdline_init);
