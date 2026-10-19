/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implements the functionalities of file browser app flow(fbaf).
 */

/*******************************************
 *              INCLUDES
 ******************************************/
#include "filebrowser_flow.h"
#include "alv_app_mgr.h"
#include "alv_btnmatrix.h"
#include "alv_menu.h"
#include "app_filebrowser/ui.h"
#include "filebrowser_service.h"
#include <aic_errno.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <ui_helpers.h>

/*******************************************
 *              DEFINES
 ******************************************/
#define FB_FOLDER_LIST_FILENAME_FONT &lv_font_montserrat_14
#define FB_FOLDER_LIST_DATETIME_FONT &lv_font_montserrat_10
#define FB_DEVICE_LIST_DEVICE_FONT &lv_font_montserrat_14
#define FB_DEVICE_LIST_CAPACITY_FONT &lv_font_montserrat_10
#define FB_FOLDER_THUMBNAIL_FILENAME_FONT &lv_font_montserrat_10
#define FB_FOLDER_THUMBNAIL_IMAGE_HEIGHT 48
#define FB_FOLDER_THUMBNAIL_SELECTED_BG_COLOR 0xCCE8EB
#define FB_MEN_ENTRY_KEY LV_KEY_ESC
#define FB_BACKWARDS_KEY LV_KEY_BACKSPACE
#define FB_SYMBOL_SELECTED_COLOR 0xE5F3FF
#define FB_CHECKBOX_COLOR 0x707070
#define FB_MSGBOX_BTN_FOCUS_COLOR 0xCCE8EB
#define FB_DETAIL_INFO_BUFFER_SIZE 500

/*******************************************
 *              TYPEDEFS
 ******************************************/
/**
 * enum - fb app ui states.
 *
 * @FB_APP_UI_STATE_UNKNOWN: initial state
 * @FB_APP_UI_STATE_DEVICE_LIST: device list
 * @FB_APP_UI_STATE_FOLDER_LIST: folder list
 */
enum fb_app_ui_fb_state {
    FB_APP_UI_STATE_DEVICE_LIST,
    FB_APP_UI_STATE_FOLDER_LIST,

    FB_APP_UI_STATE_UNKNOWN = 0xFFFF
};

/**
 * enum fb_app_menu_id - menu ids
 *
 * @FB_APP_MENU_OPEN: open / play file
 * @FB_APP_MENU_BACK_TO_NORMAL: go back to normal from multi-selection
 * @FB_APP_MENU_SELECT: go to multi-selection mode
 * @FB_APP_MENU_SELECT_ALL: select all items
 * @FB_APP_MENU_COPY: copy item(s)
 * @FB_APP_MENU_MOVE: move item(s)
 * @FB_APP_MENU_RENAME: rename
 * @FB_APP_MENU_DELETE: delete item(s)
 * @FB_APP_MENU_DELETE_ALL: delete all items
 * @FB_APP_MENU_DETAILS: show details
 * @FB_APP_MENU_NEW_FOLDER: create a new folder
 * @FB_APP_MENU_FORMAT: format the volume
 */
enum fb_app_menu_id {
    FB_APP_MENU_OPEN,
    FB_APP_MENU_BACK_TO_NORMAL,
    FB_APP_MENU_SELECT,
    FB_APP_MENU_SELECT_ALL,
    FB_APP_MENU_UNSELECT_ALL,
    FB_APP_MENU_COPY,
    FB_APP_MENU_MOVE,
    FB_APP_MENU_PASTE,
    FB_APP_MENU_RENAME,
    FB_APP_MENU_DELETE,
    FB_APP_MENU_DELETE_ALL,
    FB_APP_MENU_DETAILS,
    FB_APP_MENU_NEW_FOLDER,
    FB_APP_MENU_THUMBNAIL,
    FB_APP_MENU_LIST,
    FB_APP_MENU_FORMAT
};

/**
 * enum - fb app folder list mode.
 * @FB_APP_FL_MODE_LIST: list mode
 * @FB_APP_FL_MODE_THUMBNAIL: thumbnail mode
 */
enum fb_app_fl_mode {
    FB_APP_FL_MODE_LIST,
    FB_APP_FL_MODE_THUMBNAIL,

    FB_APP_FL_MODE_UNKNOWN = 0xFFFF
};

/**
 * enum - fb app state of format operation.
 * @FB_APP_FORMAT_STATE_UNKNOWN: initial state
 * @FB_APP_FORMAT_STATE_ONGOING: formating
 * @FB_APP_FORMAT_STATE_SUCCESS: succeeded
 * @FB_APP_FORMAT_STATE_FAIL: failed to format
 */
enum fb_app_format_state {
    FB_APP_FORMAT_STATE_UNKNOWN,
    FB_APP_FORMAT_STATE_ONGOING,
    FB_APP_FORMAT_STATE_SUCCESS,
    FB_APP_FORMAT_STATE_FAIL,
};

/**
 * enum - msgbox id.
 * @FB_APP_MSGBOX_ID_NOT_USED: reserved
 * @FB_APP_MSGBOX_ID_NOTIFICATION: common notification only
 * @FB_APP_MSGBOX_ID_FORMAT_SD: query to format sd card
 * @FB_APP_MSGBOX_ID_FORMAT_LOCAL: query to format local storage
 */
enum fb_app_msgbox_id {
    FB_APP_MSGBOX_ID_NOT_USED = 0,
    FB_APP_MSGBOX_ID_NOTIFICATION,
    FB_APP_MSGBOX_ID_FORMAT_SD,
    FB_APP_MSGBOX_ID_FORMAT_LOCAL,
    FB_APP_MSGBOX_ID_DELETE,
};

/**
 * enum - fb_app_entry_oper
 * @FB_APP_ENTRY_OPER_NONE: no operation
 * @FB_APP_ENTRY_OPER_COPY: copy
 * @FB_APP_ENTRY_OPER_MOVE: move
 */
enum fb_app_entry_oper {
    FB_APP_ENTRY_OPER_NONE,
    FB_APP_ENTRY_OPER_COPY,
    FB_APP_ENTRY_OPER_MOVE,
};
/**
 * struct fbaf_status - the app flow status of file browser
 * @is_suspended: true for suspended, false for running
 * @ui_fb_state: current UI page
 * @fl_mode: current folder mode, list of thumbnail
 * @cur_folder: the folder name in browsing
 * @cur_page: current folder page
 * @menu: current running menu, only one avail at most
 * @spinner: current running spinner, only one avail at most
 * @msgbox_list: msgbox list
 */
struct fbaf_status {
    bool is_suspended;
    enum fb_app_ui_fb_state ui_fb_state;
    enum fb_app_fl_mode fl_mode;
    char cur_folder[FB_MAX_PATH];
    lv_obj_t *cur_page;
    lv_obj_t *menu;
    lv_obj_t *spinner;
    lv_ll_t msgbox_list;
    enum fb_app_entry_oper entry_oper;
    struct fb_entry_list *copied_list;
    enum fb_app_format_state format_state;
};

/**
 * struct fbaf_folder_list_btn_attr - used to save btn attributes used in folder
 * list
 * @list: file list
 * @btnm_map: button map
 */
struct fbaf_folder_list_btn_attr {
    struct app_fs_scan_list_entry list;
    bool is_selecting;
    bool is_thumbnail;
    char **btnm_map;
};

/**
 * struct fbaf_device_list_btn_attr - used to save btn attributes used in device
 * list
 * @details: detail list
 * @btnm_map: button map
 */
struct fbaf_device_list_btn_attr {
    struct fb_volume_detail details[APP_FS_DEV_ID_CNT];
    char **btnm_map;
};

/*******************************************
 *              STATIC PROTOTYPES
 ******************************************/
/**
 * fbaf_enter_subfolder - enter new folder
 * @new_folder_name: the short name of the new folder
 *
 * Return: the new folder page to display
 */
static lv_obj_t *fbaf_enter_subfolder(const char *new_folder_name);
/**
 * ui_fb_btnmatrix_event_cb - handle the ui events triggered by folder
 * list item
 * @e: [IN] event
 *
 * Return: NA
 * Note: internal use only
 */
static void ui_fb_btnmatrix_event_cb(lv_event_t *e);
/**
 * ui_fb_device_list_event_cb - handle the ui events triggered by device
 * list item
 * @e: [IN] event
 *
 * Return: NA
 * Note: internal use only
 */
static void ui_fb_device_list_event_cb(lv_event_t *e);
/**
 * fbaf_enter_device_list - enter device list
 *
 * Return: NA
 * don't change ui page here, because the page will be switched by
 * animation.
 */
static void fbaf_enter_device_list(void);
/**
 * fbaf_create_msgbox - create message box
 * @msgbox_id: [IN] message box id
 * @info_txt: [IN] message info
 * @btn_txts: [IN] buttons of message box
 *
 * Return: NA
 * Note: internal use only
 */
static void fbaf_create_msgbox(enum fb_app_msgbox_id msgbox_id,
                               const char *info_txt, const char **btn_txts);
/**
 * fbaf_close_msgbox - close message box
 * @msgbox: [IN] pointer to the msgbox to close
 *
 * Return: NA
 * Note: internal use only
 */
static void fbaf_close_msgbox(lv_obj_t *msgbox);
/**
 * fbaf_create_info_msgbox - create a msgbox to show info only
 * @info_txt: [IN] message info
 *
 * Return: NA
 * Note: internal use only
 */
static void fbaf_create_info_msgbox(const char *info_txt);
/**
 * fbaf_compose_full_path - compose a full path
 * @dir: [IN] dir path
 * @entry_name: [IN] entry name
 *
 * Return: The composed full path. NULL for failure case
 * Note: the buffer of the short name should be freed manually
 */
static char *fbaf_compose_full_path(const char *dir, const char *entry_name);
/**
 * fbaf_refresh_folder_list - refresh the current folder
 *
 * Return: NA
 */
static void fbaf_refresh_folder_list(void);
/**
 * fbaf_change_display_mode - change the current folder display mode
 * @is_thumbnail: [IN] true to display as thumbnail, false for list
 *
 * Return: NA
 */
static void fbaf_change_display_mode(bool is_thumbnail);
/**
 * fbaf_enter_local_storage - enter local storage
 *
 * Return: NULL for failed, others for new page
 *
 */
static lv_obj_t *fbaf_enter_local_storage(void);
/**
 * fbaf_enter_sd_card - enter sd card
 *
 * Return: NULL for failed, others for new page
 */
static lv_obj_t *fbaf_enter_sd_card(void);
/**
 * fbaf_clean_menu - close menu
 *
 * Return: NA
 * Note: internal use only
 */
static void fbaf_clean_menu(void);
/*******************************************
 *              STATIC VARIABLES
 ******************************************/
/* pointer to app status */
static struct fbaf_status *g_cur_app_state = NULL;
static const char *g_msgbox_info_btns[] = {"OK", NULL};
static const char *g_msgbox_yesno_btns[] = {"Yes", "No", NULL};

/*******************************************
 *              STATIC FUNCTIONS
 ******************************************/
/**
 * fbaf_is_app_running - check if current fb app is running or not
 *
 * Return: true for running, false for maybe suspended or not exist
 */
static bool fbaf_is_app_running(void)
{
    bool result = false;
    if (g_cur_app_state && !g_cur_app_state->is_suspended) {
        result = true;
    }
    return result;
}

/**
 * fbaf_cal_volume_size_info - calcute the volume size info, including
 * decimal, fraction and unit
 * @number: [IN] capacity number of the device in bytes
 * @dec: [OUT] decimal value
 * @frac: [OUT] fraction value, take one decimal place
 * @unit: [OUT] unit
 * Return: NA
 */
