/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_IPERF_AT_PROCESS_H__
#define __AIC_IPERF_AT_PROCESS_H__

#define IPERF_DEFAULT_CID 1
#define IPERF_DEFAULT_DELTA_TIME 990
#define IPERF_DEFAULT_MAX_DELTA_TIME 1000

/* Define Iperf thread events. */
#define AIC_IPERF_UDP_RX 0x01
#define AIC_IPERF_UDP_TX 0x02
#define AIC_IPERF_MULTI_UDP_TX 0x04
#define AIC_IPERF_MULTI_UDP_RX 0x08
#define AIC_IPERF_TCP_RX 0x10
#define AIC_IPERF_TCP_TX 0x20
#define AIC_IPERF_MULTI_TCP_TX 0x40
#define AIC_IPERF_MULTI_TCP_RX 0x80
#define AIC_IPERF_ANY_EVENT 0xFF

/* Define global functions. */
u32 aic_handle_iperf_init_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_iperf_tcprx_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_iperf_tcptx_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_iperf_udprx_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_iperf_udptx_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_iperf_term_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_iperf_udprtx_exe_cmd(AT_REQ_RESP *at_ptr);

u32 aic_handle_multi_iperf_udptx_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_multi_iperf_udprx_exe_cmd(AT_REQ_RESP *at_ptr);
u32 aic_handle_multi_iperf_udprtx_exe_cmd(AT_REQ_RESP *at_ptr);

u32 aic_handle_multi_iperf_tcptx_exe_cmd(AT_REQ_RESP *at_ptr);

#endif