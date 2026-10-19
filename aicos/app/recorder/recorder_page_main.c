/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implements the main page ui and flow.
 */
/*******************************************
 *              INCLUDES
 ******************************************/
#include "recorder_page_main.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef WIN32
#include "aic_osa_ex.h"
#endif

extern struct recorder_service *g_recorder_srv;

/*******************************************
 *              STATIC PROTOTYPES
 ******************************************/
static lv_obj_t *create_cont(lv_obj_t *parent);
static lv_obj_t *create_title_box(lv_obj_t *parent);
static lv_obj_t *create_voice_icon_obj(lv_obj_t *parent);
static lv_obj_t *create_ctrl_box(lv_obj_t *parent);
static lv_obj_t *voice_img_create(lv_obj_t *parent);

static void recorder_rec_click_event_cb(lv_event_t *e);
static void recorder_play_click_event_cb(lv_event_t *e);
static void recorder_stop_click_event_cb(lv_event_t *e);
static void recorder_key_event_cb(lv_event_t *e);
static void recorder_timer_cb(lv_timer_t *t);

/*******************************************
 *              STATIC VARIABLES
 ******************************************/
static lv_obj_t *time_label;
static lv_obj_t *rec_title_label;
static uint32_t sec_time;
static lv_timer_t *sec_counter_timer;
static lv_obj_t *recorder_obj;

/*******************************************
 *              STATIC FUNCTIONS
 ******************************************/
static void app_recorder_resume(void)
{
    struct recorder_service *recs = g_recorder_srv;
    static int recd_num = 1;
    char *url = NULL;
    struct tm t;
    time_t now;

    if (recs->state == RECD_IDLE) {
        // TODO: for playnow now
        if (recs->file.url) {
            aic_free(recs->file.url);
        }
        url = aic_zalloc(RECORDER_URL_MAX_LENGTH);
        if (url == NULL) {
            app_loge("recorder start recording failed (zalloc failed).\n");
            return;
        }

        time(&now);
        localtime_r(&now, &t);
        sprintf(url, "%s/Rec-%04d-%02d-%02d-%d%s", APP_RECORDER_FILE_DIR,
                t.tm_year + 1900, t.tm_mon, t.tm_mday, recd_num++,
                APP_RECORDER_FILE_FORMAT_EXT);

        recs->file.format = APP_RECORDER_DEFAULT_FORMAT;
        recs->file.url = url;

        if (app_srv_recording_start(recs)) {
            app_loge("recorder start recording failed (srv start failed)\n");
            aic_free(url);
            recs->file.url = NULL;
            return;
        }

        lv_label_set_text(rec_title_label, url);
    } else {
        app_srv_recording_resume(recs);
    }

    lv_timer_resume(sec_counter_timer);
    lv_obj_add_state(recorder_obj, LV_STATE_CHECKED);
    lv_imgbtn_set_state(recorder_obj, LV_IMGBTN_STATE_CHECKED_RELEASED);
}

#if 0
static void app_recorder_pause(void)
{
    struct recorder_service *recs = g_recorder_srv;

    lv_timer_pause(sec_counter_timer);
    lv_obj_clear_state(recorder_obj, LV_STATE_CHECKED);

    if (app_srv_recording_pause(recs)) {
        app_loge("recorder pause recording failed (pause failed)\n");
        app_srv_recording_stop(recs);
        recs->state = RECD_IDLE;
        return;
    }
    recs->state = RECD_PAUSE;
}
#endif

static void app_recorder_stop(void)
{
    struct recorder_service *recs = g_recorder_srv;

    if(recs==NULL)
        return ;
    if (recs->state == RECD_RECORD || recs->state == RECD_PAUSE) {
        lv_timer_pause(sec_counter_timer);
        lv_obj_clear_state(recorder_obj, LV_STATE_CHECKED);
        lv_imgbtn_set_state(recorder_obj, LV_IMGBTN_STATE_RELEASED);

        if (app_srv_recording_stop(recs)) {
            app_loge("recorder stop recording failed (pause failed)\n");
            return;
        }
        lv_label_set_text(time_label, "00:00:00");
    } else if (recs->state == RECD_PLAY) {
        app_srv_recording_playstop(recs);
    }

    sec_time = 0;
}

static void app_recorder_play(void)
{
    struct recorder_service *recs = g_recorder_srv;

    if (recs->file.url == NULL) {
        app_loge("No recording to play\n");
        return;
    }

    if (recs->state != RECD_IDLE) {
        app_loge("Try to play when NOT idle\n");
        return;
    }

    if (app_srv_recording_play(recs)) {
        app_loge("recorder stop recording failed (pause failed)\n");
        return;
    }
}

