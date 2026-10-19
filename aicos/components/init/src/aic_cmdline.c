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

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1778c3
#define LOG_MODULE    AMOD_INIT

#include <aic_assert.h>
#include <aic_cmdline.h>
#include <alog_api.h>
#ifdef WIN32
#include <aos_win32_dummy.h>
#else
#include <cache.h>
#include <psram_mem_config.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#define SHM_CMDLINE_SIZE  0x800
u8 s_cmdline_buf[SHM_CMDLINE_SIZE];
#define SHM_CMDLINE_BASE  s_cmdline_buf
static char *s_pos = s_cmdline_buf;
#else
static char *s_pos;
#endif

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

/* the s_cmd_value[index] save the value string of the key */
static char *s_cmd_value[cl_key_cnt];
static u16 s_cmd_offset[cl_key_cnt];

static u32 cmd_key_find_index(const char *key)
{
    u32 i, pos;

    pos = (u32)strchr(key, '=') - (u32)key;

    for (i = 0; i < cl_key_cnt; i++) {
        if (!s_cmd_key[i])
            continue;

        if ((pos == strlen(s_cmd_key[i])) && (0 == strncmp(s_cmd_key[i], key, strlen(s_cmd_key[i]))))
            break;
    }

    return i;
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
    const char *p;

    AIC_ASSERT(key < cl_key_cnt && s_pos);
    p = s_cmd_key[key];

    /* add key */
    while(*p)
        *s_pos++ = *p++;

    *s_pos++ = '=';
    /* save the key value pos offset. */
    s_cmd_offset[key] = (u16)((u32)s_pos - (u32)SHM_CMDLINE_BASE);

    /* add value */
    p = value;
    while(*p)
        *s_pos++ = *p++;

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

void aic_cmdline_flush(void)
{
#ifndef WIN32
    dcache_clean_by_range(SHM_CMDLINE_BASE, SHM_CMDLINE_SIZE);
#endif
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
            aicx_info(0x0c66dc, "ds", "parse cmdline: key[%d]=%s\n", index, key);
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
    s_pos = p;
}

char *aic_cmdline_get_value(cl_key_t key)
{
    return s_cmd_value[key];
}

void aic_cmdline_add_key_int(cl_key_t key, int value)
{
    char str[21];

    snprintf(str, sizeof(str), "%d", value);
    aic_cmdline_add_key(key, str);
}

int aic_cmdline_get_value_int(cl_key_t key)
{
    if (!s_cmd_value[key])
        return 0;

    return atoi(s_cmd_value[key]);
}

