
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x12cd87
#define LOG_MODULE    AMOD_FOTA
#include <aic_init.h>

#ifdef CONFIG_ATFOTA
extern int atfota_init(void);
#ifndef CONFIG_FOTA_CMIOT_CMD
extern int atfota_callback_init(void);
#endif
#endif
#ifdef CONFIG_FOTA_CMIOT_CMD
extern int atfota_cmiot_init(void);
#endif
extern int fota_init(void);
extern int fota_check_update(void);

static int fota_early_init(void)
{
#ifdef CONFIG_ATFOTA
    atfota_init();
#ifndef CONFIG_FOTA_CMIOT_CMD
    if (aic_boot_mode_is_basic())
        atfota_callback_init();
#endif
#endif
#ifdef CONFIG_FOTA_CMIOT_CMD
    atfota_cmiot_init();
#endif
    if (aic_boot_mode_is_basic())
        fota_init();
    fota_check_update();

    return 0;
}
DIS_CHARGE_COMP_P2_INIT_EXPORT(fota_early_init);
