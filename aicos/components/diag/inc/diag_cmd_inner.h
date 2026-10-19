/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This inner diag cmd inner header file
 *
 */
#ifndef __DIAG_CMD_INNNER_H__
#define __DIAG_CMD_INNNER_H__

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io_dev.h>
#include <aic_math.h>
#include <aic_osa.h>
#include <aic_size.h>
#include <aipc_api.h>
#include <diag_cmd_api.h>
#include <diag_cmd_codec.h>

#define ENABLE_DIAG_DEBUG  0
#if IS_CONF_ENABLED(ENABLE_DIAG_DEBUG)
#define diag_debug  alog_info
#define diag_iddbg  aicx_info
#else
#define diag_debug  alog_debug
#define diag_iddbg  aicx_debug
#endif

/* the diag processor data struct */
typedef struct diag_proc {
    aos_thread_id   thread;
    aos_queue_ptr   queue;
} diag_proc_t;

/* the diag responser data struct */
typedef struct diag_rsp {
    aos_thread_id   thread;
    aos_queue_ptr   queue;
    void            *handle;
} diag_rsp_t;

/**
 * diag_rsp_response - Send a response cmd to PC.
 *
 * @rsp: the diag responser pointer.
 * @rsp_cmd: the response cmd. The more info see struct diag_rsp_cmd define.
 */
void diag_rsp_response(diag_rsp_t *rsp, diag_rsp_cmd_t *rsp_cmd);

/*
 * diag_cmd_rsp_create - Create a diag responser.
 *
 * @handle: In ap, it is codec handle; In cp it is brr handle.
 *
 * @return: the diag responser pointer, never NULL.
 */
diag_rsp_t *diag_cmd_rsp_create(void *handle);

/*
 * diag_cmd_rsp_destroy - Destroy a diag responser.
 * @rsp: the diag responser pointer.
 */
void diag_cmd_rsp_destroy(diag_rsp_t *rsp);

/*
 * diag_proc_add_cmd - Add a recv cmd to cmd process queue.
 *
 * @proc: the diag processor pointer.
 * @cmd: the diag cmd.
 */
void diag_proc_add_cmd(diag_proc_t *proc, diag_recv_cmd_t *cmd);

/*
 * diag_cmd_proc_create - Create a diag responser.
 *
 * @handle: In ap, it is codec handle; In cp it is brr handle.
 *
 * @return: the diag porcessor pointer, never NULL.
 */
diag_proc_t *diag_cmd_proc_create(void);

/*
 * diag_cmd_proc_destroy - Destory the diag responser.
 *
 * @handle: In ap, it is codec handle; In cp it is brr handle.
 */
void diag_cmd_proc_destroy(diag_proc_t *proc);

/*
 * diag_cmd_send_cmd_by_brr - Send a diag cmd by brr.
 *
 * @brr: the brr handle.
 * @link: link id
 * @cmd: the pointer of the send cmd.
 */
void diag_cmd_send_cmd_by_brr(ABRR_HANDLE brr, u8 link, diag_recv_cmd_t *cmd);

/*
 * diag_cmd_recv_cmd_by_brr - Receive a diag cmd by brr.
 *
 * @brr: the brr handle.
 * @link: link id
 *
 * @return: the cmd pointer.
 */
diag_recv_cmd_t *diag_cmd_recv_cmd_by_brr(ABRR_HANDLE brr, u8 link);

/*
 * diag_cmd_send_rsp_by_brr - Send a diag cmd rsp by brr.
 *
 * @brr: the brr handle.
 * @link: link id
 * @cmd: the pointer of the send cmd.
 */
void diag_cmd_send_rsp_by_brr(ABRR_HANDLE brr, u8 link, DIAG_CMD_HEAD_T *cmd);

/*
 * diag_cmd_recv_rsp_by_brr - Receive a cmd rsp by brr.
 *
 * @brr: the brr handle.
 * @link: link id
 *
 * @return: the cmd pointer.
 */
DIAG_CMD_HEAD_T *diag_cmd_recv_rsp_by_brr(ABRR_HANDLE brr, u8 link);

/*
 * diag_conn_write - Send a buffer to pc by diag connect.
 *
 * @buf: the data buffer.
 * @size: the size of the data.
 */
void diag_conn_write(void *buf, size_t size);

/*
 * diag_proc_find_handler - Try find the handler of the diag cmd.
 *
 * @cmd: the diag cmd.
 *
 * @return: if not found return NULL.
 */
void *diag_proc_find_handler(u8 cmd);

#endif /* __DIAG_CMD_INNNER_H__ */

