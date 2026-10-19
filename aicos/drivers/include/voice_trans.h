/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file declair APIs about voice data transfer.
 *
 */

/**
 * voice_chn_open - Open voice transfer channel.
 *
 * No input param.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int voice_chn_open(void);

/**
 * voice_chn_close - Close voice transfer channel.
 *
 * No input param.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int voice_chn_close(void);

/**
 * voice_send - Close voice transfer channel.
 *
 * @buf: voice data buffer pointer.
 * @len: data length.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int voice_send(u8 *buf, u32 len);

/**
 * voice_get - Close voice transfer channel.
 *
 * @buf: voice data buffer pointer.
 * @len: buffer length.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int voice_get(u8 *buf, u32 len);

