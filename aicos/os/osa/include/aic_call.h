/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
#ifndef __AIC_CALL_H__
#define __AIC_CALL_H__

/* Ensure that standard C is used to process the API information.  */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_ROM_CALL_SUPPORT

#ifdef CONFIG_ROM_BIN_BUILD
#define ROM_CALL_DEF(fn) rc_##fn
#define ROM_CALL(fn) rc_##fn
#define ROM_FUNC(fn) fn
#else
#define ROM_CALL_DEF(fn) _not_used_rc_##fn
#define ROM_CALL(fn) fn
#define ROM_FUNC(fn) fn
#endif

#else
#define ROM_CALL_DEF(fn) _not_used_rc_##fn
#define ROM_CALL(fn) fn
#define ROM_FUNC(fn) fn
#endif 


/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif /* __AIC_CALL_H__ */

