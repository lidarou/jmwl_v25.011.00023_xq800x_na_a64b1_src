/**
 * @file em_sketchpad.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lvgl.h"
#include "em_sketchpad.h"
#include "em.h"
#ifndef WIN32
#include <aic_osa_ex.h>
#endif

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lv_sketchpad_class

/* TODO: 后续用LCD 屏的宏替代*/
#define SCREEN_WIDTH      (240)
#define SCREEN_HEIGHT     (320)

#define ICON_X_OFFSET     (10)
#define ICON_Y_OFFSET     (10)
#define ICON_W            (30)
#define ICON_H            (30)
#define COLORWHEEL_WIDTH  (200)
#define COLORWHEEL_HEIGHT (200)
#define SLIDER_WIDTH      (200)
#define SLIDER_HEIGHT     (20)

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_sketchpad_constructor(const lv_obj_class_t *p_class, lv_obj_t *p_obj);
static void lv_sketchpad_destructor(const lv_obj_class_t *p_class, lv_obj_t *p_obj);
static void lv_sketchpad_toolbar_constructor(const lv_obj_class_t *p_class, lv_obj_t *p_obj);
static void lv_sketchpad_toolbar_destructor(const lv_obj_class_t *p_class, lv_obj_t *p_obj);
static void lv_sketchpad_event(const lv_obj_class_t *p_class, lv_event_t *p_event);
static void lv_sketchpad_toolbar_event(const lv_obj_class_t *p_class, lv_event_t *p_event);
static void sketchpad_toolbar_event_cb(lv_event_t *p_event);
static void toolbar_set_event_cb(lv_event_t *p_event);
static lv_obj_t *lv_sketchpad_create(lv_obj_t *p_parent);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_sketchpad_class = {
    .constructor_cb = lv_sketchpad_constructor,
    .destructor_cb = lv_sketchpad_destructor,
    .event_cb = lv_sketchpad_event,
    .instance_size = sizeof(lv_sketchpad_t),
    .base_class = &lv_canvas_class
};

const lv_obj_class_t lv_sketchpad_toolbar_class = {
    .constructor_cb = lv_sketchpad_toolbar_constructor,
    .destructor_cb = lv_sketchpad_toolbar_destructor,
    .event_cb = lv_sketchpad_toolbar_event,
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .base_class = &lv_obj_class
};

