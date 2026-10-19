/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This system cmd realize file.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0a9861
#define LOG_MODULE    AMOD_INIT

#ifdef CONFIG_PLAT_CONSOLE
#include <aic_cmd.h>
#include <aic_console.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_list_os_components.h>
#include <aic_version.h>

int aic_software_version(void)
{
    char version[AIC_VERSION_LEN + 1];

    aic_get_version(version, sizeof(version));

    //CONS_OUT("------------------------------------\n");
    CONS_OUT("%s", version);
    //CONS_OUT("------------------------------------\n");

    return 0;
}

int aic_version_show(int argc, char **argv)
{
    UNUSED_PARAM(argv);

    if (argc > 0)
        return AIC_EINVAL;

    aic_software_version();

    return 0;
}
CONSOLE_EXPORT_CMD(version, aic_version_show, show the aic software version);

#ifdef CONFIG_OS_COMPONENTS_CMD
int aic_timer_list(int argc, char **argv)
{
    int cnt;

    UNUSED_PARAM(argv);

    if (argc > 0)
        return AIC_EINVAL;

    //CONS_OUT("------------------------------------\n");
    CONS_OUT("Timer list:\n");
    cnt = aic_comp_list(AIC_COMP_TIMER);
    CONS_OUT("cnt=%d:\n", cnt);
    //CONS_OUT("------------------------------------\n");

    return 0;
}
CONSOLE_EXPORT_CMD(timer, aic_timer_list, timer list);

int aic_mutex_list(int argc, char **argv)
{
    int cnt;

    UNUSED_PARAM(argv);

    if (argc > 0)
        return AIC_EINVAL;

    //CONS_OUT("------------------------------------\n");
    CONS_OUT("Mutex list:\n");
    cnt = aic_comp_list(AIC_COMP_MUTEX);
    CONS_OUT("cnt=%d:\n", cnt);
    //CONS_OUT("------------------------------------\n");

    return 0;
}
CONSOLE_EXPORT_CMD(mutex, aic_mutex_list, mutex list);

int aic_semaphore_list(int argc, char **argv)
{
    int cnt;

    UNUSED_PARAM(argv);

    if (argc > 0)
        return AIC_EINVAL;

    //CONS_OUT("------------------------------------\n");
    CONS_OUT("Sema list:\n");
    cnt = aic_comp_list(AIC_COMP_SEMAPHORE);
    CONS_OUT("cnt=%d:\n", cnt);
    //CONS_OUT("------------------------------------\n");

    return 0;
}
CONSOLE_EXPORT_CMD(semaphore, aic_semaphore_list, semaphore list);

int aic_queue_list(int argc, char **argv)
{
    int cnt;

    UNUSED_PARAM(argv);

    if (argc > 0)
        return AIC_EINVAL;

    //CONS_OUT("------------------------------------\n");
    CONS_OUT("Queue list:\n");
    cnt = aic_comp_list(AIC_COMP_QUEUE);
    CONS_OUT("cnt =%d:\n", cnt);
    //CONS_OUT("------------------------------------\n");

    return 0;
}
CONSOLE_EXPORT_CMD(queue, aic_queue_list, queue list);

int aic_event_list(int argc, char **argv)
{
    int cnt;

    UNUSED_PARAM(argv);

    if (argc > 0)
        return AIC_EINVAL;

    //CONS_OUT("------------------------------------\n");
    CONS_OUT("Event list:\n");
    cnt = aic_comp_list(AIC_COMP_EVENT);
    CONS_OUT("cn=%d:\n", cnt);
    //CONS_OUT("------------------------------------\n");

    return 0;
}
CONSOLE_EXPORT_CMD(event, aic_event_list, event list);

int aic_thread_list(int argc, char **argv)
{
    int cnt;

    UNUSED_PARAM(argv);

    if (argc > 0)
        return AIC_EINVAL;

    //CONS_OUT("------------------------------------\n");
    CONS_OUT("Thread list:\n");
    cnt = aic_comp_list(AIC_COMP_THREAD);
    CONS_OUT("cnt=%d:\n", cnt);
    //CONS_OUT("------------------------------------\n");

    return 0;
}
CONSOLE_EXPORT_CMD(thread, aic_thread_list, thread list);
#endif

int aic_assert_sys_cmd(int argc, char **argv)
{
    AIC_ASSERT(0);
    return 0;
}

CONSOLE_EXPORT_CMD(assertsys, aic_assert_sys_cmd, aic assert sys);

#endif /* CONFIG_PLAT_CONSOLE */

