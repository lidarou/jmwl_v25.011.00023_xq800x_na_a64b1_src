/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * Multi hardware auto adapt header file.
 *
 */

#ifndef __MAH_H__
#define __MAH_H__

struct mha_pin {
    void *cfg;
};

struct mha_lcd {
    const char *name;
};

struct mha_camera {
    const char *name;
};

#define ES8311_VOLT_LOW_DOMAIN     0x1

struct mha_sound {
    const char *codec_name; /* NULL,ES8311,CJC8910,CJC8911 */

    const char *codec_core_pwr_name; /* NULL,DVDD3V */
    u32 codec_core_pwr_volt;

    const char *codec_io_pwr_name; /* NULL,DVDD3V */
    u32 codec_io_pwr_volt;

    const char *codec_micbias_name; /* NULL,VDDMIC,DVDD3V */
    u32 codec_micbias_volt; /* uV */

    const char *pa_name; /* NULL,GPIO_TYPE,AW87390,AW88261,HAA9210 */
    u32 pa_gpio; /* if no, must set to 0xFFFFFFFF */

    u32 reserve; /* for special use */
};

struct mha_cfg {
    char *board_id;

#ifdef CONFIG_MHA_PIN
    struct mha_pin dig_pin;
    struct mha_pin pmu_pin;
#endif

#ifdef CONFIG_MHA_LCD_CFG
    struct mha_lcd lcd;
#endif

#ifdef CONFIG_MHA_CAMERA
    struct mha_camera camera;
#endif

#ifdef CONFIG_MHA_SOUND
    struct mha_sound sound;
#endif

};

void *mha_get_dig_pin_cfg(void);
void *mha_get_pmu_pin_cfg(void);
void *mha_get_lcd_cfg(void);
void *mha_get_camera_cfg(void);
void *mha_get_sound_cfg(void);

#endif