static void fbaf_cal_volume_size_info(uint64_t number, int *dec, int *frac,
                                      char **unit)
{
#ifdef CONFIG_FB_VOLUME_SIZE_BASE_1000 /* customized by kconfig item */
    if (number >= 1000000000UL) {
        *dec = (int)(number / 1000000000UL);
        *frac = (int)(number - ((uint64_t)*dec * 1000000000UL)) / 100000000UL;
        *unit = "GB";
    } else if (number >= 1000000UL) {
        *dec = (int)(number / 1000000UL);
        *frac = (int)(number - ((uint64_t)*dec * 1000000UL)) / 100000UL;
        *unit = "MB";
    } else if (number >= 1000UL) {
        *dec = (int)(number / 1000UL);
        *frac = (int)(number - ((uint64_t)*dec * 1000UL)) / 100;
        *unit = "KB";
    } else {
        *dec = (int)number;
        *frac = 0;
        *unit = "B";
    }
#else
    if (number >= (uint64_t)(1 << 30)) {
        *dec = (int)(number >> 30);
        *frac = (int)(((number & 0x3FFFFFFF) * 10) >> 30);
        *unit = "GB";
    } else if (number >= (uint64_t)(1 << 20)) {
        *dec = (int)(number >> 20);
        *frac = (int)(((number & 0xFFFFF) * 10) >> 20);
        *unit = "MB";
    } else if (number >= (uint64_t)(1 << 10)) {
        *dec = (int)(number >> 10);
        *frac = (int)(((number & 0x3FF) * 10) >> 10);
        *unit = "KB";
    } else {
        *dec = (int)number;
        *frac = 0;
        *unit = "B";
    }
#endif
}

/**
 * fbaf_load_page - load page to show
 * @target: [IN] target page
 * @fademode: [IN] animation mode
 * @spd: [IN] Animation speed
 * @delay: [IN] delay to show
 *
 * Return: NA
 */
static void fbaf_load_page(lv_obj_t *target, lv_scr_load_anim_t fademode,
                           int spd, int delay)
{
    _ui_screen_change(target, fademode, spd, delay);
}

/**
 * fbaf_clean_unused_folder_list - clean unused folder list
 *
 * Return: NA
 */
static void fbaf_clean_unused_folder_list(void)
{
    if (g_cur_app_state->cur_page == ui_fb_folder_panel) {
        /* clean folder swap page*/
        lv_obj_clean(ui_fb_folder_swap_list_panel);
    } else {
        /* clean folder page */
        lv_obj_clean(ui_fb_folder_list_panel);
    }
}

/**
 * fbaf_get_list_item - get list item
 * @obj: [IN] pointer to the btnmatrix widget
 * @index: [IN] index of the item
 *
 * Return: NA
 * Note: internal use only
 */
static struct app_fs_scan_list_item *fbaf_get_list_item(lv_obj_t *obj,
                                                        int index)
{
    void *user_data = lv_obj_get_user_data(obj);
    struct app_fs_scan_list_item *result = NULL;

    if (user_data) {
        struct fbaf_folder_list_btn_attr *btn_attr =
            (struct fbaf_folder_list_btn_attr *)user_data;
        if (index > btn_attr->list.total) {
            return NULL;
        }
        if (index == btn_attr->list.total) {
            return btn_attr->list.last;
        }
        struct app_fs_scan_list_item *p = btn_attr->list.first;
        for (int i = 0; i < index; i++) {
            p = p->next;
        }
        result = p;
    }
    return result;
}

/**
 * fbaf_create_spinner - create a spinner
 *
 * Return: NA
 */
