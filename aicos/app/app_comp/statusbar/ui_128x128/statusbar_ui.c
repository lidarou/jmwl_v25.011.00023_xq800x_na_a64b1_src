/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is ui of statusbar
 */
/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "comp_statusbar/ui.h"
#include "statusbar_ui.h"
#include "statusbar_flow.h"

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void statusbar_fresh_charge(uint8_t bat_capacity, uint8_t charge_status);
static void statusbar_fresh_card(bool sim_exist);
static void statusbar_fresh_signal_strength(uint32_t signal_level);
static void statusbar_fresh_opera_name(char *name);
static void statusbar_fresh_ims_status(bool ims_status);
static void statusbar_fresh_time(char *p_str_time);

static const statusbar_flow_t g_statubar_fresh_items = {
    statusbar_fresh_charge,
    statusbar_fresh_card,
    statusbar_fresh_signal_strength,
    statusbar_fresh_opera_name,
    statusbar_fresh_ims_status,
    statusbar_fresh_time,
};

static void statusbar_fresh_charge(uint8_t bat_capacity, uint8_t charge_status)
{
    LV_LOG_INFO("%s,charge_status = %d,charge_capacity =%d",__func__,charge_status,bat_capacity);

    /*bat_capacity:0-20,level 0;20-40,level 1;40-60,level 2;60-80,level 3;80-100,level 4*/
    /* fresh battery img */
    if((true == lv_obj_is_valid(ui_PnlStatusBar))&&(false == lv_obj_has_flag(ui_ImgStatusBarBattery, LV_OBJ_FLAG_HIDDEN))) {
        if(20 >= bat_capacity) {
            lv_img_set_src(ui_ImgStatusBarBattery,&ui_img_battery0_png);
        } else if(20 <bat_capacity && bat_capacity <= 40) {
            lv_img_set_src(ui_ImgStatusBarBattery,&ui_img_battery25_png);
        } else if(40 < bat_capacity && bat_capacity <= 60) {
            lv_img_set_src(ui_ImgStatusBarBattery,&ui_img_battery50_png);
        } else if(60 < bat_capacity && bat_capacity <= 80) {
            lv_img_set_src(ui_ImgStatusBarBattery,&ui_img_battery75_png);
        } else {
            lv_img_set_src(ui_ImgStatusBarBattery,&ui_img_battery100_png);
        }
        /* fresh charging img POWER_SUPPLY_STATUS_CHARGING:1;POWER_SUPPLY_STATUS_FULL:4 */
        if(1 == charge_status || 4 == charge_status) {
            lv_obj_clear_flag(ui_ImgStatusBarCharge, LV_OBJ_FLAG_HIDDEN);
        }else {
            lv_obj_add_flag(ui_ImgStatusBarCharge, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void statusbar_fresh_card(bool sim_exist)
{
    if((true == lv_obj_is_valid(ui_PnlStatusBar))&&(false == lv_obj_has_flag(ui_ImgStatusBarCard, LV_OBJ_FLAG_HIDDEN))) {
        if (false == sim_exist) {
            /* control simcard position is same with signal,if simcard exist,display signal,if simcard is not exist,display simcard is not present */
            lv_obj_clear_flag(ui_ImgStatusBarCard, LV_OBJ_FLAG_HIDDEN);
            lv_img_set_src(ui_ImgStatusBarCard, &ui_img_stb_simcard_n_png);
            lv_obj_add_flag(ui_ImgStatusBarSignal, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(ui_ImgStatusBarSignal, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(ui_ImgStatusBarCard, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void statusbar_fresh_signal_strength(uint32_t signal_level)
{
    LV_LOG_INFO("%s,signal_level =%d", __func__, signal_level);

    if(4 < signal_level) {
        LV_LOG_ERROR("%s,signal_level greater 4", __func__);
        return;
    }

    if((true == lv_obj_is_valid(ui_PnlStatusBar))&&(false == lv_obj_has_flag(ui_ImgStatusBarSignal, LV_OBJ_FLAG_HIDDEN))) {
        switch(signal_level){
        case 0: {
            if(-56 == lv_obj_get_x_aligned(ui_ImgStatusBarSignal)) {
                lv_img_set_src(ui_ImgStatusBarSignal, &ui_img_signal0_png);
            } else {
                lv_img_set_src(ui_ImgStatusBarSignal, &ui_img_signal00_0_png);
            }
            break;
        }
        case 1:{
            if(-56 == lv_obj_get_x_aligned(ui_ImgStatusBarSignal)) {
                lv_img_set_src(ui_ImgStatusBarSignal, &ui_img_signal1_png);
            } else {
                lv_img_set_src(ui_ImgStatusBarSignal, &ui_img_signal00_1_png);
            }
            break;
        }
        case 2:{
            if(-56 == lv_obj_get_x_aligned(ui_ImgStatusBarSignal)) {
                lv_img_set_src(ui_ImgStatusBarSignal, &ui_img_signal2_png);
            } else {
                lv_img_set_src(ui_ImgStatusBarSignal, &ui_img_signal00_2_png);
            }
            break;
        }
        case 3:{
            if(-56 == lv_obj_get_x_aligned(ui_ImgStatusBarSignal)) {
                lv_img_set_src(ui_ImgStatusBarSignal, &ui_img_signal3_png);
            } else {
                lv_img_set_src(ui_ImgStatusBarSignal, &ui_img_signal00_3_png);
            }
            break;
        }
        case 4:{
            if(-56 == lv_obj_get_x_aligned(ui_ImgStatusBarSignal)) {
                lv_img_set_src(ui_ImgStatusBarSignal, &ui_img_signal4_png);
            } else {
                lv_img_set_src(ui_ImgStatusBarSignal, &ui_img_signal00_4_png);
            }
            break;
        }
        default:{
            if(-56 == lv_obj_get_x_aligned(ui_ImgStatusBarSignal)) {
                lv_img_set_src(ui_ImgStatusBarSignal, &ui_img_signal0_png);
            }
            else {
               lv_img_set_src(ui_ImgStatusBarSignal, &ui_img_signal00_4_png);
           }
           break;
           }
        }
    }
}

static void statusbar_fresh_opera_name(char *name)
{
    if((true == lv_obj_is_valid(ui_PnlStatusBar))&&(false == lv_obj_has_flag(ui_LblStatusBarOperator, LV_OBJ_FLAG_HIDDEN))) {
        lv_label_set_text(ui_LblStatusBarOperator, name);
    }
}

static void statusbar_fresh_ims_status(bool ims_status)
{
    if((true == lv_obj_is_valid(ui_PnlStatusBar))&&(false == lv_obj_has_flag(ui_ImgStatusBarIms, LV_OBJ_FLAG_HIDDEN))) {
        if(true == ims_status) {
            /* ims open */
            lv_img_set_src(ui_ImgStatusBarIms, &ui_img_stb_imssuc_png);
        } else {
            /* ims close */
            lv_img_set_src(ui_ImgStatusBarIms, &ui_img_stb_imsfail_png);
        }
    }
}

static void statusbar_fresh_time(char *p_str_time)
{
    if((true == lv_obj_is_valid(ui_PnlStatusBar))&&(false == lv_obj_has_flag(ui_LblStatusBarTime, LV_OBJ_FLAG_HIDDEN))) {
        lv_label_set_text(ui_LblStatusBarTime, p_str_time);
    }
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void statusbar_ui_init(statusbar_type_e type, char *win_name)
{
     lv_obj_t *p_cur_scr = lv_scr_act();

     /* Del old statusbar and Initnal new statusbar if ui_PnlStatusBar is deleted*/
     if(lv_obj_is_valid(ui_PnlStatusBar) == false) {
         ui_statusbar_init();
         LV_LOG_INFO("[%s] Create statusbar!!!", __func__);
     }

     lv_obj_set_parent(ui_PnlStatusBar, p_cur_scr);

    switch(type) {
    case STATUSBAR_TYPE_ALL:
        lv_obj_clear_flag(ui_LblStatusBarOperator, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_ImgStatusBarIms, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_ImgStatusBarSignal, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_ImgStatusBarCard, LV_OBJ_FLAG_HIDDEN);

        lv_obj_set_x(ui_ImgStatusBarSignal, -56);
        lv_obj_set_y(ui_ImgStatusBarSignal, 0);
        lv_obj_set_x(ui_ImgStatusBarCard, -56);
        lv_obj_set_y(ui_ImgStatusBarCard, 0);

        lv_obj_add_flag(ui_ImgStatusBarData, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_LblStatusBarWinTitle, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_LblStatusBarTime, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_ImgbtnStatusBar, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_bg_opa(ui_PnlStatusBar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_img_recolor(ui_ImgStatusBarBattery, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_img_recolor_opa(ui_ImgStatusBarBattery, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        break;
    case STATUSBAR_TYPE_MAIN_WIN:
        lv_label_set_text(ui_LblStatusBarWinTitle, win_name);
        lv_obj_add_flag(ui_LblStatusBarOperator, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_ImgStatusBarIms, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_x(ui_ImgStatusBarSignal, 40);
        lv_obj_set_y(ui_ImgStatusBarSignal, 0);
        lv_obj_set_x(ui_ImgStatusBarCard, 40);
        lv_obj_set_y(ui_ImgStatusBarCard, 0);
        lv_obj_add_flag(ui_ImgStatusBarData, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_LblStatusBarWinTitle, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_LblStatusBarTime, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_ImgbtnStatusBar, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_bg_opa(ui_PnlStatusBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_img_recolor(ui_ImgStatusBarBattery, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_img_recolor_opa(ui_ImgStatusBarBattery, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        break;
    case STATUSBAR_TYPE_SUB_WIN:
        lv_label_set_text(ui_LblStatusBarWinTitle, win_name);
        lv_obj_align_to(ui_LblStatusBarWinTitle, ui_ImgbtnStatusBar, LV_ALIGN_LEFT_MID, 14, 0);
        lv_obj_add_flag(ui_LblStatusBarOperator, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_ImgStatusBarIms, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_ImgStatusBarData, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_LblStatusBarWinTitle, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_LblStatusBarTime, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_ImgbtnStatusBar, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_x(ui_ImgStatusBarSignal, 40);
        lv_obj_set_y(ui_ImgStatusBarSignal, 0);
        lv_obj_set_x(ui_ImgStatusBarCard, 40);
        lv_obj_set_y(ui_ImgStatusBarCard, 0);
        lv_obj_set_style_bg_opa(ui_PnlStatusBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_img_recolor(ui_ImgStatusBarBattery, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_img_recolor_opa(ui_ImgStatusBarBattery, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        break;
    default:
        LV_LOG_ERROR("[%s] the type is error!!!", __func__);
        break;
    }

    statusbar_flow_register_ui_actions(&g_statubar_fresh_items);
}
