/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 *  DESCRIPTION
 *
 *    This file is enter of engineering mode SMS test display
 */

/*********************
 *      INCLUDES
 *********************/
#include "aic_srv_tele_sms.h"
#include "aic_type.h"
#include "alv_app_mgr.h"
#include "app_log.h"
#include "app_msg.h"
#include "lvgl.h"
#include "sms_test.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*********************
 *      DEFINES
 *********************/
#define SMS_WRITE_NUMBER_LEN_MAX 21
#define SMS_WRITE_CONTENT_LEN_MAX 600
#define SMS_LIST_CONTENT_LEN_MAX 20
#define SMS_LIST_CONTENT_BYTE_LEN_MAX SMS_LIST_CONTENT_LEN_MAX * 4
#define SMS_OPERATION_SUCCEED "SUCCESS"
#define SMS_OPERATION_FAILED "FAIL"
#define sms_test_malloc(x) aic_zalloc(x)
#define sms_test_free(x) aic_free(x)
#define APP_EM_SMS_GET_LIST_NOTIFY_APP __APP_CALLBACK_NAME__(app_sms_test_get_list)
#define APP_EM_SMS_READ_LIST_NOTIFY_APP __APP_CALLBACK_NAME__(app_sms_test_read_in_list)
#define APP_EM_SMS_READ_PAGE_NOTIFY_APP __APP_CALLBACK_NAME__(app_sms_test_read_in_page)
#define APP_EM_SMS_GET_SMSC_NOTIFY_APP __APP_CALLBACK_NAME__(app_sms_test_get_smsc)
#define APP_EM_SMS_DELETE_NOTIFY_APP __APP_CALLBACK_NAME__(app_sms_test_delete)
#define APP_EM_SMS_SEND_NOTIFY_APP __APP_CALLBACK_NAME__(app_sms_test_send)
#define APP_EM_SMS_NEW_IN_NOTIFY_APP __APP_CALLBACK_NAME__(app_sms_test_new_in)
#define APP_EM_SMS_SET_SMSC_NOTIFY_APP __APP_CALLBACK_NAME__(app_sms_test_set_smsc)
#define APP_EM_SMS_SIM_STATE_NOTIFY_APP __APP_CALLBACK_NAME__(app_sms_test_sim_state)

/**********************
 *      TYPEDEFS
 **********************/
/**
 * enum - btnm id.
 *
 * @BTNM_ACTION_WRITE_ID: write
 * @BTNM_ACTION_SMSC_ID: smsc
 */
enum app_sms_test_btnm_action_id {
    BTNM_ACTION_WRITE_ID,
    BTNM_ACTION_SMSC_ID,
};

/**
 * enum - app_sms_test_current_page_state.
 *
 * @BTNM_ACTION_WRITE_ID: write
 * @BTNM_ACTION_SMSC_ID: smsc
 */
enum app_sms_test_current_page_state {
    EM_SMS_LIST_PAGE = 1,
    EM_SMS_READ_PAGE,
    EM_SMS_SMSC_PAGE,
    EM_SMS_WRITE_PAGE,
    EM_SMS_MSGBOX_STATE,
};

/**********************
 *   STATIC VARIABLES
 **********************/
static lv_obj_t *g_app_sms_test_win = NULL;
static lv_obj_t *g_app_sms_test_panel = NULL;
static lv_obj_t *g_app_sms_test_list_panel = NULL;
static lv_obj_t *g_app_sms_test_list_table = NULL;
static lv_obj_t *g_app_sms_test_write_panel = NULL;
static lv_obj_t *g_app_sms_test_read_panel = NULL;
static lv_obj_t *g_app_sms_test_smsc_panel = NULL;
static lv_obj_t *g_app_sms_test_keyboard = NULL;
static lv_obj_t *g_app_sms_test_receiver = NULL;
static lv_obj_t *g_app_sms_test_send_msgbox = NULL;
static lv_obj_t *g_app_sms_test_msgbox_get_list = NULL;
static lv_obj_t *g_app_sms_test_msgbox_loading = NULL;
static lv_obj_t *g_app_sms_test_btnm_action = NULL;
static uint16_t *g_app_sms_test_list_index = NULL;
static uint16_t *g_app_sms_test_table_index = NULL;

static uint16_t g_app_sms_test_index_count = 0;
static uint16_t g_app_sms_test_list_count = 0;
static uint16_t g_app_sms_test_table_row = 0;
static uint16_t g_app_sms_test_current_state = 0;
static bool g_app_sms_test_sim_state_ready = false;
static bool g_app_sms_test_need_list_update = false;
static const char *g_app_sms_test_btnm_map_action[] = {"WRITE", "SMSC", ""};
static const char *g_app_sms_test_delete_msgbox[] = {"Delete", "Cancel", ""};
static const char *g_app_sms_test_magbox_ok[] = {"OK", ""};

static int32_t g_sms_test_cb_handle = TS_SMS_INVALID_HANDLE;

/*******************************************
 *              STATIC PROTOTYPES
 ******************************************/
static void app_sms_test_event_cb_msgbox_delete(lv_event_t *e);
static void app_sms_test_event_cb_msgbox_delete_ok(lv_event_t *e);
static void app_sms_test_event_cb_msgbox_set_smsc(lv_event_t *e);
static void app_sms_test_event_cb_msgbox_send(lv_event_t *e);
static void app_sms_test_event_cb_msgbox_new(lv_event_t *e);
static void app_sms_test_event_cb_msgbox_full(lv_event_t *e);

static void app_sms_test_event_cb_btnm(lv_event_t *e);
static void app_sms_test_event_cb_kb(lv_event_t *e);
static void app_sms_test_event_cb_send(lv_event_t *e);
static void app_sms_test_event_cb_set_smsc(lv_event_t *e);
static void app_sms_test_event_cb_return(lv_event_t *e);
static void app_sms_test_event_cb_gesture(lv_event_t *e);
static void app_sms_test_event_cb_textarea(lv_event_t *e);
static void app_sms_test_event_cb_list(lv_event_t *e);

static void app_sms_test_write_smsc_btnm(void);
static void app_sms_test_read_page(char *number, char *timestamp, char *text);
static void app_sms_test_write_page(void);
static void app_sms_test_smsc_page(char *old_smsc);

static int32_t app_sms_test_cb(void *p_param, uint32_t size);
static int32_t app_sms_test_cb_read(void *p_param, uint32_t size);
static int32_t app_sms_test_get_list(void *p_param, uint32_t size);
static int32_t app_sms_test_get_smsc(void *p_param, uint32_t size);
static int32_t app_sms_test_read_in_page(void *p_param, uint32_t size);
static int32_t app_sms_test_read_in_list(void *p_param, uint32_t size);
static int32_t app_sms_test_delete(void *p_param, uint32_t size);
static int32_t app_sms_test_send(void *p_param, uint32_t size);
static int32_t app_sms_test_new_in(void *p_param, uint32_t size);
static int32_t app_sms_test_set_smsc(void *p_param, uint32_t size);
static int32_t app_sms_test_sim_state(void *p_param, uint32_t size);

/**********************
 *   STATIC FUNCTIONS
 **********************/
__APP_CALLBACK_FUNCTION__(app_sms_test_read_in_page);
__APP_CALLBACK_FUNCTION__(app_sms_test_get_list);
__APP_CALLBACK_FUNCTION__(app_sms_test_get_smsc);
__APP_CALLBACK_FUNCTION__(app_sms_test_read_in_list);
__APP_CALLBACK_FUNCTION__(app_sms_test_delete);
__APP_CALLBACK_FUNCTION__(app_sms_test_send);
__APP_CALLBACK_FUNCTION__(app_sms_test_new_in);
__APP_CALLBACK_FUNCTION__(app_sms_test_set_smsc);
__APP_CALLBACK_FUNCTION__(app_sms_test_sim_state);

/**
 * app_sms_test_cb_read - sms cb
 * @p_param: [IN]
 * @size: [IN]
 * Return: NA
 */
static int32_t app_sms_test_cb_read(void *p_param, uint32_t size)
{
    if (!p_param || !size) {
        return -1;
    }
    uint32_t event = *(uint32_t *)p_param;
    app_logi("[em sms test]enter app_sms_test_cb_read,read SMS.");
    if (event == TS_EVENT_SMS_READ_IN_SIM_RESP) {
        APP_EM_SMS_READ_PAGE_NOTIFY_APP(p_param, size);
    }
    return 0;
}

