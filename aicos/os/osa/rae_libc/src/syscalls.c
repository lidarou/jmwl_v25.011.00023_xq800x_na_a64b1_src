/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1e4143
#define LOG_MODULE    AMOD_AOS

#include <errno.h>
#include <unistd.h>

#include <aic_assert.h>

/* 
 * CONFIG_REALIZE_SBRK
 * default disable it, will use _malloc_r/_free_r.in mem_ex.c,
 * use outself memory heap sys.
 */
#define CONFIG_REALIZE_SBRK     0

/* 
 * CONFIG_REALIZE_SYS
 * if libc use nosys lib, this featuse can be disable.
 * use outself memory heap sys.
 */
#define CONFIG_REALIZE_SYS      0

#if CONFIG_REALIZE_SYS
int __errno;

void _exit(int status)
{
    /* not support, just assert */
    AICX_ASSERT_IFX(0, 0x1d39a9, "", "_exit");
}

void _abort(void)
{
   /* not support, just assert */
   AICX_ASSERT_IFX(0, 0x01fc2b, "", "_abort");
}

pid_t _getpid (void)
{
    /* not support, just return -1 */
    return -1;
}

int _kill (pid_t pid, int sin)
{
    /* not support, just return -1 */
    return -1;
}
#endif


#if CONFIG_REALIZE_SBRK
typedef int ptrdiff_t;

void *_sbrk (ptrdiff_t __incr)
{
    return NULL;
}

void *
_sbrk_r (struct _reent *reent, int32_t incr)
{
    reent->_errno = ENOMEM;
    return (void *) -1;
}
#endif
