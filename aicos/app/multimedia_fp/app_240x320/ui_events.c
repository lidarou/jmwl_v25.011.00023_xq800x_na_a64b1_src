
#include "ui.h"
#include "alv_app_mgr.h"
#include "lv_port_indev_keypad.h"
#include "app_log.h"

#define __inf                   APP_LOGI





void fp_multimedia_menu_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    switch (event_code)
    {
        case LV_EVENT_SCREEN_LOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_add_obj(lv_group_get_default(),obj);
                    lv_group_focus_obj(obj);
                    }

                }
            break;

        case LV_EVENT_SCREEN_UNLOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_remove_obj(obj);
                    }
                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);
                lv_obj_t *      obj = e->user_data;

                multimedia_menu_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_file_browser_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    switch (event_code)
    {
        case LV_EVENT_SCREEN_LOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_add_obj(lv_group_get_default(),obj);
                    lv_group_focus_obj(obj);
                    }

                }
            break;

        case LV_EVENT_SCREEN_UNLOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_remove_obj(obj);
                    }
                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);
                lv_obj_t *      obj = e->user_data;

                file_browser_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}



void fp_muisc_play_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    switch (event_code)
    {
        case LV_EVENT_SCREEN_LOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_add_obj(lv_group_get_default(),obj);
                    lv_group_focus_obj(obj);
                    }

                }
            break;

        case LV_EVENT_SCREEN_UNLOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_remove_obj(obj);
                    }
                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);
                lv_obj_t *      obj = e->user_data;

                muisc_play_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}






void fp_muisc_option_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    switch (event_code)
    {
        case LV_EVENT_SCREEN_LOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_add_obj(lv_group_get_default(),obj);
                    lv_group_focus_obj(obj);
                    }

                }
            break;

        case LV_EVENT_SCREEN_UNLOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_remove_obj(obj);
                    }
                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);
                lv_obj_t *      obj = e->user_data;

                muisc_option_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}



void fp_muisc_setup_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    switch (event_code)
    {
        case LV_EVENT_SCREEN_LOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_add_obj(lv_group_get_default(),obj);
                    lv_group_focus_obj(obj);
                    }

                }
            break;

        case LV_EVENT_SCREEN_UNLOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_remove_obj(obj);
                    }
                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);
                lv_obj_t *      obj = e->user_data;

                muisc_setup_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}




void fp_muisc_list_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    switch (event_code)
    {
        case LV_EVENT_SCREEN_LOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_add_obj(lv_group_get_default(),obj);
                    lv_group_focus_obj(obj);
                    }

                }
            break;

        case LV_EVENT_SCREEN_UNLOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_remove_obj(obj);
                    }
                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);
                lv_obj_t *      obj = e->user_data;

                muisc_list_key_event(key,obj);
                }
            break;

        default:
            break;
        }

}






void fp_record_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    switch (event_code)
    {
        case LV_EVENT_SCREEN_LOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_add_obj(lv_group_get_default(),obj);
                    lv_group_focus_obj(obj);
                    }

                }
            break;

        case LV_EVENT_SCREEN_UNLOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_remove_obj(obj);
                    }
                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);
                lv_obj_t *      obj = e->user_data;

                record_key_event(key,obj);
                }
            break;

        default:
            break;
        }

}



void fp_record_option_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    switch (event_code)
    {
        case LV_EVENT_SCREEN_LOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_add_obj(lv_group_get_default(),obj);
                    lv_group_focus_obj(obj);
                    }

                }
            break;

        case LV_EVENT_SCREEN_UNLOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_remove_obj(obj);
                    }
                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);
                lv_obj_t *      obj = e->user_data;

                record_option_key_event(key,obj);
                }
            break;

        default:
            break;
        }

}



void fp_record_list_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    switch (event_code)
    {
        case LV_EVENT_SCREEN_LOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_add_obj(lv_group_get_default(),obj);
                    lv_group_focus_obj(obj);
                    }

                }
            break;

        case LV_EVENT_SCREEN_UNLOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_remove_obj(obj);
                    }
                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);
                lv_obj_t *      obj = e->user_data;

                record_rev_key_event(key,obj);
                }
            break;

        default:
            break;
        }

}




void fp_record_play_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    switch (event_code)
    {
        case LV_EVENT_SCREEN_LOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_add_obj(lv_group_get_default(),obj);
                    lv_group_focus_obj(obj);
                    }

                }
            break;

        case LV_EVENT_SCREEN_UNLOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_remove_obj(obj);
                    }
                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);
                lv_obj_t *      obj = e->user_data;

                record_play_key_event(key,obj);
                }
            break;

        default:
            break;
        }

}



void fp_file_browser_play_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    switch (event_code)
    {
        case LV_EVENT_SCREEN_LOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_add_obj(lv_group_get_default(),obj);
                    lv_group_focus_obj(obj);
                    }

                }
            break;

        case LV_EVENT_SCREEN_UNLOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_remove_obj(obj);
                    }
                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);
                lv_obj_t *      obj = e->user_data;

                file_browser_play_key_event(key,obj);
                }
            break;

        default:
            break;
        }

}