static void fbaf_create_spinner(void)
{
    if (g_cur_app_state && !g_cur_app_state->spinner) {
        /* create a mask parent at top layer */
        lv_obj_t *parent = lv_obj_create(lv_layer_top());
        if (!parent)
            return;
        lv_obj_clear_flag(parent, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_set_size(parent, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(parent, lv_palette_main(LV_PALETTE_GREY), 0);
        lv_obj_set_style_bg_opa(parent, LV_OPA_50, 0);
        lv_obj_t *spinner = lv_spinner_create(parent, 1000, 60);
        if (spinner) {
            lv_obj_set_size(spinner, 100, 100);
            lv_obj_center(spinner);
            /* get keyboard event */
            lv_group_add_obj(lv_group_get_default(), spinner);
            lv_group_focus_obj(spinner);
            g_cur_app_state->spinner = spinner;
        }
    }
}

/**
 * fbaf_clean_spinner - clean spinner
 *
 * Return: NA
 */
static void fbaf_clean_spinner(void)
{
    if (g_cur_app_state && g_cur_app_state->spinner) {
        lv_obj_del(lv_obj_get_parent(g_cur_app_state->spinner));
        g_cur_app_state->spinner = NULL;
    }
}

/**
 * fbaf_get_folder_list_btnm - get the btnmatrix of the current folder list
 *
 * Return: pointer to the btnmatrix, NULL for failure
 * Note: internal use only
 */
static lv_obj_t *fbaf_get_folder_list_btnm()
{
    lv_obj_t *btnm = NULL;
    if (FB_APP_UI_STATE_FOLDER_LIST == g_cur_app_state->ui_fb_state) {
        /* get the btnmatrix widget first */
        if (ui_fb_folder_panel == g_cur_app_state->cur_page) {
            btnm = lv_obj_get_child(ui_fb_folder_list_panel, 0);
        } else if (ui_fb_folder_swap_panel == g_cur_app_state->cur_page) {
            btnm = lv_obj_get_child(ui_fb_folder_swap_list_panel, 0);
        } else {
            app_loge("fbaf_get_folder_list_btnm: invalid page.\n");
        }
    }
    return btnm;
}
/**
 * fbaf_is_folder_list_selecting - check if selecting mode or not
 *
 * Return: true for selecting mode, else for normal mode
 * Note: internal use only
 */
static bool fbaf_is_folder_list_selecting(void)
{
    bool result = false;
    lv_obj_t *btnm = NULL;
    /* get the btnmatrix widget first */
    btnm = fbaf_get_folder_list_btnm();
    if (btnm) {
        /* change to checkable */
        lv_btnmatrix_set_btn_ctrl_all(btnm, LV_BTNMATRIX_CTRL_CHECKABLE);
        struct fbaf_folder_list_btn_attr *user_data =
            (struct fbaf_folder_list_btn_attr *)lv_obj_get_user_data(btnm);
        if (user_data) {
            result = user_data->is_selecting;
        }
    }

    return result;
}

/**
 * fbaf_enter_selection_mode - enter selection mode
 * @to_select: true to select, false to recover to normal
 *
 * Return: NA
 * Note: internal use only
 */
static void fbaf_enter_selection_mode(bool to_select)
{
    lv_obj_t *btnm = NULL;
    /* get the btnmatrix widget first */
    btnm = fbaf_get_folder_list_btnm();
    if (!btnm) {
        return;
    }
    if (to_select) {
        /* change to checkable */
        lv_btnmatrix_set_btn_ctrl_all(btnm, LV_BTNMATRIX_CTRL_CHECKABLE);
        struct fbaf_folder_list_btn_attr *user_data =
            (struct fbaf_folder_list_btn_attr *)lv_obj_get_user_data(btnm);
        if (user_data) {
            user_data->is_selecting = true;
        }
    } else {
        /* change to normal */
        lv_btnmatrix_clear_btn_ctrl_all(btnm, LV_BTNMATRIX_CTRL_CHECKED);
        lv_btnmatrix_set_btn_ctrl_all(btnm, LV_BTNMATRIX_CTRL_CLICK_TRIG |
                                                LV_BTNMATRIX_CTRL_NO_REPEAT);
        struct fbaf_folder_list_btn_attr *user_data =
            (struct fbaf_folder_list_btn_attr *)lv_obj_get_user_data(btnm);
        if (user_data) {
            user_data->is_selecting = false;
        }
    }

    /* get focus to receive keyboard events */
    lv_group_focus_obj(btnm);
}

/**
 * fbaf_folder_list_sel_all - select all
 * @to_sel: true to select all, false to unselect all
 * Return: NA
 * Note: internal use only
 */
static void fbaf_folder_list_sel_all(bool to_sel)
{
    lv_obj_t *btnm = NULL;
    /* get the btnmatrix widget first */
    btnm = fbaf_get_folder_list_btnm();
    if (!btnm) {
        return;
    }
    if (to_sel) {
        /* change to checkable */
        lv_btnmatrix_set_btn_ctrl_all(btnm, LV_BTNMATRIX_CTRL_CHECKED);
    } else {
        /* change to normal */
        lv_btnmatrix_clear_btn_ctrl_all(btnm, LV_BTNMATRIX_CTRL_CHECKED);
    }

    /* get focus to receive keyboard events */
    lv_group_focus_obj(btnm);
}

/**
 * fbaf_format_result_notifier - notifier from fb srv
 * @result: [IN] result of the format oper
 *
 * Return: NA
 * Note: this cb was triggered in non-app task, we must not do any UI change
 * directly here.
 */
static void fbaf_format_result_notifier(int result)
{
    g_cur_app_state->format_state = FB_APP_FORMAT_STATE_ONGOING;
    if (AIC_SUCCESS == result) {
        g_cur_app_state->format_state = FB_APP_FORMAT_STATE_SUCCESS;
    } else {
        g_cur_app_state->format_state = FB_APP_FORMAT_STATE_FAIL;
    }
}

/**
 * fbaf_format_state_query_handler - handle format state query timer
 * @timer: [IN] pointer to timer
 *
 * Return: NA
 *
 */
static void fbaf_format_state_query_handler(struct _lv_timer_t *timer)
{
    bool kill_timer = true;
    if (fbaf_is_app_running()) {
        switch (g_cur_app_state->format_state) {
        case FB_APP_FORMAT_STATE_SUCCESS:
            fbaf_clean_spinner();
            if (ui_fb_device_list_panel) {
                lv_obj_t *btnm = lv_obj_get_child(ui_fb_device_list_panel, 0);
                if (btnm != NULL) {
                    struct fbaf_device_list_btn_attr *user_data =
                        (struct fbaf_device_list_btn_attr *)
                            lv_obj_get_user_data(btnm);
                    if (user_data) {
                        for (int i = 0; i < (int)APP_FS_DEV_ID_CNT; i++) {
                            app_srv_fb_get_volume_detail(
                                (enum app_fs_dev_id)i, &user_data->details[i]);
                        }
                    }
                    lv_obj_invalidate(btnm);
                }
            }
            g_cur_app_state->format_state = FB_APP_FORMAT_STATE_UNKNOWN;
            fbaf_create_info_msgbox("Formatting is complete.");
            break;
        case FB_APP_FORMAT_STATE_FAIL:
            fbaf_clean_spinner();
            g_cur_app_state->format_state = FB_APP_FORMAT_STATE_UNKNOWN;
            fbaf_create_info_msgbox("Error: formatting is failed.");
            break;
        case FB_APP_FORMAT_STATE_ONGOING:
            kill_timer = false;
            break;
        default:
            break;
        }
    }
    if (kill_timer) {
        lv_timer_set_repeat_count(timer, 0);
    }
}

/**
 * fbaf_handle_delete_files - delete files handler
 *
 * Return: NA
 */
static void fbaf_handle_delete_files(void)
{
    /* get selected files */
    lv_obj_t *btnm = NULL;
    uint16_t i = 0;
    bool is_ok = false;
    /* get the btnmatrix widget first */
    btnm = fbaf_get_folder_list_btnm();
    if (!btnm) {
        return;
    }
    if (fbaf_is_folder_list_selecting()) {
        for (i = 0; i < alv_btnmatrix_get_btn_cnt(btnm); i++) {
            if (lv_btnmatrix_has_btn_ctrl(btnm, i, LV_BTNMATRIX_CTRL_CHECKED)) {
                const char *entry_name = lv_btnmatrix_get_btn_text(btnm, i);
                char *full_path = fbaf_compose_full_path(
                    g_cur_app_state->cur_folder, entry_name);
                if (full_path) {
                    is_ok = app_srv_fb_remove(full_path);
                    fb_app_free(full_path);
                } else {
                    is_ok = false;
                }
                if (!is_ok) {
                    break;
                }
            }
        }
    } else {
        i = lv_btnmatrix_get_selected_btn(btnm);
        if (i != LV_BTNMATRIX_BTN_NONE) {
            const char *entry_name = lv_btnmatrix_get_btn_text(btnm, i);
            char *full_path =
                fbaf_compose_full_path(g_cur_app_state->cur_folder, entry_name);
            if (full_path) {
                is_ok = app_srv_fb_remove(full_path);
                fb_app_free(full_path);
            }
        }
    }
    /* Regardless successful or not, refresh the folder list */
    fbaf_refresh_folder_list();
    if (!is_ok) {
        fbaf_create_info_msgbox("Failed to delete.");
    }
}
/**
 * fbaf_msgbox_event_cb - handle msg box events
 * @e: [IN] event
 *
 * Return: NA
 * Note: internal use only
 */
static void fbaf_msgbox_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    /* both LV_EVENT_VALUE_CHANGED and LV_EVENT_KEY events were triggered by
     * btns in msgbox, and bubbled to msgbox we need to use current_target to
     * get the obj of msgbox
     */
    lv_obj_t *obj = lv_event_get_current_target(e);

    if (LV_EVENT_VALUE_CHANGED == code) {
        enum fb_app_msgbox_id msgbox_id =
            (enum fb_app_msgbox_id)lv_event_get_user_data(e);
        enum app_fs_dev_id device_id = APP_FS_DEV_ID_SD;
        const char *btn_txt = lv_msgbox_get_active_btn_text(obj);
        if (!strcmp(btn_txt, "Yes")) {
            switch (msgbox_id) {
            case FB_APP_MSGBOX_ID_FORMAT_SD:
            case FB_APP_MSGBOX_ID_FORMAT_LOCAL:
                if (msgbox_id == FB_APP_MSGBOX_ID_FORMAT_LOCAL) {
                    device_id = APP_FS_DEV_ID_LOCAL;
                }
                g_cur_app_state->format_state = FB_APP_FORMAT_STATE_ONGOING;
                if (AIC_SUCCESS ==
                    app_srv_fb_format(device_id, fbaf_format_result_notifier)) {
                    fbaf_create_spinner();
                    lv_timer_create(fbaf_format_state_query_handler, 100, NULL);
                } else {
                    fbaf_create_info_msgbox("Failed to format.");
                }
                break;
            case FB_APP_MSGBOX_ID_DELETE:
                fbaf_handle_delete_files();
                break;
            default:
                break;
            }
        }

        fbaf_close_msgbox(obj);
    } else if (LV_EVENT_KEY == code) {
        char c = *((char *)lv_event_get_param(e));
        if (FB_BACKWARDS_KEY == c) {
            fbaf_close_msgbox(obj);
            /* ignore long press back */
            lv_indev_wait_release(lv_indev_get_act());
        }
    }
}

/**
 * fbaf_create_msgbox - create message box
 * @msgbox_id: [IN] message box id
 * @info_txt: [IN] message info
 * @btn_txts: [IN] buttons of message box
 *
 * Return: NA
 * Note: internal use only
 */
static void fbaf_create_msgbox(enum fb_app_msgbox_id msgbox_id,
                               const char *info_txt, const char **btn_txts)
{
    lv_obj_t *msg_box = NULL;

    msg_box = lv_msgbox_create(NULL, NULL, info_txt, btn_txts, true);
    if (msg_box) {
        lv_obj_center(msg_box);
        lv_obj_add_event_cb(msg_box, fbaf_msgbox_event_cb, LV_EVENT_ALL,
                            (void *)msgbox_id);
        lv_obj_set_style_bg_color(lv_msgbox_get_btns(msg_box),
                                  lv_color_hex(FB_MSGBOX_BTN_FOCUS_COLOR),
                                  LV_PART_ITEMS | LV_STATE_FOCUSED);
        /* get focus to receive keyboard events */
        lv_group_focus_obj(lv_msgbox_get_btns(msg_box));
        /* add to msgbox list */
        lv_obj_t **new_node =
            (lv_obj_t **)_lv_ll_ins_tail(&g_cur_app_state->msgbox_list);
        if (new_node) {
            *new_node = msg_box;
        } else {
            app_loge("fbaf_create_msgbox: failed to add to msgbox list\n");
        }
    }
}

/**
 * fbaf_close_msgbox - close message box
 * @msgbox: [IN] pointer to the msgbox to close
 *
 * Return: NA
 * Note: internal use only
 */
static void fbaf_close_msgbox(lv_obj_t *msgbox)
{
    if (!g_cur_app_state)
        return;
    void *iter = NULL;
    bool is_found = false;
    /* check if the same listener has been added before or not*/
    _LV_LL_READ(&g_cur_app_state->msgbox_list, iter)
    {
        lv_obj_t **p = (lv_obj_t **)iter;
        if (*p == msgbox) {
            is_found = true;
            break;
        }
    }
    if (is_found && iter) {
        _lv_ll_remove(&g_cur_app_state->msgbox_list, iter);
        lv_mem_free(iter);
    } else {
        app_loge("fbaf_close_msgbox: ERROR to find the msgbox.\n");
    }
    lv_msgbox_close(msgbox);
}

/**
 * fbaf_clean_msgbox - clean the message box
 *
 * Return: NA
 * Note: internal use only
 */
static void fbaf_clean_msgbox(void)
{
    if (!g_cur_app_state)
        return;
    void *iter = NULL;
    _LV_LL_READ(&g_cur_app_state->msgbox_list, iter)
    {
        lv_obj_t **p = (lv_obj_t **)iter;
        lv_msgbox_close(*p);
    }
    _lv_ll_clear(&g_cur_app_state->msgbox_list);
}

/**
 * fbaf_create_info_msgbox - create a msgbox to show info only
 * @info_txt: [IN] message info
 *
 * Return: NA
 * Note: internal use only
 */
static void fbaf_create_info_msgbox(const char *info_txt)
{
    fbaf_create_msgbox(FB_APP_MSGBOX_ID_NOTIFICATION, info_txt,
                       g_msgbox_info_btns);
}

/**
 * fbaf_handle_menu_open - menu open handler
 *
 * Return: NA
 * 1. If the selected item is in device list, go to open the selected volume
 * 2. If the selected item is in folder list
 *     a. A folder: open this folder
 *     b. A file: try to find the releted app to open this file, if not found,
 *         pop up a message to indicate the failure
 */
static void fbaf_handle_menu_open(void)
{
    /* device list or folder list */
    if (FB_APP_UI_STATE_DEVICE_LIST == g_cur_app_state->ui_fb_state) {
        /* get the selected item */
        lv_obj_t *btnm = lv_obj_get_child(ui_fb_device_list_panel, 0);
        if (btnm) {
            uint16_t sel_item = lv_btnmatrix_get_selected_btn(btnm);
            if (APP_FS_DEV_ID_LOCAL == (enum app_fs_dev_id)sel_item) {
                lv_obj_t *new_page = fbaf_enter_local_storage();
                if (new_page) {
                    fbaf_load_page(new_page, LV_SCR_LOAD_ANIM_OVER_LEFT, 500,
                                   0);
                }
            } else if (APP_FS_DEV_ID_SD == (enum app_fs_dev_id)sel_item) {
                /* sd card */
                lv_obj_t *new_page = fbaf_enter_sd_card();
                if (new_page) {
                    fbaf_load_page(new_page, LV_SCR_LOAD_ANIM_OVER_LEFT, 500,
                                   0);
                }
            }
        }
    } else if (FB_APP_UI_STATE_FOLDER_LIST == g_cur_app_state->ui_fb_state) {
        lv_obj_t *btnm = fbaf_get_folder_list_btnm();
        if (btnm) {
            struct fbaf_folder_list_btn_attr *user_data =
                (struct fbaf_folder_list_btn_attr *)lv_obj_get_user_data(btnm);
            if (!user_data || user_data->is_selecting) {
                return;
            }
            uint32_t id = lv_btnmatrix_get_selected_btn(btnm);
            struct app_fs_scan_list_item *list_item =
                fbaf_get_list_item(btnm, id);
            if (!list_item) {
                return;
            }
            if ((APP_FS_FILE_TYPE_FOLDER == list_item->type)) {
                /*enter folder*/
                lv_obj_t *new_page = fbaf_enter_subfolder(list_item->name);
                if (new_page) {
                    fbaf_load_page(new_page, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0);
                    fbaf_clean_unused_folder_list();
                }
            } else if (APP_FS_FILE_TYPE_FILE == list_item->type) {
                // TBD
                fbaf_create_info_msgbox("No available app to open this file.");
            }
        }
    }
}

/**
 * fbaf_handle_menu_details - menu details handler
 *
 * Return: NA
 */
static void fbaf_handle_menu_details(void)
{
    lv_obj_t *btnm = NULL;
    btnm = fbaf_get_folder_list_btnm();
    if (btnm) {
        struct fbaf_folder_list_btn_attr *user_data =
            (struct fbaf_folder_list_btn_attr *)lv_obj_get_user_data(btnm);
        if (!user_data || user_data->is_selecting) {
            return;
        }
        char *full_path = NULL;
        uint32_t id = lv_btnmatrix_get_selected_btn(btnm);
        struct app_fs_scan_list_item *list_item = fbaf_get_list_item(btnm, id);
        if (list_item != NULL) {
            full_path = fbaf_compose_full_path(g_cur_app_state->cur_folder,
                                               list_item->name);
            if (full_path) {
                int size_d, size_f;
                char *size_unit = "B";
                if (APP_FS_FILE_TYPE_FOLDER == list_item->type) {
                    struct fb_folder_detail detail;
                    if (app_srv_fb_get_folder_detail(full_path, &detail)) {
                        char *msg_info =
                            fb_app_malloc(FB_DETAIL_INFO_BUFFER_SIZE);
                        if (msg_info) {
                            fbaf_cal_volume_size_info((uint64_t)detail.size,
                                                      &size_d, &size_f,
                                                      &size_unit);
                            snprintf(
                                msg_info, FB_DETAIL_INFO_BUFFER_SIZE,
                                "Name: %s\n"
                                "Dir:      %s\n"
                                "Size:    %d.%02d %s\n"
                                "\n%d subfolder(s) and %d file(s) included",
                                list_item->name, g_cur_app_state->cur_folder,
                                size_d, size_f, size_unit, detail.subfolder_cnt,
                                detail.file_cnt);
                            fbaf_create_info_msgbox(msg_info);
                            fb_app_free(msg_info);
                        }
                    } else {
                        fbaf_create_info_msgbox("Failed to get the details.");
                    }
                } else if (APP_FS_FILE_TYPE_FILE == list_item->type) {
                    bool result = false;
                    char *msg_info = fb_app_malloc(FB_DETAIL_INFO_BUFFER_SIZE);
                    if (msg_info) {
                        struct fb_file_detail detail;
                        result = app_srv_fb_get_file_detail(full_path, &detail);
                        if (result) {
                            fbaf_cal_volume_size_info((uint64_t)detail.size,
                                                      &size_d, &size_f,
                                                      &size_unit);
                            if (detail.is_img) {
                                /* image file */
                                snprintf(msg_info, FB_DETAIL_INFO_BUFFER_SIZE,
                                         "Name: %s\n"
                                         "Dir:      %s\n"
                                         "Size:    %d.%02d %s\n\n"
                                         "Resolution: %ux%u",
                                         list_item->name,
                                         g_cur_app_state->cur_folder, size_d,
                                         size_f, size_unit, detail.img_w,
                                         detail.img_h);

                            } else {
                                /* normal file */
                                snprintf(msg_info, FB_DETAIL_INFO_BUFFER_SIZE,
                                         "Name: %s\n"
                                         "Dir:      %s\n"
                                         "Size:    %d.%02d %s\n\n",
                                         list_item->name,
                                         g_cur_app_state->cur_folder, size_d,
                                         size_f, size_unit);
                            }
                            fbaf_create_info_msgbox(msg_info);
                        }
                        fb_app_free(msg_info);
                    }
                    if (!result) {
                        fbaf_create_info_msgbox("Failed to get the details.");
                    }
                }
                fb_app_free(full_path);
            }
        }
    }
}

/**
 * fbaf_handle_menu_format - menu format handler
 *
 * Return: NA
 */
static void fbaf_handle_menu_format(void)
{
    /* which to format */
    lv_obj_t *btnm = lv_obj_get_child(ui_fb_device_list_panel, 0);
    if (!btnm)
        return;
    bool is_valid = true;
    enum app_fs_dev_id id =
        (enum app_fs_dev_id)lv_btnmatrix_get_selected_btn(btnm);
    enum fb_app_msgbox_id msgbox_id = FB_APP_MSGBOX_ID_NOTIFICATION;
    const char *info_txt = NULL;
    const char **btn_txts = NULL;

    if (APP_FS_DEV_ID_SD == id) {
        /* check if sd card is inserted or not */
        if (!app_fs_is_sd_mounted()) {
            info_txt = "No SD card is detected.";
            btn_txts = g_msgbox_info_btns;
            msgbox_id = FB_APP_MSGBOX_ID_NOTIFICATION;
        } else {
            info_txt = "Are you sure to format the SD card?";
            btn_txts = g_msgbox_yesno_btns;
            msgbox_id = FB_APP_MSGBOX_ID_FORMAT_SD;
        }
    } else if (APP_FS_DEV_ID_LOCAL == id) {
        info_txt = "Are you sure to format the local storage?";
        btn_txts = g_msgbox_yesno_btns;
        msgbox_id = FB_APP_MSGBOX_ID_FORMAT_LOCAL;
    } else {
        is_valid = false;
    }
    if (is_valid) {
        fbaf_create_msgbox(msgbox_id, info_txt, btn_txts);
    }
}

/**
 * fbaf_handle_menu_copy - menu copy handler
 * @is_move: true to move, false to copy
 *
 * Return: NA
 */
static void fbaf_handle_menu_copy(bool is_move)
{
    lv_obj_t *btnm = fbaf_get_folder_list_btnm();
    if (!btnm) {
        return;
    }
    /* get selected items */
    struct fb_entry_list *entry_list = app_srv_fb_create_entry_list();
    int i = 0;
    if (!entry_list) {
        return;
    }
    if (fbaf_is_folder_list_selecting()) {
        for (i = 0; i < alv_btnmatrix_get_btn_cnt(btnm); i++) {
            if (lv_btnmatrix_has_btn_ctrl(btnm, i, LV_BTNMATRIX_CTRL_CHECKED)) {
                struct app_fs_scan_list_item *item =
                    fbaf_get_list_item(btnm, i);
                const char *entry_name = item->name;
                char *full_path = fbaf_compose_full_path(
                    g_cur_app_state->cur_folder, entry_name);
                if (full_path) {
                    app_srv_fb_add_to_entry_list(entry_list, item->type,
                                                 full_path);
                    fb_app_free(full_path);
                }
            }
        }
    } else {
        i = lv_btnmatrix_get_selected_btn(btnm);
        if (i != LV_BTNMATRIX_BTN_NONE) {
            struct app_fs_scan_list_item *item = fbaf_get_list_item(btnm, i);
            const char *entry_name = item->name;
            char *full_path =
                fbaf_compose_full_path(g_cur_app_state->cur_folder, entry_name);
            if (full_path) {
                app_srv_fb_add_to_entry_list(entry_list, item->type, full_path);
                fb_app_free(full_path);
            }
        }
    }
    if (is_move) {
        g_cur_app_state->entry_oper = FB_APP_ENTRY_OPER_MOVE;
    } else {
        g_cur_app_state->entry_oper = FB_APP_ENTRY_OPER_COPY;
    }
    if (g_cur_app_state->copied_list) {
        /* remove the old copied list */
        app_srv_fb_entry_list_release(g_cur_app_state->copied_list);
    }
    app_srv_fb_entry_list_add_ref(entry_list);
    g_cur_app_state->copied_list = entry_list;
}

static void fbaf_handle_menu_paste(void)
{
    bool is_sub = false;
    /* check if the target folder is the subfolder of any of the source*/
    struct fb_entry_item *p = g_cur_app_state->copied_list->first;
    while (p != NULL) {
        if (APP_FS_FILE_TYPE_FOLDER == p->type) {
            if (strstr(g_cur_app_state->cur_folder, p->name)) {
                is_sub = true;
                break;
            }
        }
        p = p->next;
    }
    if (is_sub) {
        fbaf_create_info_msgbox("Failed to paste.\n\nReason: the destination "
                                "folder is a subfolder of the source folder.");
        return;
    }
    switch (g_cur_app_state->entry_oper) {
    case FB_APP_ENTRY_OPER_MOVE:
        app_srv_fb_move(g_cur_app_state->copied_list,
                        g_cur_app_state->cur_folder, NULL, NULL);
        break;
    case FB_APP_ENTRY_OPER_COPY:
        app_srv_fb_copy(g_cur_app_state->copied_list,
                        g_cur_app_state->cur_folder, NULL, NULL);
        break;
    default:
        break;
    }
}

/**
 * fbaf_clean_popup - clean up all pop-ups
 *
 * Return: NA
 * clean pop-up menus and message-boxes
 */
static void fbaf_clean_popup(void)
{
    fbaf_clean_msgbox();
    fbaf_clean_spinner();
    fbaf_clean_menu();
}

/**
 * fbaf_menu_event_cb - handle the menu selection
 * @e: [IN] event
 *
 * Return: NA
 * Note: internal use only
 */
static void fbaf_menu_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    uint32_t id = (uint32_t)lv_event_get_param(e);
    if ((uint32_t)code == ALV_MENU_ITEM_SELECTED) {
        fbaf_clean_menu();
        switch (id) {
        case FB_APP_MENU_OPEN:
            fbaf_handle_menu_open();
            break;
        case FB_APP_MENU_SELECT:
            fbaf_enter_selection_mode(true);
            break;
        case FB_APP_MENU_BACK_TO_NORMAL:
            fbaf_enter_selection_mode(false);
            break;
        case FB_APP_MENU_SELECT_ALL:
            fbaf_folder_list_sel_all(true);
            break;
        case FB_APP_MENU_UNSELECT_ALL:
            fbaf_folder_list_sel_all(false);
            break;
        case FB_APP_MENU_FORMAT:
            fbaf_handle_menu_format();
            break;
        case FB_APP_MENU_DELETE:
            fbaf_create_msgbox(FB_APP_MSGBOX_ID_DELETE,
                               "Are you sure to delete?", g_msgbox_yesno_btns);
            break;
        case FB_APP_MENU_THUMBNAIL:
            fbaf_change_display_mode(true);
            break;
        case FB_APP_MENU_LIST:
            fbaf_change_display_mode(false);
            break;
        case FB_APP_MENU_DETAILS:
            fbaf_handle_menu_details();
            break;
        case FB_APP_MENU_COPY:
            fbaf_handle_menu_copy(false);
            break;
        case FB_APP_MENU_MOVE:
            fbaf_handle_menu_copy(true);
            break;
        case FB_APP_MENU_PASTE:
            fbaf_handle_menu_paste();
            break;
        default:
            break;
        }
    }
}

