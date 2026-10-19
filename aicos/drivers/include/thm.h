/*****************************************************************************/
/*                                                                           */
/*         Copyright 2023 by AIC Corporation. All rights reserved.           */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of thermc module.
 *   It includes all the thermc phy function api.
*/
#ifndef __THM_H__
#define __THM_H__

/**
 * get_cur_temp - get current SoC temperature.
 * non parameters.
 * return: current SoC temperature.
 */
int get_cur_temp(void);
#endif /* __THM_H__ */