/**
 * app_sms_test_cb - sms cb
 * @p_param: [IN]
 * @size: [IN]
 * Return: NA
 */
static int32_t app_sms_test_cb(void *p_param, uint32_t size)
{
    if (!p_param || !size) {
        return -1;
    }
    uint32_t event = *(uint32_t *)p_param;
    switch (event) {
    case TS_EVENT_SMS_RECEIVE_NEW:
        APP_EM_SMS_NEW_IN_NOTIFY_APP(p_param, size);
        break;
    case TS_EVENT_SMS_LOAD_IN_SIM_RESP:
        APP_EM_SMS_SIM_STATE_NOTIFY_APP(p_param, size);
        break;
    case TS_EVENT_SMS_GET_LIST_IN_SIM_RESP:
        APP_EM_SMS_GET_LIST_NOTIFY_APP(p_param, size);
        break;
    case TS_EVENT_SMS_READ_IN_SIM_RESP:
        APP_EM_SMS_READ_LIST_NOTIFY_APP(p_param, size);
        break;
    case TS_EVENT_SMS_SEND_RESP:
        APP_EM_SMS_SEND_NOTIFY_APP(p_param, size);
        break;
    case TS_EVENT_SMS_SET_SMSC_RESP:
        APP_EM_SMS_SET_SMSC_NOTIFY_APP(p_param, size);
        break;
    case TS_EVENT_SMS_GET_SMSC_RESP:
        APP_EM_SMS_GET_SMSC_NOTIFY_APP(p_param, size);
        break;
    case TS_EVENT_SMS_DEL_IN_SIM_RESP:
        APP_EM_SMS_DELETE_NOTIFY_APP(p_param, size);
        break;
    default:
        break;
    }
    return 0;
}

/**
 * app_sms_test_event_cb_list - list cb
 * @e: [IN] event
 *
 * Return: NA
 */
static void app_sms_test_event_cb_list(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    uint16_t *user_data = (uint16_t *)lv_obj_get_user_data(obj);
    uint16_t row = 0, col = 0;
    lv_table_get_selected_cell(obj, &row, &col);
    if (row != LV_TABLE_CELL_NONE && col != LV_TABLE_CELL_NONE) {
        /* Long press to delete,click to read SMS */
        if (code == LV_EVENT_LONG_PRESSED) {
            app_logi("[em sms test]row[%d]index=%d", row, user_data[row]);
            app_logi("[em sms test]LV_EVENT_LONG_PRESSED to delete SMS");
            lv_indev_wait_release(lv_indev_get_act());
            app_logi("[em sms test]create 'Delete this message?' msgbox");
            lv_obj_t *mbox = lv_msgbox_create(NULL, NULL, "Delete this message?", g_app_sms_test_delete_msgbox, false);
            lv_obj_set_align(mbox, LV_ALIGN_CENTER);
            g_app_sms_test_current_state = EM_SMS_MSGBOX_STATE;
            lv_obj_add_event_cb(mbox, app_sms_test_event_cb_msgbox_delete, LV_EVENT_VALUE_CHANGED,
                                (void *)(uint32_t)user_data[row]);
        } else if (code == LV_EVENT_VALUE_CHANGED) {
            app_logi("[em sms test]row[%d]index=%d", row, user_data[row]);
            app_logi("[em sms test]LV_EVENT_VALUE_CHANGED to read SMS");
            int32_t ret = aic_srv_tele_sms_read_in_sim(0, user_data[row], app_sms_test_cb_read);
            while (ret) {
                app_loge("[em sms test]app_sms_test_event_cb_list:req read in sim error");
                ret = aic_srv_tele_sms_read_in_sim(0, user_data[row], app_sms_test_cb_read);
            }
        }
    }
    /* delete user data when the table is deleted */
    if (code == LV_EVENT_DELETE) {
        void *user_data = lv_obj_get_user_data(obj);
        if (user_data != NULL) {
            sms_test_free(user_data);
            lv_obj_set_user_data(obj, NULL);
        }
    }
}

/**
 * app_sms_test_event_cb_textarea - textarea event cb
 * @e: [IN] event
 *
 * Return: NA
 */
static void app_sms_test_event_cb_textarea(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (lv_event_get_user_data(e)) {
        /* Get number of characters entered */
        lv_obj_t *count_label = (lv_obj_t *)lv_event_get_user_data(e);
        int i = _lv_txt_get_encoded_length(lv_textarea_get_text(obj));
        char a[10];
        sprintf(a, "%d/%d", i, SMS_WRITE_CONTENT_LEN_MAX);
        lv_label_set_text(count_label, a);
    }
    /* Show the keyboard */
    if (code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED) {
        /* focus on the clicked textarea */
        lv_keyboard_set_textarea(g_app_sms_test_keyboard, obj);
        lv_obj_clear_flag(g_app_sms_test_keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(g_app_sms_test_keyboard);
    }
}

/**
 * app_sms_test_event_cb_gesture - gesture event cb
 * @e: [IN] event
 *
 * Return: NA
 */
static void app_sms_test_event_cb_gesture(lv_event_t *e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (dir == LV_DIR_RIGHT) {
        alv_am_app_exit(NULL, "sms_test", NULL);
        lv_indev_wait_release(lv_indev_get_act());
    }
}

/**
 * app_sms_test_event_cb_return - return event cb
 * @e: [IN] event
 *
 * Return: NA
 */
static void app_sms_test_event_cb_return(lv_event_t *e)
{
    lv_obj_t *obj = lv_obj_get_parent(lv_event_get_target(e));
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        app_logi("[em sms test] return");
        g_app_sms_test_current_state = EM_SMS_LIST_PAGE;
        app_logi("[em sms test]g_app_sms_test_current_state=%d", g_app_sms_test_current_state);
        if (g_app_sms_test_need_list_update) {
            g_app_sms_test_need_list_update = 0;
            app_logi("[em sms test] a new sms received ,need to update list");
            app_logi("[em sms test] msgbox state");
            g_app_sms_test_msgbox_get_list = lv_msgbox_create(NULL, "Waiting", "Getting SMS list", NULL, false);
            lv_obj_set_align(g_app_sms_test_msgbox_get_list, LV_ALIGN_CENTER);
            ts_sms_get_list_info_t get_list_info = {0};
            get_list_info.card_id = 0;
            get_list_info.sms_type = TS_SMS_TYPE_ALL;
            int32_t ret = aic_srv_tele_sms_get_list_in_sim(&get_list_info, NULL);
            while (ret) {
                app_loge("[em sms test]app_sms_test_event_cb_return:req get list error");
                ret = aic_srv_tele_sms_get_list_in_sim(&get_list_info, NULL);
                // lv_msgbox_close(g_app_sms_test_msgbox_get_list);
                // g_app_sms_test_msgbox_get_list = NULL;
            }
        }
        app_logi("[em sms test]return to list_page");
        lv_obj_move_foreground(g_app_sms_test_panel);
        lv_obj_clean(obj);
        app_logi("[em sms test]clean other page");
    }
}

/**
 * app_sms_test_event_cb_set_smsc - set event cb
 * @e: [IN] event
 *
 * Return: NA
 */
static void app_sms_test_event_cb_set_smsc(lv_event_t *e)
{
    lv_obj_t *textarea = (lv_obj_t *)e->user_data;
    const char *new_smsc = lv_textarea_get_text(textarea);
    int32_t ret = aic_srv_tele_sms_set_smsc(0, new_smsc, NULL);
    if (ret) {
        app_loge("[em sms test]app_sms_test_event_cb_set_smsc:req set smsc error");
    }
}

/**
 * app_sms_test_event_cb_send - send event cb
 * @e: [IN] event
 *
 * Return: NA
 */
