/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the macros,structs and apis for receiving
 *    and sending AT via serial ports.
 */

#ifndef AIC_SERIAL_RECV_SEND_H
#define AIC_SERIAL_RECV_SEND_H

typedef struct AT_PORT_INFO_STRUCT {
    u32 at_com_id;
    u8 recv_com_added;
    u8 reply_com_added;
} AT_PORT_INFO;

/* Define global functions. */
void aic_serial_recv_init(void);
void aic_serial_recv_callback(u32 conn_id, u8 *at_str, size_t len, void *p);
u8 aic_get_at_port_index(u32 at_port_id);
AT_PORT_INFO *aic_add_new_at_port(u32 port_id);

#ifdef WIN32
void AIC_SEND_AT(u8 *at_req_str);
aos_status AIC_RECV_AT(u8 *at_cnf_str);
#endif

#endif  /* AIC_SERIAL_RECV_SEND_H */