/**
 * fbaf_create_menu - create menu according to app status
 *
 * Return: NA
 * Note: internal use only
 */
static void fbaf_create_menu(void)
{
    if (!g_cur_app_state || g_cur_app_state->menu) {
        return;
    }

    lv_obj_t *menu = alv_menu_create(NULL, false);
    if (menu) {
        switch (g_cur_app_state->ui_fb_state) {
        case FB_APP_UI_STATE_DEVICE_LIST:
            alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                              (uint32_t)FB_APP_MENU_OPEN, "Open",
                              LV_SYMBOL_PLAY);
            alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                              (uint32_t)FB_APP_MENU_FORMAT, "Format",
                              LV_SYMBOL_SD_CARD);
            break;
        case FB_APP_UI_STATE_FOLDER_LIST:
            if (fbaf_is_folder_list_selecting()) {
                alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                  (uint32_t)FB_APP_MENU_BACK_TO_NORMAL,
                                  "Back to normal", LV_SYMBOL_BACKSPACE);
                alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                  (uint32_t)FB_APP_MENU_SELECT_ALL,
                                  "Select all", LV_SYMBOL_ENVELOPE);
                alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                  (uint32_t)FB_APP_MENU_UNSELECT_ALL,
                                  "Unselect all", LV_SYMBOL_SHUFFLE);
                alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                  (uint32_t)FB_APP_MENU_COPY, "Copy",
                                  LV_SYMBOL_COPY);
                alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                  (uint32_t)FB_APP_MENU_MOVE, "Move",
                                  LV_SYMBOL_SHUFFLE);
                alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                  (uint32_t)FB_APP_MENU_DELETE, "Delete",
                                  LV_SYMBOL_TRASH);
            } else {
                if (fbaf_get_folder_list_btnm()) {
                    alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                      (uint32_t)FB_APP_MENU_OPEN, "Open",
                                      LV_SYMBOL_PLAY);
                    if (FB_APP_FL_MODE_LIST == g_cur_app_state->fl_mode) {
                        alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                          (uint32_t)FB_APP_MENU_THUMBNAIL,
                                          "Thumbnail", LV_SYMBOL_EYE_OPEN);
                    } else {
                        alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                          (uint32_t)FB_APP_MENU_LIST, "List",
                                          LV_SYMBOL_EYE_CLOSE);
                    }
                    alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                      (uint32_t)FB_APP_MENU_SELECT,
                                      "Select more", LV_SYMBOL_LIST);
                    alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                      (uint32_t)FB_APP_MENU_COPY, "Copy",
                                      LV_SYMBOL_COPY);
                    alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                      (uint32_t)FB_APP_MENU_MOVE, "Move",
                                      LV_SYMBOL_SHUFFLE);
                    if (g_cur_app_state->entry_oper != FB_APP_ENTRY_OPER_NONE) {
                        alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                          (uint32_t)FB_APP_MENU_PASTE, "Paste",
                                          LV_SYMBOL_PASTE);
                    }
                    alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                      (uint32_t)FB_APP_MENU_DELETE, "Delete",
                                      LV_SYMBOL_TRASH);
                    alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                      (uint32_t)FB_APP_MENU_DELETE_ALL,
                                      "Delete all", LV_SYMBOL_TRASH);

                    alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                      (uint32_t)FB_APP_MENU_DETAILS, "Details",
                                      LV_SYMBOL_ENVELOPE);
                } else {
                    /* no items inside the folder */
                    if (FB_APP_FL_MODE_LIST == g_cur_app_state->fl_mode) {
                        alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                          (uint32_t)FB_APP_MENU_THUMBNAIL,
                                          "Thumbnail", LV_SYMBOL_EYE_OPEN);
                    } else {
                        alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                          (uint32_t)FB_APP_MENU_LIST, "List",
                                          LV_SYMBOL_EYE_CLOSE);
                    }
                    if (g_cur_app_state->entry_oper != FB_APP_ENTRY_OPER_NONE) {
                        alv_menu_add_item(menu, ALV_MENU_ITEM_TYPE_BTN,
                                          (uint32_t)FB_APP_MENU_PASTE, "Paste",
                                          LV_SYMBOL_PASTE);
                    }
                }
            }
            break;
        default:
            break;
        }
        lv_obj_add_event_cb(menu, fbaf_menu_event_cb, ALV_MENU_ITEM_SELECTED,
                            NULL);
        g_cur_app_state->menu = menu;
    }
}

/**
 * fbaf_clean_menu - close menu
 *
 * Return: NA
 * Note: internal use only
 */
static void fbaf_clean_menu(void)
{
    if (g_cur_app_state && g_cur_app_state->menu) {
        alv_menu_close(g_cur_app_state->menu);
        g_cur_app_state->menu = NULL;
    }
}

/**
 * sd_mount_listener - listener to sd mount
 * @mount: [IN] mount or not
 * @user_data: [IN] user_data
 *
 * Return: NA
 */
static void sd_mount_listener(bool mount, void *user_data)
{
    if (g_cur_app_state) {
        switch (g_cur_app_state->ui_fb_state) {
        case FB_APP_UI_STATE_DEVICE_LIST:
            if (ui_fb_device_list_panel) {
                lv_obj_t *btnm = lv_obj_get_child(ui_fb_device_list_panel, 0);
                if (btnm != NULL) {
                    struct fbaf_device_list_btn_attr *user_data =
                        (struct fbaf_device_list_btn_attr *)
                            lv_obj_get_user_data(btnm);
                    if (user_data) {
                        struct fb_volume_detail volume_detail = {0};
                        app_srv_fb_get_volume_detail(APP_FS_DEV_ID_SD,
                                                     &volume_detail);

                        user_data->details[APP_FS_DEV_ID_SD].capacity =
                            volume_detail.capacity;
                        user_data->details[APP_FS_DEV_ID_SD].avail =
                            volume_detail.avail;
                        user_data->details[APP_FS_DEV_ID_SD].used =
                            volume_detail.used;
                    }
                    lv_obj_invalidate(btnm);
                }
            }
            break;
        case FB_APP_UI_STATE_FOLDER_LIST:
            /* check if the current dir is still valid or not */
            if (!app_srv_fb_is_valid_folder(g_cur_app_state->cur_folder)) {
                fbaf_clean_popup();
                fbaf_enter_device_list();
                fbaf_load_page(ui_fb_device_panel, LV_SCR_LOAD_ANIM_OVER_RIGHT,
                               500, 0);
            }
            break;
        default:
            break;
        }
    }
}
/**
 * fbaf_clear_folder_list_user_data - clean user data of folder list buttons
 * @user_data: [IN] user data
 *
 * Return: NA
 */