static void app_sms_test_event_cb_send(lv_event_t *e)
{
    lv_obj_t *textarea = (lv_obj_t *)e->user_data;
    const char *content = lv_textarea_get_text(textarea);
    const char *receiver = lv_textarea_get_text(g_app_sms_test_receiver);
    // const char *receiver = "10010"; /* China unicom as default */
    // const char *content = "cxye";
    ts_sms_send_info_t send_info = {0};
    app_logi("[em sms test]create 'Waiting Send the SMS ...' msgbox");
    g_app_sms_test_send_msgbox = lv_msgbox_create(NULL, "Waiting", "Send the SMS ...", NULL, false);
    lv_obj_set_align(g_app_sms_test_send_msgbox, LV_ALIGN_CENTER);

    send_info.is_write_to_sim = true;
    send_info.p_number = (char *)receiver;
    send_info.p_text = (char *)content;
    send_info.p_smsc = NULL;
    app_logi("[em sms test]app_sms_test_event_cb_send:p_number=%s,p_text=%s.", send_info.p_number, send_info.p_text);
    int32_t ret = aic_srv_tele_sms_send(0, &send_info, NULL);
    if (ret) {
        app_loge("[em sms test]app_sms_test_event_cb_send:req send error");
        lv_msgbox_close(g_app_sms_test_send_msgbox);
        g_app_sms_test_send_msgbox = NULL;
    }
}

/**
 * app_sms_test_event_cb_kb - kb event cb
 * @e: [IN] event
 *
 * Return: NA
 */
