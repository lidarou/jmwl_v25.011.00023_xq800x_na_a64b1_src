
#include "obj_batch_deal.h"
#include "alv_app_mgr.h"
#include "lv_port_indev_keypad.h"

//#include "softkey.h"
#include "fs_api.h"

#include "app_log.h"
#include "prompt_box_common.h"
#include "tts_app_player.h"

#define __inf                   APP_LOGI

#if LV_USE_FREETYPE
const uint8_t * freetype_file_oath = "/O/arial.ttf";

#define FREETYPE_FILE_PATH      freetype_file_oath

static lv_ft_info_t ft_info_font_common =
{
    0
};


static lv_ft_info_t ft_info_font52_common =
{
    0
};


static lv_style_t style_font_common;
static lv_style_t style_font52_common;

extern void pinyin_k9_cand_panel_str_init();
extern void pinyin_page_proc(lv_obj_t * obj,uint16_t dir);
extern void pinyin_k9_enable_bihua(lv_obj_t * obj);
extern void pinyin_k9_disable_bihua(lv_obj_t * obj);


lv_font_t * get_ft_info_font_common()
{
    if (ft_info_font_common.weight)
        return ft_info_font_common.font;

    ft_info_font_common.name = FREETYPE_FILE_PATH;
    ft_info_font_common.weight = 38;
    ft_info_font_common.style = FT_FONT_STYLE_NORMAL;
    ft_info_font_common.mem = NULL;

    if (!lv_ft_font_init(&ft_info_font_common)) {
        return NULL;
        }

    lv_style_init(&style_font_common);
    lv_style_set_text_font(&style_font_common,ft_info_font_common.font);

    //lv_style_set_text_align(&style_font_common, LV_TEXT_ALIGN_LEFT);
    return ft_info_font_common.font;

}


lv_style_t * get_lv_style_font_common()
{
    if (ft_info_font_common.weight)
        return & style_font_common;

    ft_info_font_common.name = FREETYPE_FILE_PATH;
    ft_info_font_common.weight = 38;
    ft_info_font_common.style = FT_FONT_STYLE_NORMAL;
    ft_info_font_common.mem = NULL;

    if (!lv_ft_font_init(&ft_info_font_common)) {
        return NULL;
        }

    lv_style_init(&style_font_common);
    lv_style_set_text_font(&style_font_common,ft_info_font_common.font);

    //lv_style_set_text_align(&style_font_common, LV_TEXT_ALIGN_LEFT);
    lv_style_set_bg_color(&style_font_common,lv_color_hex(0xffffff));
    lv_style_set_bg_opa(&style_font_common,LV_OPA_100);


    return & style_font_common;

}


lv_style_t * get_lv_style_font52_common()
{
    if (ft_info_font52_common.weight)
        return & style_font52_common;

    ft_info_font52_common.name = FREETYPE_FILE_PATH;
    ft_info_font52_common.weight = 52;
    ft_info_font52_common.style = FT_FONT_STYLE_NORMAL;
    ft_info_font52_common.mem = NULL;

    if (!lv_ft_font_init(&ft_info_font52_common)) {
        return NULL;
        }

    lv_style_init(&style_font52_common);
    lv_style_set_text_font(&style_font52_common,ft_info_font52_common.font);

    //lv_style_set_text_align(&style_font_common, LV_TEXT_ALIGN_LEFT);
    return & style_font52_common;

}


static lv_ft_info_t ft_info_font_custom =
{
    0
};


static lv_style_t style_font_custom;


lv_font_t * get_ft_info_font_custom(uint8_t weight)
{
    if (weight == 0) {
        return NULL;
        }
    else if (ft_info_font_custom.weight == weight) {
        return ft_info_font_custom.font;
        }
    else if (ft_info_font_custom.weight > 0) {
        lv_ft_font_destroy(&ft_info_font_custom);
        }

    ft_info_font_custom.name = FREETYPE_FILE_PATH;
    ft_info_font_custom.weight = weight;
    ft_info_font_custom.style = FT_FONT_STYLE_NORMAL;
    ft_info_font_custom.mem = NULL;

    if (!lv_ft_font_init(&ft_info_font_custom)) {
        ft_info_font_custom.weight = 0;
        return NULL;
        }

    lv_style_init(&style_font_custom);
    lv_style_set_text_font(&style_font_custom,ft_info_font_custom.font);

    //lv_style_set_text_align(&style_font_custom, LV_TEXT_ALIGN_LEFT);
    lv_style_set_bg_color(&style_font_custom,lv_color_hex(0xffffff));

    //lv_style_set_bg_opa(&style_font_custom, LV_OPA_100);
    return ft_info_font_custom.font;
}


lv_style_t * get_lv_style_font_custom(uint8_t weight)
{
    if (weight == 0) {
        return NULL;
        }
    else if (ft_info_font_custom.weight == weight) {
        return & style_font_custom;
        }
    else if (ft_info_font_custom.weight > 0) {
        lv_ft_font_destroy(&ft_info_font_custom);
        }

    ft_info_font_custom.name = FREETYPE_FILE_PATH;
    ft_info_font_custom.weight = weight;
    ft_info_font_custom.style = FT_FONT_STYLE_NORMAL;
    ft_info_font_custom.mem = NULL;

    if (!lv_ft_font_init(&ft_info_font_custom)) {
        ft_info_font_custom.weight = 0;
        return NULL;
        }

    lv_style_init(&style_font_custom);
    lv_style_set_text_font(&style_font_custom,ft_info_font_custom.font);

    //lv_style_set_text_align(&style_font_custom, LV_TEXT_ALIGN_LEFT);
    lv_style_set_bg_color(&style_font_custom,lv_color_hex(0xffffff));

    //lv_style_set_bg_opa(&style_font_custom, LV_OPA_100);
    return & style_font_custom;

}


void fp_lv_style_font_custom_init()
{

}


#else

//extern lv_font_t lv_font_montserrat_40;
//lv_font_t lv_font_montserrat1_40;
#define fp_font_chinese_40      lv_font_montserrat_40
#define fp_font_chinese_20      lv_font_montserrat_20

static lv_style_t style_font_common;
static lv_style_t style_font52_common;


lv_font_t * get_ft_info_font_common()
{


    return (lv_font_t *) &fp_font_chinese_40;

}


void lv_style_font_common_init()
{
    lv_style_init(&style_font_common);
    lv_style_set_text_font(&style_font_common,&fp_font_chinese_40);

    //lv_style_set_text_align(&style_font_common, LV_TEXT_ALIGN_LEFT);
    lv_style_set_bg_color(&style_font_common,lv_color_hex(0xffffff));
    lv_style_set_bg_opa(&style_font_common,LV_OPA_100);

}


lv_style_t * get_lv_style_font_common()
{


    return & style_font_common;

}


void lv_style_font52_common_init()
{

    lv_style_init(&style_font52_common);
    lv_style_set_text_font(&style_font52_common,&fp_font_chinese_40);

    //lv_style_set_text_align(&style_font_common, LV_TEXT_ALIGN_LEFT);
}


lv_style_t * get_lv_style_font52_common()
{


    return & style_font52_common;

}


static lv_style_t style_font_custom;


lv_font_t * get_ft_info_font_custom(uint8_t weight)
{

    return (lv_font_t *) &fp_font_chinese_20;
}


void lv_style_font_custom_init()
{

    lv_style_init(&style_font_custom);
    lv_style_set_text_font(&style_font_custom,&fp_font_chinese_20);

    //lv_style_set_text_align(&style_font_custom, LV_TEXT_ALIGN_LEFT);
    lv_style_set_bg_color(&style_font_custom,lv_color_hex(0xffffff));

    //lv_style_set_bg_opa(&style_font_custom, LV_OPA_100);
}


lv_style_t * get_lv_style_font_custom(uint8_t weight)
{

    return & style_font_custom;

}


void fp_lv_style_font_custom_init()
{
    lv_style_font_common_init();
    lv_style_font52_common_init();
    lv_style_font_custom_init();

}


#endif

typedef void(*lv_del_obj_callback_t) (lv_anim_t *);

typedef struct {
    uint32_t        delay_ms;
    lv_del_obj_callback_t fun;
    void *          user_data;
} del_obj_node_t;

static lv_timer_t * handle_timer_del_obj;
static lv_ll_t  ll_del_obj;

#define handle_timer_del_delay_time 100


static void lv_obj_del_delayed_callback_event_cb(struct _lv_timer_t *timer)
{
    del_obj_node_t * p_cur_node,*p_next_node;

    p_cur_node          = _lv_ll_get_head(& (ll_del_obj));

    while (p_cur_node != NULL) {
        p_next_node         = _lv_ll_get_next(& (ll_del_obj),p_cur_node);

        if (p_cur_node->delay_ms - handle_timer_del_delay_time > 0) {
            p_cur_node->delay_ms = p_cur_node->delay_ms - handle_timer_del_delay_time;
            }
        else {
            lv_anim_t       a;

            lv_anim_set_var(&a,p_cur_node->user_data);
            p_cur_node->fun(&a);
            _lv_ll_remove(& (ll_del_obj),p_cur_node);
            lv_mem_free((void *) p_cur_node);
            }

        p_cur_node          = p_next_node;
        }

    if (_lv_ll_get_head(& (ll_del_obj)) == NULL)
        lv_timer_pause(handle_timer_del_obj);

}



void lv_obj_del_delayed_callback(lv_obj_t * obj,uint32_t delay_ms,lv_anim_ready_cb_t ready_cb)
{
#if 0
    lv_anim_t       a;

    lv_anim_init(&a);
    lv_anim_set_var(&a,obj);
    lv_anim_set_exec_cb(&a,NULL);
    lv_anim_set_time(&a,1);
    lv_anim_set_delay(&a,delay_ms);
    lv_anim_set_ready_cb(&a,ready_cb);
    lv_anim_start(&a);

#else

    del_obj_node_t * p_node;

    if (handle_timer_del_obj == NULL) {
        handle_timer_del_obj = lv_timer_create(lv_obj_del_delayed_callback_event_cb,handle_timer_del_delay_time,
            NULL);
        _lv_ll_init(& (ll_del_obj),sizeof(del_obj_node_t));
        }

    p_node              = _lv_ll_ins_tail(& (ll_del_obj));

    p_node->user_data   = obj;
    p_node->delay_ms    = delay_ms;
    p_node->fun         = ready_cb;


    lv_timer_resume(handle_timer_del_obj);
#endif
}


static void lv_obj_del_delayed_custom_fun_cb(lv_anim_t * a)
{
    lv_obj_del(a->var);
}


void lv_obj_del_delayed_custom(lv_obj_t * obj,uint32_t delay_ms)
{
    lv_obj_del_delayed_callback(obj,delay_ms,lv_obj_del_delayed_custom_fun_cb);
}

void lv_obj_del_now_custom(void)
{
    lv_obj_del_delayed_callback_event_cb(NULL);
}

/*
 * split a decimal integer into digits
 *
 * @param value decimal integer value
 * @param buf buffer to store the digits
 * @param len buffer len
 * @param min_w minimum split digits (may pad 0 if integer too small)
 *
 * @retval number of split digits
 */
int split_decimal(uint32_t value,uint8_t * buf,int len,int min_w)
{
    uint32_t        tmp_value = value;
    int             n_digits = 0;

    /* compute digit count of value */
    do {
        n_digits++;
        tmp_value           /= 10;
        }
    while(tmp_value > 0);

    /* return digit count if buf not available */
    if (buf == NULL || len <= 0) {
        return n_digits;
        }

    /* compute filled digit count */
    if (n_digits >= len) {
        n_digits            = len;
        }

    if (min_w >= len) {
        min_w               = len;
        }
    else {
        len                 = MAX(n_digits,min_w);
        }

    /* fill digits of value */
    buf                 += len - 1;
    min_w               -= n_digits;

    for (; n_digits > 0; n_digits--) {
        *buf--              = value % 10;
        value               /= 10;
        }

    /* pad zero */
    for (; min_w > 0; min_w--) {
        *buf--              = 0;
        }

    return len;
}


