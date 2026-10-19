/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 *  DESCRIPTION
 *
 *    This file defines the apis of factoryreset.
 */
#ifndef __FACTORYRESET_H__
#define __FACTORYRESET_H__
#ifdef __cplusplus
extern "C" {
#endif


/**********************
 *      TYPEDEFS
 **********************/

typedef int32_t (*srv_func_callback)(void *p_param, uint32_t size);

typedef enum {
    FACTORYRESET_COMPLETE,
    FACTORYRESET_FAIL,
}factoryreset_msg_e;

typedef struct {
    factoryreset_msg_e fac_msg;
}factoryreset_msg_t;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * factoryreset_nv - reset nv data,include cp&ap nv.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void factoryreset_nv(void);

/**
 * factoryreset_except_folder - erase userdata except folder.
 * @arg1: p_volume_name:C disk(C:);D disk(D:).
 * @arg2: p_folder_name:need not delete folder.
 * Return: none.
 */
void factoryreset_except_folder(char *p_volume_name,char *p_folder_name);

/**
 * factoryreset_async - factory reset async api,after factoryreset,reset system.
 * @arg1: _app_fac_callback:app callback.
 * Return: after deep factoryreset,must reset system.
 */
void factoryreset_async(srv_func_callback _app_fac_callback);

#ifdef __cplusplus
} /*extern "C"*/
#endif
#endif /* __FACTORYRESET_H__ */