static void app_sms_test_event_cb_kb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    const char *txt = lv_btnmatrix_get_btn_text(obj, lv_btnmatrix_get_selected_btn(obj));
    if (txt == NULL)
        return;
    if (strcmp(txt, LV_SYMBOL_OK) == 0) {
        lv_obj_add_flag(g_app_sms_test_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

static void app_sms_test_event_cb_msgbox_delete_ok(lv_event_t *e)
{
    lv_obj_t *msgbox = lv_event_get_current_target(e);
    const char *txt = lv_msgbox_get_active_btn_text(msgbox);
    ts_sms_get_list_info_t get_list_info = {0};
    int32_t ret = 0;
    if (strcmp(txt, "OK") == 0) {
        if (strcmp((char *)(e->user_data), SMS_OPERATION_SUCCEED) == 0) {
            app_logi("[em sms test] delete sms succeeded ,need to update list");
            g_app_sms_test_msgbox_get_list = lv_msgbox_create(NULL, "Waiting", "Getting SMS list", NULL, false);
            lv_obj_set_align(g_app_sms_test_msgbox_get_list, LV_ALIGN_CENTER);
            get_list_info.card_id = 0;
            get_list_info.sms_type = TS_SMS_TYPE_ALL;
            ret = aic_srv_tele_sms_get_list_in_sim(&get_list_info, NULL);
            while (ret) {
                app_loge("[em sms test]app_sms_test_event_cb_msgbox_delete_ok:req get list error");
                ret = aic_srv_tele_sms_get_list_in_sim(&get_list_info, NULL);
                // lv_msgbox_close(g_app_sms_test_msgbox_get_list);
                // g_app_sms_test_msgbox_get_list = NULL;
            }
        } else if (strcmp((char *)(e->user_data), SMS_OPERATION_FAILED) == 0) {
            app_logi("[em sms test] delete sms failed");
            if (g_app_sms_test_need_list_update) {
                app_logi("[em sms test] a new sms received ,need to update list");
                g_app_sms_test_msgbox_get_list = lv_msgbox_create(NULL, "Waiting", "Getting SMS list", NULL, false);
                lv_obj_set_align(g_app_sms_test_msgbox_get_list, LV_ALIGN_CENTER);
                ret = aic_srv_tele_sms_get_list_in_sim(&get_list_info, NULL);
                if (ret) {
                    app_loge("[em sms test]app_sms_test_event_cb_msgbox_delete_ok:req get list error");
                    ret = aic_srv_tele_sms_get_list_in_sim(&get_list_info, NULL);
                    // lv_msgbox_close(g_app_sms_test_msgbox_get_list);
                    // g_app_sms_test_msgbox_get_list = NULL;
                }
            }
        }
        g_app_sms_test_current_state = EM_SMS_LIST_PAGE;
        lv_msgbox_close(msgbox);
        app_logi("[em sms test]close 'Delete SMS Succeeded/Failed!'msgbox");
    }
}

static void app_sms_test_event_cb_msgbox_new(lv_event_t *e)
{
    lv_obj_t *msgbox = lv_event_get_current_target(e);
    /* update list in list_page */
    app_logi("[em sms test]g_app_sms_test_current_state=%d", g_app_sms_test_current_state);
    if (g_app_sms_test_current_state == EM_SMS_LIST_PAGE) {
        g_app_sms_test_need_list_update = false;
        if (g_app_sms_test_msgbox_get_list) {
            app_logi("[em sms test]g_app_sms_test_msgbox_get_list");
            lv_msgbox_close(g_app_sms_test_msgbox_get_list);
            g_app_sms_test_msgbox_get_list = NULL;
        }
        app_logi("[em sms test]a new sms received ,need to update list");
        g_app_sms_test_msgbox_get_list = lv_msgbox_create(NULL, "Waiting", "Getting SMS list", NULL, false);
        lv_obj_set_align(g_app_sms_test_msgbox_get_list, LV_ALIGN_CENTER);
        ts_sms_get_list_info_t get_list_info = {0};
        get_list_info.card_id = 0;
        get_list_info.sms_type = TS_SMS_TYPE_ALL;
        int32_t ret = aic_srv_tele_sms_get_list_in_sim(&get_list_info, NULL);
        while (ret) {
            app_loge("[em sms test]app_sms_test_event_cb_msgbox_new:req get list error");
            ret = aic_srv_tele_sms_get_list_in_sim(&get_list_info, NULL);
            // lv_msgbox_close(g_app_sms_test_msgbox_get_list);
            // g_app_sms_test_msgbox_get_list = NULL;
        }
    }
    app_logi("[em sms test]close 'NEW SMS RECEIVED' msgbox");
    lv_msgbox_close(msgbox);
}

/**
 * app_sms_test_event_cb_msgbox_delete - msgbox event cb
 * @e: [IN] event
 *
 * Return: NA
 */
static void app_sms_test_event_cb_msgbox_delete(lv_event_t *e)
{
    lv_obj_t *msgbox = lv_event_get_current_target(e);
    const char *txt = lv_msgbox_get_active_btn_text(msgbox);
    app_logi("txt=%s", txt);
    int32_t index = (int32_t)e->user_data;
    if (strcmp(txt, "Delete") == 0) {
        app_logi("[em sms test]'Delete',index=%d.", index);
        int32_t err_code = aic_srv_tele_sms_del_in_sim(0, &index, 1, NULL);
        if (err_code) {
            app_loge("[em sms test]app_sms_test_event_cb_msgbox_delete:req delete in sim error");
        }
    } else if (strcmp(txt, "Cancel") == 0) {
        app_logi("[em sms test]'Cancel'");
        if (g_app_sms_test_need_list_update) {
            g_app_sms_test_need_list_update = 0;
            app_logi("[em sms test]a new sms received ,need to update list");
            g_app_sms_test_msgbox_get_list = lv_msgbox_create(NULL, "Waiting", "Getting SMS list", NULL, false);
            lv_obj_set_align(g_app_sms_test_msgbox_get_list, LV_ALIGN_CENTER);
            ts_sms_get_list_info_t get_list_info = {0};
            get_list_info.card_id = 0;
            get_list_info.sms_type = TS_SMS_TYPE_ALL;
            int32_t ret = aic_srv_tele_sms_get_list_in_sim(&get_list_info, NULL);
            while (ret) {
                app_loge("[em sms test]app_sms_test_event_cb_msgbox_delete:req get list error");
                ret = aic_srv_tele_sms_get_list_in_sim(&get_list_info, NULL);
            }
        } else {
            app_logi("[em sms test]'Cancel',return to list_page");
            lv_obj_move_foreground(g_app_sms_test_panel);
        }
    }
    lv_msgbox_close(msgbox);
    app_logi("[em sms test]close 'Delete this message?' msgbox");
}

/**
 * app_sms_test_event_cb_msgbox_set_smsc - msgbox event cb
 * @e: [IN] event
 *
 * Return: NA
 */
static void app_sms_test_event_cb_msgbox_set_smsc(lv_event_t *e)
{
    lv_obj_t *msgbox = lv_event_get_current_target(e);
    app_logi("[em sms test]app_sms_test_event_cb_msgbox_set_smsc.");
    if (e->user_data) {
        app_logi("[em sms test]app_sms_test_event_cb_msgbox_set_smsc:(char *)(e->user_data):%s",
                 (char *)(e->user_data));
        if (strcmp((char *)(e->user_data), SMS_OPERATION_SUCCEED) == 0) {
            app_logi("[em sms test]app_sms_test_event_cb_msgbox_set_smsc:set succeeded,get smsc.");
            int32_t ret = aic_srv_tele_sms_get_smsc(0, NULL);
            if (ret) {
                app_loge("[em sms test]app_sms_test_event_cb_btnm:req get smsc error");
            }
        }
    }
    lv_msgbox_close(msgbox);
}

/**
 * app_sms_test_event_cb_msgbox_send - msgbox event cb
 * @e: [IN] event
 *
 * Return: NA
 */
static void app_sms_test_event_cb_msgbox_send(lv_event_t *e)
{
    lv_obj_t *msgbox = lv_event_get_current_target(e);
    if (e->user_data) {
        // if (strcmp((char *)(e->user_data), SMS_OPERATION_SUCCEED) == 0) {
        // /* if sms send succeed,need to update sms list */
        g_app_sms_test_need_list_update = true;
        // }
        app_logi("[em sms test]g_app_sms_test_need_list_update=%d", g_app_sms_test_need_list_update);
    }
    lv_msgbox_close(msgbox);
    app_logi("[em sms test]close 'Send SMS Succeeded/Failed' msgbox");
}

/**
 * app_sms_test_event_cb_msgbox_full - msgbox event cb
 * @e: [IN] event
 *
 * Return: NA
 */
static void app_sms_test_event_cb_msgbox_full(lv_event_t *e)
{
    lv_obj_t *msgbox = lv_event_get_current_target(e);
    lv_msgbox_close(msgbox);
    app_logi("[em sms test]close 'SIM sms memory is full' msgbox");
}

/**
 * app_sms_test_event_cb_btnm - btnm action event
 * @e: [IN] event
 *
 * Return: NA
 */
static void app_sms_test_event_cb_btnm(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    uint8_t id = lv_btnmatrix_get_selected_btn(obj);
    switch (id) {
    case BTNM_ACTION_WRITE_ID:
        lv_obj_move_foreground(g_app_sms_test_write_panel);
        app_logi("[em sms test]list_panel to write_panel");
        g_app_sms_test_current_state = EM_SMS_WRITE_PAGE;
        app_logi("[em sms test]g_app_sms_test_current_state=%d", g_app_sms_test_current_state);
        app_sms_test_write_page();
        break;
    case BTNM_ACTION_SMSC_ID:
        lv_obj_move_foreground(g_app_sms_test_smsc_panel);
        app_logi("[em sms test]list_panel to smsc_panel");
        g_app_sms_test_current_state = EM_SMS_SMSC_PAGE;
        app_logi("[em sms test]g_app_sms_test_current_state=%d", g_app_sms_test_current_state);
        app_sms_test_smsc_page("");
        int32_t ret = aic_srv_tele_sms_get_smsc(0, NULL);
        if (ret) {
            app_loge("[em sms test]app_sms_test_event_cb_btnm:req get smsc error");
        }
        break;
    default:
        break;
    }
}

/**
 * app_sms_test_read_page - sms read page
 * @number: [IN] number of sms in list
 * @timestamp: [IN]
 * @text: [IN]
 * Return: NA
 */
static void app_sms_test_read_page(char *number, char *timestamp, char *text)
{
    lv_obj_set_size(g_app_sms_test_read_panel, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(g_app_sms_test_read_panel, LV_OBJ_FLAG_SCROLLABLE);
    app_logi("[em sms test]app_sms_test_read_page:create in g_app_sms_test_read_panel");

    lv_obj_t *read_label = lv_label_create(g_app_sms_test_read_panel);
    lv_obj_set_width(read_label, LV_SIZE_CONTENT);
    lv_obj_set_height(read_label, LV_SIZE_CONTENT);
    lv_obj_set_align(read_label, LV_ALIGN_TOP_MID);
    lv_label_set_text(read_label, "READ");

    lv_obj_t *number_label = lv_label_create(g_app_sms_test_read_panel);
    lv_obj_set_width(number_label, LV_SIZE_CONTENT);
    lv_obj_set_height(number_label, LV_SIZE_CONTENT);
    lv_obj_set_x(number_label, 15);
    lv_obj_set_y(number_label, -72);
    lv_obj_set_align(number_label, LV_ALIGN_LEFT_MID);
    lv_label_set_text(number_label, number);

    lv_obj_t *time_label = lv_label_create(g_app_sms_test_read_panel);
    lv_obj_set_width(time_label, LV_SIZE_CONTENT);
    lv_obj_set_height(time_label, LV_SIZE_CONTENT);
    lv_obj_set_x(time_label, 15);
    lv_obj_set_y(time_label, -41);
    lv_obj_set_align(time_label, LV_ALIGN_LEFT_MID);
    lv_label_set_text(time_label, timestamp);

    lv_obj_t *content_label = lv_label_create(g_app_sms_test_read_panel);
    lv_obj_set_width(content_label, 180);
    lv_obj_set_height(content_label, 103);
    lv_obj_set_x(content_label, 0);
    lv_obj_set_y(content_label, 30);
    lv_obj_set_align(content_label, LV_ALIGN_CENTER);
    lv_label_set_long_mode(content_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(content_label, text);

    lv_obj_t *return_btn = lv_btn_create(g_app_sms_test_read_panel);
    lv_obj_set_width(return_btn, 80);
    lv_obj_set_height(return_btn, 39);
    lv_obj_set_x(return_btn, 0);
    lv_obj_set_y(return_btn, 125);
    lv_obj_set_align(return_btn, LV_ALIGN_CENTER);
    lv_obj_clear_flag(return_btn, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *return_label = lv_label_create(return_btn);
    lv_obj_set_width(return_label, LV_SIZE_CONTENT);
    lv_obj_set_height(return_label, LV_SIZE_CONTENT);
    lv_obj_set_align(return_label, LV_ALIGN_CENTER);
    lv_label_set_text(return_label, "RETURN");
    lv_obj_add_event_cb(return_btn, app_sms_test_event_cb_return, LV_EVENT_CLICKED, NULL);
}

/**
 * app_sms_test_write_page - sms write page
 *
 * Return: NA
 */
static void app_sms_test_write_page(void)
{
    lv_obj_set_size(g_app_sms_test_write_panel, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(g_app_sms_test_write_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *write_label = lv_label_create(g_app_sms_test_write_panel);
    lv_obj_set_width(write_label, LV_SIZE_CONTENT);
    lv_obj_set_height(write_label, LV_SIZE_CONTENT);
    lv_obj_set_align(write_label, LV_ALIGN_TOP_MID);
    lv_label_set_text(write_label, "WRITE");

    lv_obj_t *receiver_label = lv_label_create(g_app_sms_test_write_panel);
    lv_obj_set_width(receiver_label, LV_SIZE_CONTENT);
    lv_obj_set_height(receiver_label, LV_SIZE_CONTENT);
    lv_obj_set_x(receiver_label, -69);
    lv_obj_set_y(receiver_label, -97);
    lv_obj_set_align(receiver_label, LV_ALIGN_CENTER);
    lv_label_set_text(receiver_label, "receiver");

    g_app_sms_test_receiver = lv_textarea_create(g_app_sms_test_write_panel);
    lv_obj_set_width(g_app_sms_test_receiver, 196);
    lv_obj_set_height(g_app_sms_test_receiver, 36);
    lv_obj_set_x(g_app_sms_test_receiver, -2);
    lv_obj_set_y(g_app_sms_test_receiver, -71);
    lv_obj_set_align(g_app_sms_test_receiver, LV_ALIGN_CENTER);
    lv_textarea_set_max_length(g_app_sms_test_receiver, SMS_WRITE_NUMBER_LEN_MAX);
    lv_obj_add_event_cb(g_app_sms_test_receiver, app_sms_test_event_cb_textarea, LV_EVENT_ALL, NULL);

    lv_obj_t *content_label = lv_label_create(g_app_sms_test_write_panel);
    lv_obj_set_width(content_label, LV_SIZE_CONTENT);
    lv_obj_set_height(content_label, LV_SIZE_CONTENT);
    lv_obj_set_x(content_label, -69);
    lv_obj_set_y(content_label, -39);
    lv_obj_set_align(content_label, LV_ALIGN_CENTER);
    lv_label_set_text(content_label, "content");

    lv_obj_t *count_label = lv_label_create(g_app_sms_test_write_panel);
    lv_obj_set_width(count_label, LV_SIZE_CONTENT);
    lv_obj_set_height(count_label, LV_SIZE_CONTENT);
    lv_obj_set_x(count_label, 59);
    lv_obj_set_y(count_label, -39);
    lv_obj_set_align(count_label, LV_ALIGN_CENTER);

    lv_obj_t *content_t = lv_textarea_create(g_app_sms_test_write_panel);
    lv_obj_set_width(content_t, 196);
    lv_obj_set_height(content_t, 102);
    lv_obj_set_x(content_t, -2);
    lv_obj_set_y(content_t, 19);
    lv_obj_set_align(content_t, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(content_t, app_sms_test_event_cb_textarea, LV_EVENT_ALL, (void *)count_label);

    lv_obj_t *return_btn = lv_btn_create(g_app_sms_test_write_panel);
    lv_obj_set_width(return_btn, 80);
    lv_obj_set_height(return_btn, 39);
    lv_obj_set_x(return_btn, 60);
    lv_obj_set_y(return_btn, 120);
    lv_obj_set_align(return_btn, LV_ALIGN_CENTER);
    lv_obj_clear_flag(return_btn, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *return_label = lv_label_create(return_btn);
    lv_obj_set_width(return_label, LV_SIZE_CONTENT);
    lv_obj_set_height(return_label, LV_SIZE_CONTENT);
    lv_obj_set_align(return_label, LV_ALIGN_CENTER);
    lv_label_set_text(return_label, "RETURN");
    lv_obj_add_event_cb(return_btn, app_sms_test_event_cb_return, LV_EVENT_CLICKED, NULL);

    lv_obj_t *send_btn = lv_btn_create(g_app_sms_test_write_panel);
    lv_obj_set_width(send_btn, 80);
    lv_obj_set_height(send_btn, 39);
    lv_obj_set_x(send_btn, -60);
    lv_obj_set_y(send_btn, 120);
    lv_obj_set_align(send_btn, LV_ALIGN_CENTER);
    lv_obj_clear_flag(send_btn, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *send_label = lv_label_create(send_btn);
    lv_obj_set_width(send_label, LV_SIZE_CONTENT);
    lv_obj_set_height(send_label, LV_SIZE_CONTENT);
    lv_obj_set_align(send_label, LV_ALIGN_CENTER);
    lv_label_set_text(send_label, "SEND");
    lv_obj_add_event_cb(send_btn, app_sms_test_event_cb_send, LV_EVENT_CLICKED, (void *)content_t);
}

/**
 * app_sms_test_smsc_page - sms write page
 *
 * Return: NA
 */
static void app_sms_test_smsc_page(char *old_smsc)
{
    lv_obj_set_size(g_app_sms_test_smsc_panel, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(g_app_sms_test_smsc_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *smsc_label = lv_label_create(g_app_sms_test_smsc_panel);
    lv_obj_set_width(smsc_label, LV_SIZE_CONTENT);
    lv_obj_set_height(smsc_label, LV_SIZE_CONTENT);
    lv_obj_set_align(smsc_label, LV_ALIGN_TOP_MID);
    lv_label_set_text(smsc_label, "SMSC");

    lv_obj_t *old_smsc_label = lv_label_create(g_app_sms_test_smsc_panel);
    lv_obj_set_width(old_smsc_label, LV_SIZE_CONTENT);
    lv_obj_set_height(old_smsc_label, LV_SIZE_CONTENT);
    lv_obj_set_x(old_smsc_label, -65);
    lv_obj_set_y(old_smsc_label, -93);
    lv_obj_set_align(old_smsc_label, LV_ALIGN_CENTER);
    lv_label_set_text(old_smsc_label, "old smsc");

    lv_obj_t *old_smsc_panel = lv_obj_create(g_app_sms_test_smsc_panel);
    lv_obj_clear_flag(old_smsc_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_align(old_smsc_panel, LV_ALIGN_CENTER);
    lv_obj_set_width(old_smsc_panel, 196);
    lv_obj_set_height(old_smsc_panel, 36);
    lv_obj_set_x(old_smsc_panel, -2);
    lv_obj_set_y(old_smsc_panel, -63);

    lv_obj_t *old_smsc_text = lv_label_create(old_smsc_panel);
    lv_obj_set_align(old_smsc_text, LV_ALIGN_LEFT_MID);
    lv_label_set_text(old_smsc_text, old_smsc);

    lv_obj_t *new_smsc_label = lv_label_create(g_app_sms_test_smsc_panel);
    lv_obj_set_width(new_smsc_label, LV_SIZE_CONTENT);
    lv_obj_set_height(new_smsc_label, LV_SIZE_CONTENT);
    lv_obj_set_x(new_smsc_label, -60);
    lv_obj_set_y(new_smsc_label, -22);
    lv_obj_set_align(new_smsc_label, LV_ALIGN_CENTER);
    lv_label_set_text(new_smsc_label, "new smsc");

    lv_obj_t *new_smsc_t = lv_textarea_create(g_app_sms_test_smsc_panel);
    lv_obj_set_width(new_smsc_t, 196);
    lv_obj_set_height(new_smsc_t, 38);
    lv_obj_set_x(new_smsc_t, -2);
    lv_obj_set_y(new_smsc_t, 9);
    lv_obj_set_align(new_smsc_t, LV_ALIGN_CENTER);
    lv_textarea_set_placeholder_text(new_smsc_t, "Placeholder...");
    lv_obj_add_event_cb(new_smsc_t, app_sms_test_event_cb_textarea, LV_EVENT_ALL, NULL);

    lv_obj_t *return_btn = lv_btn_create(g_app_sms_test_smsc_panel);
    lv_obj_set_width(return_btn, 80);
    lv_obj_set_height(return_btn, 39);
    lv_obj_set_x(return_btn, -60);
    lv_obj_set_y(return_btn, 120);
    lv_obj_set_align(return_btn, LV_ALIGN_CENTER);
    lv_obj_clear_flag(return_btn, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *return_label = lv_label_create(return_btn);
    lv_obj_set_width(return_label, LV_SIZE_CONTENT);
    lv_obj_set_height(return_label, LV_SIZE_CONTENT);
    lv_obj_set_align(return_label, LV_ALIGN_CENTER);
    lv_label_set_text(return_label, "RETURN");
    lv_obj_add_event_cb(return_btn, app_sms_test_event_cb_return, LV_EVENT_CLICKED, NULL);

    lv_obj_t *set_btn = lv_btn_create(g_app_sms_test_smsc_panel);
    lv_obj_set_width(set_btn, 80);
    lv_obj_set_height(set_btn, 39);
    lv_obj_set_x(set_btn, 60);
    lv_obj_set_y(set_btn, 120);
    lv_obj_set_align(set_btn, LV_ALIGN_CENTER);
    lv_obj_clear_flag(set_btn, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *set_label = lv_label_create(set_btn);
    lv_obj_set_width(set_label, LV_SIZE_CONTENT);
    lv_obj_set_height(set_label, LV_SIZE_CONTENT);
    lv_obj_set_align(set_label, LV_ALIGN_CENTER);
    lv_label_set_text(set_label, "SET");
    lv_obj_add_event_cb(set_btn, app_sms_test_event_cb_set_smsc, LV_EVENT_CLICKED, (void *)new_smsc_t);
}

static void app_sms_test_write_smsc_btnm(void)
{
    g_app_sms_test_btnm_action = lv_btnmatrix_create(g_app_sms_test_panel);
    lv_btnmatrix_set_map(g_app_sms_test_btnm_action, g_app_sms_test_btnm_map_action);
    lv_obj_set_height(g_app_sms_test_btnm_action, 50);
    lv_obj_set_width(g_app_sms_test_btnm_action, LV_HOR_RES);
    lv_obj_set_pos(g_app_sms_test_btnm_action, -15, 260);
    lv_obj_set_style_radius(g_app_sms_test_btnm_action, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_app_sms_test_btnm_action, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(g_app_sms_test_btnm_action, LV_BORDER_SIDE_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(g_app_sms_test_btnm_action, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(g_app_sms_test_btnm_action, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(g_app_sms_test_btnm_action, app_sms_test_event_cb_btnm, LV_EVENT_VALUE_CHANGED, NULL);
}
/**
 * app_sms_test_panel_page - sms page
 *
 * Return: NA
 */
static void app_sms_test_panel_page(void)
{
    lv_obj_clear_flag(g_app_sms_test_panel, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_set_size(g_app_sms_test_panel, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_radius(g_app_sms_test_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(g_app_sms_test_panel, app_sms_test_event_cb_gesture, LV_EVENT_GESTURE, NULL);

    lv_obj_t *g_app_sms_test_title_panel = lv_obj_create(g_app_sms_test_panel);
    lv_obj_set_height(g_app_sms_test_title_panel, 20);
    lv_obj_set_width(g_app_sms_test_title_panel, LV_HOR_RES);
    lv_obj_set_pos(g_app_sms_test_title_panel, 0, 0);
    lv_obj_set_style_radius(g_app_sms_test_title_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_app_sms_test_title_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(g_app_sms_test_title_panel, LV_BORDER_SIDE_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(g_app_sms_test_title_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *g_app_sms_test_title_label = lv_label_create(g_app_sms_test_title_panel);
    lv_label_set_text(g_app_sms_test_title_label, "SMS List");
    lv_obj_set_align(g_app_sms_test_title_label, LV_ALIGN_CENTER);

    g_app_sms_test_list_panel = lv_obj_create(g_app_sms_test_panel);
    lv_obj_set_height(g_app_sms_test_list_panel, 240);
    lv_obj_set_width(g_app_sms_test_list_panel, LV_HOR_RES);
    lv_obj_set_pos(g_app_sms_test_list_panel, 0, 20);
    lv_obj_set_style_border_width(g_app_sms_test_list_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(g_app_sms_test_list_panel, LV_BORDER_SIDE_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    g_app_sms_test_list_table = lv_table_create(g_app_sms_test_list_panel);
    lv_table_set_col_width(g_app_sms_test_list_table, 0, 230);
    lv_obj_set_pos(g_app_sms_test_list_table, -20, 0);
    lv_obj_set_style_border_width(g_app_sms_test_list_table, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(g_app_sms_test_list_table, LV_BORDER_SIDE_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_table_set_row_cnt(g_app_sms_test_list_table, 0);
    lv_table_set_col_cnt(g_app_sms_test_list_table, 1);
}

/**
 * app_sms_test_read_in_page - read sms in read page
 * @p_param: [IN] user data
 * @size: [IN] size of user data
 *
 * Return: NA
 */
static int32_t app_sms_test_read_in_page(void *p_param, uint32_t size)
{
    if (!p_param) {
        return -1;
    }
    app_logi("[em sms test]enter app_sms_test_read_in_page");
    ts_sms_read_in_sim_resp_t *resp = (ts_sms_read_in_sim_resp_t *)p_param;
    lv_obj_move_foreground(g_app_sms_test_read_panel);
    if (resp->time_stamp.year == 0 && resp->time_stamp.month == 0 && resp->time_stamp.day == 0) {
        app_sms_test_read_page(resp->number, "", (char *)&resp->p_content);
        g_app_sms_test_current_state = EM_SMS_READ_PAGE;
        return 0;
    }
    char timestamp_str[30] = {0};
    sprintf(timestamp_str, "%u-%02u-%02u %02u:%02u:%02u", resp->time_stamp.year, resp->time_stamp.month,
            resp->time_stamp.day, resp->time_stamp.hour, resp->time_stamp.min, resp->time_stamp.sec);
    app_logi("[em sms test]in g_app_sms_test_read_panel");
    app_sms_test_read_page(resp->number, timestamp_str, (char *)&resp->p_content);
    app_logi("[em sms test]number:%s,time:%s,text:%s", resp->number, timestamp_str, (char *)&resp->p_content);
    g_app_sms_test_current_state = EM_SMS_READ_PAGE;
    return 0;
}

/**
 * app_sms_test_read_in_list - read sms in list
 * @p_param: [IN] user data
 * @size: [IN] size of user data
 *
 * Return: NA
 */
static int32_t app_sms_test_read_in_list(void *p_param, uint32_t size)
{
    if (!p_param) {
        return -1;
    }
    // app_logi("[em sms test]enter app_sms_test_read_in_list");
    ts_sms_read_in_sim_resp_t *resp = (ts_sms_read_in_sim_resp_t *)p_param;
    if (!resp->base_info.err_code) {
        char temp[SMS_LIST_CONTENT_BYTE_LEN_MAX + 30] = {0};
        char content20[SMS_LIST_CONTENT_BYTE_LEN_MAX] = {0};
        char content23[SMS_LIST_CONTENT_BYTE_LEN_MAX + 3] = {0};

        if (_lv_txt_get_encoded_length((char *)&resp->p_content) > SMS_LIST_CONTENT_LEN_MAX) {
            uint32_t i = 0, j = 0;
            while (j < SMS_LIST_CONTENT_LEN_MAX) {
                _lv_txt_encoded_next((char *)&resp->p_content, &i);
                j++;
            }
            memcpy(content20, (char *)&resp->p_content, i);
            sprintf(content23, "%s%s", content20, "...");
        } else {
            strcpy(content23, (char *)&resp->p_content);
        }
        sprintf(temp, "%s  %s%s    %s", LV_SYMBOL_UPLOAD, resp->number, "\n", content23);
        lv_table_set_cell_value(g_app_sms_test_list_table, g_app_sms_test_table_row, 0, temp);
        app_logi("[em sms test]row:%d,index:%d,%s", g_app_sms_test_table_row, resp->index, temp);
        g_app_sms_test_table_index[g_app_sms_test_table_row] = resp->index;
        g_app_sms_test_table_row++;
        app_logi("[em sms test]g_app_sms_test_table_row=%d", g_app_sms_test_table_row);

        if (g_app_sms_test_index_count >= g_app_sms_test_list_count) {
            if (g_app_sms_test_msgbox_get_list) {
                app_logi("[em sms test]close the update list msgbox");
                lv_msgbox_close(g_app_sms_test_msgbox_get_list);
                g_app_sms_test_msgbox_get_list = NULL;
            }
            app_logi("[em sms test]read in list:Getting list finished,in list_page");
            lv_obj_set_user_data(g_app_sms_test_list_table, (void *)g_app_sms_test_table_index);
            return 0;
        }
        int32_t ret = aic_srv_tele_sms_read_in_sim(0, g_app_sms_test_list_index[g_app_sms_test_index_count], NULL);
        while (ret) {
            app_loge("[em sms test]app_sms_test_get_list:req read in sim error");
            app_logi("[em sms test]g_app_sms_test_index_count=%d,index=%d", g_app_sms_test_index_count,
                     g_app_sms_test_list_index[g_app_sms_test_index_count]);
            ret = aic_srv_tele_sms_read_in_sim(0, g_app_sms_test_list_index[g_app_sms_test_index_count], NULL);
        }
        g_app_sms_test_index_count++;
    } else {
        int32_t ret = aic_srv_tele_sms_read_in_sim(0, g_app_sms_test_list_index[g_app_sms_test_index_count], NULL);
        while (ret) {
            app_loge("[em sms test]app_sms_test_get_list:req read in sim error");
            if (g_app_sms_test_index_count >= g_app_sms_test_list_count) {
                if (g_app_sms_test_msgbox_get_list) {
                    app_logi("[em sms test]close the update list msgbox");
                    lv_msgbox_close(g_app_sms_test_msgbox_get_list);
                    g_app_sms_test_msgbox_get_list = NULL;
                }
                app_logi("[em sms test]read in list:Getting list finished,in list_page");
                return 0;
            }
            app_logi("[em sms test]g_app_sms_test_index_count=%d,index=%d", g_app_sms_test_index_count,
                     g_app_sms_test_list_index[g_app_sms_test_index_count]);
            ret = aic_srv_tele_sms_read_in_sim(0, g_app_sms_test_list_index[g_app_sms_test_index_count], NULL);
        }
        g_app_sms_test_index_count++;
    }
    return 0;
}

/**
 * app_sms_test_get_list - sms get list
 * @p_param: [IN] user data
 * @size: [IN] size of user data
 *
 * Return: NA
 */
static int32_t app_sms_test_get_list(void *p_param, uint32_t size)
{
    if (!p_param) {
        app_loge("[em sms test]app_sms_test_get_list:p_param=null");
        return -1;
    }
    app_logi("[em sms test]enter app_sms_test_get_list");
    ts_sms_get_list_in_sim_resp_t *resp = (ts_sms_get_list_in_sim_resp_t *)p_param;
    ts_sms_entry_t *list = (ts_sms_entry_t *)(&resp->list_start);

    app_logi("[em sms test]app_sms_test_get_list:resp->count=%d", resp->count);
    lv_table_set_row_cnt(g_app_sms_test_list_table, resp->count);
    bool a = lv_obj_remove_event_cb(g_app_sms_test_list_table, app_sms_test_event_cb_list);
    app_logi("[em sms test]lv_obj_remove_event_cb,a=%d", a);
    lv_obj_add_event_cb(g_app_sms_test_list_table, app_sms_test_event_cb_list, LV_EVENT_ALL, NULL);

    void *old_user_data = lv_obj_get_user_data(g_app_sms_test_list_table);
    if (old_user_data) {
        sms_test_free(old_user_data);
        lv_obj_set_user_data(g_app_sms_test_list_table, NULL);
    }
    if (g_app_sms_test_list_index) {
        sms_test_free(g_app_sms_test_list_index);
        g_app_sms_test_list_index = NULL;
    }
    if (resp->count > 0) {
        g_app_sms_test_list_count = resp->count;
        g_app_sms_test_table_row = 0;
        g_app_sms_test_list_index = (uint16_t *)sms_test_malloc(sizeof(uint16_t) * resp->count);
        g_app_sms_test_table_index = (uint16_t *)sms_test_malloc(sizeof(uint16_t) * resp->count);
        for (int i = 0; i < resp->count; i++) {
            g_app_sms_test_list_index[i] = list[i].index;
        }
        g_app_sms_test_index_count = 0;
        app_logi("[em sms test]g_app_sms_test_index_count=%d,index=%d", g_app_sms_test_index_count,
                    list[g_app_sms_test_index_count].index);
        int32_t ret = aic_srv_tele_sms_read_in_sim(0, list[g_app_sms_test_index_count].index, NULL);
        while (ret) {
            app_loge("[em sms test]app_sms_test_get_list:req read in sim error");
            app_logi("[em sms test]g_app_sms_test_index_count=%d,index=%d", g_app_sms_test_index_count,
                        list[g_app_sms_test_index_count].index);
            ret = aic_srv_tele_sms_read_in_sim(0, list[g_app_sms_test_index_count].index, NULL);
        }
        /* if requset of read_in_sim is successful,count++ */
        g_app_sms_test_index_count++;
    } else {
        app_logi("[em sms test]list_count=0;in list_page");
        if (g_app_sms_test_msgbox_get_list) {
            app_logi("[em sms test]close the update list msgbox");
            lv_msgbox_close(g_app_sms_test_msgbox_get_list);
            g_app_sms_test_msgbox_get_list = NULL;
        }
    }

    return 0;
}

/**
 * app_sms_test_get_smsc - sms get smsc
 * @p_param: [IN] user data
 * @size: [IN] size of user data
 *
 * Return: NA
 */
static int32_t app_sms_test_get_smsc(void *p_param, uint32_t size)
{
    if (!p_param) {
        return -1;
    }
    ts_sms_get_smsc_resp_t *resp = (ts_sms_get_smsc_resp_t *)p_param;
    lv_obj_clean(g_app_sms_test_smsc_panel);
    app_sms_test_smsc_page(resp->smsc);
    return 0;
}

/**
 * app_sms_test_delete - delete sms
 * @p_param: [IN] user data
 * @size: [IN] size of user data
 *
 * Return: NA
 */
static int32_t app_sms_test_delete(void *p_param, uint32_t size)
{
    if (!p_param) {
        return -1;
    }
    ts_sms_del_in_sim_resp_t *resp = (ts_sms_del_in_sim_resp_t *)p_param;
    if (!resp->base_info.err_code) {
        app_logi("[em sms test]create 'Delete SMS Succeeded!'msgbox");
        lv_obj_t *msgbox = lv_msgbox_create(NULL, NULL, "Delete SMS Succeeded!", g_app_sms_test_magbox_ok, false);
        lv_obj_set_align(msgbox, LV_ALIGN_CENTER);
        lv_obj_add_event_cb(msgbox, app_sms_test_event_cb_msgbox_delete_ok, LV_EVENT_VALUE_CHANGED,
                            (void *)SMS_OPERATION_SUCCEED);
    } else {
        app_logi("[em sms test]create 'Delete SMS Failed!'msgbox");
        lv_obj_t *msgbox = lv_msgbox_create(NULL, NULL, "Delete SMS Failed!", g_app_sms_test_magbox_ok, false);
        lv_obj_set_align(msgbox, LV_ALIGN_CENTER);
        lv_obj_add_event_cb(msgbox, app_sms_test_event_cb_msgbox_delete_ok, LV_EVENT_VALUE_CHANGED,
                            (void *)SMS_OPERATION_FAILED);
    }
    return 0;
}

/**
 * app_sms_test_send - send sms
 * @p_param: [IN] user data
 * @size: [IN] size of user data
 *
 * Return: NA
 */
static int32_t app_sms_test_send(void *p_param, uint32_t size)
{
    if (!p_param) {
        app_loge("[em sms test]app_sms_test_send:p_param=null");
        return -1;
    }
    ts_sms_send_resp_t *resp = (ts_sms_send_resp_t *)p_param;
    // app_logi("[em sms test]app_sms_test_send:resp->base_info.err_code=%d", resp->base_info.err_code);
    app_logi("[em sms test]close 'Waiting Send the SMS ...' msgbox");
    if (g_app_sms_test_send_msgbox) {
        lv_msgbox_close(g_app_sms_test_send_msgbox);
        g_app_sms_test_send_msgbox = NULL;
    }
    if (!resp->base_info.err_code) {
        // app_logi("[em sms test]send sms success :index(%d)", resp->index);
        app_logi("[em sms test]create 'Send SMS Succeeded!' msgbox");
        lv_obj_t *msgbox = lv_msgbox_create(NULL, NULL, "Send SMS Succeeded!", g_app_sms_test_magbox_ok, false);
        lv_obj_set_align(msgbox, LV_ALIGN_CENTER);
        lv_obj_add_event_cb(msgbox, app_sms_test_event_cb_msgbox_send, LV_EVENT_VALUE_CHANGED, SMS_OPERATION_SUCCEED);
    } else {
        app_logi("[em sms test]app_sms_test_send:create 'Send SMS Failed!' msgbox");
        lv_obj_t *msgbox = lv_msgbox_create(NULL, NULL, "Send SMS Failed!", g_app_sms_test_magbox_ok, false);
        lv_obj_set_align(msgbox, LV_ALIGN_CENTER);
        lv_obj_add_event_cb(msgbox, app_sms_test_event_cb_msgbox_send, LV_EVENT_VALUE_CHANGED, SMS_OPERATION_FAILED);
    }
    return 0;
}

/**
 * app_sms_test_new_in
 * @p_param: [IN] user data
 * @size: [IN] size of user data
 *
 * Return: NA
 */
static int32_t app_sms_test_new_in(void *p_param, uint32_t size)
{
    app_logi("[em sms test]a new sms");
    lv_obj_t *msgbox = lv_msgbox_create(NULL, NULL, "NEW SMS RECEIVED!", g_app_sms_test_magbox_ok, false);
    lv_obj_set_align(msgbox, LV_ALIGN_CENTER);
    app_logi("[em sms test]create 'NEW SMS RECEIVED!' msgbox");
    app_logi("[em sms test]g_app_sms_test_current_state=%d", g_app_sms_test_current_state);
    /* set the flag indicate that list need to be updated */
    g_app_sms_test_need_list_update = true;
    lv_obj_add_event_cb(msgbox, app_sms_test_event_cb_msgbox_new, LV_EVENT_VALUE_CHANGED, NULL);
    return 0;
}

static int32_t app_sms_test_sim_state(void *p_param, uint32_t size)
{
    if (!p_param) {
        return -1;
    }
    ts_sms_load_in_sim_resp_t *resp = (ts_sms_load_in_sim_resp_t *)p_param;
    app_logi("[em sms test]app_sms_test_cb:status response.");
    if (resp->is_full_in_sim) {
        app_logi("[em sms test]is_full,create 'SIM sms memory is full' msgbox");
        lv_obj_t *msgbox = lv_msgbox_create(NULL, NULL, "SIM sms memory is full", g_app_sms_test_magbox_ok, false);
        lv_obj_add_event_cb(msgbox, app_sms_test_event_cb_msgbox_full, LV_EVENT_VALUE_CHANGED, NULL);
        lv_obj_set_align(msgbox, LV_ALIGN_CENTER);
    }
    /* TODO: modify later. */
    if (1) {//!g_app_sms_test_sim_state_ready) {
        if (g_app_sms_test_msgbox_loading) {
            lv_msgbox_close(g_app_sms_test_msgbox_loading);
            g_app_sms_test_msgbox_loading = NULL;
        }
        g_app_sms_test_sim_state_ready = true;
        app_sms_test_write_smsc_btnm();
        ts_sms_get_list_info_t get_list_info = {0};
        get_list_info.card_id = 0;
        get_list_info.sms_type = TS_SMS_TYPE_ALL;
        int32_t ret = aic_srv_tele_sms_get_list_in_sim(&get_list_info, NULL);
        while (ret) {
            app_loge("[em sms test]app_sms_test_sim_state:req get list error");
            ret = aic_srv_tele_sms_get_list_in_sim(&get_list_info, NULL);
        }
    }

    return 0;
}

/**
 * app_sms_test_set_smsc - sms set smsc
 * @p_param: [IN] user data
 * @size: [IN] size of user data
 *
 * Return: NA
 */
static int32_t app_sms_test_set_smsc(void *p_param, uint32_t size)
{
    if (!p_param) {
        return -1;
    }
    ts_sms_set_smsc_resp_t *resp = (ts_sms_set_smsc_resp_t *)p_param;
    if (!resp->base_info.err_code) {
        app_logi("[em sms test]set smsc success");
        lv_obj_t *msgbox = lv_msgbox_create(NULL, NULL, "SET NEW SMSC Succeeded!", g_app_sms_test_magbox_ok, false);
        lv_obj_set_align(msgbox, LV_ALIGN_CENTER);
        lv_obj_add_event_cb(msgbox, app_sms_test_event_cb_msgbox_set_smsc, LV_EVENT_VALUE_CHANGED,
                            (void *)SMS_OPERATION_SUCCEED);
    } else {
        app_logi("[em sms test]set smsc error");
        lv_obj_t *msgbox = lv_msgbox_create(NULL, NULL, "SET NEW SMSC Failed!", g_app_sms_test_magbox_ok, false);
        lv_obj_set_align(msgbox, LV_ALIGN_CENTER);
        lv_obj_add_event_cb(msgbox, app_sms_test_event_cb_msgbox_set_smsc, LV_EVENT_VALUE_CHANGED, NULL);
    }
    return 0;
}

/**
 * sms_test_deinit - deinit sms test
 * @win: [IN] parent obj
 *
 * Return: null
 */
static void sms_test_deinit(void)
{
    int ret = aic_srv_tele_sms_unregister(g_sms_test_cb_handle);
    if (ret) {
        app_loge("[em sms test]aic_srv_tele_sms_unregister req unregister error");
    }
    if (g_app_sms_test_win) {
        lv_obj_del(g_app_sms_test_win);
    }
    g_app_sms_test_win = NULL;
    g_app_sms_test_panel = NULL;
    g_app_sms_test_list_panel = NULL;
    g_app_sms_test_list_table = NULL;
    g_app_sms_test_write_panel = NULL;
    g_app_sms_test_read_panel = NULL;
    g_app_sms_test_smsc_panel = NULL;
    g_app_sms_test_keyboard = NULL;
    g_app_sms_test_receiver = NULL;
    g_app_sms_test_msgbox_loading = NULL;
    if (g_app_sms_test_send_msgbox) {
        lv_msgbox_close(g_app_sms_test_send_msgbox);
        g_app_sms_test_send_msgbox = NULL;
    }
    if (g_app_sms_test_msgbox_get_list) {
        lv_msgbox_close(g_app_sms_test_msgbox_get_list);
        g_app_sms_test_msgbox_get_list = NULL;
    }
    if (g_app_sms_test_list_index) {
        sms_test_free(g_app_sms_test_list_index);
        g_app_sms_test_list_index = NULL;
    }
    g_app_sms_test_table_index = NULL;
    g_app_sms_test_btnm_action = NULL;
    app_logi("[em sms test]app_sms_test_deinit");
    g_app_sms_test_sim_state_ready = false;
}

/**
 * sms_test_init - init sms test
 * @win: [IN] parent obj
 *
 * Return: null
 */
static void sms_test_init(void)
{
    app_logi("[em sms test]Enter smstest");
    g_sms_test_cb_handle = aic_srv_tele_sms_register(NULL, app_sms_test_cb);
    if (g_sms_test_cb_handle == TS_SMS_INVALID_HANDLE) {
        app_loge("[em sms test]aic_srv_tele_sms_register req register error");
    }
    g_app_sms_test_win = lv_obj_create(NULL);
    g_app_sms_test_write_panel = lv_obj_create(g_app_sms_test_win);
    g_app_sms_test_read_panel = lv_obj_create(g_app_sms_test_win);
    g_app_sms_test_smsc_panel = lv_obj_create(g_app_sms_test_win);
    g_app_sms_test_panel = lv_obj_create(g_app_sms_test_win);
    g_app_sms_test_keyboard = lv_keyboard_create(g_app_sms_test_win);
    lv_obj_set_size(g_app_sms_test_keyboard, LV_HOR_RES, 90);
    lv_obj_add_flag(g_app_sms_test_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(g_app_sms_test_keyboard, app_sms_test_event_cb_kb, LV_EVENT_VALUE_CHANGED, NULL);

    g_app_sms_test_msgbox_loading = lv_msgbox_create(g_app_sms_test_win, NULL, "Loading...", NULL, false);
    lv_obj_set_width(g_app_sms_test_msgbox_loading, 200);
    lv_obj_set_align(g_app_sms_test_msgbox_loading, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_color(g_app_sms_test_msgbox_loading, lv_color_hex(0xDBDBDB), 0);
    app_sms_test_panel_page();
    g_app_sms_test_current_state = EM_SMS_LIST_PAGE;
    app_logi("[em sms test]empty list,in list_page");

    app_logi("[em sms test]end smstest init");
}

static void app_sms_test_focus(lv_obj_t *app, void *p_param)
{
    LV_UNUSED(app);
    LV_UNUSED(p_param);
    lv_disp_load_scr(g_app_sms_test_win);
}

static void app_sms_test_defocus(lv_obj_t *app, void *p_param)
{
    LV_UNUSED(app);
    LV_UNUSED(p_param);
}

static void app_sms_test_init(lv_obj_t *app, void *p_param)
{
    LV_UNUSED(p_param);
    sms_test_init();
}

static void app_sms_test_deinit(lv_obj_t *app, void *p_param)
{
    LV_UNUSED(p_param);
    sms_test_deinit();
}

/*******************************************
 *              GLOBAL FUNCTIONS
 ******************************************/
void app_sms_test_register(void)
{
    lv_obj_t *app = alv_am_app_create(lv_scr_act(), "sms_test");
    if (app) {
        alv_am_app_set_active(app, app_sms_test_init);
        alv_am_app_set_deactive(app, app_sms_test_deinit);
        alv_am_app_set_focus(app, app_sms_test_focus);
        alv_am_app_set_defocus(app, app_sms_test_defocus);
    } else {
        app_loge("Failed to start sms_test APP\n");
    }
}
