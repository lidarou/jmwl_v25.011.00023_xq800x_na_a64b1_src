/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#ifndef INITCALL_H
#define INITCALL_H

struct initcall_t {
    int (*func)(void);
    const char *name;
};

extern const struct initcall_t __initcall_begin;
extern const struct initcall_t __initcall_end;

#define __define_initcall(lvl, fn) \
        static struct initcall_t __initcall_##fn \
        __attribute__((used, section(".initcall" #lvl ".init"))) \
        = {.func = (fn), .name = #fn,}

#define initcall_begin    (&__initcall_begin)
#define initcall_end    (&__initcall_end)

#define core_init(fn)            __define_initcall(1, fn)
#define core_init_late(fn)        __define_initcall(2, fn)
#define early_init(fn)            __define_initcall(3, fn)
#define early_init_late(fn)        __define_initcall(4, fn)
#define service_init(fn)        __define_initcall(5, fn)
#define service_init_late(fn)    __define_initcall(6, fn)
#define driver_init(fn)            __define_initcall(7, fn)
#define driver_init_late(fn)    __define_initcall(8, fn)

#endif
