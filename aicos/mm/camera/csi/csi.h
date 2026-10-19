/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __CSI_H__
#define __CSI_H__

#include <aic_bitmap.h>

#include "csi_rf.h"

void csi_enable(void);
void csi_disable(void);
int csi_start(void);
void csi_stop(void);
int is_csi_enabled(void);
int csi_cnt_mode_enable(boolean enable);
int csi_cnt_mode_config(u32 image_width, u32 pixel_size);
int csi_endian_mode_config(u32 endian);
int csi_lsb_mode(u32 mode);
int csi_wire_swap(u32 swap);
int csi_wire_num_config(u32 wire_num);
int csi_crc_enable(int enable);
int csi_mformat_enable(int enable);
int csi_debug_enable(int enable);
int csi_cap_mode_enable(int enable);
int csi_output_plane_config(u32 plane_mode);
int csi_ddr_mode_config(u32 ddr_mode);
int csi_rxfifo_thrd_config(int size);
int csi_set_sync_word_head(u32 sync_word);
int csi_set_sync_word_id(u32 sof, u32 sol, u32 eol, u32 eof);
int csi_frame_addr_config(u32 addr);
u32 csi_get_frame_addr_config(void);
int csi_frame_addr2_config(u32 addr2);
int csi_set_addr_limit(u32 image_size);
void csi_int_unmask_all(void);
void csi_int_mask_all(void);
void csi_int_clear_all(void);
int csi_eof_int_enable(u32 enable);
void csi_int_mask_sof(void);
void csi_int_mask_eol(void);
void csi_int_mask_sol(void);
void csi_int_clear_eof(void);
void csi_int_clear_sof(void);
void csi_int_clear_rxfifo_full(void);
void csi_int_clear_timeout(void);
void csi_int_clear_eol(void);
void csi_int_clear_sol(void);

#endif /* __CSI_H__ */

