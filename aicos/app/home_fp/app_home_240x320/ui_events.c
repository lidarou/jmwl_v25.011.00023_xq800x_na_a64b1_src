
#include "ui.h"
#include "alv_app_mgr.h"
#include "lv_port_indev_keypad.h"
#include "home_fp.h"

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void home_fp_app_ui_event_actions(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    //lv_obj_t * target = lv_event_get_target(e);
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
                    home_app_win_uninit(obj);
                    }
                }
            break;

        case LV_EVENT_KEY:
            {
                uint32_t        key = lv_event_get_key(e);

                home_app_win_key_action(key,e->user_data);

                }
            break;

        default:
            break;
        }


}


