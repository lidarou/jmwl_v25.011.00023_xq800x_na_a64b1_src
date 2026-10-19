/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *   This file implements the API of a customized menu widget
 * (alv_menu).
 */

/*******************************************
 *              INCLUDES
 ******************************************/
#include "alv_menu.h"

/*******************************************
 *              DEFINES
 ******************************************/
#define MY_CLASS &alv_menu_class
/*Mark parent as automatically created*/
#define ALV_MENU_FLAG_AUTO_PARENT LV_OBJ_FLAG_WIDGET_1
#define ALV_MENU_SELECTED_COLOR 0xE5F3FF
#define ALV_MENU_BACKWARDS_KEY LV_KEY_BACKSPACE

/*******************************************
 *              STATIC PROTOTYPES
 ******************************************/
/**
 * alv_menu_constructor - constructor
 * @class_p: [IN] pointer to class
 * @obj: [OUT] pointer to object
 * Return: NA
 */
static void alv_menu_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj);
/**
 * alv_menu_destructor - destructor
 * @class_p: [IN] pointer to class
 * @obj: [OUT] pointer to object
 * Return: NA
 */
static void alv_menu_destructor(const lv_obj_class_t *class_p, lv_obj_t *obj);
/**
 * alv_menu_event - event handler
 * @class_p: [IN] pointer to class
 * @e: [IN] events
 * Return: NA
 */
static void alv_menu_event(const lv_obj_class_t *class_p, lv_event_t *e);

/*******************************************
 *              STATIC VARIABLES
 ******************************************/
const lv_obj_class_t alv_menu_class = {.constructor_cb = alv_menu_constructor,
                                       .destructor_cb = alv_menu_destructor,
                                       .event_cb = alv_menu_event,
                                       .width_def = LV_PCT(100),
                                       .height_def = LV_PCT(100),
                                       .instance_size = sizeof(struct alv_menu),
                                       .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
                                       .base_class = &lv_obj_class};

const lv_obj_class_t alv_menu_backdrop_class = {.base_class = &lv_obj_class,
                                                .width_def = LV_PCT(100),
                                                .height_def = LV_PCT(100),
                                                .instance_size =
                                                    sizeof(lv_obj_t)};
uint32_t ALV_MENU_ITEM_SELECTED = 0;

/*******************************************
 *              STATIC FUNCTIONS
 ******************************************/
/**
 * alv_menu_constructor - constructor
 * @class_p: [IN] pointer to class
 * @obj: [OUT] pointer to object
 * Return: NA
 */
static void alv_menu_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    struct alv_menu *menu = (struct alv_menu *)obj;
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    menu->back_btn = NULL;
    menu->menu_list = NULL;
    menu->selected_item = ALV_MENU_SELECT_NOTHING;

    LV_TRACE_OBJ_CREATE("finished");
}

/**
 * alv_menu_destructor - destructor
 * @class_p: [IN] pointer to class
 * @obj: [OUT] pointer to object
 * Return: NA
 */
static void alv_menu_destructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");
    LV_UNUSED(obj);
    /* do nothing here */

    LV_TRACE_OBJ_CREATE("finished");
}

/**
 * move_to_next_btn - handle key down and up
 * @obj: [IN] pointer to menu widget
 * @is_up: [IN] true for KEY UP, false for KEY DOWN
 *
 * Return: NA
 * Design note: must skip text item during the move
 */
static void move_to_next_btn(lv_obj_t *obj, bool is_up)
{
    struct alv_menu *this = (struct alv_menu *)obj;
    int old_sel = this->selected_item;
    int next = ALV_MENU_SELECT_NOTHING;
    int i = 0;

    if (ALV_MENU_SELECT_NOTHING == old_sel) {
        /*select the first menu item*/
        for (i = 0; i < lv_obj_get_child_cnt(this->menu_list); i++) {
            lv_obj_t *child = lv_obj_get_child(this->menu_list, i);
            /* skip text item */
            if (lv_obj_check_type(child, &lv_list_text_class)) {
                continue;
            } else {
                next = i;
                break;
            }
        }
    } else {
        /* find the next item */
        if (is_up) {
            for (i = old_sel - 1; i >= 0; i--) {
                lv_obj_t *child = lv_obj_get_child(this->menu_list, i);
                /* skip text item */
                if (lv_obj_check_type(child, &lv_list_text_class)) {
                    continue;
                } else {
                    next = i;
                    break;
                }
            }
        } else {
            for (i = old_sel + 1; i < lv_obj_get_child_cnt(this->menu_list);
                 i++) {
                lv_obj_t *child = lv_obj_get_child(this->menu_list, i);
                /* skip text item */
                if (lv_obj_check_type(child, &lv_list_text_class)) {
                    continue;
                } else {
                    next = i;
                    break;
                }
            }
        }
    }

    if (next != ALV_MENU_SELECT_NOTHING) {
        /* clear pressed state upon the old item */
        if (old_sel != ALV_MENU_SELECT_NOTHING) {
            lv_obj_clear_state(lv_obj_get_child(this->menu_list, old_sel),
                               LV_STATE_CHECKED);
        }
        /* set pressed state upon the new item */
        lv_obj_add_state(lv_obj_get_child(this->menu_list, next),
                         LV_STATE_CHECKED);
        this->selected_item = next;
        lv_obj_scroll_to_view(lv_obj_get_child(this->menu_list, next),
                              LV_ANIM_ON);
    }
}

