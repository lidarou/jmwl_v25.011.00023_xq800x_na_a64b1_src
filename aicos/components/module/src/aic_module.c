/******************************************************************************/
/*                                                                             */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                             */
 /******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file define all mudule in aic software.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0932d9
#define LOG_MODULE    AMOD_SYS

#include <aic_assert.h>
#include <aic_module.h>
#include <alog_api.h>

struct mud_data {
    u8      mud_id;
    u8      def_loglv;
    char    *name;
};

static struct mud_data mud_table[AMOD_CNT] = {
    /* componets module */
    {AMOD_AOS,      ALOG_LV_INFO,   "aos"},
    {AMOD_SYS,      ALOG_LV_INFO,   "sys"},
    {AMOD_INIT,     ALOG_LV_INFO,   "init"},
    {AMOD_CONS,     ALOG_LV_INFO,   "cons"},
    {AMOD_AIPC,     ALOG_LV_INFO,   "aipc"},
    {AMOD_ALOG,     ALOG_LV_INFO,   "alog"},
    {AMOD_DIAG,     ALOG_LV_INFO,   "diag"},
    {AMOD_SUBSYS,   ALOG_LV_INFO,   "subsys"},
    {AMOD_NVRAM,    ALOG_LV_INFO,   "nvram"},
    {AMOD_MISC,     ALOG_LV_INFO,   "misc"},
    {AMOD_FS,       ALOG_LV_INFO,   "fs"},
    {AMOD_MCX,      ALOG_LV_INFO,   "mcx"},
    {AMOD_LVGL,     ALOG_LV_INFO,   "lvgl"},
    {AMOD_WIFI,     ALOG_LV_INFO,   "wifi"},
    {AMOD_BLT,      ALOG_LV_INFO,   "blt"},
    {AMOD_PRODINFO, ALOG_LV_INFO,   "prodinfo"},

    /* mm module */
    {AMOD_AUDIO,    ALOG_LV_INFO,   "audio"},
    {AMOD_CAMERA,   ALOG_LV_INFO,   "camera"},

    /* chip dirver module */
    {AMOD_DRV,      ALOG_LV_INFO,   "drv"},
    {AMOD_AMBOX,    ALOG_LV_INFO,   "ambox"},
    {AMOD_BUSMON,   ALOG_LV_INFO,   "busmonitor"},
    {AMOD_USB,      ALOG_LV_INFO,   "usb"},
    {AMOD_FLASH,    ALOG_LV_INFO,   "flash"},
    {AMOD_SOUND,    ALOG_LV_INFO,   "snd"},
    {AMOD_I2C,      ALOG_LV_INFO,   "i2c"},
    {AMOD_TIMER,    ALOG_LV_INFO,   "timer"},
    {AMOD_DMA,      ALOG_LV_INFO,   "dma"},
    {AMOD_GPD,      ALOG_LV_INFO,   "gpd"},
    {AMOD_GPIO,     ALOG_LV_INFO,   "gpio"},
    {AMOD_SoCMPU,   ALOG_LV_INFO,   "SoC MPU"},
    {AMOD_PCDUMP,   ALOG_LV_INFO,   "pcdump"},
    {AMOD_PERFMON,  ALOG_LV_INFO,   "perf monitor"},
    {AMOD_PMIC_I2C, ALOG_LV_INFO,   "pmi2c"},
    {AMOD_SLP,      ALOG_LV_INFO,   "slp"},
    {AMOD_EIPU,     ALOG_LV_INFO,   "eipu"},
    {AMOD_ZJTAG,    ALOG_LV_INFO,   "zjtag"},
    {AMOD_KPC,      ALOG_LV_INFO,   "kpc"},
    {AMOD_TSC,      ALOG_LV_INFO,   "tsc"},
    {AMOD_DBG,      ALOG_LV_INFO,   "debug"},
    {AMOD_SDMMC,    ALOG_LV_INFO,   "sdmmc"},
    {AMOD_RTC,      ALOG_LV_INFO,   "rtc"},
    {AMOD_CLK_CALI, ALOG_LV_INFO,   "clock cali"},
    {AMOD_SSP,      ALOG_LV_INFO,   "ssp"},
    {AMOD_CMUX,     ALOG_LV_ERROR,  "cmux"},
    {AMOD_CAN,      ALOG_LV_INFO,   "can"},

    /* modem module */
    {AMOD_MODEM,    ALOG_LV_INFO,   "modem"},
    {AMOD_AT,       ALOG_LV_INFO,   "at"},
    {AMOD_IMS,      ALOG_LV_INFO,   "ims"},
    {AMOD_SIM,      ALOG_LV_INFO,   "sim"},
    {AMOD_TEL,      ALOG_LV_INFO,   "tel"},
    {AMOD_PB,       ALOG_LV_INFO,   "pb"},
    {AMOD_IPCC,     ALOG_LV_INFO,   "ipcc"},
    {AMOD_ATM,      ALOG_LV_INFO,   "atm"},
    {AMOD_PDCP,     ALOG_LV_INFO,   "pdcp"},
    {AMOD_RLC,      ALOG_LV_INFO,   "rlc"},
    {AMOD_MAC,      ALOG_LV_INFO,   "mac"},
    {AMOD_HC,     ALOG_LV_INFO,     "rohc"},

    /* net module */
    {AMOD_NET,      ALOG_LV_INFO,   "net"},

    /* secure boot*/
    {AMOD_SECBOOT,  ALOG_LV_INFO,   "secboot"},

    /* app module */
    {AMOD_APP,      ALOG_LV_INFO,   "app"},
    {AMOD_FOTA,     ALOG_LV_INFO,   "fota"},
    {AMOD_GNSS,     ALOG_LV_INFO,   "gnss"},

    /* phy module */
    {AMOD_PHY,      ALOG_LV_INFO,   "phy"},

    /* EL2 module */
    {AMOD_EL2,      ALOG_LV_INFO,   "el2"},

    /* new add module */
    {AMOD_VENDOR,   ALOG_LV_INFO,   "vendor"},
    {AMOD_I2S,      ALOG_LV_INFO,   "i2s"},
    {AMOD_PMIC,     ALOG_LV_INFO,   "pmic"},
    {AMOD_GPAPH,    ALOG_LV_INFO,   "gpaph"},

    {AMOD_AS,       ALOG_LV_INFO,   "as"},
    {AMOD_ERRC,     ALOG_LV_INFO,   "errc"},
    {AMOD_L2,       ALOG_LV_INFO,   "l2"},
    {AMOD_MMA,      ALOG_LV_INFO,   "mma"},

    {AMOD_NAS,      ALOG_LV_INFO,   "nas"},
    {AMOD_UAI,      ALOG_LV_INFO,   "uai"},
    {AMOD_ACCDD,    ALOG_LV_INFO,   "accdd"},
    {AMOD_RFDD,     ALOG_LV_INFO,   "rfdd"},

    {AMOD_D2D,      ALOG_LV_INFO,   "d2d"},
    {AMOD_L1C,      ALOG_LV_INFO,   "l1c"},

};

