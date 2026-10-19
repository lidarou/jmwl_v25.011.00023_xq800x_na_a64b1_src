/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file aic_str.h
 *
 */

#ifndef __AIC_STR_H__
#define __AIC_STR_H__

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

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * @brief  常见unicode(UCS-2)转utf-8
 * @note   暂不支持UCS-4 (4字节)编码
 * @param  *src: 输入unicode指针(被转换成unsigned short类型进行识别)
 * @param  slen: 输出unicode字节数
 * @param  *dst: 输出utf-8缓冲区
 * @param  dlen: utf-8缓冲区长度
 * @retval <0:错误，>=0:输出utf-8字节数
 */
int str_unicode_to_utf8(unsigned char *src, int slen, unsigned char *dst,int dlen);

/**
 * @brief  utf-8转unicode(UCS-2)
 * @note   暂不支持UCS-4 (4字节)编码
 * @param  src: 输入utf-8指针
 * @param  dst: 输出Unicode缓冲区
 * @param  dlen: Unicode缓冲区长度
 * @retval <0:错误，>=0:输出uinicode字节数
 */
int str_utf8_to_unicode(char *src, char *dst, int dlen);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __AIC_STR_H__ */

