
#include "ui.h"
#include "alv_app_mgr.h"
#include "lv_port_indev_keypad.h"
#include "app_log.h"

#define __inf                   APP_LOGI





void fp_incoming_ui_event_actions(lv_event_t * e)
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

                incoming_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_on_the_phone_ui_event_actions(lv_event_t * e)
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
                    on_the_phone_timer_start(obj);
                    }

                }
            break;

        case LV_EVENT_SCREEN_UNLOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_remove_obj(obj);
                    on_the_phone_timer_stop(obj);
                    }
                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);
                lv_obj_t *      obj = e->user_data;

                on_the_phone_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}




void fp_dial_ui_event_actions(lv_event_t * e)
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
                    dial_timer_start(obj);
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

                dial_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_hangup_ui_event_actions(lv_event_t * e)
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

                hangup_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}





void fp_on_the_phone_option_ui_event_actions(lv_event_t * e)
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

                on_the_phone_option_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}