#ifdef CONFIG_MOD_PATH_CFG
struct mud_path {
    u8      mud_id;
    char    *path;
};

static struct mud_path path_table[] = {
    {AMOD_AOS,      "os/osa"},
    {AMOD_SYS,      "projects"},
    {AMOD_APP,      "app"},
    {AMOD_VENDOR,   "vendors"},

    /* componets module */
    {AMOD_SYS,      "components"},
    {AMOD_INIT,     "components/init"},
    {AMOD_CONS,     "components/console"},
    {AMOD_AIPC,     "components/aipc"},
    {AMOD_ALOG,     "components/alog"},
    {AMOD_ALOG,     "components/almsg"},
    {AMOD_DIAG,     "components/diag"},
    {AMOD_SUBSYS,   "components/subsys_ctrl"},
    {AMOD_NVRAM,    "components/nvram"},
    {AMOD_MISC,     "components/misc"},
    {AMOD_FS,       "components/fs"},
    {AMOD_MCX,      "components/mcx"},
    {AMOD_LVGL,     "components/graphics"},
    {AMOD_WIFI,     "components/wifi"},
    {AMOD_BLT,      "components/blt"},
    {AMOD_PRODINFO, "components/prodinfo"},
    {AMOD_AT,       "components/at"},
    {AMOD_FOTA,     "components/fota"},
    {AMOD_GNSS,     "components/gps"},
    {AMOD_SECBOOT,  "components/secure"},
    {AMOD_NET,      "components/network"},
    {AMOD_D2D,      "components/d2d"},
    {AMOD_D2D,      "components/d2d_app"},
    {AMOD_D2D,      "components/d2d_l3_dummy"},
    {AMOD_D2D,      "components/d2d_mgr"},
    {AMOD_USB,      "components/usb"},

    /* mm module */
    {AMOD_AUDIO,    "mm"},
    {AMOD_AUDIO,    "mm/audio"},
    {AMOD_AUDIO,    "mm/codec"},
    {AMOD_CAMERA,   "mm/camera"},

