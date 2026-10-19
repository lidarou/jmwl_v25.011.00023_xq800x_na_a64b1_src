/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *   This file implements the API of a customized button matrix widget
 * (alv_btnmatrix).
 */

/*******************************************
 *              INCLUDES
 ******************************************/
#include "alv_btnmatrix.h"

/*******************************************
 *              DEFINES
 ******************************************/
#define MY_CLASS &alv_btnmatrix_class
#define LVCW_NO_SCROLL ((lv_coord_t)-1)

/*******************************************
 *              STATIC PROTOTYPES
 ******************************************/
/**
 * alv_btnmatrix_constructor - constructor
 * @class_p: [IN] pointer to class
 * @obj: [OUT] pointer to object
 * Return: NA
 */
static void alv_btnmatrix_constructor(const lv_obj_class_t *class_p,
                                      lv_obj_t *obj);
/**
 * alv_menu_destructor - destructor
 * @class_p: [IN] pointer to class
 * @obj: [OUT] pointer to object
 * Return: NA
 */
static void alv_btnmatrix_destructor(const lv_obj_class_t *class_p,
                                     lv_obj_t *obj);
/**
 * alv_btnmatrix_event - event handler
 * @class_p: [IN] pointer to class
 * @e: [IN] events
 * Return: NA
 */
static void alv_btnmatrix_event(const lv_obj_class_t *class_p, lv_event_t *e);

/**
 * draw_main - draw handler
 * @e: [IN] events
 * Return: NA
 */
static void draw_main(lv_event_t *e);

/*******************************************
 *              STATIC VARIABLES
 ******************************************/
const lv_obj_class_t alv_btnmatrix_class = {
    .constructor_cb = alv_btnmatrix_constructor,
    .destructor_cb = alv_btnmatrix_destructor,
    .event_cb = alv_btnmatrix_event,
    .width_def = LV_DPI_DEF * 2,
    .height_def = LV_DPI_DEF,
    .instance_size = sizeof(struct alv_btnmatrix),
    .editable = LV_OBJ_CLASS_EDITABLE_TRUE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .base_class = &lv_btnmatrix_class};

/*******************************************
 *              STATIC FUNCTIONS
 ******************************************/
/* static functions copied from lv_btnmatrix - Begin */
static bool button_is_inactive(lv_btnmatrix_ctrl_t ctrl_bits)
{
    return (ctrl_bits & LV_BTNMATRIX_CTRL_DISABLED) ? true : false;
}

static bool button_get_checked(lv_btnmatrix_ctrl_t ctrl_bits)
{
    return (ctrl_bits & LV_BTNMATRIX_CTRL_CHECKED) ? true : false;
}

static bool button_is_recolor(lv_btnmatrix_ctrl_t ctrl_bits)
{
    return (ctrl_bits & LV_BTNMATRIX_CTRL_RECOLOR) ? true : false;
}

static bool button_is_hidden(lv_btnmatrix_ctrl_t ctrl_bits)
{
    return (ctrl_bits & LV_BTNMATRIX_CTRL_HIDDEN) ? true : false;
}

/* static functions copied from lv_btnmatrix - End */

/**
 * alv_btnmatrix_constructor - constructor
 * @class_p: [IN] pointer to class
 * @obj: [OUT] pointer to object
 * Return: NA
 */
static void alv_btnmatrix_constructor(const lv_obj_class_t *class_p,
                                      lv_obj_t *obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");
    LV_UNUSED(obj);
    /* do nothing here */

    LV_TRACE_OBJ_CREATE("finished");
}

/**
 * alv_btnmatrix_constructor - destructor
 * @class_p: [IN] pointer to class
 * @obj: [OUT] pointer to object
 * Return: NA
 */
static void alv_btnmatrix_destructor(const lv_obj_class_t *class_p,
                                     lv_obj_t *obj)
{
    LV_TRACE_OBJ_CREATE("begin");
    LV_UNUSED(class_p);
    LV_UNUSED(obj);
    /* do nothing here */

    LV_TRACE_OBJ_CREATE("finished");
}

/**
 * lvcw_get_y_to_scroll - calculate the next y to scroll to
 * @obj: [IN] pointer to alv_btnmatrix obj
 * @is_dow: [IN] Current scroll direction, true for down, false for up
 * Return: next y.
 *   if returns LVCW_NO_SCROLL, means no need to scroll
 */
