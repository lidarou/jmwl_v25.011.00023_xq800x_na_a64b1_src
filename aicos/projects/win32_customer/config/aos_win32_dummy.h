/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines AIC OSA configurations.
 */

#ifndef __AOS_WIN32_DUMMY_H__
#define __AOS_WIN32_DUMMY_H__

void aic_sys_assert(char *fmt, ...);
void printf_message(void *message);
void printf_data(u8 *data, size_t size);

#ifdef snprintf
#undef snprintf
#endif
#define snprintf(buf, size, fmt, ...) _snprintf(buf, size, fmt, ##__VA_ARGS__)

#ifdef vsnprintf
#undef vsnprintf
#endif
#define vsnprintf(buf, size, fmt, ...) _vsnprintf(buf, size, fmt, ##__VA_ARGS__)

#endif /* __AOS_WIN32_DUMMY_H__ */

