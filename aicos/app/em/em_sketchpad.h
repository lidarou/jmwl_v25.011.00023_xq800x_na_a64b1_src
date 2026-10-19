/**
 * @file em_sketchpad.h
 *
 */

#ifndef __EM_SKETCHPAD_H__
#define __EM_SKETCHPAD_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    LV_SKETCHPAD_TOOLBAR_OPT_ALL = 0,
    LV_SKETCHPAD_TOOLBAR_OPT_CW,
    LV_SKETCHPAD_TOOLBAR_OPT_WIDTH,
    LV_SKETCHPAD_TOOLBAR_OPT_ERASE,
    LV_SKETCHPAD_TOOLBAR_OPT_BACK,
    LV_SKETCHPAD_TOOLBAR_OPT_LAST
}lv_sketchpad_toolbar_t;

/*Data of canvas*/
typedef struct {
    lv_img_t img;
    lv_img_dsc_t dsc;
    lv_draw_line_dsc_t line_rect_dsc;
} lv_sketchpad_t;

/***********************
 * GLOBAL VARIABLES
 ***********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
* em_sketchpad_enter_win - Brief description of function.
* Parameter: [In]: None
*            [Out]: None
*            [Return]: None
* Description: This function is used to enter sketchpad test UI.
*/
void em_sketchpad_enter_win(void);

/*=====================
 * Setter functions
 *====================*/

/*=====================
 * Getter functions
 *====================*/

/*=====================
 * Other functions
 *====================*/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*__EM_SKETCHPAD_H__*/