static lv_coord_t lvcw_get_y_to_scroll(lv_obj_t *obj, bool is_down)
{
    lv_coord_t result = LVCW_NO_SCROLL;
    lv_area_t cont_area; /*container area*/
    lv_coord_t cur_scroll_y;
    lv_obj_t *parent = lv_obj_get_parent(obj);
    lv_btnmatrix_t *base = (lv_btnmatrix_t *)obj;

    if (!parent || LV_BTNMATRIX_BTN_NONE == base->btn_id_sel)
        return result;
    if (!lv_obj_has_flag(parent, LV_OBJ_FLAG_SCROLLABLE))
        return result;
    cur_scroll_y = lv_obj_get_scroll_y(parent);
    cont_area.x1 = 0;
    cont_area.y1 = cur_scroll_y;
    cont_area.x2 = lv_obj_get_width(parent) - 1;
    cont_area.y2 = cur_scroll_y + lv_obj_get_height(parent) - 1;

    /* check if the current button is inside viewable area */
    if (_lv_area_is_in(&base->button_areas[base->btn_id_sel], &cont_area, 0))
        return LVCW_NO_SCROLL;

    if (is_down) {
        uint16_t i;
        uint16_t sel = base->btn_id_sel;
        for (i = 0; i < sel; i++) {
            if (base->button_areas[sel].y2 - base->button_areas[i].y1 + 1 <=
                lv_obj_get_height(parent)) {
                break;
            }
        }
        result = base->button_areas[i].y1;
    } else {
        result = base->button_areas[base->btn_id_sel].y1;
    }

    return result;
}
/**
 * alv_btnmatrix_event - event handler
 * @class_p: [IN] pointer to class
 * @e: [IN] events
 * Return: NA
 */
static void alv_btnmatrix_event(const lv_obj_class_t *class_p, lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_res_t res;

    if (code == LV_EVENT_DRAW_MAIN) {
        draw_main(e);
    } else if (code == LV_EVENT_DEFOCUSED || code == LV_EVENT_LEAVE) {
        /* Do not bubble this event to base class, because we
         * need to keep the current selection id */
        lv_obj_clear_state(obj, LV_STATE_FOCUSED | LV_STATE_EDITED |
                                    LV_STATE_FOCUS_KEY);
    } else {
        /*Call the ancestor's event handler*/
        res = lv_obj_event_base(class_p, e);
        if (res != LV_RES_OK)
            return;
    }
    /* the following codes should be handled after ancestor handled*/
    if (code == LV_EVENT_KEY) {
        char c = *((char *)lv_event_get_param(e));
        if (LV_KEY_UP == c || LV_KEY_DOWN == c || LV_KEY_LEFT == c ||
            LV_KEY_RIGHT == c) {
            bool is_down = (LV_KEY_DOWN == c) || (LV_KEY_UP == c);
            lv_coord_t y = lvcw_get_y_to_scroll(obj, is_down);
            if (y != LVCW_NO_SCROLL) {
                lv_obj_t *parent = lv_obj_get_parent(obj);
                lv_obj_scroll_to_y(parent, y, LV_ANIM_ON);
            }
        }
    }
}

/**
 * draw_main - draw handler
 * @e: [IN] events
 * Return: NA
 */
