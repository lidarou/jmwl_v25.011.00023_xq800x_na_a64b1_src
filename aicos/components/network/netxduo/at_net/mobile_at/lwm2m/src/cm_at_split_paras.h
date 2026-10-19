/*****************************************************************************/
/*                                                                           */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Define APIs to split parameters of AT commands
 *
 */

#ifndef __CM_AT_AT_SPLIT_PARAS_H__
#define __CM_AT_AT_SPLIT_PARAS_H__

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus
/* Yes, C++ compiler is present.  Use standard C.  */
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/
#include <aic_errno.h>
#include <aic_osa_ex.h>

/*******************************************
 *              TYPEDEFS
 ******************************************/
enum {
    AT_CM_SPLIT_MODE_NORMAL = 0,

    AT_CM_SPLIT_MODE_MAX
};

/*******************************************
 *              EXTERNAL PROTOTYPES
 ******************************************/
/**
 * aic_cm_split_paras_to_str_array - split paramters in AT set command
 * @raw_str: [IN] original AT command string. e.g. AT+COPS=0
 * @start_str: [IN] prefix of AT commands, e.g. AT+COPS
 * @paras_cnt: [OUT] the count of parameter entries in the command
 * @format: [IN] Reserved for future use. AIC_NULL for non-specific format
 *
 * Return: a pointer to the splitted parameter array
 * Note: Please free memory of the returned parameter array by aic_cm_free_paras_mem
 */
ulong *aic_cm_split_paras_to_str_array(const u8 *raw_str, const u8 *start_str, u16 *paras_cnt, const u8 *format);

/**
 * aic_cm_free_paras_mem - free memory for parameter array allocated by aic_cm_split_paras_to_str_array
 * @ptr: [IN] parameter array
 * @paras_cnt: [IN] count of entries in the parameter array returned in aic_cm_split_paras_to_str_array
 *
 * Return: NA
 * Note: ptr will be determined if it is NULL inside the implementation before free.
 */
void aic_cm_free_paras_mem(ulong *ptr, u16 paras_cnt);

/**
 * aic_cm_split_para_is_quotated_str - check if the parameter entry is quotated string in original AT
 * @para: [IN] parameter entry
 *
 * Return: AIC_TRUE if quotated, otherwise AIC_FALSE
 * Note: the para entry must be returned by aic_cm_split_paras_to_str_array
 */
u8 aic_cm_split_para_is_quotated_str(ulong para);

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef __cplusplus
}
#endif

#endif /* __CM_AT_AT_SPLIT_PARAS_H__ */