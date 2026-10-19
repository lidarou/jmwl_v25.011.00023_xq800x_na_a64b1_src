#ifdef CONFIG_FP
#include <aic_osa_ex.h>
#include <power_supply.h>
#endif

#include <aic_errno.h>
#include "app_log.h"
#include <hlzma.h>

#include "cache.h"
#include "lvgl.h"
#include "display.h"
#include "display_cfg.h"
#include "bootanimation.h"
#ifdef CONFIG_FP
#include "app_msg.h"
#include "backlight.h"
#include "obj_batch_deal.h"

#define __inf APP_LOGI
#endif

char logo_charge[] __attribute__((aligned(32U)))= {
    #include "charge_logo.h"
};

#ifdef CONFIG_FP
lv_obj_t * charge_screen;
lv_obj_t * img_gif_charge;
lv_obj_t * label_charge;
extern int srv_tmr_init(void);
typedef void (*status_bat_func_callback)(void *user_data);

typedef struct {
u8 bat_capacity;            /*current battery level*/
u8 bat_chg_status;          /*current battery charging state*/
status_bat_func_callback fun_callback;
void *user_data;
}status_bat_fp_t;

static int32_t handle_only_charge_callback(void *p_param, uint32_t size)
{
    struct psy_info *p_charge_info = NULL;

    if(NULL == p_param) {
        __inf("[%s], p_param is null.", __func__);
        return -1;
    }

    p_charge_info = (struct psy_info *)p_param;
    if(p_charge_info->bat_chg_status == POWER_SUPPLY_STATUS_FULL) //POWER_SUPPLY_STATUS_DISCHARGING
    {
        lv_obj_add_flag(img_gif_charge, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(label_charge, "已充满");
        backlight_turn_on();
    }

    //__inf("[%s],:%d,cha:%d,type:%d", __func__,p_charge_info->bat_capacity,p_charge_info->bat_chg_status,p_charge_info->bat_chg_type);
    return 0;
}

__APP_CALLBACK_FUNCTION__(handle_only_charge_callback);


static void handle_only_charge_info(const struct psy_info *p_psy_info)
{
    int32_t ret = 0;
    uint32_t param_size = sizeof(struct psy_info);
    if(NULL == p_psy_info) {
        __inf("%s,psy_info is null",__func__);
        return;
    }
    ret = __APP_CALLBACK_NAME__(handle_only_charge_callback((void *)p_psy_info, param_size));
}

void charge_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t * target = lv_event_get_target(e);
    //__inf("charge_ui_event_actions 00 =%d\r\n",event_code);

    switch(event_code)
    {
        case LV_EVENT_SCREEN_LOADED:
        {
            lv_obj_t *obj = e->user_data;
            if(obj){
                lv_group_add_obj(lv_group_get_default(), obj);
                lv_group_focus_obj(obj);			
            }

        }
        break;

        case LV_EVENT_SCREEN_UNLOADED:
        {
            lv_obj_t *obj = e->user_data;
            if(obj){
                lv_group_remove_obj(obj);
            }
        }
        break;

        case LV_EVENT_KEY:
        {
        }
        break;

        default:
            break;
    }


}

void ui_FpCharge_screen_init(void *p_param)
{
    int ret = 0;
    srv_tmr_init();
    charge_screen = lv_scr_act();
    lv_obj_set_style_bg_color(charge_screen, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    img_gif_charge = lv_gif_create(charge_screen);
    lv_gif_set_src(img_gif_charge, &charging);
    lv_obj_align(img_gif_charge, LV_ALIGN_TOP_RIGHT, -10, 10);
    label_charge = lv_label_create(charge_screen);
    lv_label_set_text(label_charge, "充电中...");
    lv_obj_add_style(label_charge, (lv_style_t *)get_lv_style_font_common(), 0);
    lv_obj_align(label_charge, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(charge_screen, charge_ui_event_actions, LV_EVENT_ALL, charge_screen);

    ret = psy_changed_notifier_register("charge", handle_only_charge_info);

}
#endif

int show_boot_animation(void)
{
#ifdef CONFIG_BOOT_ANIMATION
#if LV_USE_GIF // TODO: temp use this Macro
    /* If you want to play a GIF, need LV_USE_GIF is on */
    /* In menu: (Top) -> Components Options -> Graphics Options -> LVGL configuration -> 3rd Party Libraries */
    lv_obj_t *imgLogo = lv_gif_create(lv_scr_act());
    lv_gif_set_src(imgLogo, &power_on);
    lv_obj_align(imgLogo, LV_ALIGN_CENTER, 0, 0);
#endif
#else
    lv_obj_t *imgLogo = lv_img_create(lv_scr_act());
#ifdef CONFIG_FP
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_img_set_src(imgLogo, &ui_img_logo_png);
#else
    lv_img_set_src(imgLogo, &power_on_picture);
#endif
    lv_obj_align(imgLogo, LV_ALIGN_CENTER, 0, 0);
#endif
    return AIC_SUCCESS;
}

int show_charge_animation(void)
{
#if LV_USE_GIF // TODO: temp use this Macro
    /* If you want to play a GIF, need LV_USE_GIF is on */
    /* In menu: (Top) -> Components Options -> Graphics Options -> LVGL configuration -> 3rd Party Libraries */
#ifndef CONFIG_FP
    lv_obj_t *imgLogo = lv_gif_create(lv_scr_act());
    lv_gif_set_src(imgLogo, &charge);
    lv_obj_align(imgLogo, LV_ALIGN_CENTER, 0, 0);
#else
    ui_FpCharge_screen_init(NULL);
#endif
#else
    /* Show a picture of PNG */
    extern const lv_img_dsc_t ui_img_consol_png;

    lv_obj_t *imgLogo = lv_img_create(lv_scr_act());
    lv_img_set_src(imgLogo, &ui_img_consol_png);
    lv_obj_align(imgLogo, LV_ALIGN_CENTER, 0, 0);
#endif
    return AIC_SUCCESS;
}

int show_charge_picture(void)
{
    int ret = -1;
    char *logo_buf = NULL;

    logo_buf = aic_zalloc( LCD_PANEL_WIDTH * LCD_PANEL_HEIGHT * 2 );//240*320*2 RGB565
    if (!logo_buf) {
        APP_LOGW("allocate charge logo buf failed\n");
        return AIC_ENOMEM;
    }
    dcache_clean_by_range((u32)logo_buf, LCD_PANEL_WIDTH * LCD_PANEL_HEIGHT * 2);

    ret = hlzma_dec_polling((u32)logo_charge, (u32)logo_buf);
    if (AIC_SUCCESS != ret ) {
        APP_LOGW("decompress logo failed, %d\n", ret);
        aic_free(logo_buf);
        return ret;
    }

    display_refresh((u32)logo_buf);

    return AIC_SUCCESS;
}

