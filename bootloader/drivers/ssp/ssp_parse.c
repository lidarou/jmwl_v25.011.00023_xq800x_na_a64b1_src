/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *
 */

#include <aic_errno.h>
#include <ssp.h>
#include <stdlib.h>

#include "console/console.h"
#include "download/dload_cmd.h"

enum cs_type {
    CS_NORMAL = 0,
    CS_GPIO,
    CS_MAX,
};

#define SSP_BUS_NAME_MAX (20)

struct ssp_info {
    char bus_name[SSP_BUS_NAME_MAX];
    enum cs_type cs_type;
    u32 cs;
    u32 work_mode;
    u32 frequence;
    boolean valid;
};

static struct ssp_info s_ssp_info = {0};

char *ssp_parse_get_bus_name(void)
{
    ASSERT(s_ssp_info.valid);

    return s_ssp_info.bus_name;
}

u8 ssp_parse_get_cs(void)
{
    ASSERT(s_ssp_info.valid);

    if (s_ssp_info.cs_type == CS_NORMAL)
        return (u8)(s_ssp_info.cs & 0xFF);
    else
        return SSP_NO_CS;
}

GPIO_PIN_ID ssp_parse_get_cs_gpio(void)
{
    ASSERT(s_ssp_info.valid);

    if (s_ssp_info.cs_type == CS_GPIO)
        return (GPIO_PIN_ID)(s_ssp_info.cs);
    else
        return GPIO_PIN_MAX;
}

enum work_mode ssp_parse_get_work_mode(void)
{
    ASSERT(s_ssp_info.valid);

    return (enum work_mode)(s_ssp_info.work_mode);
}

u32 ssp_parse_get_frequence(void)
{
    return s_ssp_info.frequence;
}

#define SSP_INFO_BUS_NAME_OFS          (0x00)
#define SSP_INFO_CS_TYPE_OFS           (0x14)
#define SSP_INFO_CS_OFS                (0x28)
#define SSP_INFO_WORK_MODE_OFS         (0x2C)
#define SSP_INFO_FREQ_OFS              (0x30)

int ssp_parse_cfg_info(u8 *buf)
{
    struct ssp_info *info = &s_ssp_info;
    const char *bus_name;

    /* bus name */
    memcpy(info->bus_name, buf, sizeof(info->bus_name));

    /* cs type */
    bus_name = (const char *)&buf[SSP_INFO_CS_TYPE_OFS];
    if (strncmp(bus_name, "GPIO", 4) == 0)
        info->cs_type = CS_GPIO;
    else if (strncmp(bus_name, "NORMAL", 6) == 0)
        info->cs_type = CS_NORMAL;
    else
        return PCK_REP_OPERATION_FAILED;

    /* cs */
    memcpy(&(info->cs), &buf[SSP_INFO_CS_OFS], 4);

    /* work mode */
    memcpy(&(info->work_mode), &buf[SSP_INFO_WORK_MODE_OFS], 4);

    /* frequence */
    memcpy(&(info->frequence), &buf[SSP_INFO_FREQ_OFS], 4);

    info->valid = TRUE;

    return 0;
}


