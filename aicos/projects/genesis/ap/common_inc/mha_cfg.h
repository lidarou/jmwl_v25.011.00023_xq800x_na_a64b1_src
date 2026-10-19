#ifndef __MHA_CFG_H__
#define __MHA_CFG_H__

#ifdef CONFIG_MHA_PIN
#include <pin_cfg.h>
#endif

struct mha_cfg mha_cfg[] = {
/* 0 */
    {
        .board_id = "0",

#ifdef CONFIG_MHA_PIN
        .dig_pin = {
            .cfg = NULL,
        },
        .pmu_pin = {
            .cfg = NULL,
        },
#endif

#ifdef CONFIG_MHA_LCD_CFG
        .lcd = {
            .name = NULL,
        },
#endif

#ifdef CONFIG_MHA_CAMERA
        .camera = {
            .name = NULL,
        },
#endif

#ifdef CONFIG_MHA_SOUND
        .sound = {
            .codec_name = "ES8311", /* NULL,ES8311,CJC8910,CJC8911 */

            .codec_core_pwr_name = "DVDD3V", /* NULL,DVDD3V */
            .codec_core_pwr_volt = 3300000,

            .codec_io_pwr_name = NULL, /* NULL,DVDD3V */
            .codec_io_pwr_volt = 3300000,

            .codec_micbias_name = "VDDMIC", /* NULL,VDDMIC,DVDD3V */
            .codec_micbias_volt = 1800000,

            .pa_name = "AW87390", /* NULL,GPIO_TYPE,AW87390,AW88261,HAA9210 */
            .pa_gpio = 0xFFFFFFFF, /* if no, must set to 0xFFFFFFFF */

            .reserve = ES8311_VOLT_LOW_DOMAIN,
        },
#endif
    },

/* 1 */
    {
    },
};

#endif
