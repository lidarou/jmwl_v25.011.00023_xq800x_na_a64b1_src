/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file define the audio focus policy.
 */


#ifndef __AF_CTX_MATRIX_CFG_H__
#define __AF_CTX_MATRIX_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "audiofocus.h"

enum af_context_action {
    AF_ACT_EXCLUSIVE,
    AF_ACT_REJECT,
    AF_ACT_CONCURRENT,
    AF_ACT_INVALID
};

/* TODO: Should be configed by scene define*/
static enum af_context_action s_af_context_matrix[AF_CTX_MAX][AF_CTX_MAX] = {
    {AF_ACT_INVALID, AF_ACT_EXCLUSIVE, AF_ACT_EXCLUSIVE},
    {AF_ACT_REJECT, AF_ACT_INVALID, AF_ACT_REJECT},
    {AF_ACT_REJECT, AF_ACT_EXCLUSIVE, AF_ACT_EXCLUSIVE}};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __AF_CTX_MATRIX_CFG_H__ */