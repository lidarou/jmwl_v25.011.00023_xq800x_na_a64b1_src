/*****************************************************************************/
/*                                                                           */
/*         Copyright 2022 by GUIXIN Corporation. All rights reserved.           */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head file of tzc.
 */

#ifdef CONFIG_TZC
void tzc_init(void);
#else
inline void tzc_init(void)
{

}
#endif