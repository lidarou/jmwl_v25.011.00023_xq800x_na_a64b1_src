/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#ifndef __CONSOLE_CONSOLE_H__
#define __CONSOLE_CONSOLE_H__

#include "inttypes.h"
#include "io.h"

struct console_dev {
    char *name;
    int (*setup)(struct console_dev *dev);
    void (*putc)(struct console_dev *dev, char ch);
    const void *data;
};

/* mask log_print for banyan */
#ifdef CONFIG_LOG_DECREASE
#define log_print(...)
int log_print_r(const char *fmt, ...);
#else
int log_print(const char *fmt, ...);
#define log_print_r(...)
#endif

void console_init(void);
int console_register(struct console_dev *cons);
#endif /*__CONSOLE_CONSOLE_H__*/
