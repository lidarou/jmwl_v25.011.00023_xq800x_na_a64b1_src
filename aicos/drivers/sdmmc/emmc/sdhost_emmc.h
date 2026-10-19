/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines sdhost emmc Interface.
 */

#ifndef _SD_HOST_EMMC_H_
#define _SD_HOST_EMMC_H_

// eMMC modes
#define eMMC_MODE_SDR     0
#define eMMC_MODE_HS      1
#define eMMC_MODE_HS200   2
#define eMMC_MODE_HS400   3
//  #define eMMC_MODE_HS_DDR  5

#define DDR_MODE_BIT      4
#define E_STROBE_BIT      5

typedef struct {
    // [31] Reliable Write Request
    uint32_t rel_wr_req;
    // [30] DD: "1" read / "0" write
    uint32_t read;
    // [29] Tag request
    uint32_t tag_req;
    // [28:25] Context ID
    uint32_t cont_id;
    // [24]: Forced Programming
    uint32_t force_prog;
    // [23] Priority: "0" simple / "1" high
    uint32_t priority;
    // [22:21] reserved
    // [20:16] Task ID
    uint32_t id;
    // [15:0] Number of Blocks
    uint32_t block_cnt;
    // Address
    uint32_t address;
    // System address
    uint32_t sys_address;
    // CMD44 response
    uint32_t cmd44_resp;
    // CMD45 response
    uint32_t cmd45_resp;
    // CMD46/47 response
    uint32_t cmd_cqexe_resp;
} cq_task_t;

extern uint32_t ext_csd[512/4];
extern cq_task_t cq_task[32];

int eMMC_Card_init (char low_voltage_en);
int eMMC_select_card (unsigned int sd_card_rca);
int eMMC_set_mode (char bit_width, char mode);
unsigned long eMMC_card_tuning (unsigned char _data_width);
int eMMC_ext_csd_write (unsigned int addr, unsigned int arg);
int eMMC_ext_csd_read ();
unsigned int eMMC_ext_csd_read_byte (unsigned int addr);
int eMMC_prep_card_for_boot (unsigned int arg);
int eMMC_hw_reset ();

int eMMC_card_features ();
int eMMC_card_cq_enable (unsigned int value);
uint32_t eMMC_card_cq_status ();
uint32_t eMMC_card_cq_put (uint32_t cq_task_num);
uint32_t eMMC_card_cq_exec (uint32_t cq_task_num, uint32_t use_adma2);
uint32_t eMMC_cq_init ();
uint32_t eMMC_cq_desc (uint32_t pos, unsigned char* desc_buff,
                       uint32_t conf, uint32_t len, uint32_t blk_addr,
                       uint64_t sys_addr);
uint32_t eMMC_cq_desc_64 (uint32_t pos, unsigned char* desc_buff,
                          uint32_t conf, uint32_t len, uint32_t blk_addr,
                          uint64_t sys_addr);
uint32_t eMMC_cq_desc_dcmd (uint32_t pos, unsigned char* desc_buff,
                            uint32_t conf, uint32_t arg);
uint32_t eMMC_cq_desc_64_dcmd (uint32_t pos, unsigned char* desc_buff,
                               uint32_t conf, uint32_t arg);
uint32_t eMMC_cq_desc_disp (unsigned char* desc_buff);
uint32_t eMMC_cq_desc_64_disp (unsigned char* desc_buff);

void eMMC_CQ_halt();
void eMMC_CQ_release();
void eMMC_CQ_reset();
int eMMC_CQ_discard_all();
int eMMC_CQ_discard(int task);

int eMMC_CQ_is_error();
int eMMC_CQ_error_recovery(uint16_t rca);
int eMMC_CQ_abort(uint16_t rca);
uint32_t eMMC_CQ_task_submit(uint32_t _var);

int count_onces(uint32_t vector);
int task_id(uint32_t vector);
uint32_t eMMC_CQ_taskdoorbell();
uint32_t eMMC_CQ_task_complete_notificaiton();

#endif /* _SD_HOST_EMMC_H_ */