static lv_color_t *s_cbuf_ptr = NULL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/* TODO: move to instance file */
void em_sketchpad_enter_win(void)
{
    lv_obj_t *p_sketchpad = lv_sketchpad_create(lv_scr_act());

    if (NULL == s_cbuf_ptr) {
#ifdef WIN32
        s_cbuf_ptr = lv_mem_alloc(sizeof(lv_color_t) * LV_CANVAS_BUF_SIZE_TRUE_COLOR(SCREEN_WIDTH, SCREEN_HEIGHT));
        if (NULL == s_cbuf_ptr) {
            return;
        }
        memset(s_cbuf_ptr, 0, sizeof(lv_color_t) * LV_CANVAS_BUF_SIZE_TRUE_COLOR(SCREEN_WIDTH, SCREEN_HEIGHT));
#else
        s_cbuf_ptr = aic_zalloc(sizeof(lv_color_t) * LV_CANVAS_BUF_SIZE_TRUE_COLOR(SCREEN_WIDTH, SCREEN_HEIGHT));
        if (NULL == s_cbuf_ptr) {
            app_logi("[%s]: s_cbuf_ptr malloc failed!", __func__);
            return;
        }
#endif
    }

    lv_canvas_set_buffer(p_sketchpad, s_cbuf_ptr, SCREEN_WIDTH, SCREEN_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    lv_obj_center(p_sketchpad);
    lv_canvas_fill_bg(p_sketchpad, lv_palette_lighten(LV_PALETTE_GREY, 3), LV_OPA_COVER);
}

/*=====================
 * Other functions
 *====================*/

/**********************
 *   STATIC FUNCTIONS
 **********************/
static lv_obj_t *lv_sketchpad_create(lv_obj_t *p_parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t *p_obj = lv_obj_class_create_obj(MY_CLASS, p_parent);
    lv_obj_class_init_obj(p_obj);
    return p_obj;
}

static void lv_sketchpad_constructor(const lv_obj_class_t *p_class, lv_obj_t *p_obj)
{
    LV_UNUSED(p_class);
    LV_TRACE_OBJ_CREATE("begin");

    lv_sketchpad_t *p_sketchpad = (lv_sketchpad_t *)p_obj;

    p_sketchpad->dsc.header.always_zero = 0;
    p_sketchpad->dsc.header.cf          = LV_IMG_CF_TRUE_COLOR;
    p_sketchpad->dsc.header.h           = 0;
    p_sketchpad->dsc.header.w           = 0;
    p_sketchpad->dsc.data_size          = 0;
    p_sketchpad->dsc.data               = NULL;

    lv_draw_line_dsc_init(&p_sketchpad->line_rect_dsc);
    p_sketchpad->line_rect_dsc.width = 2;
    p_sketchpad->line_rect_dsc.round_start = true;
    p_sketchpad->line_rect_dsc.round_end = true;
    p_sketchpad->line_rect_dsc.color = lv_color_black();
    p_sketchpad->line_rect_dsc.opa = LV_OPA_COVER;

    lv_img_set_src(p_obj, &p_sketchpad->dsc);

    lv_obj_add_flag(p_obj, LV_OBJ_FLAG_CLICKABLE);

    /*toolbar*/
    lv_obj_t *p_toolbar = lv_obj_class_create_obj(&lv_sketchpad_toolbar_class, p_obj);
    lv_obj_class_init_obj(p_toolbar);

    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_sketchpad_destructor(const lv_obj_class_t *p_class, lv_obj_t *p_obj)
{
    LV_UNUSED(p_class);
    LV_TRACE_OBJ_CREATE("begin");

    lv_canvas_t *p_canvas = (lv_canvas_t *)p_obj;
    lv_img_cache_invalidate_src(&p_canvas->dsc);
}


static void lv_sketchpad_event(const lv_obj_class_t *p_class, lv_event_t *p_event)
{
    LV_UNUSED(p_class);

    lv_res_t res;

    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(MY_CLASS, p_event);
    if (res != LV_RES_OK) {
        return;
    }

    lv_event_code_t code = lv_event_get_code(p_event);
    lv_obj_t *p_obj = lv_event_get_target(p_event);
    lv_sketchpad_t *p_sketchpad = (lv_sketchpad_t *)p_obj;

    static lv_coord_t last_x, last_y = -32768;

    if (code == LV_EVENT_PRESSING)
    {
        lv_indev_t *p_indev = lv_indev_get_act();
        if (p_indev == NULL) {
            return;
        }

        lv_point_t point;
        lv_indev_get_point(p_indev, &point);

        lv_color_t c0;
        c0.full = 10;

        lv_point_t points[2];

        /*Release or first use*/
        if ((last_x == -32768) || (last_y == -32768)) {
            last_x = point.x;
            last_y = point.y;
        } else {
            points[0].x = last_x;
            points[0].y = last_y;
            points[1].x = point.x;
            points[1].y = point.y;
            last_x = point.x;
            last_y = point.y;

            lv_canvas_draw_line(p_obj, points, 2, &p_sketchpad->line_rect_dsc);
        }
    } else if(code == LV_EVENT_RELEASED) {
        /*Loosen the brush*/
        last_x = -32768;
        last_y = -32768;
    }
}

static void sketchpad_btn_event_cb(lv_event_t *p_event)
{
    lv_event_code_t code = lv_event_get_code(p_event);
    lv_obj_t *p_obj = lv_event_get_target(p_event);
    lv_obj_t *p_toolbar= lv_obj_get_parent(p_obj);
    lv_obj_t *p_sketchpad= lv_obj_get_parent(p_toolbar);
    uint32_t key_val = lv_event_get_key(p_event);

    if((LV_EVENT_KEY == code) && (LV_KEY_BACKSPACE == key_val)) {/*keypad:support enter quite UI*/
        if (NULL != p_sketchpad) {
            lv_obj_del(p_sketchpad);
        }

        /* TODO: move to instance file */
        /* free s_cbuf_ptr */
        if (NULL != s_cbuf_ptr) {
#ifdef WIN32
            lv_mem_free(s_cbuf_ptr);
#else
            aic_free(s_cbuf_ptr);
#endif
            s_cbuf_ptr = NULL;
        }
        /* TODO: work around. */
        em_main_menu_appear();
        em_version_btn_set_focus();
    }
}

/*toolbar*/
static void lv_sketchpad_toolbar_constructor(const lv_obj_class_t *p_class, lv_obj_t *p_obj)
{
    LV_UNUSED(p_class);
    LV_TRACE_OBJ_CREATE("begin");

    lv_obj_add_flag(p_obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(p_obj, LV_ALIGN_TOP_MID, ICON_X_OFFSET, ICON_Y_OFFSET);
    lv_obj_set_flex_grow(p_obj, 1);
    lv_obj_set_flex_flow(p_obj, LV_FLEX_FLOW_ROW);

    static lv_coord_t sketchpad_toolbar_cw = LV_SKETCHPAD_TOOLBAR_OPT_CW;
    lv_obj_t *p_color = lv_label_create(p_obj);
    lv_label_set_text(p_color, LV_SYMBOL_EDIT);
    lv_obj_align(p_color, LV_ALIGN_CENTER, ICON_X_OFFSET, ICON_Y_OFFSET);
    lv_obj_set_size(p_color, ICON_W, ICON_H);
    lv_obj_add_flag(p_color, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(p_color, sketchpad_toolbar_event_cb, LV_EVENT_ALL, &sketchpad_toolbar_cw);

    static lv_coord_t sketchpad_toolbar_width = LV_SKETCHPAD_TOOLBAR_OPT_WIDTH;
    lv_obj_t *p_width_size = lv_label_create(p_obj);
    lv_label_set_text(p_width_size, LV_SYMBOL_EJECT);
    lv_obj_set_size(p_width_size, ICON_W, ICON_H);
    lv_obj_add_flag(p_width_size, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(p_width_size, sketchpad_toolbar_event_cb, LV_EVENT_ALL, &sketchpad_toolbar_width);

    static lv_coord_t sketchpad_toolbar_erase = LV_SKETCHPAD_TOOLBAR_OPT_ERASE;
    lv_obj_t *p_size_erase = lv_label_create(p_obj);
    lv_label_set_text(p_size_erase, LV_SYMBOL_TRASH);
    lv_obj_set_size(p_size_erase, ICON_W, ICON_H);
    lv_obj_add_flag(p_size_erase, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(p_size_erase, sketchpad_toolbar_event_cb, LV_EVENT_ALL, &sketchpad_toolbar_erase);

    static lv_coord_t sketchpad_toolbar_back = LV_SKETCHPAD_TOOLBAR_OPT_BACK;
    lv_obj_t *p_size_back = lv_label_create(p_obj);
    lv_label_set_text(p_size_back, LV_SYMBOL_BACKSPACE);
    lv_obj_set_size(p_size_back, ICON_W, ICON_H);
    lv_obj_add_flag(p_size_back, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(p_size_back, sketchpad_toolbar_event_cb, LV_EVENT_ALL, &sketchpad_toolbar_back);

    static lv_style_t style_btn = {0};
    lv_obj_t *p_back_btn = NULL;
    lv_style_init(&style_btn);
    lv_style_set_opa(&style_btn, LV_OPA_0);
    p_back_btn = lv_btn_create(p_obj);
    lv_obj_add_style(p_back_btn, &style_btn, 0);
    lv_obj_add_event_cb(p_back_btn, sketchpad_btn_event_cb, LV_EVENT_ALL, NULL);

    lv_group_focus_obj(p_back_btn);

    LV_TRACE_OBJ_CREATE("finished");
}


static void lv_sketchpad_toolbar_destructor(const lv_obj_class_t *p_class, lv_obj_t *p_obj)
{

}


static void lv_sketchpad_toolbar_event(const lv_obj_class_t *p_class, lv_event_t *p_event)
{
    LV_UNUSED(p_class);

    lv_res_t res;

    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(MY_CLASS, p_event);
    if(res != LV_RES_OK) return;

    lv_event_code_t code = lv_event_get_code(p_event);
    lv_obj_t *p_obj = lv_event_get_target(p_event);

    if (code == LV_EVENT_PRESSING) {
        lv_indev_t *p_indev = lv_indev_get_act();
        if (p_indev == NULL) {
            return;
        }

        lv_point_t vect;
        lv_indev_get_vect(p_indev, &vect);

        lv_coord_t x = lv_obj_get_x(p_obj) + vect.x;
        lv_coord_t y = lv_obj_get_y(p_obj) + vect.y;
        lv_obj_set_pos(p_obj, x, y);
    }
}

static void sketchpad_toolbar_event_cb(lv_event_t *p_event)
{
    lv_coord_t *p_toolbar_opt = lv_event_get_user_data(p_event);
    lv_event_code_t code = lv_event_get_code(p_event);
    lv_obj_t *p_obj = lv_event_get_target(p_event);
    lv_obj_t *p_toolbar= lv_obj_get_parent(p_obj);
    lv_obj_t *p_sketchpad= lv_obj_get_parent(p_toolbar);
    lv_sketchpad_t *p_sketchpad_t = (lv_sketchpad_t *)p_sketchpad;

    if (code == LV_EVENT_CLICKED) {
        if ((*p_toolbar_opt) == LV_SKETCHPAD_TOOLBAR_OPT_CW) {
            static lv_coord_t sketchpad_toolbar_cw = LV_SKETCHPAD_TOOLBAR_OPT_CW;
            lv_obj_t *p_cw = lv_colorwheel_create(p_sketchpad, true);
            lv_obj_set_size(p_cw, COLORWHEEL_WIDTH, COLORWHEEL_HEIGHT);
            lv_obj_align_to(p_cw, p_obj, LV_ALIGN_OUT_BOTTOM_MID, (ICON_W + ICON_X_OFFSET), 0);
            lv_obj_add_event_cb(p_cw, toolbar_set_event_cb, LV_EVENT_RELEASED, &sketchpad_toolbar_cw);
        } else if((*p_toolbar_opt) == LV_SKETCHPAD_TOOLBAR_OPT_WIDTH) {
            static lv_coord_t sketchpad_toolbar_width = LV_SKETCHPAD_TOOLBAR_OPT_WIDTH;
            lv_obj_t *p_slider = lv_slider_create(p_sketchpad);
            lv_obj_set_size(p_slider, SLIDER_WIDTH, SLIDER_HEIGHT);
            lv_slider_set_value(p_slider, (int32_t)(p_sketchpad_t->line_rect_dsc.width), LV_ANIM_OFF);
            lv_obj_align_to(p_slider, p_obj, LV_ALIGN_OUT_BOTTOM_MID, (ICON_W - ICON_X_OFFSET), 0);
            lv_obj_add_event_cb(p_slider, toolbar_set_event_cb, LV_EVENT_ALL, &sketchpad_toolbar_width);
        } else if ((*p_toolbar_opt) == LV_SKETCHPAD_TOOLBAR_OPT_ERASE) {
            if (NULL != p_sketchpad) {
                /* clear canvas */
                lv_canvas_fill_bg(p_sketchpad, lv_palette_lighten(LV_PALETTE_GREY, 3), LV_OPA_COVER);
            }
        } else if ((*p_toolbar_opt) == LV_SKETCHPAD_TOOLBAR_OPT_BACK) {
            if (NULL != p_sketchpad) {
                lv_obj_del(p_sketchpad);
            }

            /* TODO: move to instance file */
            /* free s_cbuf_ptr */
            if (NULL != s_cbuf_ptr) {
#ifdef WIN32
                lv_mem_free(s_cbuf_ptr);
#else
                aic_free(s_cbuf_ptr);
#endif
                s_cbuf_ptr = NULL;
            }
            em_main_menu_appear();
        }
    }
}

static void toolbar_set_event_cb(lv_event_t *p_event)
{
    lv_coord_t *p_toobar_opt = lv_event_get_user_data(p_event);
    lv_event_code_t code = lv_event_get_code(p_event);
    lv_obj_t *p_obj = lv_event_get_target(p_event);
    lv_sketchpad_t *p_sketchpad = (lv_sketchpad_t *)lv_obj_get_parent(p_obj);

    if (code == LV_EVENT_RELEASED) {
        if ((*p_toobar_opt) == LV_SKETCHPAD_TOOLBAR_OPT_CW) {
            p_sketchpad->line_rect_dsc.color = lv_colorwheel_get_rgb(p_obj);
            lv_obj_del(p_obj);
        } else if (*(p_toobar_opt) == LV_SKETCHPAD_TOOLBAR_OPT_WIDTH) {
            lv_obj_del(p_obj);
        }
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        if((*p_toobar_opt) == LV_SKETCHPAD_TOOLBAR_OPT_WIDTH) {
            p_sketchpad->line_rect_dsc.width = (lv_coord_t)lv_slider_get_value(p_obj);
        }
    }
}
