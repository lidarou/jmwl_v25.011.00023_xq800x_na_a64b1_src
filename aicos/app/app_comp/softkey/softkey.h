/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of softkey.
 */

#ifndef __SOFTKEY_H__
#define __SOFTKEY_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

 /**********************
  *      TYPEDEFS
  **********************/
 typedef struct{
    char * p_txt;
    lv_event_cb_t _clicked_callback; //TODO:Support this callback
    /*TODO: Add more controllor*/
}softkey_t;

/*********************
 *      DEFINES
 *********************/
#ifdef CONFIG_COMP_SOFTKEY
#define SHOW_SOFTKEY(left_softkey, center_softkey, right_softkey) comp_softkey_show( left_softkey, center_softkey, right_softkey)
#else
#define SHOW_SOFTKEY(left_softkey, center_softkey, right_softkey)
#endif

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * comp_softkey_show(softkey_t *, softkey_t *, softkey_t*) - soft key whether display and the displayed text.
 * @arg1: left_softkey:the infomation of left softkey, NULL to hidden left softkey.
 * @arg2: center_softkey:the infomation of center softkey, NULL to hidden center softkey.
 * @arg3: right_softkey:the infomation of right softkey, NULL to hidden right softkey.
 * Return: none.
 */
void comp_softkey_show( softkey_t *left_softkey, softkey_t *center_softkey, softkey_t *right_softkey);

/**
 * comp_softkey_init
 * @arg1: none
 * @arg2: none.
 * Return: none.
 */
void comp_softkey_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __SOFTKEY_H__ */