/**
 * alv_menu_handle_key_enter - handle key enter event
 * @menu: [IN] pointer to menu object
 *
 * Return: NA
 */
static void alv_menu_handle_key_enter(lv_obj_t *obj)
{
    struct alv_menu *this = (struct alv_menu *)obj;
    if (!this || !this->menu_list) {
        return;
    }
    /* check if any menu item is selected or not */
    if (this->selected_item != ALV_MENU_SELECT_NOTHING) {
        /* check if the item is disabled or not */
        lv_obj_t *menu_item =
            lv_obj_get_child(this->menu_list, this->selected_item);
        if (menu_item != NULL) {
            if (!lv_obj_has_state(menu_item, LV_STATE_DISABLED)) {
                uint32_t id = (uint32_t)lv_obj_get_user_data(menu_item);
                lv_event_send((lv_obj_t *)this, ALV_MENU_ITEM_SELECTED,
                              (void *)id);
            }
        }
    }
}

/**
 * alv_menu_event - event handler
 * @class_p: [IN] pointer to class
 * @e: [IN] events
 * Return: NA
 */
static void alv_menu_event(const lv_obj_class_t *class_p, lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_res_t res;

    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(class_p, e);
    if (res != LV_RES_OK)
        return;
    if (LV_EVENT_KEY == code) {
        char c = *((char *)lv_event_get_param(e));
        switch (c) {
        case LV_KEY_UP:
        case LV_KEY_DOWN:
            move_to_next_btn(obj, LV_KEY_UP == c);
            break;
        case LV_KEY_ENTER:
            /* don't handle this key event directly, because it triggers
             * LV_EVENT_PRESSED/CLICKED/LONG_PRESSED etc. events*/
            break;
        case ALV_MENU_BACKWARDS_KEY:
            lv_event_send(obj, ALV_MENU_ITEM_SELECTED,
                          (void *)ALV_MENU_SELECT_NOTHING);
            /* ignore long pressed key */
            lv_indev_wait_release(lv_indev_get_act());
            break;
        default:
            break;
        }
    } else if (LV_EVENT_CLICKED == code) {
        alv_menu_handle_key_enter(obj);
    }
}

/**
 * menu_back_btn_event - event handler for back button
 * @e: [IN] events
 * Return: NA
 */
static void menu_back_btn_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (LV_EVENT_CLICKED == code) {
        struct alv_menu *this = (struct alv_menu *)lv_obj_get_parent(obj);
        lv_event_send((lv_obj_t *)this, ALV_MENU_ITEM_SELECTED,
                      (void *)ALV_MENU_SELECT_NOTHING);
    }
}

/**
 * menu_item_event_cb - event handler
 * @e: [IN] events
 * Return: NA
 */
static void menu_item_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (LV_EVENT_CLICKED == code) {
        lv_obj_t *parent = lv_obj_get_parent(obj);
        if (!parent) {
            return;
        }
        /* clear checked state of other items */
        for (int i = 0; i < lv_obj_get_child_cnt(parent); i++) {
            lv_obj_t *child = lv_obj_get_child(parent, i);
            if (child != obj) {
                lv_obj_clear_state(child, LV_STATE_CHECKED);
            }
        }
        if (!lv_obj_has_state(obj, LV_STATE_DISABLED)) {
            uint32_t id = (uint32_t)lv_obj_get_user_data(obj);
            lv_obj_t *this = lv_obj_get_parent(parent);
            if (!this) {
                return;
            }
            ((struct alv_menu *)this)->selected_item = lv_obj_get_index(obj);
            lv_event_send((lv_obj_t *)this, ALV_MENU_ITEM_SELECTED, (void *)id);
        }
    }
}

