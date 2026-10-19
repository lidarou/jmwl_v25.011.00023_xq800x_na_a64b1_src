
#include "ui.h"
#include "alv_app_mgr.h"
#include "lv_port_indev_keypad.h"
#include "app_log.h"

#define __inf                   APP_LOGI


void fp_sms_menu_ui_event_actions(lv_event_t * e)
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

                sms_menu_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}



void fp_sms_Write_ui_event_actions(lv_event_t * e)
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
                    sms_write_pinyin_create(obj);
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

                sms_write_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_write_option_ui_event_actions(lv_event_t * e)
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

                sms_write_option_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_write_phone_num_ui_event_actions(lv_event_t * e)
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
                    sms_write_phonenum_pinyin_create(obj);
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

                sms_write_phonenum_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}




void fp_sms_list_rev_ui_event_actions(lv_event_t * e)
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

                sms_list_rev_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_list_send_ui_event_actions(lv_event_t * e)
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

                sms_list_send_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_list_draft_ui_event_actions(lv_event_t * e)
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

                sms_list_draft_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_detail_rev_ui_event_actions(lv_event_t * e)
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

                sms_detail_rev_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}



void fp_sms_detail_send_ui_event_actions(lv_event_t * e)
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

                sms_detail_send_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_write_phone_num_find_ui_event_actions(lv_event_t * e)
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

                sms_write_phone_num_find_select_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_send_option_ui_event_actions(lv_event_t * e)
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

                sms_send_option_select_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_select_phone_num_ui_event_actions(lv_event_t * e)
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

                sms_select_phone_num_select_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_insert_option_ui_event_actions(lv_event_t * e)
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

                sms_insert_option_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_insert_word_ui_event_actions(lv_event_t * e)
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
                    sms_insert_word_pinyin_create(obj);
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

                sms_insert_word_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_insert_number_ui_event_actions(lv_event_t * e)
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
                    sms_insert_number_pinyin_create(obj);
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

                sms_insert_number_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_detail_rev_option_ui_event_actions(lv_event_t * e)
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

                sms_detail_rev_option_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_detail_rev_option_del_ui_event_actions(lv_event_t * e)
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

                sms_detail_rev_option_del_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_detail_rev_option_inf_ui_event_actions(lv_event_t * e)
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

                sms_detail_rev_option_inf_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_detail_send_option_ui_event_actions(lv_event_t * e)
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

                sms_detail_send_option_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}



void fp_sms_detail_send_option_del_ui_event_actions(lv_event_t * e)
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

                sms_detail_send_option_del_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}



void fp_sms_detail_send_option_inf_ui_event_actions(lv_event_t * e)
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

                sms_detail_send_option_inf_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}



void fp_sms_detail_draft_ui_event_actions(lv_event_t * e)
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

                sms_detail_draft_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_detail_draft_option_ui_event_actions(lv_event_t * e)
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

                sms_detail_draft_option_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_detail_draft_option_del_ui_event_actions(lv_event_t * e)
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

                sms_detail_draft_option_del_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_list_del_ui_event_actions(lv_event_t * e)
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

                sms_list_del_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_del_all_read_file_ui_event_actions(lv_event_t * e)
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

                sms_del_all_read_del_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_list_del_rev_ui_event_actions(lv_event_t * e)
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

                sms_list_del_rev_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_list_del_rev_select_ui_event_actions(lv_event_t * e)
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

                sms_list_del_rev_select_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_del_select_file_ui_event_actions(lv_event_t * e)
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

                sms_del_select_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_list_del_send_ui_event_actions(lv_event_t * e)
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

                sms_list_del_send_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}





void fp_sms_list_del_draft_ui_event_actions(lv_event_t * e)
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

                sms_list_del_draft_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_list_del_draft_select_ui_event_actions(lv_event_t * e)
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

                sms_list_del_draft_select_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}



void fp_sms_list_del_send_select_ui_event_actions(lv_event_t * e)
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

                sms_list_del_send_select_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}



void fp_sms_del_rev_all_file_ui_event_actions(lv_event_t * e)
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

                sms_del_rev_all_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_del_rev_read_file_ui_event_actions(lv_event_t * e)
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

                sms_del_rev_read_del_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}




void fp_sms_del_draft_read_file_ui_event_actions(lv_event_t * e)
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

                sms_del_draft_read_del_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_del_send_read_file_ui_event_actions(lv_event_t * e)
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

                sms_del_send_read_del_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_extract_number_file_ui_event_actions(lv_event_t * e)
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

                sms_extract_number_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}



void fp_sms_extract_num_edit_ui_event_actions(lv_event_t * e)
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
                    sms_extract_num_edit_pinyin_create(obj);
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

                sms_extract_num_edit_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}


void fp_sms_extract_num_option_ui_event_actions(lv_event_t * e)
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

                sms_extract_num_option_key_event(key,obj);
                }
            break;

        default:
            break;
        }


}















