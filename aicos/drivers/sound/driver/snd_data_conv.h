/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file declair pcm convert functions.
 */

void pcm_conv_init(u32 rate);
int pcm_conv(char *in, char *out, u32 bytes);
u32 pcm_conv_multi(void);