static void fbaf_clean_folder_list_user_data(void *user_data)
{
    struct fbaf_folder_list_btn_attr *btn_attr =
        (struct fbaf_folder_list_btn_attr *)user_data;
    /* clear old one */
    if (btn_attr) {
        app_srv_fb_free_scan_list(&btn_attr->list);
        if (btn_attr->btnm_map) {
            fb_app_free(btn_attr->btnm_map);
            btn_attr->btnm_map = NULL;
        }
        fb_app_free(btn_attr);
    }
}

/**
 * fbaf_update_state_machine - change app states
 * @state: [IN] ui state
 * @cur_folder: [IN] full path of current folder
 * @cur_page: [IN] current page
 *
 * Return: true for valid, false for not
 * Note: internal use only
 */
static void fbaf_update_state_machine(enum fb_app_ui_fb_state state,
                                      const char *cur_folder,
                                      lv_obj_t *cur_page)
{
    if (g_cur_app_state) {
        g_cur_app_state->ui_fb_state = state;
        strcpy(g_cur_app_state->cur_folder, cur_folder);
        g_cur_app_state->cur_page = cur_page;
    }
}

/**
 * fbaf_extract_folder_name - extract the short folder name from full path
 * @full_path: [IN] the full path of the folder
 * @parent: [OUT] parent name
 *
 * Return: true for device list, false for folder list
 *
 * WANNING: If the current folder is already root, the parent dir is the
 * device list. The buffer of pointer parent should be freed manually only
 * when parent is not device list.
 */
static bool fbaf_get_parent_dir(const char *full_path, char **parent)
{
    bool result = false;

    if (full_path) {
        int len = strlen(full_path);

        if (len) {
            if (FB_FILE_SEPERATOR_CHR == full_path[len - 1]) {
                /* device root */
                result = true;
            } else {
                const char *p = strrchr(full_path, FB_FILE_SEPERATOR_CHR);
                if (p) {
                    int new_len = p - full_path + 1;

                    *parent = fb_app_malloc(new_len + 1);
                    if (*parent) {
                        memcpy(*parent, full_path, new_len - 1);
                        (*parent)[new_len - 1] = 0;
                        /* add file seperator in case X: */
                        if (!strchr(*parent, FB_FILE_SEPERATOR_CHR)) {
                            (*parent)[new_len - 1] = FB_FILE_SEPERATOR_CHR;
                            (*parent)[new_len] = 0;
                        }
                    }
                } else {
                    result = true;
                }
            }
        }
    }

    return result;
}

/**
 * fbaf_extract_folder_name - extract the short folder name from full path
 * @full_path: [IN] the full path of the folder
 * @short_name: [OUT] extracted name
 *
 * Return: true for success, false for failure
 * Note: the buffer of the short name should be freed manually
 */
static bool fbaf_extract_folder_name(const char *full_path, char **short_name)
{
    bool result = false;

    if (full_path) {
        int len = strlen(full_path);

        if (len) {
            if (FB_FILE_SEPERATOR_CHR == full_path[len - 1]) {
                /* device root */
                *short_name = fb_app_malloc(len + 1);
                if (*short_name) {
                    memcpy(*short_name, full_path, len - 1);
                    (*short_name)[len - 1] = 0;
                    result = true;
                }
            } else {
                const char *p = strrchr(full_path, FB_FILE_SEPERATOR_CHR);
                if (p) {
                    *short_name = fb_app_malloc(strlen(p));
                    if (*short_name) {
                        strcpy(*short_name, p + 1);
                        result = true;
                    }
                } else {
                    /* device root */
                    *short_name = fb_app_malloc(len + 1);
                    if (*short_name) {
                        strcpy(*short_name, full_path);
                        result = true;
                    }
                }
            }
        }
    }

    return result;
}

/**
 * fbaf_compose_full_path - compose a full path
 * @dir: [IN] dir path
 * @entry_name: [IN] entry name
 *
 * Return: The composed full path. NULL for failure case
 * Note: the buffer of the short name should be freed manually
 */
static char *fbaf_compose_full_path(const char *dir, const char *entry_name)
{
    char *result = NULL;
    if (!dir || !entry_name) {
        return NULL;
    }
    int len = 0;
    int dir_len = strlen(dir);
    len = dir_len + strlen(entry_name) + 1 + 1;
    result = fb_app_malloc(len);
    if (result) {
        strcpy(result, dir);
        if (FB_FILE_SEPERATOR_CHR != dir[dir_len - 1]) {
            strcat(result, FB_FILE_SEPERATOR);
        }
        strcat(result, entry_name);
    }
    return result;
}

/**
 * fbaf_format_storage_info_txt - format time_t value to date time string
 * @capacity: [IN] capacity of the device in bytes
 * @avail: [IN] avail bytes
 * @storage_info: [OUT] storage info
 *
 * Return: pointer to the formated storage info string
 */
static char *fbaf_format_storage_info_txt(uint64_t capacity, uint64_t avail,
                                          char *storage_info)
{
    char *result = storage_info;
    char *total_unit = "B";
    char *avail_unit = "B";
    int total_d, total_f, avail_d, avail_f;

    if (capacity == 0 && avail == 0) {
        strcpy(storage_info, "Unknown capacity");
        return result;
    }
    fbaf_cal_volume_size_info(capacity, &total_d, &total_f, &total_unit);
    fbaf_cal_volume_size_info(avail, &avail_d, &avail_f, &avail_unit);
    sprintf(storage_info, "%d.%d %s available/%d.%d %s total", avail_d, avail_f,
            avail_unit, total_d, total_f, total_unit);
    return result;
}

/**
 * fbaf_format_date_time - format time_t value to date time string
 * @m_time: [IN] time_t value time
 * @date_time: [OUT] output date time string
 *
 * Return: pointer to the formated date time string
 */
static char *fbaf_format_date_time(time_t m_time, char *date_time)
{
    char *result = date_time;

#ifdef WIN32
    struct tm *timeptr = NULL;
    timeptr = localtime(&m_time); /* non-thread safe API, but current MINGW
                                     does not support localtime_r */
    strftime(date_time, 32, "%Y/%m/%d %H:%M:%S", timeptr);
#else
    struct tm time_local = {0};
    localtime_r(&m_time, &time_local);
    strftime(date_time, 32, "%Y/%m/%d %H:%M:%S", &time_local);
#endif
    return result;
}

/**
 * fbaf_create_folder_list_pages - init folder lists
 *
 * Return: true for success, false for failure
 *
 */
static bool fbaf_create_folder_list_pages(void)
{
    bool result = false;

    if (!ui_fb_folder_swap_panel) {
        ui_fb_folder_swap_screen_init();
    }

    if (!ui_fb_folder_panel) {
        ui_fb_folder_screen_init();
    }

    if (ui_fb_folder_swap_panel && ui_fb_folder_panel) {
        result = true;
    }

    return result;
}

/**
 * fbaf_update_folder_list - update folder list
 * @folder: [IN] the full path of the folder
 *
 * Return: the new page
 */
static lv_obj_t *fbaf_update_folder_list(const char *folder)
{
    bool is_ok = true;
    lv_obj_t *dest_list_panel = NULL;
    lv_obj_t *update_page = NULL;
    lv_obj_t *title_label = NULL;
    struct fbaf_folder_list_btn_attr *user_data = NULL;

    if (!app_srv_fb_is_valid_folder(folder) ||
        !fbaf_create_folder_list_pages()) {
        return NULL;
    }

    user_data = (struct fbaf_folder_list_btn_attr *)fb_app_malloc(
        sizeof(struct fbaf_folder_list_btn_attr));
    if (!user_data) {
        return NULL;
    }
    memset(user_data, 0, sizeof(struct fbaf_folder_list_btn_attr));
    /* determine which folder to be update */
    if (g_cur_app_state->cur_page == ui_fb_folder_panel) {
        update_page = ui_fb_folder_swap_panel;
        dest_list_panel = ui_fb_folder_swap_list_panel;
        title_label = ui_fb_folder_swap_title_label;
    } else {
        update_page = ui_fb_folder_panel;
        dest_list_panel = ui_fb_folder_list_panel;
        title_label = ui_fb_folder_title_label;
    }
    /* clear old list panel */
    if (dest_list_panel != NULL) {
        lv_obj_clean(dest_list_panel);
    }
    lv_obj_set_scrollbar_mode(dest_list_panel, LV_SCROLLBAR_MODE_ACTIVE);
    app_srv_fb_scandir(folder, false, &user_data->list);
    if (user_data->list.total > 0) {
        /*Add items to the list*/
        struct app_fs_scan_list_item *p = NULL;
        lv_obj_t *btnm = alv_btnmatrix_create(dest_list_panel);
        if (btnm) {
            /* calc size of the widget */
            if (FB_APP_FL_MODE_LIST == g_cur_app_state->fl_mode) {
                /* 2 px top padding, 2 px bottom padding for both. 8 px in
                 * total*/
                lv_obj_set_size(
                    btnm, LV_PCT(100),
                    user_data->list.total *
                        (lv_font_get_line_height(FB_FOLDER_LIST_FILENAME_FONT) +
                         lv_font_get_line_height(FB_FOLDER_LIST_DATETIME_FONT) +
                         8));
                user_data->btnm_map = (char **)fb_app_malloc(
                    sizeof(char *) * user_data->list.total * 2);
                p = user_data->list.first;
                for (int i = 0, j = 0; i < user_data->list.total; i++, j += 2) {
                    user_data->btnm_map[j] = p->name;
                    user_data->btnm_map[j + 1] = "\n";
                    p = p->next;
                }
                user_data->btnm_map[user_data->list.total * 2 - 1] = NULL;
                user_data->is_thumbnail = false;
                user_data->is_selecting = false;
                lv_btnmatrix_set_map(btnm, (const char **)user_data->btnm_map);
            } else {
                int total_line = (user_data->list.total + 2) / 3;
                /* image height + font height + 2 px apart*/
                int line_height =
                    FB_FOLDER_THUMBNAIL_IMAGE_HEIGHT +
                    lv_font_get_line_height(FB_FOLDER_THUMBNAIL_FILENAME_FONT) +
                    2;
                int j = 0;
                lv_obj_set_size(btnm, LV_PCT(100), total_line * line_height);
                user_data->btnm_map =
                    (char **)fb_app_malloc(sizeof(char *) * (total_line * 4));
                p = user_data->list.first;
                for (j = 0; p != NULL; j++) {
                    if ((j & 0x3) != 0x3) {
                        user_data->btnm_map[j] = p->name;
                        p = p->next;
                    } else {
                        user_data->btnm_map[j] = "\n";
                    }
                }
                while (j < total_line * 4 - 1) {
                    user_data->btnm_map[j++] = "dummy";
                }
                user_data->btnm_map[total_line * 4 - 1] = NULL;
                user_data->is_thumbnail = true;
                user_data->is_selecting = false;
                lv_btnmatrix_set_map(btnm, (const char **)user_data->btnm_map);
                j = user_data->list.total;
                while (j < total_line * 3) {
                    lv_btnmatrix_set_btn_ctrl(btnm, j,
                                              LV_BTNMATRIX_CTRL_HIDDEN);
                    j++;
                }
            }
            lv_btnmatrix_set_btn_ctrl_all(btnm,
                                          LV_BTNMATRIX_CTRL_CLICK_TRIG |
                                              LV_BTNMATRIX_CTRL_NO_REPEAT);
            lv_obj_set_user_data(btnm, (void *)user_data);
            lv_obj_add_event_cb(btnm, ui_fb_btnmatrix_event_cb, LV_EVENT_ALL,
                                NULL);
            /* get focus to receive keyboard events */
            lv_group_focus_obj(btnm);
        } else {
            is_ok = false;
        }
    } else {
        /* free non-used user data */
        fb_app_free(user_data);
    }

    if (is_ok) {
        /*set title name*/
        char *title_name = NULL;
        fbaf_extract_folder_name(folder, &title_name);
        if (title_name) {
            lv_label_set_text(title_label, title_name);
            fb_app_free(title_name);
        }
    } else {
        fbaf_clean_folder_list_user_data(user_data);
    }

    return update_page;
}

/**
 * fbaf_refresh_folder_list - refresh the current folder
 *
 * Return: NA
 */
