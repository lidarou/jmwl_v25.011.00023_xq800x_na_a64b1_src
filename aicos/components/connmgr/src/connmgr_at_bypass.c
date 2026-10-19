/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the connect at bypass source file.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x11a5cf
#define LOG_MODULE    AMOD_SYS

#include <aic_init.h>
#include <aic_osa.h>
#include <aipc_api.h>
#include <connmgr_api.h>
#include <connmgr_at_bypass.h>

#define AT_BYPSS_STACK_SIZE     768
#define AT_BYPSS_THREAD_PRI     10
#define AT_BYPSS_THREAD_NAME    "at_bypass"
#define once_read_size          128

typedef struct bypass_at {
    u32             conn_id;
    ABRR_HANDLE     brr;
    aos_thread_id   thread;
} bypass_at_t;

static bypass_at_t s_bypass_at;

static void bypass_at_recv_thread(void *p)
{
    bypass_at_t *bypass = (bypass_at_t *)p;
    char buf[once_read_size];
    int size;

    while (1) {
        size = abuf_read(bypass->brr, CONNECT_BYPSS_AT_LINK,
                         buf, sizeof(buf), AIPC_WAIT_FOREVER);
        if (size < 0) {
            aos_sleep(1000);
            continue;
        }

        /* bypss to pc */
        if (size > 0)
            conn_cmd_send(CONN_CMD_AT, s_bypass_at.conn_id,  buf, size, AOS_WAIT_FOREVER);
    }
}

void conn_at_by_pass_cp(u32 conn_id, char c)
{
    char buf[1];

    buf[0] = c;
    s_bypass_at.conn_id = conn_id;

    abuf_write(s_bypass_at.brr, CONNECT_BYPSS_AT_LINK, buf, 1, AIPC_WAIT_FOREVER);
}

int conn_at_by_pass_init()
{
    s_bypass_at.brr = ablk_request_brr(AIC_SYS_CP, AIPC_BRR_AT);
    AIC_ASSERT(s_bypass_at.brr);

    s_bypass_at.thread = aos_create_thread_simply(AT_BYPSS_THREAD_NAME,
                                                  bypass_at_recv_thread,
                                                  &s_bypass_at,
                                                  AT_BYPSS_STACK_SIZE,
                                                  AT_BYPSS_THREAD_PRI);
    AOS_ASSERT(AOS_INVALID_THREAD_ID != s_bypass_at.thread);

    return AOS_SUCCESS;
}
AIC_NORMAL_APP_P1_INIT_EXPORT(conn_at_by_pass_init);

