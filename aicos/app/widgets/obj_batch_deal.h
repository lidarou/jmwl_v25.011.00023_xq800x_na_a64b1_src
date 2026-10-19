

#ifndef __OBJ_BATCH_DEAL_H__
#define __OBJ_BATCH_DEAL_H__
#include "lvgl.h"

#if LV_USE_FREETYPE
extern const uint8_t * freetype_file_oath;

#endif

lv_style_t * get_lv_style_font52_common();
lv_style_t * get_lv_style_font_common();
lv_font_t * get_ft_info_font_common();

void fp_lv_style_font_custom_init();

lv_style_t * get_lv_style_font_custom(uint8_t weight);

void lv_obj_del_delayed_callback(lv_obj_t * obj,uint32_t delay_ms,lv_anim_ready_cb_t ready_cb);


int split_decimal(uint32_t value,uint8_t * buf,int len,int min_w);

typedef struct obj_batch_img_param_array_h {
    void *          src;
    lv_coord_t      x;
    lv_coord_t      y;
    lv_coord_t      witdh;
    lv_coord_t      high;
    lv_align_t      align;
    lv_obj_flag_t   f_add;
    lv_obj_flag_t   f_clear;
} obj_batch_img_param_array_t;


int32_t obj_batch_img_create(lv_obj_t * parent,lv_obj_t * *img_array,obj_batch_img_param_array_t * array,
    uint32_t array_count);


typedef struct obj_batch_label_param_array_h {
    lv_coord_t      x;
    lv_coord_t      y;
    lv_coord_t      witdh;
    lv_coord_t      high;
    lv_align_t      align;
    void *          text;
} obj_batch_label_param_array_t;


int32_t obj_batch_label_create(lv_obj_t * parent,lv_obj_t * *label_array,lv_obj_t * *style_array,
    obj_batch_label_param_array_t * array,uint32_t array_count);

/** maximum images in the image label object */
#define IMG_LABEL_MAX_COUNT     15

typedef struct {
    lv_area_t       coords;
    lv_obj_t *      parent;
    lv_obj_t *      obj_img[IMG_LABEL_MAX_COUNT];
    uint8_t         indices[IMG_LABEL_MAX_COUNT];   /* indices of the charactors to show */
    uint8_t         count;                          /* number of indices */
    const lv_img_dsc_t * *src_chars;                /* image src array */
    uint8_t         num_chars;

    uint8_t         layout_flag;
    uint8_t         num_gap;
} img_label_param_t;


img_label_param_t * img_label_create(lv_obj_t * parent);
void img_label_destroy(img_label_param_t * label_param);
void img_label_set_pos(img_label_param_t * label_param,lv_coord_t x,lv_coord_t y);
void img_label_set_size(img_label_param_t * label_param,lv_coord_t w,lv_coord_t h);
void img_label_set_src(img_label_param_t * label_param,const lv_img_dsc_t * *chars,uint8_t cnt);
void img_label_set_text(img_label_param_t * label_param,const uint8_t * indices,uint8_t cnt);

typedef enum {
    format_pinyin,
    format_num,
    format_abc,
    format_bihua,
} pinyin_format;


typedef struct {
    lv_obj_t *      parent;
    lv_obj_t *      pinyin_ime;
    lv_obj_t *      kb;
    lv_obj_t *      cand_panel;                     //汉字显示
    lv_obj_t *      cand_panel1;                    //拼音显示
    lv_obj_t *      cand_punctuation_marks;         //标点符号	
    lv_obj_t *      cand_abc;                       //英文字母
    lv_obj_t *      ta_user;
    lv_obj_t *      label_format;                   //显示当前选中的是拼音，数字，字母
    uint8_t         active_flag;
    uint8_t         cand_panel_index1;
    uint8_t         cand_panel_index;
    uint8_t         cand_punctuation_marks_count;
    uint8_t         cand_abc_count;
    uint8_t         format_flag_auto;
    pinyin_format   format;                         //0:拼音，1：数字	
} pinyin_param_t;


pinyin_param_t * ime_pinyin_keyboard_create(void);
pinyin_param_t * ime_pinyin_keyboard_create1(void);
pinyin_param_t * ime_pinyin_keyboard_create_2(lv_obj_t * parent);

void ime_pinyin_keyboard_destroy(pinyin_param_t * pinyin_param);
void ime_pinyin_ui_event_actions(uint32_t key,pinyin_param_t * pinyin_param);
void ime_pinyin_ui_event_actions1(uint32_t key,pinyin_param_t * pinyin_param,uint32_t counter);
void ime_pinyin_ui_set_fortmat(pinyin_param_t * pinyin_param,pinyin_format format); //0:拼音，1：数字	
void ime_pinyin_ui_set_fortmat_flag_auto(pinyin_param_t * pinyin_param,uint8_t format_flag_auto);
uint32_t get_ime_pinyin_keyboard_active_state(pinyin_param_t * pinyin_param);
uint32_t get_list_child_focused_id(lv_obj_t * list,uint32_t child_cnt);
uint32_t key_release_all();
void key_update_pressed(uint32_t key,uint32_t code);
int32_t key_update_release(uint32_t key,uint32_t code);
uint32_t key_get_pressed_flag(uint32_t key);
uint32_t key_is_pressed(uint32_t key);
void lv_obj_del_delayed_callback(lv_obj_t * obj,uint32_t delay_ms,lv_anim_ready_cb_t ready_cb);
int32_t fp_fs_remove_folder(const char * path);
lv_font_t * get_ft_info_font_custom(uint8_t weight);
int32_t fp_del_file_by_folder_off(char * folder_path,uint32_t file_d_off);
void lv_obj_del_delayed_custom(lv_obj_t * obj,uint32_t delay_ms);
void create_comp_msgbox_remind(uint8_t * content);
int32_t sd_storage_space_is_enough();
void lv_obj_del_now_custom(void);
void create_comp_msgbox_remind_play_tts_menu(uint8_t * content);

#endif