static lv_obj_t *create_cont(lv_obj_t *parent)
{
    lv_obj_t *recorder = lv_obj_create(parent);
    lv_obj_clear_flag(recorder, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(recorder, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(recorder, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_width(recorder, 0, 0);
    lv_obj_set_style_pad_all(recorder, 0, 0);

    return recorder;
}

static lv_obj_t *create_title_box(lv_obj_t *parent)
{
    const lv_font_t *font_large = &lv_font_montserrat_14;

    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_height(cont, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    time_label = lv_label_create(cont);
    lv_obj_set_style_text_font(time_label, font_large, 0);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0x504d6d), 0);
    lv_label_set_text(time_label, "00:00:00");
    lv_obj_set_height(time_label, lv_font_get_line_height(font_large) * 3 / 2);

    rec_title_label = lv_label_create(cont);
    lv_obj_set_style_text_font(rec_title_label, font_large, 0);
    lv_obj_set_style_text_color(rec_title_label, lv_color_hex(0x504d6d), 0);
    lv_label_set_text(rec_title_label, "Rec...");

    return cont;
}

static lv_obj_t *create_voice_icon_obj(lv_obj_t *parent)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);

    lv_obj_set_height(obj, 250);

    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    voice_img_create(obj);

    return obj;
}

static lv_obj_t *create_ctrl_box(lv_obj_t *parent)
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_height(cont, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_bottom(cont, 8, 0);

    static const lv_coord_t grid_col[] = {LV_GRID_FR(1), LV_GRID_FR(5),
                                          LV_GRID_FR(5), LV_GRID_FR(5),
                                          LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t grid_row[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(cont, grid_col, grid_row);

    LV_IMG_DECLARE(ui_img_play_png);
    LV_IMG_DECLARE(ui_img_stop_png);
    LV_IMG_DECLARE(ui_img_record_pause_png);
    LV_IMG_DECLARE(ui_img_record_png);

    lv_obj_t *playback_obj = lv_img_create(cont);
    lv_img_set_src(playback_obj, &ui_img_play_png);
    lv_obj_set_grid_cell(playback_obj, LV_GRID_ALIGN_CENTER, 1, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_event_cb(playback_obj, recorder_play_click_event_cb,
                        LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(playback_obj, LV_OBJ_FLAG_CLICKABLE);

    recorder_obj = lv_imgbtn_create(cont);
    lv_imgbtn_set_src(recorder_obj, LV_IMGBTN_STATE_RELEASED, NULL,
                      &ui_img_record_png, NULL);
    lv_imgbtn_set_src(recorder_obj, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL,
                      &ui_img_record_pause_png, NULL);
    lv_obj_add_flag(recorder_obj, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_grid_cell(recorder_obj, LV_GRID_ALIGN_CENTER, 2, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_add_event_cb(recorder_obj, recorder_rec_click_event_cb,
                        LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(recorder_obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_width(recorder_obj, ui_img_record_pause_png.header.w);

    lv_obj_t *stop_obj = lv_img_create(cont);
    lv_img_set_src(stop_obj, &ui_img_stop_png);
    lv_obj_set_grid_cell(stop_obj, LV_GRID_ALIGN_CENTER, 3, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_event_cb(stop_obj, recorder_stop_click_event_cb,
                        LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(stop_obj, LV_OBJ_FLAG_CLICKABLE);

    lv_group_add_obj(lv_group_get_default(), cont);
    lv_obj_add_event_cb(cont, recorder_key_event_cb, LV_EVENT_KEY, NULL);
    lv_group_focus_obj(cont);

    return cont;
}

static lv_obj_t *voice_img_create(lv_obj_t *parent)
{
    LV_IMG_DECLARE(ui_img_voice_png);

    lv_obj_t *img;
    img = lv_img_create(parent);
    lv_img_set_src(img, &ui_img_voice_png);
    lv_img_set_antialias(img, false);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    return img;
}

static void recorder_rec_click_event_cb(lv_event_t *e)
{
    struct recorder_service *recs = g_recorder_srv;

    app_logi("%s: click_event\n", __func__);

    if (recs->state == RECD_IDLE || recs->state == RECD_PAUSE) {
        app_recorder_resume();
    } else if (recs->state == RECD_RECORD) {
#if 0
        //TODO: audio service not support pause now
        app_recorder_pause();
#else
        app_recorder_stop();
#endif
    } else if (recs->state == RECD_PLAY) {
        // revert auto state
        lv_imgbtn_set_state(recorder_obj, LV_IMGBTN_STATE_RELEASED);
    } else {
        // do nothing
    }
}

static void recorder_key_event_cb(lv_event_t *e)
{
    struct recorder_service *recs = g_recorder_srv;
    app_logi("recorder: recorder_key_event_cb %d\n",
             *((char *)lv_event_get_param(e)));
    lv_event_code_t code = lv_event_get_code(e);
    if (LV_EVENT_KEY == code) {
        char key = *((char *)lv_event_get_param(e));
        if (LV_KEY_BACKSPACE == key) {
            app_logi("recorder: LV_KEY_BACKSPACE key\n");
            if (recs->state == RECD_RECORD) {
                lv_imgbtn_set_state(recorder_obj, LV_IMGBTN_STATE_RELEASED);
            }
            if (recs->state == RECD_PAUSE || recs->state == RECD_RECORD ||
                recs->state == RECD_PLAY) {
                app_recorder_stop();
            }
        } else if (LV_KEY_RIGHT == key) {
            app_logi("recorder: LV_KEY_RIGHT key\n");
            if (recs->state == RECD_IDLE) {
                app_recorder_play();
            }
        } else if (LV_KEY_ENTER == key) {
            app_logi("recorder: LV_KEY_ENTER key\n");
            if (recs->state == RECD_PAUSE || recs->state == RECD_IDLE) {
                app_recorder_resume();
            } else if (recs->state == RECD_RECORD) {
#if 0
                //TODO: audio service not support pause now
                app_recorder_pause();
#else
                app_recorder_stop();
#endif
            }
        }
    } else {
        app_loge("recorder: NOT ENTERT KEY CB\n");
    }
}

static void recorder_play_click_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    app_logi("%s: click_event code %d\n", __func__, code);
    if (LV_EVENT_CLICKED == code) {
        app_recorder_play();
    }
}

static void recorder_stop_click_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    app_logi("%s: click_event code %d\n", __func__, code);
    if (LV_EVENT_CLICKED == code) {
        app_recorder_stop();
    }
}

static void recorder_timer_cb(lv_timer_t *t)
{
    sec_time++;
    lv_label_set_text_fmt(time_label, "%02lu:%02lu:%02lu", sec_time / 60 / 60,
                          sec_time / 60, sec_time % 60);
}

/*******************************************
 *              GLOBAL FUNCTIONS
 ******************************************/
lv_obj_t *app_recorder_main_create(lv_obj_t *parent)
{
    /*Create the content of the recorder*/
    lv_obj_t *cont = create_cont(parent);
    lv_obj_t *title_box = create_title_box(cont);
    lv_obj_t *ctrl_box = create_ctrl_box(cont);
    lv_obj_t *voice_icon_obj = create_voice_icon_obj(cont);

    /*Arrange the content into a grid*/
    static const lv_coord_t grid_cols[] = {LV_GRID_FR(1),
                                           LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t grid_rows[] = {
        RECORDER_HANDLE_SIZE, /*Spacing*/
        LV_GRID_FR(1),        /*Spacer*/
        LV_GRID_FR(20),       /*Voice icon obj*/
        LV_GRID_FR(1),        /*Spacer*/
        LV_GRID_FR(4),        /*Title box LV_GRID_CONTENT */
        LV_GRID_FR(1),        /*Spacer*/
        LV_GRID_FR(4),        /*Ctrl obj*/
        LV_GRID_FR(3),        /*Spacer*/
        RECORDER_HANDLE_SIZE, /*Spacing*/
        LV_GRID_TEMPLATE_LAST};

    lv_obj_set_grid_dsc_array(cont, grid_cols, grid_rows);
    lv_obj_set_style_grid_row_align(cont, LV_GRID_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_grid_cell(title_box, LV_GRID_ALIGN_STRETCH, 0, 1,
                         LV_ALIGN_CENTER, 4, 1);
    lv_obj_set_grid_cell(voice_icon_obj, LV_GRID_ALIGN_STRETCH, 0, 1,
                         LV_GRID_ALIGN_CENTER, 2, 1);
    lv_obj_set_grid_cell(ctrl_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_ALIGN_CENTER,
                         6, 1);

    sec_counter_timer = lv_timer_create(recorder_timer_cb, 1000, NULL);
    lv_timer_pause(sec_counter_timer);

    lv_obj_update_layout(cont);

    return cont;
}