/**
 * menu_item_gesture_event - event handler
 * @e: [IN] events
 * Return: NA
 *
 */
static void menu_item_gesture_event(lv_event_t *e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (LV_DIR_RIGHT == dir) {
        lv_obj_t *obj = lv_event_get_target(e);
        lv_event_send(obj, ALV_MENU_ITEM_SELECTED,
                      (void *)ALV_MENU_SELECT_NOTHING);
        /* ignore long pressed and release key */
        lv_indev_wait_release(lv_indev_get_act());
    }
}

/*******************************************
 *              GLOBAL FUNCTIONS
 ******************************************/
/**
 * alv_menu_create - Creater of alv_menu
 * @parent: [IN] parent widget
 * @back_enabled: [IN] if true, create a back btn at the top
 *
 * Return: the created menu, NULL for failure
 *
 */
lv_obj_t *alv_menu_create(lv_obj_t *parent, bool back_enabled)
{
    LV_LOG_INFO("begin");
    bool auto_parent = false;
    if (parent == NULL) {
        auto_parent = true;
        parent =
            lv_obj_class_create_obj(&alv_menu_backdrop_class, lv_layer_top());
        LV_ASSERT_MALLOC(parent);
        lv_obj_class_init_obj(parent);
        lv_obj_clear_flag(parent, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_set_size(parent, LV_PCT(100), LV_PCT(100));
    }
    lv_obj_t *obj = lv_obj_class_create_obj(MY_CLASS, parent);
    if (obj) {
        struct alv_menu *this = (struct alv_menu *)obj;
        lv_obj_class_init_obj(obj);
        if (auto_parent) {
            lv_obj_add_flag(obj, ALV_MENU_FLAG_AUTO_PARENT);
        }
        lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
        /* create back btn */
        if (back_enabled) {
            this->back_btn = lv_obj_class_create_obj(&lv_list_btn_class, obj);
            if (this->back_btn) {
                lv_obj_class_init_obj(this->back_btn);
                lv_obj_set_size(this->back_btn, LV_PCT(100), LV_SIZE_CONTENT);
                lv_obj_set_flex_flow(this->back_btn, LV_FLEX_FLOW_ROW);
                lv_obj_t *img = lv_img_create(this->back_btn);
                if (img) {
                    lv_img_set_src(img, LV_SYMBOL_LEFT);
                }
                /* add callback to the inserted menu item */
                lv_obj_add_event_cb(this->back_btn, menu_back_btn_event,
                                    LV_EVENT_CLICKED, NULL);
                lv_group_remove_obj(this->back_btn);
            }
        }
        /* create menu list panel */
        this->menu_list = lv_list_create(obj);
        if (this->menu_list) {
            lv_obj_set_width(this->menu_list, LV_PCT(100));
            lv_obj_set_flex_grow(this->menu_list, 1);
            lv_obj_set_style_radius(this->menu_list, 0, 0);
            lv_obj_set_style_border_width(this->menu_list, 0, 0);
        }
        /* set focus on this menu list */
        lv_group_focus_obj(obj);
        /* clear LV_OBJ_FLAG_GESTURE_BUBBLE to make it possible to receive
         * gesture in menu */
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_GESTURE_BUBBLE);
        /* add event cb to gesture */
        lv_obj_add_event_cb(obj, menu_item_gesture_event, LV_EVENT_GESTURE,
                            NULL);
    }

    /* initialize the ALV MENU ITEM */
    if (!ALV_MENU_ITEM_SELECTED) {
        ALV_MENU_ITEM_SELECTED = lv_event_register_id();
    }
    return obj;
}

/**
 * alv_menu_close - close menu
 * @obj: [IN] pointer to menu obj
 *
 * Return: NA
 *
 */
void alv_menu_close(lv_obj_t *obj)
{
    if (lv_obj_has_flag(obj, ALV_MENU_FLAG_AUTO_PARENT))
        lv_obj_del(lv_obj_get_parent(obj));
    else
        lv_obj_del(obj);
}

/**
 * alv_menu_close_async - close menu async
 * @obj: [IN] pointer to menu obj
 *
 * Return: NA
 *
 */