int32_t obj_batch_img_create(lv_obj_t * parent,lv_obj_t * *img_array,obj_batch_img_param_array_t * array,
    uint32_t array_count)
{
    if ((parent == NULL) || (img_array == NULL) || (array == NULL))
        return 0;

    for (uint32_t i = 0; i < array_count; i++) {
        img_array[i]        = lv_img_create(parent);

        lv_img_set_src(img_array[i],array[i].src);
        lv_obj_set_x(img_array[i],array[i].x);
        lv_obj_set_y(img_array[i],array[i].y);
        lv_obj_set_width(img_array[i],array[i].witdh);
        lv_obj_set_height(img_array[i],array[i].high);

        if (array[i].align)
            lv_obj_set_align(img_array[i],array[i].align);

        if (array[i].f_add)
            lv_obj_add_flag(img_array[i],array[i].f_add); /// Flags

        if (array[i].f_clear)
            lv_obj_clear_flag(img_array[i],array[i].f_clear); /// Flags
        }

    return 0;
}


int32_t obj_batch_label_create(lv_obj_t * parent,lv_obj_t * *label_array,lv_obj_t * *style_array,
    obj_batch_label_param_array_t * array,uint32_t array_count)
{
    if ((parent == NULL) || (label_array == NULL) || (array == NULL))
        return 0;

    for (uint32_t i = 0; i < array_count; i++) {

        label_array[i]      = lv_label_create(parent);

        lv_obj_set_x(label_array[i],array[i].x);
        lv_obj_set_y(label_array[i],array[i].y);
        lv_obj_set_width(label_array[i],array[i].witdh); /// 1
        lv_obj_set_height(label_array[i],array[i].high); /// 1

        if (array[i].align)
            lv_obj_set_align(label_array[i],array[i].align);

        lv_label_set_text(label_array[i],array[i].text);

        if ((style_array) && (style_array[i]))
            lv_obj_add_style(label_array[i],(lv_style_t *) style_array[i],0);
        }

    return 0;
}


img_label_param_t * img_label_create(lv_obj_t * parent)
{
    img_label_param_t * label_param = lv_mem_alloc(sizeof(img_label_param_t));

    memset(label_param,0,sizeof(img_label_param_t));
    label_param->parent = parent;

    return label_param;
}


void img_label_destroy(img_label_param_t * label_param)
{
    lv_mem_free(label_param);
}


void img_label_set_pos(img_label_param_t * label_param,lv_coord_t x,lv_coord_t y)
{

    label_param->coords.x1 = x;
    label_param->coords.y1 = y;
}


void img_label_set_size(img_label_param_t * label_param,lv_coord_t w,lv_coord_t h)
{

    label_param->coords.x2 = label_param->coords.x1 + w;
    label_param->coords.y2 = label_param->coords.y1 + h;

}


static void img_label_layout(img_label_param_t * label_param)
{
    lv_obj_t *      parent;
    lv_coord_t      x,y;
    uint8_t         index;

    if ((label_param->src_chars == NULL) || (label_param->num_chars == 0) || (label_param->count == 0))
        return;

    parent              = label_param->parent;

    if (label_param->layout_flag) {
        for (uint32_t i = 0; i < label_param->count; i++) {
            index               = label_param->indices[i];
            lv_img_set_src(label_param->obj_img[i],label_param->src_chars[index]);
            }

        return;
        }

    label_param->layout_flag = 1;

    x                   = label_param->coords.x1;
    y                   = label_param->coords.y1;

    for (uint32_t i = 0; i < label_param->count; i++) {
        label_param->obj_img[i] = lv_img_create(parent);

        lv_obj_set_x(label_param->obj_img[i],x);
        lv_obj_set_y(label_param->obj_img[i],y);
        lv_obj_set_width(label_param->obj_img[i],LV_SIZE_CONTENT); /// 1
        lv_obj_set_height(label_param->obj_img[i],LV_SIZE_CONTENT); /// 1
        lv_obj_add_flag(label_param->obj_img[i],LV_OBJ_FLAG_ADV_HITTEST); /// Flags
        lv_obj_clear_flag(label_param->obj_img[i],LV_OBJ_FLAG_SCROLLABLE); /// Flags

        index               = label_param->indices[i];
        lv_img_set_src(label_param->obj_img[i],label_param->src_chars[index]);


        x                   = x + label_param->src_chars[index]->header.w + label_param->num_gap;

        }
}


void img_label_set_src(img_label_param_t * label_param,const lv_img_dsc_t * *chars,uint8_t cnt)
{

    label_param->src_chars = chars;
    label_param->num_chars = cnt;

    img_label_layout(label_param);
}


void img_label_set_text(img_label_param_t * label_param,const uint8_t * indices,uint8_t cnt)
{

    label_param->count  = LV_MIN(cnt,IMG_LABEL_MAX_COUNT);
    memcpy(label_param->indices,indices,sizeof(*indices) *label_param->count);

    img_label_layout(label_param);
}


uint32_t get_list_child_focused_id(lv_obj_t * list,uint32_t child_cnt)
{
    /*Modify manually*/
    lv_obj_t *      ui_entry_child = NULL;
    uint32_t        child_id;

    for (child_id = 0; child_id < child_cnt; child_id++) {
        ui_entry_child      = lv_obj_get_child(list,child_id);

        if (lv_obj_has_state(ui_entry_child,LV_STATE_FOCUSED)) {
            break;
            }

        }

    if (child_id >= child_cnt) {
        child_id            = 0;
        }

    //__inf("get_setting_childId:%d,child_cnt:%d\r\n",child_id,child_cnt);
    return child_id;
}






#if 0
extern char *   lv_pinyin_k9_cand_str_map[];


static void ime_keyboard_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *      kb  = lv_event_get_target(e);
    pinyin_param_t * pinyin_param = lv_event_get_user_data(e);
    lv_ime_pinyin_t * pinyin_ime = pinyin_param->pinyin_ime;
    lv_obj_t *      cand_panel1 = pinyin_param->cand_panel1;
    lv_obj_t *      cand_panel = pinyin_param->cand_panel;
    uint32_t        i,str_count,input_char_len;

    ;


    if (code != LV_EVENT_VALUE_CHANGED)
        return;

    //__inf("ime_keyboard_event_cb");	
    //__inf("ime_keyboard_event_cb:%s,%s,%s",pinyin_ime->input_char,pinyin_ime->cand_str,pinyin_ime->input_char);	
    //__inf("ime_keyboard_event_cb:%d,%d,%d",pinyin_ime->k9_py_ll_pos,pinyin_ime->k9_legal_py_count,pinyin_ime->k9_input_str_len);
    //__inf("ime_keyboard_event_cb:%d,%d,%d",pinyin_ime->ta_count,pinyin_ime->cand_num,pinyin_ime->py_page);
    input_char_len      = strlen(pinyin_ime->input_char);

    for (i = 0; i < LV_IME_PINYIN_K9_CAND_TEXT_NUM; i++) {
        str_count           = strlen(lv_pinyin_k9_cand_str_map[i]);

        if (str_count > input_char_len)
            str_count = input_char_len;

        if (str_count == 0) {
            lv_obj_add_flag(cand_panel1,LV_OBJ_FLAG_HIDDEN);
            break;
            }

        //__inf("ime_keyboard_event_cb:%s,%s",pinyin_ime->input_char,lv_pinyin_k9_cand_str_map[i]);
        if (0 == strncmp(pinyin_ime->input_char,lv_pinyin_k9_cand_str_map[i],str_count)) {
            lv_obj_clear_flag(cand_panel1,LV_OBJ_FLAG_HIDDEN);
            lv_btnmatrix_set_btn_ctrl(cand_panel1,i,LV_BTNMATRIX_CTRL_CHECKED);
            pinyin_param->cand_panel_index1 = i;

            if (pinyin_param->active_flag > 0)
                ;

            {
                pinyin_param->active_flag = 0;
                lv_btnmatrix_clear_btn_ctrl(cand_panel,pinyin_param->cand_panel_index,LV_BTNMATRIX_CTRL_CHECKED);
                pinyin_param->cand_panel_index = 0;
                }

            break;
            }
        }



}


#else

extern char *   lv_pinyin_k9_cand_str_map[];


void ime_pinyin_pinyin_destory(pinyin_param_t * pinyin_param)
{
    if (pinyin_param->cand_panel1) {
        lv_obj_del(pinyin_param->cand_panel1);
        pinyin_param->cand_panel1 = NULL;
        }

}


static const char * bihua_str_map[] =
{
    "一","丨","丿","丶","乛","\0"
};


