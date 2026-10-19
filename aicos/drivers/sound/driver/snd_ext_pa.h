/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This is external PA module header file.
 */


int snd_ext_pa_on(void);
int snd_ext_pa_off(void);
int snd_ext_pa_reg(int (*pa_on)(u32), int (*pa_off)(u32), u32 data);