static void fbaf_refresh_folder_list(void)
{
    /*re-enter folder*/
    lv_obj_t *new_page = fbaf_update_folder_list(g_cur_app_state->cur_folder);
    if (new_page) {
        fbaf_update_state_machine(FB_APP_UI_STATE_FOLDER_LIST,
                                  g_cur_app_state->cur_folder, new_page);
        fbaf_load_page(new_page, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0);
        fbaf_clean_unused_folder_list();
    }
}

/**
 * fbaf_change_display_mode - change the current folder display mode
 * @is_thumbnail: [IN] true to display as thumbnail, false for list
 *
 * Return: NA
 */
static void fbaf_change_display_mode(bool is_thumbnail)
{
    g_cur_app_state->fl_mode =
        is_thumbnail ? FB_APP_FL_MODE_THUMBNAIL : FB_APP_FL_MODE_LIST;
    fbaf_refresh_folder_list();
}

/**
 * fbaf_enter_local_storage - enter local storage
 *
 * Return: NULL for failed, others for new page
 *
 */
static lv_obj_t *fbaf_enter_local_storage(void)
{
    lv_obj_t *result = NULL;
    char volume[APP_FS_VOLUME_LEN] = {0};

    if (AIC_SUCCESS == app_fs_get_volume(APP_FS_DEV_ID_LOCAL, volume)) {
        result = fbaf_update_folder_list(volume);
        if (result) {
            /* clear device list */
            if (ui_fb_device_list_panel != NULL) {
                lv_obj_clean(ui_fb_device_list_panel);
            }
            fbaf_update_state_machine(FB_APP_UI_STATE_FOLDER_LIST, volume,
                                      result);
        }
    }

    return result;
}

/**
 * fbaf_enter_sd_card - enter sd card
 *
 * Return: NULL for failed, others for new page
 */
static lv_obj_t *fbaf_enter_sd_card(void)
{
    lv_obj_t *result = NULL;
    char volume[APP_FS_VOLUME_LEN] = {0};

    if (AIC_SUCCESS == app_fs_get_volume(APP_FS_DEV_ID_SD, volume)) {
        result = fbaf_update_folder_list(volume);
        if (result) {
            /* clear device list */
            if (ui_fb_device_list_panel != NULL) {
                lv_obj_clean(ui_fb_device_list_panel);
            }
            fbaf_update_state_machine(FB_APP_UI_STATE_FOLDER_LIST, volume,
                                      result);
        }
    }

    return result;
}

/**
 * fbaf_enter_device_list - enter device list
 *
 * Return: NA
 * don't change ui page here, because the page will be switched by
 * animation.
 */
static void fbaf_enter_device_list(void)
{
    /* clear folder list items */
    if (ui_fb_folder_list_panel)
        lv_obj_clean(ui_fb_folder_list_panel);
    if (ui_fb_folder_swap_list_panel)
        lv_obj_clean(ui_fb_folder_swap_list_panel);

    /* init list */
    int i = 0, j = 0;
    struct fbaf_device_list_btn_attr *user_data =
        (struct fbaf_device_list_btn_attr *)fb_app_malloc(
            sizeof(struct fbaf_device_list_btn_attr));
    if (!user_data) {
        return;
    }
    for (i = 0; i < (int)APP_FS_DEV_ID_CNT; i++) {
        app_srv_fb_get_volume_detail((enum app_fs_dev_id)i,
                                     &user_data->details[i]);
    }
    /* clear old list panel */
    if (ui_fb_device_list_panel != NULL) {
        lv_obj_clean(ui_fb_device_list_panel);
    }
    /*Add items to the list*/
    lv_obj_t *btnm = alv_btnmatrix_create(ui_fb_device_list_panel);
    if (btnm) {
        /* calc size of the widget */
        lv_obj_set_size(
            btnm, LV_PCT(100),
            (lv_coord_t)APP_FS_DEV_ID_CNT *
                (lv_font_get_line_height(FB_FOLDER_LIST_FILENAME_FONT) +
                 lv_font_get_line_height(FB_FOLDER_LIST_DATETIME_FONT) +
                 8)); /* 2 px top padding, 2 px bottom padding for both.
                         8 px in total*/
        user_data->btnm_map = (char **)fb_app_malloc(
            sizeof(char *) * (size_t)APP_FS_DEV_ID_CNT * 2);
        for (i = 0, j = 0; i < (int)APP_FS_DEV_ID_CNT; i++, j += 2) {
            user_data->btnm_map[j] = " "; /* place holder */
            user_data->btnm_map[j + 1] = "\n";
        }
        user_data->btnm_map[APP_FS_DEV_ID_CNT * 2 - 1] = NULL;
        lv_btnmatrix_set_map(btnm, (const char **)user_data->btnm_map);
        lv_btnmatrix_set_btn_ctrl_all(btnm, LV_BTNMATRIX_CTRL_CLICK_TRIG |
                                                LV_BTNMATRIX_CTRL_NO_REPEAT);
        lv_obj_set_user_data(btnm, (void *)user_data);
        lv_obj_add_event_cb(btnm, ui_fb_device_list_event_cb, LV_EVENT_ALL,
                            NULL);
        /* get focus to receive keyboard events */
        lv_group_focus_obj(btnm);
    }

    fbaf_update_state_machine(FB_APP_UI_STATE_DEVICE_LIST, "",
                              ui_fb_device_panel);
}

/**
 * fbaf_enter_subfolder - enter new folder
 * @new_folder_name: the short name of the new folder
 *
 * Return: the new folder page to display
 */
static lv_obj_t *fbaf_enter_subfolder(const char *new_folder_name)
{
    char *full_path = NULL;
    int path_len = 0;
    int new_folder_name_len = 0;
    int cur_folder_path_len = strlen(g_cur_app_state->cur_folder);
    lv_obj_t *new_page = NULL;

    if (new_folder_name) {
        new_folder_name_len = strlen(new_folder_name);
    }
    if (!new_folder_name_len) {
        return NULL;
    }
    /* check if the string ends with file seperator*/
    if (cur_folder_path_len > 0 &&
        0 == strcmp(&g_cur_app_state->cur_folder[cur_folder_path_len - 1],
                    FB_FILE_SEPERATOR)) {
        cur_folder_path_len--;
        g_cur_app_state->cur_folder[cur_folder_path_len] = 0;
    }
    path_len = cur_folder_path_len + new_folder_name_len + 1;
    if (path_len >= FB_MAX_PATH - 1) {
        return NULL;
    }
    full_path = fb_app_malloc(path_len + 1);
    if (full_path) {
        strcpy(full_path, g_cur_app_state->cur_folder);
        strcat(full_path, FB_FILE_SEPERATOR);
        strcat(full_path, new_folder_name);
        new_page = fbaf_update_folder_list(full_path);
        if (new_page) {
            fbaf_update_state_machine(FB_APP_UI_STATE_FOLDER_LIST, full_path,
                                      new_page);
        }
        fb_app_free(full_path);
    } else {
        app_loge("!!WARNING: unable to malloc now");
    }

    return new_page;
}

/**
 * fbaf_handle_folder_list_back - handle the ui events triggered by back
 * button in folder/folder swap list
 *
 * Return: NA
 */
static void fbaf_handle_folder_list_back(void)
{
    bool back_to_device_list;
    char *parent_dir = NULL;

    back_to_device_list =
        fbaf_get_parent_dir(g_cur_app_state->cur_folder, &parent_dir);
    if (back_to_device_list) {
        fbaf_enter_device_list();
        fbaf_load_page(ui_fb_device_panel, LV_SCR_LOAD_ANIM_OVER_RIGHT, 500, 0);
    } else {
        lv_obj_t *new_page = fbaf_update_folder_list(parent_dir);
        if (new_page) {
            fbaf_update_state_machine(FB_APP_UI_STATE_FOLDER_LIST, parent_dir,
                                      new_page);
            fbaf_load_page(new_page, LV_SCR_LOAD_ANIM_OVER_RIGHT, 500, 0);
            fbaf_clean_unused_folder_list();
        }
    }
    if (parent_dir) {
        fb_app_free(parent_dir);
    }
}

/**
 * fbaf_is_drawable_area - check if the area is inside the display screen
 * @draw_area: [IN] area to draw
 *
 * Return: true for yes, false for no
 * Note: internal use only
 */
static bool fbaf_is_drawable_area(const lv_area_t *draw_area)
{
    lv_area_t screen_area;
    screen_area.x1 = 0;
    screen_area.x2 = LV_HOR_RES - 1;
    screen_area.y1 = 0;
    screen_area.y2 = LV_VER_RES - 1;

    return _lv_area_is_on(draw_area, &screen_area);
}

/**
 * fbaf_draw_list_item - draw list item
 * @obj: [IN] pointer to btnmatrix
 * @dsc: [IN] descriptor of draw part
 *
 * Return: NA
 * Note: internal use only
 */
