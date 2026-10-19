
/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the interface of modem sys.
 */

#define ON    1
#define OFF   0

void aon_rfcslp_ctrl(bool on);
void cpdrv_mdmpwr_ctrl(bool on);

void cpdrv_ciphc_ctrl(bool on);

void cpdrv_rfc_ctrl(bool on);

void cpdrv_rfclk_ctrl(bool on);

void cpdrv_sleep_ctrl(bool on);

void cpdrv_sim0_ctrl(bool on);

void cpdrv_sim1_ctrl(bool on);

void cpdrv_acc_ctrl(bool on);

void mdmdrv_ipglb_ctrl(bool on);

void mdmdrv_rxdfe_ctrl(bool on);

void mdmdrv_tpp_ctrl(bool on);

void mdmdrv_symbuf_ctrl(bool on);

void mdmdrv_cbd_ctrl(bool on);

void mdmdrv_ace_ctrl(bool on);

void mdmdrv_rshbuf_ctrl(bool on);

void mdmdrv_mobi_ctrl(bool on);

void mdmdrv_pds_ctrl(bool on);

void mdmdrv_srch_ctrl(bool on);

void mdmdrv_meas_ctrl(bool on);

void mdmdrv_ul_ctrl(bool on);

void mdmdrv_dbuf_ctrl(bool on);

void mdqos_ctrl(void);

