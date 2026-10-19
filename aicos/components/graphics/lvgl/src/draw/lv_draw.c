/**
 * @file lv_draw.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x14fe59
#define LOG_MODULE    AMOD_LVGL

#include "lv_draw.h"
#include "sw/lv_draw_sw.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_init(void)
{
    /*Nothing to init now*/
}

void lv_draw_wait_for_finish(lv_draw_ctx_t * draw_ctx)
{
    if(draw_ctx->wait_for_finish) draw_ctx->wait_for_finish(draw_ctx);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