void alv_menu_close_async(lv_obj_t *obj)
{
    if (lv_obj_has_flag(obj, ALV_MENU_FLAG_AUTO_PARENT))
        lv_obj_del_async(lv_obj_get_parent(obj));
    else
        lv_obj_del_async(obj);
}

/**
 * alv_menu_add_item - add a menu item to the tail
 * @obj: [IN] pointer to menu obj
 * @type: [IN] type of the menu item, if the type is
 * ALV_MENU_ITEM_TYPE_TEXT, argument icon will be ignored
 * @id: [IN] unique id to identify the menu item. Note: ID 0xFFFFFFF0 ~
 * 0xFFFFFFFF are reserved for alv_menu
 * @text: [IN] text of the menu item
 * @icon: [IN] image of the menu btn item, it could be any of the following
 * types.
 *     1) pointer to an ::lv_img_dsc_t descriptor (converted by LVGL's image
 *        converter) (e.g. &my_img) or
 *     2) path to an image file (e.g.
 *        "S:/dir/img.bin")or
 *     3) a SYMBOL (e.g. LV_SYMBOL_OK)
 *     4) NULL, no icon to display
 * Return: the created menu, NULL for failure
 */
lv_obj_t *alv_menu_add_item(lv_obj_t *obj, enum alv_menu_item_type type,
                            uint32_t id, const char *text, void *icon)
{
    lv_obj_t *item = NULL;
    struct alv_menu *this = (struct alv_menu *)obj;
    LV_ASSERT(id < 0xFFFFFFF0);
    if (!this || !this->menu_list)
        return NULL;

    if (ALV_MENU_ITEM_TYPE_TEXT == type) {
        item = lv_list_add_text(this->menu_list, text);
        if (item) {
            /* add left pad 2 pixels*/
            lv_obj_set_style_pad_left(item, 2, 0);
            lv_obj_set_user_data(item, (void *)id);
        }
    } else if (ALV_MENU_ITEM_TYPE_BTN == type) {
        item = lv_list_add_btn(this->menu_list, icon, text);
        if (item) {
            lv_obj_add_flag(item, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_set_style_bg_color(item,
                                      lv_color_hex(ALV_MENU_SELECTED_COLOR),
                                      LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_bg_opa(item, 255, LV_PART_MAIN | LV_STATE_CHECKED);
            /* add callback to the inserted menu item */
            lv_obj_add_event_cb(item, menu_item_event_cb, LV_EVENT_CLICKED,
                                NULL);
            /* if this item is the first one, set it as checked*/
            if (this->selected_item == ALV_MENU_SELECT_NOTHING) {
                this->selected_item = lv_obj_get_index(item);
                lv_obj_add_state(item, LV_STATE_CHECKED);
            }
            lv_obj_set_user_data(item, (void *)id);
            /*do not ack the key event*/
            lv_group_remove_obj(item);
        }
    } else {
        LV_LOG_ERROR("alv_menu_add_item: invalid log type to add");
    }

    return item;
}

/**
 * alv_menu_remove_item - remove an item from menu by menu id
 * @obj: [IN] pointer to menu obj
 * @id: unique id to identify the menu item. Note: ID 0xFFFFFFF0 ~
 * 0xFFFFFFFF are reserved for alv_menu
 *
 * Return: the created menu, NULL for failure
 */
bool alv_menu_remove_item(lv_obj_t *obj, uint32_t id)
{
    bool result = false;
    struct alv_menu *this = (struct alv_menu *)obj;
    int i;

    if (!this || !this->menu_list) {
        return false;
    }
    LV_ASSERT(id < 0xFFFFFFF0);
    /*select the first menu item*/
    for (i = 0; i < lv_obj_get_child_cnt(this->menu_list); i++) {
        lv_obj_t *child = lv_obj_get_child(this->menu_list, i);
        uint32_t item_id = (uint32_t)lv_obj_get_user_data(child);
        if (item_id == id) {
            lv_obj_del(child);
            result = true;
            break;
        }
    }
    /* reset the current selection to the first one */
    this->selected_item = ALV_MENU_SELECT_NOTHING;
    for (i = 0; i < lv_obj_get_child_cnt(this->menu_list); i++) {
        lv_obj_t *child = lv_obj_get_child(this->menu_list, i);
        /* skip text item */
        if (lv_obj_check_type(child, &lv_list_text_class)) {
            continue;
        } else {
            this->selected_item = i;
            break;
        }
    }

    return result;
}
