/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the interfaces to convert between unicode and gb2312
 */
#ifndef __AIC_CONVERTER_H__
#define __AIC_CONVERTER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/
#ifdef WIN32
#include "stdint.h"
#else
#include "aic_type.h"
#endif

/*******************************************
 *              DEFINES
 ******************************************/
/* Return code if invalid input after a shift sequence of n bytes was read.
   (xxx_mbtowc) */
#define AIC_RET_SHIFT_ILSEQ(n) (-1 - 2 * (n))
/* Return code if invalid. (xxx_mbtowc) */
#define AIC_RET_ILSEQ AIC_RET_SHIFT_ILSEQ(0)
/* Return code if only a shift sequence of n bytes was read. (xxx_mbtowc) */
#define AIC_RET_TOOFEW(n) (-2 - 2 * (n))
/* Return code if invalid. (xxx_wctomb) */
#define AIC_RET_ILUNI -1
/* Return code if output buffer is too small. (xxx_wctomb, xxx_reset) */
#define AIC_RET_TOOSMALL -2

/*******************************************
 *              TYPEDEFS
 ******************************************/
/* CJK character sets [CCS = coded character set] [CJKV.INF chapter 3] */
typedef struct {
    uint16_t indx; /* index into big table */
    uint16_t used; /* bitmask of used entries */
} summary16_t;

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __AIC_CONVERTER_H__ */