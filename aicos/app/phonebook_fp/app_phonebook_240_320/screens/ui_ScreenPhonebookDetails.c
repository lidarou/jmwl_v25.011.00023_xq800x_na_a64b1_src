
#include "../ui.h"
#include "phonebook.h"
#include "obj_batch_deal.h"
#include "app_call_list.h"
#include "app_log.h"
#include <stdio.h>

#define __inf                   APP_LOGI




enum {
    OBJ_LABEL_PHONEBOOK,

    //OBJ_LABEL_NAME,
    //OBJ_LABEL_NUM,
    OBJ_LABEL_CONTENT,
    OBJ_LABEL_OPTION,
    OBJ_LABEL_BACK,
    OBJ_LABEL_EDIT,
    OBJ_LABEL_MAX,
};


static obj_batch_label_param_array_t obj_batch_label_param_array[OBJ_LABEL_MAX] =
{
    {
        0,0,LV_SIZE_CONTENT,LV_SIZE_CONTENT,LV_ALIGN_TOP_MID,"电话本"
        },

    //{5, 	50,		LV_SIZE_CONTENT,	LV_SIZE_CONTENT,	0,					NULL},
    //{5, 	100,	LV_SIZE_CONTENT,	LV_SIZE_CONTENT,	0,					NULL},
    {
        0,50,240,LV_SIZE_CONTENT,0,NULL
        },
    {
        1,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"选项"
        },
    {
        162,277,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"返回"
        },
    {
        100,290,LV_SIZE_CONTENT,LV_SIZE_CONTENT,0,"编辑"
        },

};


typedef struct ui_screen_phonebook_details_obj_h {
    lv_obj_t *      obj_label[OBJ_LABEL_MAX];
    ui_screen_phonebook_enter_param_t enter_param;
} ui_screen_phonebook_details_obj_t;


lv_obj_t * ui_ScreenPhonebookDetails_screen_init(lv_obj_t * obj,ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebookDetails = NULL;
    uint8_t         content[100];

    phonebook_param_t phonebook_param;

    ui_screen_phonebook_details_obj_t * p_screen_param_obj;

    p_screen_param_obj  = lv_mem_alloc(sizeof(ui_screen_phonebook_details_obj_t));

    if (p_screen_param_obj == NULL)
        return 0;

    if (enter_param != NULL) {
        memcpy(&p_screen_param_obj->enter_param,enter_param,sizeof(ui_screen_phonebook_enter_param_t));
        memcpy(&phonebook_param,&p_screen_param_obj->enter_param.phonebook_param,sizeof(phonebook_param_t));
        }

    ui_ScreenPhonebookDetails = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScreenPhonebookDetails,LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_add_style(ui_ScreenPhonebookDetails,(lv_style_t *) get_lv_style_font_common(),0);

    obj_batch_label_create(ui_ScreenPhonebookDetails,&p_screen_param_obj->obj_label[0],NULL,
        obj_batch_label_param_array,OBJ_LABEL_MAX);
    lv_obj_add_style(p_screen_param_obj->obj_label[OBJ_LABEL_EDIT],(lv_style_t *) get_lv_style_font_custom(20),0);


    //lv_label_set_text(p_screen_param_obj->obj_label[OBJ_LABEL_NAME], phonebook_param.name);
    //lv_label_set_text(p_screen_param_obj->obj_label[OBJ_LABEL_NUM], phonebook_param.number);
    sprintf(content,"%s\n%s",phonebook_param.name,phonebook_param.number);
    lv_label_set_text(p_screen_param_obj->obj_label[OBJ_LABEL_CONTENT],content);

    lv_obj_set_user_data(ui_ScreenPhonebookDetails,p_screen_param_obj);

    lv_obj_add_event_cb(ui_ScreenPhonebookDetails,phonebook_fp_details_ui_event_actions,LV_EVENT_ALL,
        ui_ScreenPhonebookDetails);

    lv_obj_set_user_data(obj,ui_ScreenPhonebookDetails);


    return ui_ScreenPhonebookDetails;

}


static void lv_obj_del_screen_ready_cb(lv_anim_t * a)
{
    ui_screen_phonebook_details_obj_t * p_screen_param_obj;

    p_screen_param_obj  = (ui_screen_phonebook_details_obj_t *) lv_obj_get_user_data(a->var);
    lv_mem_free(p_screen_param_obj);
    lv_obj_set_user_data(a->var,NULL);
    lv_obj_del(a->var);

}


void ui_ScreenPhonebookDetails_screen_uninit(lv_obj_t * obj,ui_screen_phonebook_enter_param_t * enter_param)
{
    lv_obj_t *      ui_ScreenPhonebookDetails = NULL;

    ui_ScreenPhonebookDetails = lv_obj_get_user_data(obj);

    if (ui_ScreenPhonebookDetails) {
        lv_obj_del_delayed_callback(ui_ScreenPhonebookDetails,500,lv_obj_del_screen_ready_cb);
        }

    lv_obj_set_user_data(obj,NULL);
}


ui_screen_phonebook_enter_param_t * get_ui_ScreenPhonebookDetails_enter_param(ui_screen_phonebook_details_obj_t * details_obj)
{
    if (details_obj != NULL)
        return & details_obj->enter_param;

    return 0;
}


void ui_ScreenPhonebookDetailsDialNumber(lv_obj_t * ui_Screen)
{
#ifdef CONFIG_APP_CALL_FP
    ui_screen_phonebook_details_obj_t * p_screen_param_obj =
         (ui_screen_phonebook_details_obj_t *) lv_obj_get_user_data (ui_Screen);

    call_dial_number(p_screen_param_obj->enter_param.phonebook_param.number,
        p_screen_param_obj->enter_param.phonebook_param.name);

#endif
}


