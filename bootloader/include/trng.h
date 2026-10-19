/*****************************************************************************/
/*                                                                           */
/*         Copyright 2025 by AICXTEK Corporation. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of trng module.
 *   It includes all the adc function api.
*/
#ifndef __TRNG_H__
#define __TRNG_H__

#ifdef CONFIG_TRNG
int trng_init(void);
void trng_deinit(void);
int trng_get_data(int *data, int size);
#else
static int trng_init(void) {return 0;}
static void trng_deinit(void) {return;}
static int trng_get_data(int *data, int size) { return 0;}
#endif

#endif /* __TRNG_H__ */
