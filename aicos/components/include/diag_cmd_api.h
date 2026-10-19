/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the diag module-related data and api.
 */
#ifndef __DIAG_CMD_API_H__
#define __DIAG_CMD_API_H__

#include <aic_type.h>
#include <diag_cmd.h>

/*
 * diag_rsp_cmd - the diag rsp data define.
 *
 * @sn : the sn of this diag response frame.
 * @sys : the respond cmd will send to which sys. Send to PC use AIC_SYS_PC.
 * @err_no: succ is DIAG_RSP_SUCC, failed is the error number.
 *         use the sn saved in diag handler function.
 * @m2: is DIAG_MAGIC_2;
 * @cmd: the diag cmd.
 * @subcmd: the sub diag cmd.
 * @data: the data of the cmd, can be null.
 * @data_len: the len of the cmd data, if no data, the data len must be 0.
*/
typedef struct diag_cmd {
    u32     sn;
    u8      err_no;
    u8      m2;
    u8      sys;
    u8      cmd;
    u16     subcmd;
    size_t  data_len;
    void    *data;
} diag_cmd_t;

#define diag_set_cmd(_diagcmd, _sn, _err, _m2, _sys, _cmd, _sub, _data, _len) \
    do { \
        (_diagcmd)->sn = (_sn); \
        (_diagcmd)->err_no = (_err); \
        (_diagcmd)->m2 = _m2; \
        (_diagcmd)->sys = (_sys); \
        (_diagcmd)->cmd = (_cmd); \
        (_diagcmd)->subcmd = (_sub); \
        (_diagcmd)->data = (_data); \
        (_diagcmd)->data_len = (_len); \
    } while (0)

typedef struct diag_cmd diag_recv_cmd_t;
typedef struct diag_cmd diag_send_cmd_t;
typedef struct diag_cmd diag_rsp_cmd_t;

#define diag_set_send   diag_set_cmd
#define diag_set_recv   diag_set_cmd
#define diag_set_rsp    diag_set_cmd

/**
 * Diag cmd handler function define.
 * @diag_cmd: the diag cmd ptr.
 * @pri: the private callback pointer.
 *
 */
typedef void (*DIAG_HANDLER_T)(diag_send_cmd_t *diag_cmd, void *pri);

#ifdef CONFIG_DIAG_SUPPORT
/**
 * diag_register_cmd_handler - Register the handler for the specified cmd.
 *
 * @sn : the sn of this diag frame, user must save the sn,
 *       response cmd need use it.
 * @subcmd: the diag cmd.
 *
 * @handler: the handler of the cmd.
 * @pri: the private callback pointer.
 */
void diag_register_cmd_handler(u8 cmd, DIAG_HANDLER_T handler, void *pri);

/**
 * diag_cmd_response - Send a response cmd to PC.
 *
 * @rsp_cmd: the response cmd. The more info see struct diag_rsp_cmd define.
 */
void diag_cmd_response(diag_rsp_cmd_t *rsp_cmd);

/**
 * ap_send_cmd_to_cp - Send a diag cmd to cp.
 *
 * @cmd: the cmd;
 * @subcmd: the subcmd;
 * @data: the data pointer, can be null, if it is null, the size must be 0.
 * @size: the size of the data.
 */
void ap_send_diag_cmd_to_cp(u8 cmd, u16 subcmd, void *data, size_t size);
#define ap_send_cmd_to_cp(cmd, subcmd) ap_send_diag_cmd_to_cp(cmd, subcmd, NULL, 0)

/**
 * diag_cmd_agency_reset_iodev
 *
 * will set the new diag form nv，and create a new thread to listen it.
 */
void diag_cmd_agency_reset_iodev(void);

#else /* CONFIG_DIAG_SUPPORT */
/* dummy all api. */
static inline void diag_register_cmd_handler(u8 cmd, DIAG_HANDLER_T handler,
                                             void *pri) {}
static inline void diag_cmd_response(diag_rsp_cmd_t *rsp_cmd) {}
static inline void ap_send_cmd_to_cp(u8 cmd, u16 subcmd) {}
static inline void ap_send_diag_cmd_to_cp(u8 cmd, u16 subcmd, void *data, size_t size) {}
#endif /* CONFIG_DIAG_SUPPORT */

#endif /* __DIAG_CMD_API_H__ */