void ime_pinyin_pinyin_create(pinyin_param_t * pinyin_param,uint32_t index)
{
    lv_obj_t *      obj;
    int             i;
    lv_obj_t *      ui_Entry,*ui_LabelEntry;
    lv_coord_t      width,x_off;

    if (pinyin_param->cand_panel1) {
        ime_pinyin_pinyin_destory(pinyin_param);
        }



    obj                 = lv_obj_create(pinyin_param->parent);
    lv_obj_set_pos(obj,0,186);
    lv_obj_set_size(obj,LV_PCT(100),40);
    lv_obj_set_align(obj,LV_ALIGN_TOP_MID);
    lv_obj_add_flag(obj,LV_OBJ_FLAG_SCROLL_WITH_ARROW); /// Flags
    lv_obj_set_scrollbar_mode(obj,LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(obj,LV_DIR_HOR);
    lv_obj_set_style_radius(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);

    //lv_obj_set_style_bg_color(ui_PnlSettings, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT );
    //lv_obj_set_style_bg_opa(ui_PnlSettings, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(obj,LV_BORDER_SIDE_NONE,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(obj,0,LV_PART_MAIN | LV_STATE_DEFAULT);

    //lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    if (pinyin_param->format == format_bihua) {
        int             len,index;
        uint8_t *       str;

        str                 = lv_pinyin_k9_cand_str_map[0];
        len                 = strlen(str);

        width               = 38 + 5;

        for (i = 0; i < len; i++) {
            ui_Entry            = lv_obj_create(obj);
            lv_obj_set_width(ui_Entry,width);
            lv_obj_set_height(ui_Entry,40);
            lv_obj_set_x(ui_Entry,width * i);
            lv_obj_set_y(ui_Entry,0);
            lv_obj_set_style_pad_left(ui_Entry,0,LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(ui_Entry,0,LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(ui_Entry,0,LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(ui_Entry,0,LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_side(ui_Entry,LV_BORDER_SIDE_NONE,LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(ui_Entry,0,LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_flag(ui_Entry,LV_OBJ_FLAG_SCROLLABLE);


            ui_LabelEntry       = lv_label_create(ui_Entry);
            lv_obj_set_width(ui_LabelEntry,width);
            lv_obj_set_height(ui_LabelEntry,40);
            lv_obj_set_x(ui_LabelEntry,2);
            lv_obj_set_y(ui_LabelEntry,-2);
            lv_obj_set_align(ui_LabelEntry,LV_ALIGN_TOP_LEFT);
            index               = str[i] - 'a';

            if ((index >= 0) && (index < 5))
                lv_label_set_text(ui_LabelEntry,bihua_str_map[index]);
            }

        }
    else {

        x_off               = 0;
        width               = strlen(lv_pinyin_k9_cand_str_map[0]) * 20 + 10;

        for (i = 0; i < 7; i++) {

            ui_Entry            = lv_obj_create(obj);
            lv_obj_set_width(ui_Entry,width);
            lv_obj_set_height(ui_Entry,40);
            lv_obj_set_x(ui_Entry,width * i);
            lv_obj_set_y(ui_Entry,0);
            lv_obj_set_style_pad_left(ui_Entry,0,LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(ui_Entry,0,LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(ui_Entry,0,LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(ui_Entry,0,LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_side(ui_Entry,LV_BORDER_SIDE_NONE,LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(ui_Entry,0,LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_flag(ui_Entry,LV_OBJ_FLAG_SCROLLABLE);


            lv_obj_set_style_text_color(ui_Entry,lv_color_hex(0xfffff),LV_PART_MAIN | LV_STATE_FOCUSED);
            lv_obj_set_style_text_opa(ui_Entry,LV_OPA_COVER,LV_PART_MAIN | LV_STATE_FOCUSED);
            lv_obj_set_style_bg_color(ui_Entry,lv_color_white(),LV_PART_MAIN | LV_STATE_FOCUSED);
            lv_obj_set_style_bg_opa(ui_Entry,LV_OPA_COVER,LV_PART_MAIN | LV_STATE_FOCUSED);



            ui_LabelEntry       = lv_label_create(ui_Entry);
            lv_obj_set_width(ui_LabelEntry,width);
            lv_obj_set_height(ui_LabelEntry,40);
            lv_obj_set_x(ui_LabelEntry,2);
            lv_obj_set_y(ui_LabelEntry,-6);
            lv_obj_set_align(ui_LabelEntry,LV_ALIGN_TOP_LEFT);
            lv_label_set_text(ui_LabelEntry,lv_pinyin_k9_cand_str_map[i]);

            if (lv_pinyin_k9_cand_str_map[i][0] == 0)
                break;

            }

        lv_obj_t *      get_first_obj = lv_obj_get_child(obj,index);

        if (get_first_obj) {

            lv_obj_add_state(get_first_obj,LV_STATE_FOCUSED);

            //__inf("(index+1)*width:%d,%d,%d",index,width,(index+1)*width);
            if ((index + 1) *width > 240) //lv_obj_scroll_to_x(obj, index*width, LV_ANIM_OFF);
                lv_obj_scroll_to_view(get_first_obj,LV_ANIM_OFF);
            }
        }

    pinyin_param->cand_panel1 = obj;


}


void ime_pinyin_pinyin_focused(pinyin_param_t * pinyin_param,uint32_t index)
{
    if (pinyin_param->cand_panel1 == NULL)
        return;

    if (pinyin_param->format == format_bihua) {

        return;
        }

    lv_obj_t *      get_first_obj = lv_obj_get_child(pinyin_param->cand_panel1,index);

    if (get_first_obj) {

        lv_obj_add_state(get_first_obj,LV_STATE_FOCUSED);
        }


}


void ime_pinyin_pinyin_defocused(pinyin_param_t * pinyin_param,uint32_t index)
{
    if (pinyin_param->cand_panel1 == NULL)
        return;

    if (pinyin_param->format == format_bihua) {

        return;
        }

    lv_obj_t *      get_first_obj = lv_obj_get_child(pinyin_param->cand_panel1,index);

    if (get_first_obj) {

        lv_obj_clear_state(get_first_obj,LV_STATE_FOCUSED);
        }

}


static void ime_keyboard_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    pinyin_param_t * pinyin_param = lv_event_get_user_data(e);
    lv_ime_pinyin_t * pinyin_ime = (lv_ime_pinyin_t *) pinyin_param->pinyin_ime;
    lv_obj_t *      cand_panel = pinyin_param->cand_panel;
    uint32_t        i,str_count,input_char_len;


    if (code != LV_EVENT_VALUE_CHANGED)
        return;

    input_char_len      = strlen(pinyin_ime->input_char);

    for (i = 0; i < LV_IME_PINYIN_K9_CAND_TEXT_NUM; i++) {
        str_count           = strlen(lv_pinyin_k9_cand_str_map[i]);

        if (str_count > input_char_len)
            str_count = input_char_len;

        if (str_count == 0) {
            ime_pinyin_pinyin_destory(pinyin_param);
            break;
            }

        if (0 == strncmp(pinyin_ime->input_char,lv_pinyin_k9_cand_str_map[i],str_count)) {
            ime_pinyin_pinyin_create(pinyin_param,i);

            //lv_btnmatrix_set_btn_ctrl(pinyin_param->cand_panel1,i,LV_BTNMATRIX_CTRL_CHECKED);
            pinyin_param->cand_panel_index1 = i;

            if (pinyin_param->active_flag > 0) {
                pinyin_param->active_flag = 0;
                lv_btnmatrix_clear_btn_ctrl(cand_panel,pinyin_param->cand_panel_index,LV_BTNMATRIX_CTRL_CHECKED);
                pinyin_param->cand_panel_index = 0;
                }

            break;
            }


        }
}


#endif







static const char * btnm_map_punctuation_marks[] =
{
    "，","。","?","!","(",")","-","_","~","\n",
    "|",":",";","@","&","*","#","+","-","\n",
    "=","'","'","/","<",">","$","^","\0"
};


#define PUNCTUATION_MARKS_LINE_COUNT 9


void ime_pinyin_punctuation_marks_create(pinyin_param_t * pinyin_param)
{
    lv_obj_t *      obj;


    if (pinyin_param->cand_punctuation_marks)
        return;

    obj                 = lv_btnmatrix_create(pinyin_param->parent);
    lv_obj_add_style(obj,(lv_style_t *) get_lv_style_font_custom(20),0);

    lv_btnmatrix_set_map(obj,(const char * *) btnm_map_punctuation_marks);


    lv_btnmatrix_set_one_checked(obj,true);

    /* Set cand_panel style*/
    // Default style
    lv_obj_set_style_bg_opa(obj,LV_OPA_0,0);
    lv_obj_set_style_border_width(obj,0,0);
    lv_obj_set_style_pad_all(obj,8,0);
    lv_obj_set_style_pad_gap(obj,0,0);
    lv_obj_set_style_radius(obj,0,0);
    lv_obj_set_style_pad_gap(obj,0,0);
    lv_obj_set_style_base_dir(obj,LV_BASE_DIR_LTR,0);

    // LV_PART_ITEMS style
    lv_obj_set_style_radius(obj,12,LV_PART_ITEMS);
    lv_obj_set_style_bg_color(obj,lv_color_white(),LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(obj,LV_OPA_0,LV_PART_ITEMS);
    lv_obj_set_style_shadow_opa(obj,LV_OPA_0,LV_PART_ITEMS);

    // LV_PART_ITEMS | LV_STATE_PRESSED style
    lv_obj_set_style_bg_opa(obj,LV_OPA_COVER,LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(obj,lv_color_white(),LV_PART_ITEMS | LV_STATE_PRESSED);





    lv_obj_set_pos(obj,0,186);
    lv_obj_set_size(obj,LV_PCT(100),90);
    lv_obj_set_style_text_color(obj,lv_color_hex(0xfffff),LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(obj,LV_OPA_COVER,LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(obj,lv_color_white(),LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(obj,LV_OPA_COVER,LV_PART_ITEMS | LV_STATE_CHECKED);

    lv_btnmatrix_set_selected_btn(obj,0);
    lv_btnmatrix_set_btn_ctrl(obj,0,LV_BTNMATRIX_CTRL_CHECKED);
    pinyin_param->cand_punctuation_marks = obj;

    pinyin_param->cand_punctuation_marks_count = 26;



}


void ime_pinyin_punctuation_marks_destory(pinyin_param_t * pinyin_param)
{
    if (pinyin_param->cand_punctuation_marks) {
        lv_obj_del(pinyin_param->cand_punctuation_marks);
        pinyin_param->cand_punctuation_marks = NULL;
        }
}


static const char * btnm_map_abc[] =
{
    "a","b","c","A","B","C","\0"
};


static const char * btnm_map_def[] =
{
    "d","e","f","D","E","F","\0"
};


static const char * btnm_map_ghi[] =
{
    "g","h","i","G","H","I","\0"
};


static const char * btnm_map_jkl[] =
{
    "j","k","l","J","K","L","\0"
};


static const char * btnm_map_mno[] =
{
    "m","n","o","M","N","O","\0"
};


static const char * btnm_map_pqrs[] =
{
    "p","q","r","s","P","Q","R","S","\0"
};


static const char * btnm_map_tuv[] =
{
    "t","u","v","T","U","V","\0"
};


static const char * btnm_map_wxyz[] =
{
    "w","x","y","z","W","X","Y","Z","\0"
};


static const void * btn_map_abc_all[] =
{
    btnm_map_abc,
    btnm_map_def,
    btnm_map_ghi,
    btnm_map_jkl,
    btnm_map_mno,
    btnm_map_pqrs,
    btnm_map_tuv,
    btnm_map_wxyz
};


static const uint8_t btn_map_abc_count[] =
{
    6,
    6,
    6,
    6,
    6,
    8,
    6,
    8
};


void ime_pinyin_abc_create(pinyin_param_t * pinyin_param,uint32_t index)
{
    lv_obj_t *      obj;

    if (pinyin_param->cand_abc)
        return;



    obj                 = lv_btnmatrix_create(pinyin_param->parent);

    lv_btnmatrix_set_map(obj,(const char * *) btn_map_abc_all[index]);


    lv_btnmatrix_set_one_checked(obj,true);

    /* Set cand_panel style*/
    // Default style
    lv_obj_set_style_bg_opa(obj,LV_OPA_0,0);
    lv_obj_set_style_border_width(obj,0,0);
    lv_obj_set_style_pad_all(obj,8,0);
    lv_obj_set_style_pad_gap(obj,0,0);
    lv_obj_set_style_radius(obj,0,0);
    lv_obj_set_style_pad_gap(obj,0,0);
    lv_obj_set_style_base_dir(obj,LV_BASE_DIR_LTR,0);

    // LV_PART_ITEMS style
    lv_obj_set_style_radius(obj,12,LV_PART_ITEMS);
    lv_obj_set_style_bg_color(obj,lv_color_white(),LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(obj,LV_OPA_0,LV_PART_ITEMS);
    lv_obj_set_style_shadow_opa(obj,LV_OPA_0,LV_PART_ITEMS);

    // LV_PART_ITEMS | LV_STATE_PRESSED style
    lv_obj_set_style_bg_opa(obj,LV_OPA_COVER,LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(obj,lv_color_white(),LV_PART_ITEMS | LV_STATE_PRESSED);





    lv_obj_set_pos(obj,0,230);
    lv_obj_set_size(obj,LV_PCT(100),42);
    lv_obj_set_style_text_color(obj,lv_color_hex(0xfffff),LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(obj,LV_OPA_COVER,LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(obj,lv_color_white(),LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(obj,LV_OPA_COVER,LV_PART_ITEMS | LV_STATE_CHECKED);

    lv_btnmatrix_set_selected_btn(obj,0);
    lv_btnmatrix_set_btn_ctrl(obj,0,LV_BTNMATRIX_CTRL_CHECKED);
    pinyin_param->cand_abc = obj;
    pinyin_param->cand_abc_count = btn_map_abc_count[index];

}


void ime_pinyin_abc_destory(pinyin_param_t * pinyin_param)
{
    if (pinyin_param->cand_abc) {
        lv_obj_del(pinyin_param->cand_abc);
        pinyin_param->cand_abc = NULL;
        }
}


const lv_pinyin_dict_t lv_ime_bihua_def_dict[] =
{
#include "bihuamap.h"
    {
        NULL,NULL
        }
};


extern lv_pinyin_dict_t lv_ime_pinyin_def_dict[];


pinyin_param_t * ime_pinyin_keyboard_create_2(lv_obj_t * parent)
{
    pinyin_param_t * p_pinyin_param = lv_mem_alloc(sizeof(pinyin_param_t));

    memset(p_pinyin_param,0,sizeof(pinyin_param_t));

    p_pinyin_param->parent = parent;

    lv_obj_t *      pinyin_ime = lv_ime_pinyin_create(p_pinyin_param->parent);

    //lv_ime_pinyin_set_dict(pinyin_ime, your_dict); // Use a custom dictionary. If it is not set, the built-in dictionary will be used.

    /* ta1 */
    lv_obj_t *      ta1 = lv_textarea_create(p_pinyin_param->parent);

    lv_textarea_set_one_line(ta1,true);

    //lv_obj_set_style_text_font(ta1, &ui_font_Font40, 0);
    lv_obj_add_flag(ta1,LV_OBJ_FLAG_HIDDEN);

    /*Create a keyboard and add it to ime_pinyin*/
    lv_obj_t *      kb  = lv_keyboard_create(p_pinyin_param->parent);

    lv_keyboard_set_textarea(kb,ta1);

    lv_ime_pinyin_set_keyboard(pinyin_ime,kb);
    lv_ime_pinyin_set_mode(pinyin_ime,LV_IME_PINYIN_MODE_K9); // Set to 9-key input mode. Default: 26-key input(k26) mode.

    //lv_obj_add_event_cb(kb, ime_keyboard_event_cb, LV_EVENT_VALUE_CHANGED, p_pinyin_param);
    lv_obj_add_flag(kb,LV_OBJ_FLAG_HIDDEN);


    /*Get the cand_panel, and adjust its size and position*/
    lv_obj_t *      cand_panel = lv_ime_pinyin_get_cand_panel(pinyin_ime);

    lv_obj_set_pos(cand_panel,0,230);
    lv_obj_set_size(cand_panel,LV_PCT(100),40);
    lv_obj_set_style_text_color(cand_panel,lv_color_hex(0xfffff),LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cand_panel,LV_OPA_COVER,LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(cand_panel,lv_color_white(),LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(cand_panel,LV_OPA_COVER,LV_PART_ITEMS | LV_STATE_CHECKED);


#if 0
    lv_obj_t *      cand_panel1 = pinyin_k9_cand_panel_create();

    lv_obj_set_pos(cand_panel1,0,186);
    lv_obj_set_size(cand_panel1,LV_PCT(100),40);
    lv_obj_set_style_text_color(cand_panel1,lv_color_hex(0xfffff),LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cand_panel1,LV_OPA_COVER,LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(cand_panel1,lv_color_white(),LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(cand_panel1,LV_OPA_COVER,LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_add_flag(cand_panel1,LV_OBJ_FLAG_HIDDEN);

#else

    pinyin_k9_cand_panel_str_init();
#endif

    p_pinyin_param->pinyin_ime = pinyin_ime;
    p_pinyin_param->kb  = kb;
    p_pinyin_param->cand_panel = cand_panel;

    //p_pinyin_param->cand_panel1		= cand_panel1;
    lv_obj_add_event_cb(kb,ime_keyboard_event_cb,LV_EVENT_VALUE_CHANGED,p_pinyin_param);

    return p_pinyin_param;
}


pinyin_param_t * ime_pinyin_keyboard_create(void)
{
    return ime_pinyin_keyboard_create_2(lv_scr_act());
}

#if 0

pinyin_param_t * ime_pinyin_keyboard_create1(void)
{
    pinyin_param_t * p_pinyin_param = lv_mem_alloc(sizeof(pinyin_param_t));

    memset(p_pinyin_param,0,sizeof(pinyin_param_t));

    p_pinyin_param->parent = lv_scr_act();

    lv_obj_t *      pinyin_ime = lv_ime_pinyin_create(p_pinyin_param->parent);

    //lv_ime_pinyin_set_dict(pinyin_ime, your_dict); // Use a custom dictionary. If it is not set, the built-in dictionary will be used.

    /* ta1 */
    lv_obj_t *      ta1 = lv_textarea_create(p_pinyin_param->parent);

    lv_textarea_set_one_line(ta1,true);

    //lv_obj_set_style_text_font(ta1, &ui_font_Font40, 0);
    lv_obj_add_flag(ta1,LV_OBJ_FLAG_HIDDEN);

    /*Create a keyboard and add it to ime_pinyin*/
    lv_obj_t *      kb  = lv_keyboard_create(p_pinyin_param->parent);

    lv_keyboard_set_textarea(kb,ta1);

    lv_ime_pinyin_set_keyboard(pinyin_ime,kb);
    lv_ime_pinyin_set_mode(pinyin_ime,LV_IME_PINYIN_MODE_K9); // Set to 9-key input mode. Default: 26-key input(k26) mode.

    //lv_obj_add_event_cb(kb, ime_keyboard_event_cb, LV_EVENT_VALUE_CHANGED, p_pinyin_param);
    lv_obj_add_flag(kb,LV_OBJ_FLAG_HIDDEN);


    /*Get the cand_panel, and adjust its size and position*/
    lv_obj_t *      cand_panel = lv_ime_pinyin_get_cand_panel(pinyin_ime);

    lv_obj_set_pos(cand_panel,0,235);
    lv_obj_set_size(cand_panel,LV_PCT(100),38);
    lv_obj_set_style_text_color(cand_panel,lv_color_hex(0xfffff),LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cand_panel,LV_OPA_COVER,LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(cand_panel,lv_color_white(),LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(cand_panel,LV_OPA_COVER,LV_PART_ITEMS | LV_STATE_CHECKED);


#if 0
    lv_obj_t *      cand_panel1 = pinyin_k9_cand_panel_create();

    lv_obj_set_pos(cand_panel1,0,200);
    lv_obj_set_size(cand_panel1,LV_PCT(100),30);
    lv_obj_set_style_text_color(cand_panel1,lv_color_hex(0xfffff),LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cand_panel1,LV_OPA_COVER,LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(cand_panel1,lv_color_white(),LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(cand_panel1,LV_OPA_COVER,LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_add_flag(cand_panel1,LV_OBJ_FLAG_HIDDEN);
#endif

    p_pinyin_param->pinyin_ime = pinyin_ime;
    p_pinyin_param->kb  = kb;
    p_pinyin_param->cand_panel = cand_panel;

    //p_pinyin_param->cand_panel1		= cand_panel1;
    lv_obj_add_event_cb(kb,ime_keyboard_event_cb,LV_EVENT_VALUE_CHANGED,p_pinyin_param);

    return p_pinyin_param;
}

#endif

void ime_pinyin_keyboard_destroy(pinyin_param_t * pinyin_param)
{
    if (pinyin_param) {
        lv_mem_free(pinyin_param);
        }
}



/*
        {\
            ",\0", "1#\0",  "abc \0", "def\0",  LV_SYMBOL_BACKSPACE"\0", "\n\0",
            ".\0", "ghi\0", "jkl\0", "mno\0",  LV_SYMBOL_OK"\0", "\n\0",
            "?\0", "pqrs\0", "tuv\0", "wxyz\0",  LV_SYMBOL_NEW_LINE"\0", "\n\0",
            LV_SYMBOL_LEFT"\0", "\0"
        };
      */
void ime_pinyin_keyboard_event_actions(uint32_t key,pinyin_param_t * pinyin_param)
{
    lv_obj_t *      kb;
    lv_obj_t *      cand_panel;
    lv_obj_t *      cand_panel1;
    lv_ime_pinyin_t * pinyin_ime;
    uint32_t        key_count;

    kb                  = pinyin_param->kb;
    cand_panel          = pinyin_param->cand_panel;
    cand_panel1         = pinyin_param->cand_panel1;
    pinyin_ime          = (lv_ime_pinyin_t *) pinyin_param->pinyin_ime;
    key_count           = 0;

    switch (key)
    {
        case LV_KEY_UP: /*0x11*/
            {
                __inf("pinyin:key,%d,pinyin_ime->cand_num:%d,pinyin_param->cand_panel1:%d",key,pinyin_ime->cand_num,
                    pinyin_param->cand_panel1);

                if ((pinyin_param->cand_panel1 == NULL) &&
                     (lv_obj_has_flag(pinyin_param->cand_panel,LV_OBJ_FLAG_HIDDEN))) {
                    lv_textarea_cursor_up(pinyin_param->ta_user);
                    break;
                    }


                if (pinyin_param->active_flag > 0) {
                    if (pinyin_param->cand_panel1 == NULL) {
                        if (pinyin_param->active_flag == 1)
                            break;
                        }

                    pinyin_param->active_flag--;

                    if (pinyin_param->active_flag == 0) {
                        lv_btnmatrix_clear_btn_ctrl(cand_panel,pinyin_param->cand_panel_index,
                            LV_BTNMATRIX_CTRL_CHECKED);
                        ime_pinyin_pinyin_focused(pinyin_param,pinyin_param->cand_panel_index1);
                        }
                    else {
                        if (pinyin_ime->py_page > 0) {
                            pinyin_page_proc((lv_obj_t *) pinyin_ime,0);
                            pinyin_param->cand_panel_index = 0;
                            lv_btnmatrix_set_btn_ctrl(cand_panel,pinyin_param->cand_panel_index,
                                LV_BTNMATRIX_CTRL_CHECKED);
                            }
                        }
                    }


                }
            break;

        case LV_KEY_DOWN: /*0x12*/
            {
                if ((pinyin_param->cand_panel1 == NULL) &&
                     (lv_obj_has_flag(pinyin_param->cand_panel,LV_OBJ_FLAG_HIDDEN))) {
                    lv_textarea_cursor_down(pinyin_param->ta_user);
                    break;
                    }

                if (pinyin_param->active_flag == 0) {
                    if (pinyin_param->cand_panel1 == NULL)
                        break;

                    pinyin_param->active_flag++;

                    if (pinyin_param->active_flag > 0) {
                        pinyin_param->cand_panel_index = 0;
                        lv_btnmatrix_set_btn_ctrl(cand_panel,pinyin_param->cand_panel_index,
                            LV_BTNMATRIX_CTRL_CHECKED);

                        ime_pinyin_pinyin_defocused(pinyin_param,pinyin_param->cand_panel_index1);
                        }
                    }
                else {
                    uint8_t         page_num = pinyin_ime->cand_num / LV_IME_PINYIN_CAND_TEXT_NUM;
                    uint8_t         sur = pinyin_ime->cand_num % LV_IME_PINYIN_CAND_TEXT_NUM;

                    if ((sur == 0) && (page_num > 0)) {
                        page_num            -= 1;
                        }

                    if (pinyin_ime->py_page < page_num) {
                        pinyin_page_proc((lv_obj_t *) pinyin_ime,1);
                        pinyin_param->active_flag++;
                        pinyin_param->cand_panel_index = 0;
                        lv_btnmatrix_set_btn_ctrl(cand_panel,pinyin_param->cand_panel_index,
                            LV_BTNMATRIX_CTRL_CHECKED);
                        }
                    }

                }
            break;

        case LV_KEY_RIGHT: /*0x13*/
            {
                uint32_t        btn_id;

                if ((pinyin_param->cand_panel1 == NULL) &&
                     (lv_obj_has_flag(pinyin_param->cand_panel,LV_OBJ_FLAG_HIDDEN))) {
                    lv_textarea_cursor_right(pinyin_param->ta_user);
                    break;
                    }


                if (pinyin_param->active_flag == 0) {
                    if (pinyin_param->cand_panel1 == NULL)
                        break;

                    if (pinyin_param->cand_panel_index1 < pinyin_ime->k9_py_ll_pos - 1)
                        btn_id = 16 + pinyin_param->cand_panel_index1 + 1;
                    else 
                        btn_id = 16;

                    lv_btnmatrix_set_selected_btn(kb,btn_id);
                    lv_event_send(kb,LV_EVENT_VALUE_CHANGED,&btn_id);
                    }
                else {
                    uint8_t         count;
                    uint8_t         page_num = pinyin_ime->cand_num / LV_IME_PINYIN_CAND_TEXT_NUM;
                    uint8_t         sur = pinyin_ime->cand_num % LV_IME_PINYIN_CAND_TEXT_NUM;

                    if ((sur == 0) && (page_num > 0)) {
                        page_num            -= 1;
                        }

                    if (pinyin_ime->py_page < page_num) {
                        count               = LV_IME_PINYIN_CAND_TEXT_NUM;
                        }
                    else {
                        if (sur > 0)
                            count = sur;
                        else 
                            count = LV_IME_PINYIN_CAND_TEXT_NUM;
                        }

                    if (pinyin_param->cand_panel_index < count - 1)
                        pinyin_param->cand_panel_index++;
                    else 
                        pinyin_param->cand_panel_index = 0;

                    lv_btnmatrix_set_btn_ctrl(cand_panel,pinyin_param->cand_panel_index,LV_BTNMATRIX_CTRL_CHECKED);
                    }


                }
            break;

        case LV_KEY_LEFT: /*0x14*/
            {
                uint32_t        btn_id;

                if ((pinyin_param->cand_panel1 == NULL) &&
                     (lv_obj_has_flag(pinyin_param->cand_panel,LV_OBJ_FLAG_HIDDEN))) {
                    lv_textarea_cursor_left(pinyin_param->ta_user);
                    break;
                    }


                if (pinyin_param->active_flag == 0) {
                    if (pinyin_param->cand_panel1 == NULL)
                        break;

                    if (pinyin_param->cand_panel_index1 > 0)
                        btn_id = 16 + pinyin_param->cand_panel_index1 - 1;
                    else 
                        btn_id = 16 + pinyin_ime->k9_py_ll_pos - 1;

                    lv_btnmatrix_set_selected_btn(kb,btn_id);
                    lv_event_send(kb,LV_EVENT_VALUE_CHANGED,&btn_id);
                    }
                else {
                    uint8_t         count;
                    uint8_t         page_num = pinyin_ime->cand_num / LV_IME_PINYIN_CAND_TEXT_NUM;
                    uint8_t         sur = pinyin_ime->cand_num % LV_IME_PINYIN_CAND_TEXT_NUM;

                    if ((sur == 0) && (page_num > 0)) {
                        page_num            -= 1;
                        }

                    if (pinyin_ime->py_page < page_num) {
                        count               = LV_IME_PINYIN_CAND_TEXT_NUM;
                        }
                    else {
                        if (sur > 0)
                            count = sur;
                        else 
                            count = LV_IME_PINYIN_CAND_TEXT_NUM;
                        }


                    if (pinyin_param->cand_panel_index > 0)
                        pinyin_param->cand_panel_index--;
                    else 
                        pinyin_param->cand_panel_index = count - 1;

                    lv_btnmatrix_set_btn_ctrl(cand_panel,pinyin_param->cand_panel_index,LV_BTNMATRIX_CTRL_CHECKED);

                    }


                }
            break;

        case LV_KEY_ESC: /*0x1B*/
            break;

        case LV_KEY_DEL: /*0x7F*/
            {
                uint32_t        btn_id = 4;         //删除

                __inf("ime_pinyin_ui_event_actions btn_id:%d\r\n",btn_id);

                if ((pinyin_param->cand_panel1 == NULL) &&
                     (false == lv_obj_has_flag(pinyin_param->cand_panel,LV_OBJ_FLAG_HIDDEN)))
                    btn_id = 14;


                lv_btnmatrix_set_selected_btn(kb,btn_id);

                lv_event_send(kb,LV_EVENT_VALUE_CHANGED,&btn_id);
                }
            break;

        case LV_KEY_BACKSPACE: /*0x08*/
            break;

        case LV_KEY_ENTER: /*0x0A, '\n'*/
            {
                if (pinyin_param->ta_user == NULL)
                    break;

                if ((pinyin_param->active_flag == 0) && (pinyin_param->cand_panel1 == NULL))
                    break;
                else if (pinyin_param->active_flag == 0) {
                    pinyin_param->active_flag++;

                    if (pinyin_param->active_flag > 0) {
                        pinyin_param->cand_panel_index = 0;
                        lv_btnmatrix_set_btn_ctrl(cand_panel,pinyin_param->cand_panel_index,
                            LV_BTNMATRIX_CTRL_CHECKED);

                        ime_pinyin_pinyin_defocused(pinyin_param,pinyin_param->cand_panel_index1);
                        }
                    }
                else {
                    uint32_t        btn_id = 9;
                    const char *    txt = lv_btnmatrix_get_btn_text(cand_panel,pinyin_param->cand_panel_index);

                    lv_textarea_add_text(pinyin_param->ta_user,txt);

                    lv_btnmatrix_set_selected_btn(kb,btn_id);
                    lv_event_send(kb,LV_EVENT_VALUE_CHANGED,&btn_id);
                    }
                }
            break;

        case LV_KEY_NEXT: /*0x09, '\t'*/
        case LV_KEY_PREV: /*0x0B, '*/
        case LV_KEY_HOME: /*0x02, STX*/
            break;

        case LV_KEY_END: /*0x03, ETX*/
            alv_am_app_exit_action_all();
            break;

        case LV_KEY_GREEN: // = 200,
        case LV_KEY_RED:
            break;

        case LV_KEY_STAR:
            if (0 == get_ime_pinyin_keyboard_active_state(pinyin_param)) {
                ime_pinyin_punctuation_marks_create(pinyin_param);
                }

            break;

        case LV_KEY_HASH:
            if (pinyin_param->format_flag_auto) {
                if (get_ime_pinyin_keyboard_active_state(pinyin_param)) {
                    uint32_t        btn_id = 4;     //删除

                    __inf("ime_pinyin_ui_event_actions btn_id:%d\r\n",btn_id);

                    if ((pinyin_param->cand_panel1 == NULL) &&
                         (false == lv_obj_has_flag(pinyin_param->cand_panel,LV_OBJ_FLAG_HIDDEN))) {
                        ime_pinyin_ui_set_fortmat(pinyin_param,format_bihua);
                        btn_id              = 14;
                        }

                    lv_btnmatrix_set_selected_btn(kb,btn_id);

                    lv_event_send(kb,LV_EVENT_VALUE_CHANGED,&btn_id);


                    }
                else {
                    if (pinyin_param->format == format_pinyin) {
                        ime_pinyin_ui_set_fortmat(pinyin_param,format_bihua);
                        }
                    else {
                        ime_pinyin_ui_set_fortmat(pinyin_param,format_abc);
                        }
                    }
                }

            break;

        /*
        {\
            ",\0", "1#\0",  "abc \0", "def\0",   LV_SYMBOL_BACKSPACE"\0", "\n\0",
            ".\0", "ghi\0", "jkl\0",  "mno\0", 	 LV_SYMBOL_OK"\0", "\n\0",
            "?\0", "pqrs\0", "tuv\0", "wxyz\0",  LV_SYMBOL_NEW_LINE"\0", "\n\0",
            LV_SYMBOL_LEFT"\0", "\0"
        };
        */
        case LV_KEY_0:
            {
                uint32_t        data;

                data                = ' ';

                if (pinyin_param->ta_user)
                    lv_textarea_add_char(pinyin_param->ta_user,data);

                //__inf("ime_pinyin_num_event_actions:0x%x",data);
                }
            break;

        case LV_KEY_1:
            {
                uint32_t        data;

                data                = '0' + (key - LV_KEY_0);

                if (pinyin_param->ta_user)
                    lv_textarea_add_char(pinyin_param->ta_user,data);

                //__inf("ime_pinyin_num_event_actions:0x%x",data);
                }
            break;

        case LV_KEY_9:
        case LV_KEY_8:
        case LV_KEY_7:
            key_count++;
            key_count++;
        case LV_KEY_6:
        case LV_KEY_5:
        case LV_KEY_4:
            key_count++;
            key_count++;
        case LV_KEY_3:
        case LV_KEY_2:
            {
                uint32_t        btn_id = key - LV_KEY_0 + key_count;

                __inf("ime_pinyin_ui_event_actions btn_id:%d\r\n",btn_id);


                lv_btnmatrix_set_selected_btn(kb,btn_id);

                lv_event_send(kb,LV_EVENT_VALUE_CHANGED,&btn_id);
                }
            break;
        }
}

lv_timer_t *    key_star_timer = NULL;
uint8_t         key_star_counter = 0;

static void start_key_star_timer_event_cb(struct _lv_timer_t *timer)
{
    if (key_star_timer) {
        lv_timer_del(key_star_timer);
        key_star_timer      = NULL;
        }

    key_star_counter    = 0;
}

void start_key_star_timer(void)
{

    if (key_star_timer == NULL) {
        key_star_timer      = lv_timer_create(start_key_star_timer_event_cb,1000,0);
        }
    else {
        lv_timer_reset(key_star_timer);
        lv_timer_resume(key_star_timer);
        }

}

void stop_key_star_timer(void)
{
    if (key_star_timer) {
        lv_timer_del(key_star_timer);
        key_star_timer      = NULL;
        }

    key_star_counter    = 0;
}

void reset_key_star(void)
{
    stop_key_star_timer();
    start_key_star_timer();
}

uint8_t output_char_by_location_for_star(lv_obj_t * label,char * star)
{
    char            star_array[4] = {
        '*','+','P','W'
        };

    if (key_star_timer == NULL) {
        start_key_star_timer();
        *star               = '*';
        return 0;
        }

    uint16_t        counter = _lv_txt_get_encoded_length(lv_label_get_text(label));

    if (key_star_timer) {
        key_star_counter    += 1;

        if (counter == 1) {
            if (key_star_counter >= 2)
                key_star_counter = 0;
            }
        else {
            if (key_star_counter >= 4)
                key_star_counter = 0;
            }

        start_key_star_timer();
        *star               = star_array[key_star_counter];
        return 1;
        }

    return 0;
}

void ime_pinyin_num_event_actions(uint32_t key,pinyin_param_t * pinyin_param)
{
    lv_obj_t *      kb;
    lv_obj_t *      cand_panel;
    lv_obj_t *      cand_panel1;
    lv_ime_pinyin_t * pinyin_ime;

    kb                  = pinyin_param->kb;
    cand_panel          = pinyin_param->cand_panel;
    cand_panel1         = pinyin_param->cand_panel1;
    pinyin_ime          = (lv_ime_pinyin_t *) pinyin_param->pinyin_ime;

    if (key != LV_KEY_STAR) {
        stop_key_star_timer();
        }

    switch (key)
    {
        case LV_KEY_UP: /*0x11*/
            {
                lv_textarea_cursor_up(pinyin_param->ta_user);
                }
            break;

        case LV_KEY_DOWN: /*0x12*/
            {

                lv_textarea_cursor_down(pinyin_param->ta_user);
                }
            break;

        case LV_KEY_RIGHT: /*0x13*/
            {

                lv_textarea_cursor_right(pinyin_param->ta_user);

                }
            break;

        case LV_KEY_LEFT: /*0x14*/
            {


                lv_textarea_cursor_left(pinyin_param->ta_user);

                }
            break;

        case LV_KEY_ESC: /*0x1B*/
            break;

        case LV_KEY_DEL: /*0x7F*/

            if (pinyin_param->ta_user)
                lv_textarea_del_char(pinyin_param->ta_user);

            break;

        case LV_KEY_BACKSPACE: /*0x08*/
            break;

        case LV_KEY_ENTER: /*0x0A, '\n'*/
            {

                }
            break;

        case LV_KEY_NEXT: /*0x09, '\t'*/
        case LV_KEY_PREV: /*0x0B, '*/
        case LV_KEY_HOME: /*0x02, STX*/
            break;

        case LV_KEY_END: /*0x03, ETX*/
            alv_am_app_exit_action_all();
            break;

        case LV_KEY_GREEN: // = 200,
        case LV_KEY_RED:
            break;

        case LV_KEY_STAR:
            {
                if (pinyin_param->format_flag_auto) {
                    if (0 == get_ime_pinyin_keyboard_active_state(pinyin_param))
                        ime_pinyin_punctuation_marks_create(pinyin_param);
                    }
                else {
                    char            data;
                    uint8_t         result;

                    //data = '*';
                    if (pinyin_param->ta_user) {
                        lv_obj_t *      label = lv_textarea_get_label(pinyin_param->ta_user);

                        result              = output_char_by_location_for_star(label,&data);

                        if (result)
                            lv_textarea_del_char(pinyin_param->ta_user);

                        lv_textarea_add_char(pinyin_param->ta_user,data);
                        }
                    }

                }
            break;

        case LV_KEY_HASH:
            {
                if (pinyin_param->format_flag_auto) {
                    ime_pinyin_ui_set_fortmat(pinyin_param,format_pinyin);
                    }
                else {
                    uint32_t        data;

                    data                = '#';

                    if (pinyin_param->ta_user)
                        lv_textarea_add_char(pinyin_param->ta_user,data);

                    }

                }
            break;

        case LV_KEY_0:
        case LV_KEY_1:
        case LV_KEY_2:
        case LV_KEY_3:
        case LV_KEY_4:
        case LV_KEY_5:
        case LV_KEY_6:
        case LV_KEY_7:
        case LV_KEY_8:
        case LV_KEY_9:
            {
                uint32_t        data;

                data                = '0' + (key - LV_KEY_0);

                if (pinyin_param->ta_user)
                    lv_textarea_add_char(pinyin_param->ta_user,data);

                //__inf("ime_pinyin_num_event_actions:0x%x",data);
                }
            break;
        }
}


void ime_pinyin_punctuation_marks_event_actions(uint32_t key,pinyin_param_t * pinyin_param)
{
    lv_obj_t *      cand_punctuation_marks;         //标点符号
    uint32_t        btn_count;

    cand_punctuation_marks = pinyin_param->cand_punctuation_marks;
    btn_count           = pinyin_param->cand_punctuation_marks_count;

    //__inf("ime_pinyin_punctuation_marks_event_actions");
    switch (key)
    {
        case LV_KEY_UP: /*0x11*/
            {
                uint16_t        btn_id;

                btn_id              = lv_btnmatrix_get_selected_btn(cand_punctuation_marks);

                if (btn_id > PUNCTUATION_MARKS_LINE_COUNT)
                    btn_id = btn_id - PUNCTUATION_MARKS_LINE_COUNT;

                lv_btnmatrix_set_selected_btn(cand_punctuation_marks,btn_id);

                lv_btnmatrix_set_btn_ctrl(cand_punctuation_marks,btn_id,LV_BTNMATRIX_CTRL_CHECKED);
                }
            break;

        case LV_KEY_DOWN: /*0x12*/
            {
                uint16_t        btn_id;

                btn_id              = lv_btnmatrix_get_selected_btn(cand_punctuation_marks);

                if (btn_id + PUNCTUATION_MARKS_LINE_COUNT < btn_count - 1)
                    btn_id = btn_id + PUNCTUATION_MARKS_LINE_COUNT;

                lv_btnmatrix_set_selected_btn(cand_punctuation_marks,btn_id);

                lv_btnmatrix_set_btn_ctrl(cand_punctuation_marks,btn_id,LV_BTNMATRIX_CTRL_CHECKED);

                }
            break;

        case LV_KEY_RIGHT: /*0x13*/
            {
                uint16_t        btn_id;

                btn_id              = lv_btnmatrix_get_selected_btn(cand_punctuation_marks);

                if (btn_id < btn_count - 1)
                    btn_id++;

                lv_btnmatrix_set_selected_btn(cand_punctuation_marks,btn_id);

                lv_btnmatrix_set_btn_ctrl(cand_punctuation_marks,btn_id,LV_BTNMATRIX_CTRL_CHECKED);

                }
            break;

        case LV_KEY_LEFT: /*0x14*/
            {
                uint16_t        btn_id;

                btn_id              = lv_btnmatrix_get_selected_btn(cand_punctuation_marks);

                if (btn_id > 0)
                    btn_id--;

                lv_btnmatrix_set_selected_btn(cand_punctuation_marks,btn_id);

                lv_btnmatrix_set_btn_ctrl(cand_punctuation_marks,btn_id,LV_BTNMATRIX_CTRL_CHECKED);
                }
            break;

        case LV_KEY_ESC: /*0x1B*/
            break;

        case LV_KEY_DEL: /*0x7F*/
            {
                ime_pinyin_punctuation_marks_destory(pinyin_param);
                }
            break;

        case LV_KEY_BACKSPACE: /*0x08*/
            break;

        case LV_KEY_ENTER: /*0x0A, '\n'*/
            {
                uint16_t        btn_id;
                char *          txt;

                btn_id              = lv_btnmatrix_get_selected_btn(cand_punctuation_marks);
                txt                 = (char *) lv_btnmatrix_get_btn_text(cand_punctuation_marks,btn_id);

                if (pinyin_param->ta_user)
                    lv_textarea_add_text(pinyin_param->ta_user,txt);


                ime_pinyin_punctuation_marks_destory(pinyin_param);

                }
            break;

        case LV_KEY_NEXT: /*0x09, '\t'*/
        case LV_KEY_PREV: /*0x0B, '*/
        case LV_KEY_HOME: /*0x02, STX*/
            break;

        case LV_KEY_END: /*0x03, ETX*/
            alv_am_app_exit_action_all();
            break;

        case LV_KEY_GREEN: // = 200,
        case LV_KEY_RED:
        case LV_KEY_STAR:
            break;

        case LV_KEY_HASH:
            break;

        case LV_KEY_0:
            break;

        case LV_KEY_1:
            break;

        case LV_KEY_9:
        case LV_KEY_8:
        case LV_KEY_7:
        case LV_KEY_6:
        case LV_KEY_5:
        case LV_KEY_4:
        case LV_KEY_3:
        case LV_KEY_2:
            break;
        }
}


void ime_pinyin_abc_event_actions(uint32_t key,pinyin_param_t * pinyin_param)
{
    lv_obj_t *      cand_abc;                       //标点符号
    uint32_t        btn_count;

    cand_abc            = pinyin_param->cand_abc;
    btn_count           = pinyin_param->cand_abc_count;

    //__inf("ime_pinyin_punctuation_marks_event_actions");
    switch (key)
    {
        case LV_KEY_UP: /*0x11*/
            {
                if (cand_abc == NULL)
                    lv_textarea_cursor_up(pinyin_param->ta_user);
                }
            break;

        case LV_KEY_DOWN: /*0x12*/
            {
                if (cand_abc == NULL)
                    lv_textarea_cursor_down(pinyin_param->ta_user);

                }
            break;

        case LV_KEY_RIGHT: /*0x13*/
            {
                uint16_t        btn_id;

                if (cand_abc == NULL) {
                    lv_textarea_cursor_right(pinyin_param->ta_user);
                    break;
                    }

                btn_id              = lv_btnmatrix_get_selected_btn(cand_abc);

                if (btn_id < btn_count - 1)
                    btn_id++;

                lv_btnmatrix_set_selected_btn(cand_abc,btn_id);

                lv_btnmatrix_set_btn_ctrl(cand_abc,btn_id,LV_BTNMATRIX_CTRL_CHECKED);

                }
            break;

        case LV_KEY_LEFT: /*0x14*/
            {
                uint16_t        btn_id;

                if (cand_abc == NULL) {
                    lv_textarea_cursor_left(pinyin_param->ta_user);
                    break;
                    }

                btn_id              = lv_btnmatrix_get_selected_btn(cand_abc);

                if (btn_id > 0)
                    btn_id--;

                lv_btnmatrix_set_selected_btn(cand_abc,btn_id);

                lv_btnmatrix_set_btn_ctrl(cand_abc,btn_id,LV_BTNMATRIX_CTRL_CHECKED);
                }
            break;

        case LV_KEY_ESC: /*0x1B*/
            break;

        case LV_KEY_DEL: /*0x7F*/
            {
                ime_pinyin_abc_destory(pinyin_param);
                }
            break;

        case LV_KEY_BACKSPACE: /*0x08*/
            break;

        case LV_KEY_ENTER: /*0x0A, '\n'*/
            {
                uint16_t        btn_id;
                char *          txt;

                if (cand_abc == NULL)
                    break;

                btn_id              = lv_btnmatrix_get_selected_btn(cand_abc);
                txt                 = (char *) lv_btnmatrix_get_btn_text(cand_abc,btn_id);

                if (pinyin_param->ta_user)
                    lv_textarea_add_text(pinyin_param->ta_user,txt);


                //ime_pinyin_abc_destory(pinyin_param);
                }
            break;

        case LV_KEY_NEXT: /*0x09, '\t'*/
        case LV_KEY_PREV: /*0x0B, '*/
        case LV_KEY_HOME: /*0x02, STX*/
            break;

        case LV_KEY_END: /*0x03, ETX*/
            alv_am_app_exit_action_all();
            break;

        case LV_KEY_GREEN: // = 200,
        case LV_KEY_RED:
            break;

        case LV_KEY_STAR:
            if (0 == get_ime_pinyin_keyboard_active_state(pinyin_param))
                ime_pinyin_punctuation_marks_create(pinyin_param);

            break;

        case LV_KEY_HASH:
            ime_pinyin_abc_destory(pinyin_param);
            ime_pinyin_ui_set_fortmat(pinyin_param,format_num);
            break;

        case LV_KEY_0:
            {
                uint32_t        data;

                data                = ' ';

                if (pinyin_param->ta_user)
                    lv_textarea_add_char(pinyin_param->ta_user,data);

                //__inf("ime_pinyin_num_event_actions:0x%x",data);
                }
            break;

        case LV_KEY_1:
            {
                uint32_t        data;

                data                = '0' + (key - LV_KEY_0);

                if (pinyin_param->ta_user)
                    lv_textarea_add_char(pinyin_param->ta_user,data);

                //__inf("ime_pinyin_num_event_actions:0x%x",data);
                }
            break;

        case LV_KEY_9:
        case LV_KEY_8:
        case LV_KEY_7:
        case LV_KEY_6:
        case LV_KEY_5:
        case LV_KEY_4:
        case LV_KEY_3:
        case LV_KEY_2:
            ime_pinyin_abc_create(pinyin_param,key - LV_KEY_2);
            break;
        }
}


/*
        {\
            ",\0", "1#\0",  "abc \0", "def\0",  LV_SYMBOL_BACKSPACE"\0", "\n\0",
            ".\0", "ghi\0", "jkl\0", "mno\0",  LV_SYMBOL_OK"\0", "\n\0",
            "?\0", "pqrs\0", "tuv\0", "wxyz\0",  LV_SYMBOL_NEW_LINE"\0", "\n\0",
            LV_SYMBOL_LEFT"\0", "\0"
        };
      */
void ime_pinyin_bihua_event_actions(uint32_t key,pinyin_param_t * pinyin_param)
{
    lv_obj_t *      kb;
    lv_obj_t *      cand_panel;
    lv_obj_t *      cand_panel1;
    lv_ime_pinyin_t * pinyin_ime;
    uint32_t        key_count;

    kb                  = pinyin_param->kb;
    cand_panel          = pinyin_param->cand_panel;
    cand_panel1         = pinyin_param->cand_panel1;
    pinyin_ime          = (lv_ime_pinyin_t *) pinyin_param->pinyin_ime;
    key_count           = 0;

    switch (key)
    {
        case LV_KEY_UP: /*0x11*/
            {
                if ((pinyin_param->cand_panel1 == NULL) &&
                     (lv_obj_has_flag(pinyin_param->cand_panel,LV_OBJ_FLAG_HIDDEN))) {
                    lv_textarea_cursor_up(pinyin_param->ta_user);
                    break;
                    }

                if (pinyin_param->active_flag > 0) {
                    if (pinyin_param->cand_panel1 == NULL) {
                        if (pinyin_param->active_flag == 1)
                            break;
                        }

                    pinyin_param->active_flag--;

                    if (pinyin_param->active_flag == 0) {
                        lv_btnmatrix_clear_btn_ctrl(cand_panel,pinyin_param->cand_panel_index,
                            LV_BTNMATRIX_CTRL_CHECKED);
                        ime_pinyin_pinyin_focused(pinyin_param,pinyin_param->cand_panel_index1);
                        }
                    else {
                        if (pinyin_ime->py_page > 0) {
                            pinyin_page_proc((lv_obj_t *) pinyin_ime,0);
                            pinyin_param->cand_panel_index = 0;
                            lv_btnmatrix_set_btn_ctrl(cand_panel,pinyin_param->cand_panel_index,
                                LV_BTNMATRIX_CTRL_CHECKED);
                            }
                        }
                    }
                }
            break;

        case LV_KEY_DOWN: /*0x12*/
            {
                if ((pinyin_param->cand_panel1 == NULL) &&
                     (lv_obj_has_flag(pinyin_param->cand_panel,LV_OBJ_FLAG_HIDDEN))) {
                    lv_textarea_cursor_down(pinyin_param->ta_user);
                    break;
                    }

                if (pinyin_param->active_flag == 0) {
                    if (pinyin_param->cand_panel1 == NULL)
                        break;

                    pinyin_param->active_flag++;

                    if (pinyin_param->active_flag > 0) {
                        pinyin_param->cand_panel_index = 0;
                        lv_btnmatrix_set_btn_ctrl(cand_panel,pinyin_param->cand_panel_index,
                            LV_BTNMATRIX_CTRL_CHECKED);

                        ime_pinyin_pinyin_defocused(pinyin_param,pinyin_param->cand_panel_index1);
                        }
                    }
                else {
                    uint8_t         page_num = pinyin_ime->cand_num / LV_IME_PINYIN_CAND_TEXT_NUM;
                    uint8_t         sur = pinyin_ime->cand_num % LV_IME_PINYIN_CAND_TEXT_NUM;

                    if ((sur == 0) && (page_num > 0)) {
                        page_num            -= 1;
                        }

                    if (pinyin_ime->py_page < page_num) {
                        pinyin_page_proc((lv_obj_t *) pinyin_ime,1);
                        pinyin_param->active_flag++;
                        pinyin_param->cand_panel_index = 0;
                        lv_btnmatrix_set_btn_ctrl(cand_panel,pinyin_param->cand_panel_index,
                            LV_BTNMATRIX_CTRL_CHECKED);
                        }
                    }

                }
            break;

        case LV_KEY_RIGHT: /*0x13*/
            {
                uint32_t        btn_id;

                if ((pinyin_param->cand_panel1 == NULL) &&
                     (lv_obj_has_flag(pinyin_param->cand_panel,LV_OBJ_FLAG_HIDDEN))) {
                    lv_textarea_cursor_right(pinyin_param->ta_user);
                    break;
                    }

                if (pinyin_param->active_flag == 0) {
                    if (pinyin_param->cand_panel1 == NULL)
                        break;


                    if (pinyin_param->cand_panel_index1 < pinyin_ime->k9_py_ll_pos - 1)
                        btn_id = 16 + pinyin_param->cand_panel_index1 + 1;
                    else 
                        btn_id = 16;

                    lv_btnmatrix_set_selected_btn(kb,btn_id);
                    lv_event_send(kb,LV_EVENT_VALUE_CHANGED,&btn_id);
                    }
                else {
                    uint8_t         count;
                    uint8_t         page_num = pinyin_ime->cand_num / LV_IME_PINYIN_CAND_TEXT_NUM;
                    uint8_t         sur = pinyin_ime->cand_num % LV_IME_PINYIN_CAND_TEXT_NUM;

                    if ((sur == 0) && (page_num > 0)) {
                        page_num            -= 1;
                        }

                    if (pinyin_ime->py_page < page_num) {
                        count               = LV_IME_PINYIN_CAND_TEXT_NUM;
                        }
                    else {
                        if (sur > 0)
                            count = sur;
                        else 
                            count = LV_IME_PINYIN_CAND_TEXT_NUM;
                        }

                    if (pinyin_param->cand_panel_index < count - 1)
                        pinyin_param->cand_panel_index++;
                    else 
                        pinyin_param->cand_panel_index = 0;

                    lv_btnmatrix_set_btn_ctrl(cand_panel,pinyin_param->cand_panel_index,LV_BTNMATRIX_CTRL_CHECKED);
                    }


                }
            break;

        case LV_KEY_LEFT: /*0x14*/
            {
                uint32_t        btn_id;

                if ((pinyin_param->cand_panel1 == NULL) &&
                     (lv_obj_has_flag(pinyin_param->cand_panel,LV_OBJ_FLAG_HIDDEN))) {
                    lv_textarea_cursor_left(pinyin_param->ta_user);
                    break;
                    }

                if (pinyin_param->active_flag == 0) {
                    if (pinyin_param->cand_panel1 == NULL)
                        break;

                    if (pinyin_param->cand_panel_index1 > 0)
                        btn_id = 16 + pinyin_param->cand_panel_index1 - 1;
                    else 
                        btn_id = 16 + pinyin_ime->k9_py_ll_pos - 1;

                    lv_btnmatrix_set_selected_btn(kb,btn_id);
                    lv_event_send(kb,LV_EVENT_VALUE_CHANGED,&btn_id);
                    }
                else {
                    uint8_t         count;
                    uint8_t         page_num = pinyin_ime->cand_num / LV_IME_PINYIN_CAND_TEXT_NUM;
                    uint8_t         sur = pinyin_ime->cand_num % LV_IME_PINYIN_CAND_TEXT_NUM;

                    if ((sur == 0) && (page_num > 0)) {
                        page_num            -= 1;
                        }

                    if (pinyin_ime->py_page < page_num) {
                        count               = LV_IME_PINYIN_CAND_TEXT_NUM;
                        }
                    else {
                        if (sur > 0)
                            count = sur;
                        else 
                            count = LV_IME_PINYIN_CAND_TEXT_NUM;
                        }


                    if (pinyin_param->cand_panel_index > 0)
                        pinyin_param->cand_panel_index--;
                    else 
                        pinyin_param->cand_panel_index = count - 1;

                    lv_btnmatrix_set_btn_ctrl(cand_panel,pinyin_param->cand_panel_index,LV_BTNMATRIX_CTRL_CHECKED);

                    }


                }
            break;

        case LV_KEY_ESC: /*0x1B*/
            break;

        case LV_KEY_DEL: /*0x7F*/
            {
                uint32_t        btn_id = 4;         //删除

                __inf("ime_pinyin_bihua_event_actions btn_id:%d\r\n",btn_id);

                if ((pinyin_param->cand_panel1 == NULL) &&
                     (false == lv_obj_has_flag(pinyin_param->cand_panel,LV_OBJ_FLAG_HIDDEN)))
                    btn_id = 14;


                lv_btnmatrix_set_selected_btn(kb,btn_id);

                lv_event_send(kb,LV_EVENT_VALUE_CHANGED,&btn_id);
                }
            break;

        case LV_KEY_BACKSPACE: /*0x08*/
            break;

        case LV_KEY_ENTER: /*0x0A, '\n'*/
            {
                if (pinyin_param->ta_user == NULL)
                    break;

                if ((pinyin_param->active_flag == 0) && (pinyin_param->cand_panel1 == NULL))
                    break;
                else if (pinyin_param->active_flag == 0) {
                    pinyin_param->active_flag++;

                    if (pinyin_param->active_flag > 0) {
                        pinyin_param->cand_panel_index = 0;
                        lv_btnmatrix_set_btn_ctrl(cand_panel,pinyin_param->cand_panel_index,
                            LV_BTNMATRIX_CTRL_CHECKED);
                        ime_pinyin_pinyin_defocused(pinyin_param,pinyin_param->cand_panel_index1);
                        }
                    }
                else {
                    uint32_t        btn_id = 9;
                    const char *    txt = lv_btnmatrix_get_btn_text(cand_panel,pinyin_param->cand_panel_index);

                    lv_textarea_add_text(pinyin_param->ta_user,txt);

                    lv_btnmatrix_set_selected_btn(kb,btn_id);
                    lv_event_send(kb,LV_EVENT_VALUE_CHANGED,&btn_id);
                    }
                }
            break;

        case LV_KEY_NEXT: /*0x09, '\t'*/
        case LV_KEY_PREV: /*0x0B, '*/
        case LV_KEY_HOME: /*0x02, STX*/
            break;

        case LV_KEY_END: /*0x03, ETX*/
            alv_am_app_exit_action_all();
            break;

        case LV_KEY_GREEN: // = 200,
        case LV_KEY_RED:
            break;

        case LV_KEY_STAR:
            if (0 == get_ime_pinyin_keyboard_active_state(pinyin_param)) {
                ime_pinyin_punctuation_marks_create(pinyin_param);
                }

            break;

        case LV_KEY_HASH:
            if (pinyin_param->format_flag_auto) {
                if (get_ime_pinyin_keyboard_active_state(pinyin_param)) {
                    uint32_t        btn_id = 4;     //删除

                    __inf("ime_pinyin_ui_event_actions btn_id:%d\r\n",btn_id);

                    lv_btnmatrix_set_selected_btn(kb,btn_id);

                    lv_event_send(kb,LV_EVENT_VALUE_CHANGED,&btn_id);
                    }
                else {
                    if (pinyin_param->format == format_pinyin) {
                        ime_pinyin_ui_set_fortmat(pinyin_param,format_bihua);
                        }
                    else {
                        ime_pinyin_ui_set_fortmat(pinyin_param,format_abc);
                        }
                    }
                }

            break;

        /*
        {\
            ",\0", "1#\0",  "abc \0", "def\0",   LV_SYMBOL_BACKSPACE"\0", "\n\0",
            ".\0", "ghi\0", "jkl\0",  "mno\0", 	 LV_SYMBOL_OK"\0", "\n\0",
            "?\0", "pqrs\0", "tuv\0", "wxyz\0",  LV_SYMBOL_NEW_LINE"\0", "\n\0",
            LV_SYMBOL_LEFT"\0", "\0"
        };
        */
        case LV_KEY_0:
            {
                uint32_t        data;

                data                = ' ';

                if (pinyin_param->ta_user)
                    lv_textarea_add_char(pinyin_param->ta_user,data);

                //__inf("ime_pinyin_num_event_actions:0x%x",data);
                }
            break;

        case LV_KEY_9:
        case LV_KEY_8:
        case LV_KEY_7:
            key_count++;
            key_count++;
        case LV_KEY_6:
            break;

        case LV_KEY_5:
        case LV_KEY_4:
        case LV_KEY_3:
            key_count++;
            key_count++;
        case LV_KEY_2:
        case LV_KEY_1:
            key_count++;
            {
                uint32_t        btn_id = key - LV_KEY_0 + key_count;

                __inf("ime_pinyin_ui_event_actions btn_id:%d\r\n",btn_id);


                lv_btnmatrix_set_selected_btn(kb,btn_id);

                lv_event_send(kb,LV_EVENT_VALUE_CHANGED,&btn_id);
                }
            break;
        }
}


void ime_pinyin_ui_event_actions(uint32_t key,pinyin_param_t * pinyin_param)
{

    switch (pinyin_param->format)
    {
        case format_pinyin:
            if (pinyin_param->cand_punctuation_marks) {
                ime_pinyin_punctuation_marks_event_actions(key,pinyin_param);
                }
            else {
                ime_pinyin_keyboard_event_actions(key,pinyin_param);
                }

            break;

        case format_num:
            if (pinyin_param->cand_punctuation_marks) {
                ime_pinyin_punctuation_marks_event_actions(key,pinyin_param);
                }
            else {
                ime_pinyin_num_event_actions(key,pinyin_param);
                }

            break;

        case format_abc:
            if (pinyin_param->cand_punctuation_marks) {
                ime_pinyin_punctuation_marks_event_actions(key,pinyin_param);
                }
            else {
                ime_pinyin_abc_event_actions(key,pinyin_param);
                }

            break;

        case format_bihua:
            if (pinyin_param->cand_punctuation_marks) {
                ime_pinyin_punctuation_marks_event_actions(key,pinyin_param);
                }
            else {
                ime_pinyin_bihua_event_actions(key,pinyin_param);
                }

            break;
        }
}


uint32_t get_ime_pinyin_keyboard_active_state(pinyin_param_t * pinyin_param)
{
    uint32_t        ret;

    ret                 = 0;

    if (pinyin_param == NULL) {
        __inf("get_ime_pinyin_keyboard_active_state pinyin_param==NULL");
        return 0;
        }

    if (false == lv_obj_has_flag(pinyin_param->cand_panel,LV_OBJ_FLAG_HIDDEN))
        ret = 1;

    if (pinyin_param->cand_panel1)
        ret = 1;

    if (pinyin_param->cand_punctuation_marks)
        ret = 1;

    if (pinyin_param->cand_abc)
        ret = 1;

    return ret;
}



void ime_pinyin_ui_set_fortmat_flag_auto(pinyin_param_t * pinyin_param,uint8_t format_flag_auto)
{
    pinyin_param->format_flag_auto = format_flag_auto;
}


void ime_pinyin_ui_set_fortmat(pinyin_param_t * pinyin_param,pinyin_format format) //0:拼音，1：数字	
{

    if (pinyin_param->format == format)
        return;

    pinyin_param->format = format;

    switch (pinyin_param->format)
    {
        case format_pinyin:
            if (pinyin_param->label_format)
                lv_label_set_text(pinyin_param->label_format,"拼音");

            lv_ime_pinyin_set_dict(pinyin_param->pinyin_ime,lv_ime_pinyin_def_dict);
            pinyin_k9_disable_bihua(pinyin_param->pinyin_ime);
            break;

        case format_num:
            if (pinyin_param->label_format)
                lv_label_set_text(pinyin_param->label_format,"数字");

            break;

        case format_abc:
            if (pinyin_param->label_format)
                lv_label_set_text(pinyin_param->label_format,"Abc");

            break;

        case format_bihua:
            if (pinyin_param->label_format)
                lv_label_set_text(pinyin_param->label_format,"笔画");

            lv_ime_pinyin_set_dict(pinyin_param->pinyin_ime,(lv_pinyin_dict_t *) lv_ime_bihua_def_dict);
            pinyin_k9_enable_bihua(pinyin_param->pinyin_ime);
            break;
        }


}


int32_t fp_fs_is_folder(const char * path)
{
    int32_t         result = 0;

    if (path != NULL) {
        struct stat statbuf;

        if (-1 != stat(path,&statbuf)) {
            if (S_ISDIR(statbuf.st_mode)) {
                result              = 1;
                }
            }
        }

    return result;
}


int32_t fp_fs_remove_folder(const char * path)
{
    size_t          path_len;
    char *          full_path;
    DIR *           dir;

    struct stat stat_entry;
    struct dirent * entry;
    int32_t         result = 1;

    if (!fp_fs_is_folder(path)) {
        __inf("app_fs_remove_folder: attempt to remove a non-folder\n");
        return 0;
        }

    /* if not possible to read the directory for this user */
    if ((dir = opendir(path)) == NULL) {
        __inf("app_fs_remove_folder: unable to opendir %s\n",path);
        return 0;
        }

    /* the length of the path */
    path_len            = strlen(path);

    /* iteration through entries in the directory */
    while ((entry = readdir(dir)) != NULL) {
        /* skip entries "." and ".." */
        if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name,".."))
            continue;

        /* determinate a full path of an entry */
        full_path           = lv_mem_alloc(path_len + strlen(entry->d_name) + 2);

        if (full_path) {
            strcpy(full_path,path);
            strcat(full_path,"/");
            strcat(full_path,entry->d_name);

            /* stat for the entry */
            if (-1 != stat(full_path,&stat_entry)) {
                /* recursively remove a nested directory */
                if (S_ISDIR(stat_entry.st_mode) != 0) {
                    result              = fp_fs_remove_folder(full_path);
                    }
                else {
                    /* remove a file object */
                    if (unlink(full_path) == 0) {
                        __inf("app_fs_remove_folder: Removed a file: %s\n",full_path);
                        }
                    else {
                        result              = 0;
                        __inf("app_fs_remove_folder: Can`t remove a file: %s\n",full_path);
                        }
                    }
                }

            lv_mem_free(full_path);

            if (!result) {
                break;
                }
            }
        else {
            result              = 0;
            __inf("app_fs_remove_folder: no memory\n");
            break;
            }
        }

    closedir(dir);

    if (result) {
        /* remove the devastated directory and close the object of it */
        if (rmdir(path) == 0)
            __inf("app_fs_remove_folder: Removed a directory: %s\n",path);
        else 
            __inf("app_fs_remove_folder: Can't remove a directory: %s\n",path);
        }

    return result;
}





int32_t fp_del_file_by_folder_off(char * folder_path,uint32_t file_d_off)
{
    DIR *           dir;

    struct dirent * dinfo = NULL;
    char            path[60];

    if (!fp_fs_is_folder(folder_path)) {
        __inf("fp_del_file_by_folder_off: attempt to remove a non-folder\n");
        return - 1;
        }


    dir                 = opendir(folder_path);

    while (NULL != (dinfo = readdir(dir))) {
        if (file_d_off != dinfo->d_off)
            continue;

        if (NULL != dinfo) {
            strcpy(path,folder_path);
            strcat(path,"/");
            strcat(path,dinfo->d_name);


            }

        closedir(dir);
        break;
        }

    return unlink(path);
}




typedef struct {
    uint32_t        value;
    uint32_t        flag;

} key_state_fpt;

static key_state_fpt g_key;


uint32_t key_release_all()
{
    key_state_fpt * p_key_state;

    p_key_state         = &g_key;

    memset(p_key_state,0,sizeof(key_state_fpt));

    return 0;
}


void key_update_pressed(uint32_t key,uint32_t code)
{
    key_state_fpt * p_key_state;

    p_key_state         = &g_key;

    switch (code)
    {
        case LV_EVENT_PRESSED:
            memset(p_key_state,0,sizeof(key_state_fpt));
            p_key_state->value = key;
            p_key_state->flag |= (1 << code);
            break;

        case LV_EVENT_LONG_PRESSED:
            if ((p_key_state->value == key) && (p_key_state->flag & (1 << LV_EVENT_PRESSED))) {
                p_key_state->flag   |= (1 << code);
                }

            break;

        case LV_EVENT_LONG_PRESSED_REPEAT:
            if ((p_key_state->value == key) && (p_key_state->flag & (1 << LV_EVENT_PRESSED))) {
                p_key_state->flag   |= (1 << code);
                }

            break;

        case LV_EVENT_RELEASED:
            if (p_key_state->value == key) {
                p_key_state->flag   |= (1 << code);
                }

            break;
        }
}


int32_t key_update_release(uint32_t key,uint32_t code)
{
    key_state_fpt * p_key_state;

    p_key_state         = &g_key;


    if (p_key_state->value != key)
        return - 1;

    switch (code)
    {
        case LV_EVENT_PRESSED:
            memset(p_key_state,0,sizeof(key_state_fpt));
            break;

        case LV_EVENT_LONG_PRESSED:
            p_key_state->flag &= ~(1 << code);
            break;

        case LV_EVENT_LONG_PRESSED_REPEAT:
            p_key_state->flag &= ~(1 << code);
            break;

        case LV_EVENT_RELEASED:
            memset(p_key_state,0,sizeof(key_state_fpt));
            break;
        }

    return 0;
}


uint32_t key_get_pressed_flag(uint32_t key)
{
    key_state_fpt * p_key_state;

    p_key_state         = &g_key;

    if (p_key_state->value == key) {
        return p_key_state->flag;
        }

    return 0;
}


uint32_t key_is_pressed(uint32_t key)
{
    key_state_fpt * p_key_state;

    p_key_state         = &g_key;

    if (p_key_state->value == key) {
        return p_key_state->flag & (1 << LV_EVENT_PRESSED);
        }

    return 0;
}





int32_t sd_storage_space_have_space(u64 need_free_space)
{
    u64             free_bytes;
    u64             total_bytes;
    int             ret;

    //__inf("fs_getdiskfreespace");
    ret                 = fs_getdiskfreespace("/D",&free_bytes,&total_bytes);

    if (ret != 0)
        __inf("fs_getdiskfreespace D tele_sms_data_init:ret:%d",ret);

    __inf("fs_getdiskfreespace D:total=0x%llx, free=0x%llx, need_free_space=0x%llx\n",total_bytes,free_bytes,
        need_free_space);

#if 0
    ret                 = fs_getdiskfreespace("/C",&free_bytes,&total_bytes);

    if (ret != 0)
        __inf("fs_getdiskfreespace C tele_sms_data_init:ret:%d",ret);

    __inf("fs_getdiskfreespace C:total=0x%llx, free=0x%llx\n",total_bytes,free_bytes);
#endif

    if (need_free_space >= free_bytes)
        return 0;

    else 
        return 1;

    return 0;
}

int32_t sd_storage_space_is_enough()
{
    return sd_storage_space_have_space(1024 * 10240);
}

void create_comp_msgbox_remind(uint8_t * content)
{
#ifdef CONFIG_APP_REMIND_FP
    prompt_box_common_info_t box_common_info = {
        0
        };
    box_common_info.timeout = 2000;
    box_common_info.content = content;
    prompt_box_common_create(lv_scr_act(),&box_common_info);
#endif
}

void create_comp_msgbox_remind_play_tts_menu(uint8_t * content)
{
#ifdef CONFIG_APP_REMIND_FP
    create_comp_msgbox_remind(content);
    app_fp_play_tts_menu(content);
#endif
}