static void draw_main(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    struct alv_btnmatrix *btnm = (struct alv_btnmatrix *)obj;
    if (btnm->base.btn_cnt == 0)
        return;

    lv_draw_ctx_t *draw_ctx = lv_event_get_draw_ctx(e);
    obj->skip_trans = 1;

    lv_area_t area_obj;
    lv_obj_get_coords(obj, &area_obj);

    lv_area_t btn_area;
    uint16_t btn_i = 0;
    uint16_t txt_i = 0;
    lv_draw_rect_dsc_t draw_rect_dsc_act;
    lv_draw_label_dsc_t draw_label_dsc_act;
    lv_draw_rect_dsc_t draw_rect_dsc_def;
    lv_draw_label_dsc_t draw_label_dsc_def;

    lv_state_t state_ori = obj->state;
    obj->state = LV_STATE_DEFAULT;
    obj->skip_trans = 1;
    lv_draw_rect_dsc_init(&draw_rect_dsc_def);
    lv_draw_label_dsc_init(&draw_label_dsc_def);
    lv_obj_init_draw_rect_dsc(obj, LV_PART_ITEMS, &draw_rect_dsc_def);
    lv_obj_init_draw_label_dsc(obj, LV_PART_ITEMS, &draw_label_dsc_def);
    obj->skip_trans = 0;
    obj->state = state_ori;

    lv_obj_draw_part_dsc_t part_draw_dsc;
    lv_obj_draw_dsc_init(&part_draw_dsc, draw_ctx);
    part_draw_dsc.part = LV_PART_ITEMS;
    part_draw_dsc.class_p = MY_CLASS;
    part_draw_dsc.type = LV_BTNMATRIX_DRAW_PART_BTN;
    part_draw_dsc.rect_dsc = &draw_rect_dsc_act;
    part_draw_dsc.label_dsc = &draw_label_dsc_act;
    for (btn_i = 0; btn_i < btnm->base.btn_cnt; btn_i++, txt_i++) {
        /*Search the next valid text in the map*/
        while (strcmp(btnm->base.map_p[txt_i], "\n") == 0) {
            txt_i++;
        }
        /*Skip hidden buttons*/
        if (button_is_hidden(btnm->base.ctrl_bits[btn_i]))
            continue;
        /*Get the state of the button*/
        lv_state_t btn_state = LV_STATE_DEFAULT;
        if (button_get_checked(btnm->base.ctrl_bits[btn_i]))
            btn_state |= LV_STATE_CHECKED;
        if (button_is_inactive(btnm->base.ctrl_bits[btn_i]))
            btn_state |= LV_STATE_DISABLED;
        else if (btn_i == btnm->base.btn_id_sel) {
            if (state_ori & LV_STATE_PRESSED)
                btn_state |= LV_STATE_PRESSED;
            if (state_ori & LV_STATE_FOCUSED)
                btn_state |= LV_STATE_FOCUSED;
            if (state_ori & LV_STATE_FOCUS_KEY)
                btn_state |= LV_STATE_FOCUS_KEY;
            if (state_ori & LV_STATE_EDITED)
                btn_state |= LV_STATE_EDITED;
        }
        /*Get the button's area*/
        lv_area_copy(&btn_area, &btnm->base.button_areas[btn_i]);
        btn_area.x1 += area_obj.x1;
        btn_area.y1 += area_obj.y1;
        btn_area.x2 += area_obj.x1;
        btn_area.y2 += area_obj.y1;
        /*Set up the draw descriptors*/
        if (btn_state == LV_STATE_DEFAULT) {
            lv_memcpy(&draw_rect_dsc_act, &draw_rect_dsc_def,
                      sizeof(lv_draw_rect_dsc_t));
            lv_memcpy(&draw_label_dsc_act, &draw_label_dsc_def,
                      sizeof(lv_draw_label_dsc_t));
        }
        /*In other cases get the styles directly without caching them*/
        else {
            obj->state = btn_state;
            obj->skip_trans = 1;
            lv_draw_rect_dsc_init(&draw_rect_dsc_act);
            lv_draw_label_dsc_init(&draw_label_dsc_act);
            lv_obj_init_draw_rect_dsc(obj, LV_PART_ITEMS, &draw_rect_dsc_act);
            lv_obj_init_draw_label_dsc(obj, LV_PART_ITEMS, &draw_label_dsc_act);
            obj->state = state_ori;
            obj->skip_trans = 0;
        }
        bool recolor = button_is_recolor(btnm->base.ctrl_bits[btn_i]);
        if (recolor)
            draw_label_dsc_act.flag |= LV_TEXT_FLAG_RECOLOR;
        else
            draw_label_dsc_act.flag &= ~LV_TEXT_FLAG_RECOLOR;
        part_draw_dsc.draw_area = &btn_area;
        part_draw_dsc.id = btn_i;
        lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dsc);
        /*Draw the background*/
        lv_draw_rect(draw_ctx, &draw_rect_dsc_act, &btn_area);
        lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dsc);
    }

    obj->skip_trans = 0;
}

/**
 * alv_btnmatrix_create - Creater of alv_btnmatrix
 * @parent: [IN] parent of the widget
 *
 * Return: the created button, NULL for failure
 */
lv_obj_t *alv_btnmatrix_create(lv_obj_t *parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t *obj = lv_obj_class_create_obj(MY_CLASS, parent);
    if (obj) {
        lv_obj_class_init_obj(obj);
    }
    return obj;
}

/**
 * alv_btnmatrix_get_btn_cnt - get btn count
 * @obj: [IN] pointer to alv_btnmatrix widget
 *
 * Return: the created button, NULL for failure
 */
uint16_t alv_btnmatrix_get_btn_cnt(lv_obj_t *obj)
{
    uint16_t result = 0;
    struct alv_btnmatrix *btnm = (struct alv_btnmatrix *)obj;
    if (btnm) {
        result = btnm->base.btn_cnt;
    }
    return result;
}