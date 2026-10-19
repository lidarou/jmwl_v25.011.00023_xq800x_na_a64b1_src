/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *   This file defines the basic Application Interface (API) of a customized
 * menu widget(alv_menu).
 */

#ifndef __ALV_MENU_H__
#define __ALV_MENU_H__

#ifdef __cplusplus
extern "C" {
#endif
/*******************************************
 *              INCLUDES
 ******************************************/
#include <lvgl.h>

/*******************************************
 *              DEFINES
 ******************************************/
#define ALV_MENU_SELECT_NOTHING ((uint32_t)(-1))

/*******************************************
 *              TYPEDEFS
 ******************************************/
/**
 * struct alv_menu_item_type - alv menu item type
 * @ALV_MENU_ITEM_TYPE_TEXT: text type, normally used for group naming
 * @ALV_MENU_ITEM_TYPE_BTN： btn type, normally used for menu selection
 */
enum alv_menu_item_type {
    ALV_MENU_ITEM_TYPE_TEXT,
    ALV_MENU_ITEM_TYPE_BTN,

    ALV_MENU_ITEM_TYPE_UNKNOWN,
};
/**
 * struct alv_menu - structure of alv menu widget
 * @base: base of the widget, i.e. lv_obj_t
 * @selected_item: the id of selected item, if ALV_MENU_SELECT_NOTHING means
 * nothing selected.
 */
struct alv_menu {
    lv_obj_t base;
    lv_obj_t *back_btn;
    lv_obj_t *menu_list;
    int selected_item;
};

/**
 * struct alv_menu_item - structure of menu item to init
 * @id: unique id to identify the menu item
 * @icon: image of the menu item, it could be any of the belowing types.
 *     1) pointer to an ::lv_img_dsc_t descriptor (converted by LVGL's image
 *        converter) (e.g. &my_img) or
 *     2) path to an image file (e.g.
 *        "S:/dir/img.bin")or
 *     3) a SYMBOL (e.g. LV_SYMBOL_OK)
 *     4) NULL, no icon to display
 * @text: text of the menu item
 */
struct alv_menu_item {
    int id;
    void *icon;
    char *text;
};

/*******************************************
 *       GLOBAL VARIABLE DECLARATION
 ******************************************/
extern const lv_obj_class_t alv_menu_class;
extern const lv_obj_class_t alv_menu_backdrop_class;
/**
 * ALV_MENU_ITEM_SELECTED -  a customized event,
 * which will be triggered when menu item selected or cancelled.
 * Please use (uint32_t)lv_event_get_param(e) to get the id of selection, if id
 * is equal to ALV_MENU_SELECT_NOTHING, it means user has cancelled the menu
 * selection
 * */
extern uint32_t ALV_MENU_ITEM_SELECTED;

/*******************************************
 *              GLOBAL PROTOTYPES
 ******************************************/
/**
 * alv_menu_create - Creater of alv_menu
 * @parent: [IN] parent widget
 * @back_enabled: [IN] if true, create a back btn at the top
 *
 * Return: the created menu, NULL for failure
 */
lv_obj_t *alv_menu_create(lv_obj_t *parent, bool back_enabled);

/**
 * alv_menu_close - close menu
 * @obj: [IN] pointer to menu obj
 *
 * Return: NA
 */
void alv_menu_close(lv_obj_t *obj);

/**
 * alv_menu_close_async - close menu async
 * @obj: [IN] pointer to menu obj
 *
 * Return: NA
 *
 */
void alv_menu_close_async(lv_obj_t *obj);

/**
 * alv_menu_add_item - add a menu item to the tail
 * @obj: [IN] pointer to menu obj
 * @type: [IN] type of the menu item, if the type is ALV_MENU_ITEM_TYPE_TEXT,
 *     argument icon will be ignored
 * @id: [IN] unique id to identify the menu item. Note: ID 0xFFFFFFF0 ~
 * 0xFFFFFFFF are reserved for alv_menu
 * @text: [IN] text of the menu item
 * @icon: [IN] image of the menu btn item, it could be any of the following
 *     types.
 *     1) pointer to an ::lv_img_dsc_t descriptor (converted by LVGL's image
 *        converter) (e.g. &my_img) or
 *     2) path to an image file (e.g.
 *        "S:/dir/img.bin")or
 *     3) a SYMBOL (e.g. LV_SYMBOL_OK)
 *     4) NULL, no icon to display
 * Return: the created menu, NULL for failure
 *
 */
lv_obj_t *alv_menu_add_item(lv_obj_t *obj, enum alv_menu_item_type type,
                            uint32_t id, const char *text, void *icon);

/**
 * alv_menu_remove_item - remove an item from menu by menu id
 * @obj: [IN] pointer to menu obj
 * @id: unique id to identify the menu item. Note: ID 0xFFFFFFF0 ~
 * 0xFFFFFFFF are reserved for alv_menu
 *
 * Return: the created menu, NULL for failure
 */
bool alv_menu_remove_item(lv_obj_t *obj, uint32_t id);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __ALV_MENU_H__ */
