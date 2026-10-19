/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#include "console/console.h"
#include "platform/platform.h"
#include <stdarg.h>

struct console {
    char *name;
    struct console_dev *cons;
    #define CONSOLE_REGISTER 1
    #define CONSOLE_UNREGISTER 0
    uint32_t status;
};

struct console console;

struct printf_info {
    char zs;    /* non-zero if a digit has been written */
    char *bf;    /* Digit buffer */
    char *outstr;    /* Next output position for sprintf() */
    struct console_dev *dev;
    /* Output a character */
    void (*putc)(struct console_dev *dev, char ch);
};

static void out(struct printf_info *info, char c)
{
    *info->bf++ = c;
}

static void out_dgt(struct printf_info *info, char dgt)
{
    out(info, dgt + (dgt < 10 ? '0' : 'a' - 10));
    info->zs = 1;
}

static void div_out(struct printf_info *info, unsigned long *num,
            unsigned long div)
{
    unsigned char dgt = 0;

    while (*num >= div) {
        *num -= div;
        dgt++;
    }

    if (info->zs || dgt > 0)
        out_dgt(info, dgt);
}


static int _vprintf(struct printf_info *info, const char *fmt, va_list va)
{
    unsigned long div;
    unsigned long num;
    char buf[12];
    char ch;
    char *p;


    while ((ch = *(fmt++))) {
        if (ch != '%') {
            info->putc(info->dev, ch);
        } else {
            bool lz = false;
            int width = 0;
            bool islong = false;

            ch = *(fmt++);
            if (ch == '-')
                ch = *(fmt++);

            if (ch == '0') {
                ch = *(fmt++);
                lz = 1;
            }

            if (ch >= '0' && ch <= '9') {
                width = 0;
                while (ch >= '0' && ch <= '9') {
                    width = (width * 10) + ch - '0';
                    ch = *fmt++;
                }
            }
            if (ch == 'l') {
                ch = *(fmt++);
                islong = true;
            }

            info->bf = buf;
            p = info->bf;
            info->zs = 0;

            switch (ch) {
            case '\0':
                return 0;
            case 'u':
            case 'd':
            case 'i':
                div = 1000000000;
                if (islong) {
                    num = va_arg(va, unsigned long);
                    if (sizeof(long) > 4)
                        div *= div * 10;
                } else {
                    num = va_arg(va, unsigned int);
                }

                if (ch != 'u') {
                    if (islong && (long)num < 0) {
                        num = -(long)num;
                        out(info, '-');
                    } else if (!islong && (int)num < 0) {
                        num = -(int)num;
                        out(info, '-');
                    }
                }
                if (!num) {
                    out_dgt(info, 0);
                } else {
                    for (; div; div /= 10)
                        div_out(info, &num, div);
                }
                break;
            case 'p':
                islong = true;
                /* no break */
            case 'x':
                if (islong) {
                    num = va_arg(va, unsigned long);
                    div = 1UL << (sizeof(long) * 8 - 4);
                } else {
                    num = va_arg(va, unsigned int);
                    div = 0x10000000;
                }
                if (!num) {
                    out_dgt(info, 0);
                } else {
                    for (; div; div /= 0x10)
                        div_out(info, &num, div);
                }
                break;
            case 'c':
                out(info, (char)(va_arg(va, int)));
                break;
            case 's':
                p = va_arg(va, char*);
                break;
            case '%':
                out(info, '%');
            default:
                break;
            }

            *info->bf = 0;
            info->bf = p;
            while (*info->bf++ && width > 0)
                width--;
            while (width-- > 0)
                info->putc(info->dev, lz ? '0' : ' ');
            if (p) {
                while ((ch = *p++))
                    info->putc(info->dev, ch);
            }
        }
    }

    return 0;
}

#ifdef CONFIG_LOG_DECREASE
int log_print_r(const char *fmt, ...)
#else
int log_print(const char *fmt, ...)
#endif
{
    struct printf_info info;
    va_list va;
    int ret;

    if(console.status == CONSOLE_UNREGISTER)
        return 0;

    info.dev = console.cons;
    info.putc = console.cons->putc;
    va_start(va, fmt);
    ret = _vprintf(&info, fmt, va);
    va_end(va);

    return ret;
}

int console_register(struct console_dev *cons)
{
    int ret;

    console.cons = cons;
    console.status = CONSOLE_REGISTER;

    if (!console.cons->setup)
        return -1;

    ret = console.cons->setup(cons);

    log_print("%s bootloader console register.\n", platform.name);

    return ret;
}

void console_init(void)
{
    console.status = CONSOLE_UNREGISTER;
    console.cons = NULL;
    console.name = "console";
}