static void fbaf_draw_list_item(lv_obj_t *obj, lv_obj_draw_part_dsc_t *dsc)
{
    /* get content to draw */
    struct app_fs_scan_list_item *list_item = fbaf_get_list_item(obj, dsc->id);
    if (!list_item || !dsc || !dsc->label_dsc) {
        return;
    }
    if (!fbaf_is_drawable_area(dsc->draw_area)) {
        return;
    }
    const void *img = NULL;
    lv_img_header_t header;
    lv_coord_t remain_w;
    lv_point_t size;
    lv_draw_label_dsc_t txt_dsc;
    lv_area_t a;
    lv_coord_t sel_width = 0;
    const char *file_name = list_item->name;

    struct fbaf_folder_list_btn_attr *user_data =
        (struct fbaf_folder_list_btn_attr *)lv_obj_get_user_data(obj);
    if (user_data->is_thumbnail) {
        /* check if selecting or not, highlight the background if selected */
        if (user_data->is_selecting) {
            if (lv_btnmatrix_has_btn_ctrl(obj, dsc->id,
                                          LV_BTNMATRIX_CTRL_CHECKED)) {
                lv_draw_rect_dsc_t draw_rect;
                a.x1 = dsc->draw_area->x1 + 1;
                a.x2 = dsc->draw_area->x2 - 1;
                a.y1 = dsc->draw_area->y1 + 1;
                a.y2 = dsc->draw_area->y2 - 1;
                lv_draw_rect_dsc_init(&draw_rect);
                draw_rect.radius = 2;
                draw_rect.bg_color =
                    lv_color_hex(FB_FOLDER_THUMBNAIL_SELECTED_BG_COLOR);
                lv_draw_rect(dsc->draw_ctx, &draw_rect, &a);
            }
        }
        /* draw image */
        if (APP_FS_FILE_TYPE_FOLDER == list_item->type) {
            img = (void *)&ui_img_folder_48x48_png;
        } else {
            img = (void *)&ui_img_file_48x48_png;
        }
        lv_res_t res = lv_img_decoder_get_info(img, &header);
        if (res != LV_RES_OK)
            return;
        a.x1 = dsc->draw_area->x1 +
               (lv_area_get_width(dsc->draw_area) - header.w) / 2;
        a.x2 = a.x1 + header.w - 1;
        a.y1 = dsc->draw_area->y1;
        a.y2 = a.y1 + header.h - 1;
        lv_draw_img_dsc_t img_draw_dsc;
        lv_draw_img_dsc_init(&img_draw_dsc);
        img_draw_dsc.recolor = lv_color_black();
        if (lv_btnmatrix_get_selected_btn(obj) == dsc->id)
            img_draw_dsc.recolor_opa = LV_OPA_30;
        lv_draw_img(dsc->draw_ctx, &img_draw_dsc, &a, img);
        /* draw file name */
        a.x1 = dsc->draw_area->x1 + 2;
        a.x2 = dsc->draw_area->x2 - 2;
        a.y1 = dsc->draw_area->y2 -
               lv_font_get_line_height(FB_FOLDER_THUMBNAIL_FILENAME_FONT);
        a.y2 = dsc->draw_area->y2;
        memcpy(&txt_dsc, dsc->label_dsc, sizeof(lv_draw_label_dsc_t));
        txt_dsc.font = FB_FOLDER_THUMBNAIL_FILENAME_FONT;
        txt_dsc.align = LV_TEXT_ALIGN_CENTER;
        remain_w = lv_area_get_width(&a);
        lv_txt_get_size(&size, file_name, FB_FOLDER_THUMBNAIL_FILENAME_FONT,
                        dsc->label_dsc->letter_space,
                        dsc->label_dsc->line_space, LV_COORD_MAX,
                        LV_TEXT_FLAG_EXPAND);

        if (size.x > remain_w) {
            /* cut the text with DOT */
            /*Be sure there is space for the dots*/
            size_t txt_len = strlen(file_name);
            char *cut_str = fb_app_malloc(
                txt_len + LV_LABEL_DOT_NUM +
                1); /* worst case is only one character plus 3 dots */
            uint32_t i = 0, prev_len = 0;

            while (i < (uint32_t)txt_len) {
                _lv_txt_encoded_next(file_name, &i);
                memcpy(cut_str, file_name, i);
                for (int j = 0; j < LV_LABEL_DOT_NUM; j++) {
                    cut_str[i + j] = '.';
                }
                cut_str[i + LV_LABEL_DOT_NUM] = 0;
                lv_txt_get_size(&size, cut_str, FB_FOLDER_LIST_FILENAME_FONT,
                                dsc->label_dsc->letter_space,
                                dsc->label_dsc->line_space, LV_COORD_MAX,
                                LV_TEXT_FLAG_EXPAND);
                if (size.x > remain_w) {
                    break;
                } else {
                    prev_len = i;
                }
            }
            memcpy(cut_str, file_name, prev_len);
            for (int j = 0; j < LV_LABEL_DOT_NUM; j++) {
                cut_str[prev_len + j] = '.';
            }
            cut_str[prev_len + LV_LABEL_DOT_NUM] = 0;

            lv_draw_label(dsc->draw_ctx, &txt_dsc, &a, cut_str, NULL);
            fb_app_free(cut_str);
        } else {
            lv_draw_label(dsc->draw_ctx, &txt_dsc, &a, list_item->name, NULL);
        }
    } else {
        /* check if selecting or not, draw checkbox */
        if (user_data->is_selecting) {
            lv_draw_rect_dsc_t draw_rect;

            a.x1 = dsc->draw_area->x1 + 2;
            a.x2 = a.x1 + 16 - 1;
            a.y1 = dsc->draw_area->y1 +
                   (lv_area_get_height(dsc->draw_area) - 16) / 2;
            a.y2 = a.y1 + 16 - 1;
            lv_draw_rect_dsc_init(&draw_rect);
            draw_rect.radius = 2;
            if (lv_btnmatrix_has_btn_ctrl(obj, dsc->id,
                                          LV_BTNMATRIX_CTRL_CHECKED)) {
                draw_rect.bg_color = lv_color_hex(FB_CHECKBOX_COLOR);
            } else {
                draw_rect.border_width = 2;
                draw_rect.border_color = lv_color_hex(FB_CHECKBOX_COLOR);
            }
            lv_draw_rect(dsc->draw_ctx, &draw_rect, &a);
            sel_width = 16 + 2;
        }

        /* draw image */
        if (APP_FS_FILE_TYPE_FOLDER == list_item->type) {
            img = (void *)&ui_img_folder_png;
        } else {
            img = (void *)&ui_img_file_png;
        }
        lv_res_t res = lv_img_decoder_get_info(img, &header);
        if (res != LV_RES_OK)
            return;
        a.x1 = dsc->draw_area->x1 + 2 + sel_width;
        a.x2 = a.x1 + header.w - 1;
        a.y1 = dsc->draw_area->y1 +
               (lv_area_get_height(dsc->draw_area) - header.h) / 2;
        a.y2 = a.y1 + header.h - 1;

        lv_draw_img_dsc_t img_draw_dsc;
        lv_draw_img_dsc_init(&img_draw_dsc);
        img_draw_dsc.recolor = lv_color_black();
        if (lv_btnmatrix_get_selected_btn(obj) == dsc->id)
            img_draw_dsc.recolor_opa = LV_OPA_30;
        lv_draw_img(dsc->draw_ctx, &img_draw_dsc, &a, img);
        /* draw file name */
        a.x1 = a.x2 + 2;
        a.x2 = dsc->draw_area->x2 - 4;
        a.y1 = dsc->draw_area->y1 + 2;
        a.y2 = a.y1 + lv_font_get_line_height(FB_FOLDER_LIST_FILENAME_FONT);
        memcpy(&txt_dsc, dsc->label_dsc, sizeof(lv_draw_label_dsc_t));
        txt_dsc.font = FB_FOLDER_LIST_FILENAME_FONT;
        remain_w = lv_area_get_width(&a);
        lv_txt_get_size(&size, file_name, FB_FOLDER_LIST_FILENAME_FONT,
                        dsc->label_dsc->letter_space,
                        dsc->label_dsc->line_space, LV_COORD_MAX,
                        LV_TEXT_FLAG_EXPAND);

        if (size.x > remain_w) {
            /* cut the text with DOT */
            /*Be sure there is space for the dots*/
            size_t txt_len = strlen(file_name);
            char *cut_str = fb_app_malloc(
                txt_len + LV_LABEL_DOT_NUM +
                1); /* worst case is only one character plus 3 dots */
            uint32_t i = 0, prev_len = 0;

            while (i < (uint32_t)txt_len) {
                _lv_txt_encoded_next(file_name, &i);
                memcpy(cut_str, file_name, i);
                for (int j = 0; j < LV_LABEL_DOT_NUM; j++) {
                    cut_str[i + j] = '.';
                }
                cut_str[i + LV_LABEL_DOT_NUM] = 0;
                lv_txt_get_size(&size, cut_str, FB_FOLDER_LIST_FILENAME_FONT,
                                dsc->label_dsc->letter_space,
                                dsc->label_dsc->line_space, LV_COORD_MAX,
                                LV_TEXT_FLAG_EXPAND);
                if (size.x > remain_w) {
                    break;
                } else {
                    prev_len = i;
                }
            }
            memcpy(cut_str, file_name, prev_len);
            for (int j = 0; j < LV_LABEL_DOT_NUM; j++) {
                cut_str[prev_len + j] = '.';
            }
            cut_str[prev_len + LV_LABEL_DOT_NUM] = 0;

            lv_draw_label(dsc->draw_ctx, &txt_dsc, &a, cut_str, NULL);
            fb_app_free(cut_str);
        } else {
            lv_draw_label(dsc->draw_ctx, &txt_dsc, &a, list_item->name, NULL);
        }

        /* draw date time*/
        char date_time[32] = {0};
        fbaf_format_date_time(list_item->m_time, date_time);
        a.y1 = a.y2 + 4;
        a.y2 = dsc->draw_area->y2 - 2;
        txt_dsc.font = FB_FOLDER_LIST_DATETIME_FONT;
        lv_draw_label(dsc->draw_ctx, &txt_dsc, &a, date_time, NULL);
    }
}

/**
 * ui_fb_btnmatrix_event_cb - handle the ui events triggered by folder
 * list item
 * @e: [IN] event
 *
 * Return: NA
 * Note: internal use only
 */
static void ui_fb_btnmatrix_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_DRAW_PART_END) {
        lv_obj_draw_part_dsc_t *dsc = lv_event_get_param(e);
        fbaf_draw_list_item(obj, dsc);
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        struct fbaf_folder_list_btn_attr *user_data =
            (struct fbaf_folder_list_btn_attr *)lv_obj_get_user_data(obj);
        if (!user_data || user_data->is_selecting) {
            return;
        }
        if (lv_obj_is_scrolling(lv_obj_get_parent(obj))) {
            return;
        }
        uint32_t id = lv_btnmatrix_get_selected_btn(obj);
        struct app_fs_scan_list_item *list_item = fbaf_get_list_item(obj, id);
        if (list_item && APP_FS_FILE_TYPE_FOLDER == list_item->type) {
            /*enter folder*/
            lv_obj_t *new_page = fbaf_enter_subfolder(list_item->name);
            if (new_page) {
                fbaf_load_page(new_page, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0);
                fbaf_clean_unused_folder_list();
            }
        }
    } else if (code == LV_EVENT_DELETE) {
        /* delete the user data*/
        void *user_data = lv_obj_get_user_data(obj);
        if (user_data != NULL) {
            fbaf_clean_folder_list_user_data(user_data);
            lv_obj_set_user_data(obj, NULL);
        }
    } else if (code == LV_EVENT_KEY) {
        char c = *((char *)lv_event_get_param(e));
        if (FB_BACKWARDS_KEY == c) {
            if (fbaf_is_folder_list_selecting()) {
                fbaf_enter_selection_mode(false);
            } else {
                fbaf_handle_folder_list_back();
                /* ignore long press back */
                lv_indev_wait_release(lv_indev_get_act());
            }

        } else if (FB_MEN_ENTRY_KEY == c) {
            fbaf_create_menu();
        }
    } else if (code == LV_EVENT_LONG_PRESSED) {
        /* long press key, pop up a menu here */
        fbaf_create_menu();
        /* ignore long press back */
        lv_indev_wait_release(lv_indev_get_act());
    }
}

/**
 * fbaf_draw_label - draw label
 * @dsc: [IN] draw part descriptor
 * @label_txt: [IN] label text to draw
 * @area: [IN] area to draw
 * @font: [IN] font to draw
 *
 * Return: NA
 * Note: internal use only
 */
static void fbaf_draw_label(lv_obj_draw_part_dsc_t *dsc, const char *label_txt,
                            const lv_area_t *area, const lv_font_t *font)
{
    lv_coord_t remain_w;
    lv_point_t size;
    lv_draw_label_dsc_t txt_dsc;

    memcpy(&txt_dsc, dsc->label_dsc, sizeof(lv_draw_label_dsc_t));
    txt_dsc.font = font;
    remain_w = lv_area_get_width(area);
    lv_txt_get_size(&size, label_txt, font, dsc->label_dsc->letter_space,
                    dsc->label_dsc->line_space, LV_COORD_MAX,
                    LV_TEXT_FLAG_EXPAND);

    if (size.x > remain_w) {
        /* cut the text with DOT */
        /*Be sure there is space for the dots*/
        size_t txt_len = strlen(label_txt);
        char *cut_str =
            fb_app_malloc(txt_len + LV_LABEL_DOT_NUM +
                          1); /* worst case is only one character plus 3 dots */
        uint32_t i = 0, prev_len = 0;

        while (i < (uint32_t)txt_len) {
            _lv_txt_encoded_next(label_txt, &i);
            memcpy(cut_str, label_txt, i);
            for (int j = 0; j < LV_LABEL_DOT_NUM; j++) {
                cut_str[i + j] = '.';
            }
            cut_str[i + LV_LABEL_DOT_NUM] = 0;
            lv_txt_get_size(&size, cut_str, FB_FOLDER_LIST_FILENAME_FONT,
                            dsc->label_dsc->letter_space,
                            dsc->label_dsc->line_space, LV_COORD_MAX,
                            LV_TEXT_FLAG_EXPAND);
            if (size.x > remain_w) {
                break;
            } else {
                prev_len = i;
            }
        }
        memcpy(cut_str, label_txt, prev_len);
        for (int j = 0; j < LV_LABEL_DOT_NUM; j++) {
            cut_str[prev_len + j] = '.';
        }
        cut_str[prev_len + LV_LABEL_DOT_NUM] = 0;

        lv_draw_label(dsc->draw_ctx, &txt_dsc, area, cut_str, NULL);
        fb_app_free(cut_str);
    } else {
        lv_draw_label(dsc->draw_ctx, &txt_dsc, area, label_txt, NULL);
    }
}

/**
 * fbaf_draw_device_list_item - draw device list item
 * @obj: [IN] pointer to btnmatrix
 * @dsc: [IN] descriptor of draw part
 *
 * Return: NA
 * Note: internal use only
 */