    /* chip dirver module */
    {AMOD_DRV,      "drivers"},
    {AMOD_AMBOX,    "drivers/ambox"},
    {AMOD_BUSMON,   "drivers/busmonitor"},
    {AMOD_USB,      "drivers/usb"},
    {AMOD_FLASH,    "drivers/qspi"},
    {AMOD_FLASH,    "drivers/fmd"},
    {AMOD_SOUND,    "drivers/sound"},
    {AMOD_I2S,      "drivers/iis"},
    {AMOD_I2C,      "drivers/i2c"},
    {AMOD_TIMER,    "drivers/timer"},
    {AMOD_DMA,      "drivers/dma"},
    {AMOD_GPD,      "drivers/gpd"},
    {AMOD_GPIO,     "drivers/gpio"},
    {AMOD_SoCMPU,   "drivers/dmc_mpu"},
    {AMOD_PCDUMP,   "drivers/pcdump"},
    {AMOD_PERFMON,  "drivers/perf"},
    {AMOD_PMIC_I2C, "drivers/pmic_i2c"},
    {AMOD_PMIC,     "drivers/pmic"},
    {AMOD_TSC,      "drivers/pmic/tsc"},
    {AMOD_RTC,      "drivers/pmic/rtc"},
    {AMOD_CLK_CALI, "drivers/pmic/clk_cali"},
    {AMOD_SLP,      "drivers/sleep"},
    {AMOD_EIPU,     "drivers/eipu"},
    {AMOD_ZJTAG,    "drivers/zjtag"},
    {AMOD_KPC,      "drivers/kpc"},
    {AMOD_DBG,      "drivers/debug"},
    {AMOD_SDMMC,    "drivers/sdmmc"},
    {AMOD_GPAPH,    "drivers/graphics"},
    {AMOD_SSP,      "drivers/ssp"},
    {AMOD_CMUX,     "drivers/cmux"},
    {AMOD_CAN,      "drivers/can"},
    {AMOD_FOTA,     "drivers/bspatch"},

    /* modem module */
    {AMOD_MODEM,    "comm"},
    {AMOD_MODEM,    "comm/ps"},
    {AMOD_IMS,      "comm/ims"},
    {AMOD_SIM,      "comm/ps/code/sim"},
    {AMOD_TEL,      "comm/ps/code/telephony"},
    {AMOD_PB,       "comm/ps/code/phonebook"},
    {AMOD_IPCC,     "comm/ps/code/ipcc"},
    {AMOD_ATM,      "comm/ps/code/atm"},
    {AMOD_EL2,      "comm/ps/code/el2"},
    {AMOD_PDCP,     "comm/ps/code/el2/epdcp"},
    {AMOD_RLC,      "comm/ps/code/el2/erlc"},
    {AMOD_MAC,      "comm/ps/code/el2/emac"},
    {AMOD_HC,       "comm/ps/code/hc"},
    {AMOD_AS,       "comm/ps/code/as"},
    {AMOD_AS,       "comm/ps/code/asmc"},
    {AMOD_AS,       "comm/ps/code/asn1"},
    {AMOD_AS,       "comm/ps/code/asn1c"},
    {AMOD_ERRC,     "comm/ps/code/errc"},
    {AMOD_L2,       "comm/ps/code/l2"},
    {AMOD_MMA,      "comm/ps/code/mma"},
    {AMOD_NAS,      "comm/ps/code/nas"},
    {AMOD_UAI,      "comm/ps/code/uai"},

    /* phy module */
    {AMOD_PHY,      "comm/phy"},
    {AMOD_ACCDD,    "comm/phy/accdd"},
    {AMOD_RFDD,     "comm/phy/rfdd"},
    {AMOD_D2D,      "comm/phy/d2d"},
    {AMOD_L1C,      "comm/phy/l1c"},
};
#endif

char *aic_get_moudule_name(u8 module)
{
    u16 i;
    struct mud_data *item;

    for (i = 0; i < AMOD_CNT; i++) {
        item = mud_table + i;
        if (item->mud_id == module )
            return item->name;
    }

    AIC_ASSERT(0);
    return NULL;
}

u8 aic_get_moudule(char *name)
{
    u8 i;
    struct mud_data *item;

    for (i = 0; i < AMOD_CNT; i++) {
        item = mud_table + i;
        if (0 == strcmp(item->name, name))
            return i;
    }

    return AMOD_CNT;
}

u8 aic_get_moudule_def_loglv(u8 module)
{
    u16 i;
    struct mud_data *item;

    for (i = 0; i < AMOD_CNT; i++) {
        item = mud_table + i;
        if (item->mud_id == module )
            return item->def_loglv;
    }

    return ALOG_LV_INFO;
}
