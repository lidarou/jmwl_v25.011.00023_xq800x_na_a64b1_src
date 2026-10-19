
#include "ui.h"
#include "alv_app_mgr.h"
#include "lv_port_indev_keypad.h"
#include "app_log.h"
#include "obj_batch_deal.h"

#define __inf                   APP_LOGI

extern void screen_idle1_timer_start(lv_obj_t * ui_Screen);
extern void screen_idle1_key_event_state(uint32_t key,uint32_t code,lv_obj_t * ui_Screen);
extern void idle_num_wirte_pinyin_create(lv_obj_t * ui_Screen);
extern void idle_num_wirte_key_event_state(uint32_t key,uint32_t code,lv_obj_t * ui_Screen);
extern void idle_num_wirte_key_event(uint32_t key,lv_obj_t * ui_Screen);
extern void idle_num_option_key_event(uint32_t key,lv_obj_t * ui_ScreenIdleNumOption);
extern void screen_idle_lock_timer_start(lv_obj_t * ui_Screen);
extern void screen_idle_lock_key_event(uint32_t key,lv_obj_t * ui_Screen);
extern void imei_key_event(uint32_t key,lv_obj_t * ui_Screen);


void fp_screen_idle1_ui_event_actions(lv_event_t * e)
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
                    screen_idle1_timer_start(obj);
                    }

                key_release_all();

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

        case LV_EVENT_PRESSED:
        case LV_EVENT_LONG_PRESSED:
        case LV_EVENT_LONG_PRESSED_REPEAT:
        case LV_EVENT_RELEASED:
            {
                lv_indev_t *    indev_act;
                uint32_t        key;

                indev_act           = lv_event_get_param(e);
                key                 = indev_act->proc.types.keypad.last_key;

                screen_idle1_key_event_state(key,event_code,e->user_data);

                }
            break;

        case LV_EVENT_KEY:
            {
                //uint32_t key = lv_event_get_key(e);
                //lv_obj_t *obj = e->user_data;
                //screen_idle1_key_event(key,obj);
                }
            break;

        default:
            break;
        }

}



void fp_idle_num_write_ui_event_actions(lv_event_t * e)
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
                    idle_num_wirte_pinyin_create(obj);
                    }

                key_release_all();

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

        case LV_EVENT_PRESSED:
        case LV_EVENT_LONG_PRESSED:
        case LV_EVENT_LONG_PRESSED_REPEAT:
        case LV_EVENT_RELEASED:
            {
                lv_indev_t *    indev_act;
                uint32_t        key;

                indev_act           = lv_event_get_param(e);
                key                 = indev_act->proc.types.keypad.last_key;

                idle_num_wirte_key_event_state(key,event_code,e->user_data);

                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);
                lv_obj_t *      obj = e->user_data;

                idle_num_wirte_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}



void fp_idle_num_option_ui_event_actions(lv_event_t * e)
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

                idle_num_option_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}



void fp_screen_idle_lock_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    //	APP_LOGI("fp_screen_idle_lock_ui_event_actions =%d,LV_EVENT_KEY=%d,key=%d\r\n",event_code,LV_EVENT_KEY,key);
    switch (event_code)
    {
        case LV_EVENT_SCREEN_LOADED:
            {
                lv_obj_t *      obj = e->user_data;

                if (obj) {
                    lv_group_add_obj(lv_group_get_default(),obj);
                    lv_group_focus_obj(obj);
                    screen_idle_lock_timer_start(obj);
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

        case LV_EVENT_PRESSED:
        case LV_EVENT_LONG_PRESSED:
        case LV_EVENT_LONG_PRESSED_REPEAT:
        case LV_EVENT_RELEASED:
            {
                lv_indev_t *    indev_act;
                uint32_t        key;

                indev_act           = lv_event_get_param(e);
                key                 = indev_act->proc.types.keypad.last_key;

                //			screen_idle_lock_key_event_state(key,event_code,e->user_data);
                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);
                lv_obj_t *      obj = e->user_data;

                screen_idle_lock_key_event(key,obj);
                }
            break;

        default:
            break;
        }

}




void fp_imei_ui_event_actions(lv_event_t * e)
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

                imei_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}

void fp_sos_ui_event_actions(lv_event_t * e)
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

                sos_key_event(key,obj);
                }
            break;

        default:
            break;
        }
}

void fp_sim_pin_ui_event_actions(lv_event_t * e)
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

                screen_sim_pin_key_event(key,obj);
                }
            break;

        case LV_EVENT_DEFOCUSED:
        case LV_EVENT_FOCUSED:
            screen_sim_pin_focus_event(event_code,e->user_data);
            break;

        default:
            break;
        }


}