static void fbaf_draw_device_list_item(lv_obj_t *obj,
                                       lv_obj_draw_part_dsc_t *dsc)
{
    struct fb_volume_detail *list_item = NULL;
    const void *img = NULL;
    lv_img_header_t header;
    lv_area_t a;
    const char *device_name;
    bool img_symbol = false;
    lv_coord_t img_pad_left = 2;
    lv_coord_t txt_pad_left = 2;

    /* get content to draw */
    struct fbaf_device_list_btn_attr *user_data =
        (struct fbaf_device_list_btn_attr *)lv_obj_get_user_data(obj);
    if (!dsc || !user_data) {
        return;
    }
    list_item = &user_data->details[dsc->id];
    if (!list_item || !dsc->label_dsc) {
        return;
    }
    if (!fbaf_is_drawable_area(dsc->draw_area)) {
        return;
    }
    /* draw image */
    if (img_symbol) {
        if (APP_FS_DEV_ID_LOCAL == list_item->device) {
            img = LV_SYMBOL_DRIVE;
        } else {
            img = LV_SYMBOL_SD_CARD;
        }
        lv_point_t size;
        lv_txt_get_size(&size, (const char *)img, FB_DEVICE_LIST_DEVICE_FONT,
                        dsc->label_dsc->letter_space,
                        dsc->label_dsc->line_space, LV_COORD_MAX,
                        LV_TEXT_FLAG_EXPAND);
        header.w = 16; /* unify as 16 pixel width, because the symbols are
                          not as same width*/
        header.h = size.y;
        img_pad_left = 6;
        txt_pad_left = 6;
    } else {
        if (APP_FS_DEV_ID_LOCAL == list_item->device) {
            img = (void *)&ui_img_folder_png;
        } else {
            img = (void *)&ui_img_file_png;
        }
        lv_res_t res = lv_img_decoder_get_info(img, &header);
        if (res != LV_RES_OK)
            return;
    }

    a.x1 = dsc->draw_area->x1 + img_pad_left;
    a.x2 = a.x1 + header.w - 1;
    a.y1 = dsc->draw_area->y1 +
           (lv_area_get_height(dsc->draw_area) - header.h) / 2;
    a.y2 = a.y1 + header.h - 1;

    if (img_symbol) {
        lv_draw_label_dsc_t txt_dsc;

        memcpy(&txt_dsc, dsc->label_dsc, sizeof(lv_draw_label_dsc_t));
        txt_dsc.font = FB_DEVICE_LIST_DEVICE_FONT;
        if (lv_btnmatrix_get_selected_btn(obj) == dsc->id)
            txt_dsc.color = lv_color_hex(FB_SYMBOL_SELECTED_COLOR);
        lv_draw_label(dsc->draw_ctx, &txt_dsc, &a, (const char *)img, NULL);
    } else {
        lv_draw_img_dsc_t img_draw_dsc;
        lv_draw_img_dsc_init(&img_draw_dsc);
        img_draw_dsc.recolor = lv_color_black();
        if (lv_btnmatrix_get_selected_btn(obj) == dsc->id)
            img_draw_dsc.recolor_opa = LV_OPA_30;
        lv_draw_img(dsc->draw_ctx, &img_draw_dsc, &a, img);
    }

    /* draw device name */
    if (APP_FS_DEV_ID_LOCAL == list_item->device) {
        device_name = "Local Storage";
    } else {
        device_name = "SD Card";
    }
    a.x1 = a.x2 + txt_pad_left;
    a.x2 = dsc->draw_area->x2 - 4;
    a.y1 = dsc->draw_area->y1 + 2;
    a.y2 = a.y1 + lv_font_get_line_height(FB_DEVICE_LIST_DEVICE_FONT);
    fbaf_draw_label(dsc, device_name, &a, FB_DEVICE_LIST_DEVICE_FONT);
    /* draw capacity */
    char *stor_info = (char *)fb_app_malloc(100);
    if (stor_info) {
        fbaf_format_storage_info_txt(list_item->capacity, list_item->avail,
                                     stor_info);
        a.y1 = a.y2 + 4;
        a.y2 = dsc->draw_area->y2 - 2;
        fbaf_draw_label(dsc, stor_info, &a, FB_DEVICE_LIST_CAPACITY_FONT);
        fb_app_free(stor_info);
    }
}

/**
 * ui_fb_device_list_event_cb - handle the ui events triggered by device
 * list item
 * @e: [IN] event
 *
 * Return: NA
 * Note: internal use only
 */
static void ui_fb_device_list_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_DRAW_PART_END) {
        lv_obj_draw_part_dsc_t *dsc = lv_event_get_param(e);
        fbaf_draw_device_list_item(obj, dsc);
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        if (lv_obj_is_scrolling(lv_obj_get_parent(obj))) {
            return;
        }
        uint32_t id = lv_btnmatrix_get_selected_btn(obj);
        if (APP_FS_DEV_ID_LOCAL == (enum app_fs_dev_id)id) {
            lv_obj_t *new_page = fbaf_enter_local_storage();
            if (new_page) {
                fbaf_load_page(new_page, LV_SCR_LOAD_ANIM_OVER_LEFT, 500, 0);
            } else {
                app_loge("unable to enter local storage\n");
            }
        } else {
            /* sd card */
            lv_obj_t *new_page = fbaf_enter_sd_card();
            if (new_page) {
                fbaf_load_page(new_page, LV_SCR_LOAD_ANIM_OVER_LEFT, 500, 0);
            }
        }
    } else if (code == LV_EVENT_DELETE) {
        /* delete the user data*/
        struct fbaf_device_list_btn_attr *btn_attr =
            (struct fbaf_device_list_btn_attr *)lv_obj_get_user_data(obj);
        if (btn_attr) {
            if (btn_attr->btnm_map) {
                fb_app_free(btn_attr->btnm_map);
                btn_attr->btnm_map = NULL;
            }
            fb_app_free(btn_attr);
        }
        lv_obj_set_user_data(obj, NULL);
    } else if (code == LV_EVENT_KEY) {
        char c = *((char *)lv_event_get_param(e));
        if (FB_BACKWARDS_KEY == c) {
            /* quit app */
            alv_am_app_exit(NULL, FB_APP_NAME);
            /* ignore long press back */
            lv_indev_wait_release(lv_indev_get_act());
        } else if (FB_MEN_ENTRY_KEY == c) {
            fbaf_create_menu();
        }
    } else if (code == LV_EVENT_LONG_PRESSED) {
        /*long press to trigger menu pop up */
        fbaf_create_menu();
        /* ignore long press back */
        lv_indev_wait_release(lv_indev_get_act());
    }
}
/**
 * ui_fb_init - init first page of fb, i.e. device list
 * @e: [IN] event
 *
 * Return: NA
 * Note: used in ui.c.
 */
static void ui_fb_init(void)
{
    ui_fb_device_list_screen_init();
    fbaf_enter_device_list();
}

/*******************************************
 *              GLOBAL FUNCTIONS
 ******************************************/
/**
 * ui_fb_folder_event_handler - handle backwards key event
 * @e: [IN] event
 *
 * Return: NA
 * Note: used in ui.c
 */
void ui_fb_folder_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (LV_EVENT_KEY == code) {
        char c = *((char *)lv_event_get_param(e));
        if (FB_BACKWARDS_KEY == c) {
            fbaf_handle_folder_list_back();
            /* ignore long press back */
            lv_indev_wait_release(lv_indev_get_act());
        } else if (FB_MEN_ENTRY_KEY == c) {
            fbaf_create_menu();
        }
    } else if (code == LV_EVENT_LONG_PRESSED) {
        /* long press key, pop up a menu here */
        fbaf_create_menu();
        /* ignore long press back */
        lv_indev_wait_release(lv_indev_get_act());
    }
}
/**
 * ui_fb_event_device_gesture_back - handle the ui events triggered by
 * gesture back in device list page
 * @e: [IN] event
 *
 * Return: NA
 * Note: used in ui.c
 */
void ui_fb_event_device_gesture_back(lv_event_t *e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (LV_DIR_RIGHT == dir) {
        /* quit app */
        alv_am_app_exit(NULL, FB_APP_NAME);
        /* ignore long pressed and release key */
        lv_indev_wait_release(lv_indev_get_act());
    }
}

/**
 * ui_fb_event_gesture_back - handle the ui events triggered by gesture back
 * in folder/folder swap list item
 * @e: [IN] event
 *
 * Return: NA
 * Note: used in ui.c
 */
void ui_fb_event_gesture_back(lv_event_t *e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (LV_DIR_RIGHT == dir) {
        fbaf_handle_folder_list_back();
        /* ignore long pressed and release key */
        lv_indev_wait_release(lv_indev_get_act());
    }
}

/**
 * ui_fb_destroy - destroy ui pages
 *
 * Return: NA
 */
void ui_fb_destroy(void)
{
    /* clean up screens */
    if (ui_fb_folder_panel) {
        lv_obj_del(ui_fb_folder_panel);
        ui_fb_folder_panel = NULL;
    }
    if (ui_fb_folder_swap_panel) {
        lv_obj_del(ui_fb_folder_swap_panel);
        ui_fb_folder_swap_panel = NULL;
    }
    if (ui_fb_device_panel) {
        lv_obj_del(ui_fb_device_panel);
        ui_fb_device_panel = NULL;
    }
}

/**
 * flow_fb_init - init file browser app flow status
 * @app: [IN] pointer to app obj
 *
 * Return: NA
 */
void flow_fb_init(lv_obj_t *app)
{
    /* register sd mount listener */
    app_fs_sd_mount_listener_register(sd_mount_listener, NULL);
    g_cur_app_state = fb_app_malloc(sizeof(struct fbaf_status));
    if (g_cur_app_state) {
        /* init state machine */
        memset(g_cur_app_state, 0, sizeof(struct fbaf_status));
        g_cur_app_state->ui_fb_state = FB_APP_UI_STATE_UNKNOWN;
        g_cur_app_state->fl_mode = FB_APP_FL_MODE_LIST;
        g_cur_app_state->is_suspended = true;
        _lv_ll_init(&g_cur_app_state->msgbox_list, sizeof(lv_obj_t *));

        /* init ui */
        ui_fb_init();
        g_cur_app_state->cur_page = ui_fb_device_panel;
    }
}

/**
 * flow_fb_destory - destroy file browser app flow status
 * @app: [IN] pointer to app obj
 *
 * Return: NA
 */
void flow_fb_destory(lv_obj_t *app)
{
    LV_UNUSED(app);
    /* deregister sd mount listener */
    app_fs_sd_mount_listener_deregister(sd_mount_listener);
    fbaf_clean_popup();
    /* destroy ui */
    ui_fb_destroy();
    if (g_cur_app_state) {
        app_srv_fb_entry_list_release(g_cur_app_state->copied_list);
        fb_app_free(g_cur_app_state);
        g_cur_app_state = NULL;
    }
}

/**
 * flow_fb_suspend - suspend the app
 * @app: [IN] pointer to app obj
 *
 * Return: NA
 */
void flow_fb_suspend(lv_obj_t *app)
{
    LV_UNUSED(app);
    app_logi("flow_fb_suspend enter.\n");
    if (g_cur_app_state) {
        fbaf_clean_popup();
        g_cur_app_state->is_suspended = true;
    } else {
        app_loge("flow_fb_suspend: app not running\n");
    }
}

/**
 * flow_fb_resume - resume the app
 * @app: [IN] pointer to app obj
 *
 * Return: NA
 */
void flow_fb_resume(lv_obj_t *app)
{
    LV_UNUSED(app);
    if (g_cur_app_state && g_cur_app_state->is_suspended) {
        /* load the page of display and get the btnm */
        lv_obj_t *btnm = NULL;
        if (FB_APP_UI_STATE_DEVICE_LIST == g_cur_app_state->ui_fb_state) {
            lv_disp_load_scr(ui_fb_device_panel);
            if (ui_fb_device_list_panel) {
                btnm = lv_obj_get_child(ui_fb_device_list_panel, 0);
            }
        } else if (FB_APP_UI_STATE_FOLDER_LIST ==
                   g_cur_app_state->ui_fb_state) {
            if (g_cur_app_state->cur_page) {
                lv_disp_load_scr(g_cur_app_state->cur_page);
                btnm = fbaf_get_folder_list_btnm();
            } else {
                app_loge("flow_fb_resume: invalid current page\n");
            }
        } else {
            app_loge("flow_fb_resume: invalid state %d\n",
                     g_cur_app_state->ui_fb_state);
        }

        /* set focus */
        if (btnm) {
            /* get focus to receive keyboard events */
            lv_group_focus_obj(btnm);
            g_cur_app_state->is_suspended = false;
        } else {
            /* what if in folder list and no btnmatrix available? */
            if (FB_APP_UI_STATE_FOLDER_LIST == g_cur_app_state->ui_fb_state) {
                /* set focus to the panel */
                if (g_cur_app_state->cur_page != NULL) {
                    lv_group_focus_obj(g_cur_app_state->cur_page);
                }
            }
        }
    }
}
