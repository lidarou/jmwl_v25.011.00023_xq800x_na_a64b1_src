#ifndef _SSV_HAL_IF_H_
#define _SSV_HAL_IF_H_

#include <host_config.h>


#define LBYTESWAP(a)  ((((a) & 0x00ff00ff) << 8) | \
    (((a) & 0xff00ff00) >> 8))

#define LONGSWAP(a)   ((LBYTESWAP(a) << 16) | (LBYTESWAP(a) >> 16))

typedef struct __chip_def {
    const char *chip_str;
    ssv_type_u32     chip_id;
    ssv_type_u32     eid;
} chip_def_S;
extern chip_def_S* chip_sel;

struct ssv_hal_ops
{
    char	name[32];
    int	(* chip_init)(cali_result_info* pCliRes_info);
    int	(* init_mac)(ssv_type_u8 *self_mac, ssv6xxx_hw_mode hmode, ssv_type_u32 data1);
    int	(* init_sta_mac)(ssv_type_u32 wifi_mode);
    int	(* init_ap_mac)(ssv_type_u8 *bssid, ssv_type_u8 channel);
    int (* ap_wep_setting)(ssv6xxx_sec_type sec_type, ssv_type_u8 *password,ssv_type_u8 vif_idx, ssv_type_u8 wsid);
    int (* tx_loopback_done)(ssv_type_u8 *dat);   
    int (* add_interface)(ssv_type_u8 itf_idx,ssv6xxx_hw_mode hmode,ssv_type_u8 * selfmac,ssv_type_u8 channel);
    int (* remove_interface)(ssv_type_u8 itf_idx);
    int	(* setup_ampdu_wmm)(ssv_type_bool IsAMPDU);
    int (* rf_enable)(void);
    int (* rf_disable)(void);
    int (* rf_load_default_setting)(void);
    int (* watch_dog_enable)(void);
    int (* watch_dog_disable)(void);
    int (* mcu_enable)(void);
    int (* mcu_disable)(void);
    int (* gen_rand)(ssv_type_u8 *data, ssv_type_u32 len);
    int (* promiscuous_enable)(void);
    int (* promiscuous_disable)(void);
    ssv_type_u32 (* read_chip_id)(void);
    int (* read_efuse_mac)(ssv_type_u8 *mac);
    int (* write_efuse_mac)(ssv_type_u8 *mac);
    int (* dump_efuse_data)(void);
    int (* efuse_read_usb_vid)(ssv_type_u16 *pvid);
    int (* efuse_read_usb_pid)(ssv_type_u16 *ppid);
    int (* efuse_read_tx_power1)(ssv_type_u8* pvalue);
    int (* efuse_read_tx_power2)(ssv_type_u8* pvalue);
    int (* efuse_read_xtal)(ssv_type_u8* pxtal);
    int (* efuse_write_xtal)(ssv_type_u8 xtal);
    int (* efuse_read_ble_gain)(ssv_type_u8* pxtal);
    int (* efuse_write_ble_gain)(ssv_type_u8 xtal);
    int (* efuse_write_rate_gain_b_n40)(ssv_type_u8 value);
    int (* efuse_read_rate_gain_b_n40)(ssv_type_u8* pvalue);
    int (* efuse_write_rate_gain_g_n20)(ssv_type_u8 value);
    int (* efuse_read_rate_gain_g_n20)(ssv_type_u8* pvalue);
   ssv_type_bool (* get_diagnosis)(void);
    int (* is_heartbeat)(void);
    int (* reset_heartbeat)(void);
    int (* get_fw_status)(ssv_type_u32 *val);
    int (* set_fw_status)(ssv_type_u32 val);
    int	(* reset_soc_irq)(void);
    int (* set_short_slot_time)(ssv_type_bool en);
    int (* soc_set_bcn)(enum ssv6xxx_tx_extra_type extra_type, void *frame, struct cfg_bcn_info *bcn_info, ssv_type_u8 dtim_cnt, ssv_type_u16 bcn_itv);
    int (* beacon_set)(void* beacon_skb, int dtim_offset,ssv_type_bool block);
    int (* beacon_enable)(ssv_type_bool bEnable, ssv_type_bool block);
    ssv_type_bool (*is_beacon_enable)(void);
    int (* get_tx_resources)(void *pinfo);
    int (* bytes_to_pages)(ssv_type_u32 size);
    int (* get_rssi_from_reg)(ssv_type_u8 vif_idx);
    int (* get_rc_info)(ssv_type_u16 *tx_cnt, ssv_type_u16 *retry_cnt, ssv_type_u16 *phy_rate);
    int	(* get_agc_gain)(void);
    int	(* set_agc_gain)(ssv_type_u32 gain);
    int (* set_acs_agc_gain)(void);
    int	(* ap_listen_neighborhood)(ssv_type_bool en);
    int (* set_ext_rx_int)(ssv_type_u32 pin);
    int (* pause_resuem_recovery_int)(ssv_type_bool resume);
    int (*display_hw_queue_status)(void);
    ssv_type_bool (*support_5g_band)(void);
    ssv_type_u32 (* get_ava_wsid)(ssv_type_u32 init_id);
    ssv_type_bool (*check_wsid_free)(ssv_type_u32 wsid);
    int (*read_hw_queue)(void);
    int (*rf_mib_reset)(void);
    int (*get_b_mode_count)(ssv_type_u32 *total_count, ssv_type_u32 *err_count);     
    int (*get_gn_mode_count)(ssv_type_u32 *total_count, ssv_type_u32 *err_count);     
    int (*hw_mib_reset)(void);
    int (*hw_mib_on)(void);    
    int (*hw_mib_off)(void);
    int (*hw_mib_show_rx)(void);        
    int (*hw_mib_show_tx)(void);
    int (*get_edcca_threshold)(void);
    int (*set_edcca_threshold)(ssv_type_u32 threshold);
    /** tx descriptor **/
    int	(* dump_txinfo)(void *p);
    int	(* get_valid_txinfo_size)(void);
    int	(* get_txreq0_size)(void);
    int	(* get_txreq0_ctype)(void *p);
    int	(* set_txreq0_ctype)(void *p,ssv_type_u8 type);
    int	(* get_txreq0_len)(void *p);
    int	(* set_txreq0_len)(void *p,ssv_type_u32 len);
    int	(* get_txreq0_rsvd0)(void *p);
    int	(* set_txreq0_rsvd0)(void *p,ssv_type_u32 val);
    int	(* get_txreq0_padding)(void *p);
    int	(* set_txreq0_padding)(void *p, ssv_type_u32 val);
    int	(* get_txreq0_qos)(void *p);
    int	(* get_txreq0_ht)(void *p);
    int	(* get_txreq0_4addr)(void *p);
    int	(* set_txreq0_f80211)(void *p, ssv_type_u8 f80211);
    int	(* get_txreq0_f80211)(void *p);
    int	(* set_txreq0_bssid_idx)(void *p, ssv_type_u8 idx);
    int	(* get_txreq0_bssid_idx)(void *p);
    int	(* get_txreq0_more_data)(void *p);
    int	(* set_txreq0_more_data)(void *p, ssv_type_u8 more_data);
    int	(* get_txreq0_wsid)(void *p);
    void (* set_txreq0_txtput_desc)(void *p, int len);
    ssv_type_u8 *(* get_txreq0_qos_ptr)(void *p);
    ssv_type_u8 *(* get_txreq0_data_ptr)(void *p);
    int (* tx_3to11)(ssv_type_u8 vif_idx, void *frame, ssv_type_u32 len, ssv_type_u32 priority,
                            ssv_type_u16 *qos, ssv_type_u32 *ht, ssv_type_u8 *addr4,
                            ssv_type_bool f80211, ssv_type_u8 security, ssv_type_u8 *ampdu_ctl);
    void *(* fill_txreq0)(void *frame, ssv_type_u32 len, ssv_type_u32 priority,
                            ssv_type_u16 *qos, ssv_type_u32 *ht, ssv_type_u8 *addr4,
                            ssv_type_bool f80211, ssv_type_u8 security, 
                            ssv_type_u8 tx_dscrp_flag, ssv_type_u8 vif_idx, ssv_type_u8 ampdu_ctl);
    void *(* fill_txreq0_ext)(void *frame, ssv_type_u32 len, ssv_type_u32 priority,
                            ssv_type_u16 *qos, ssv_type_u32 *ht, ssv_type_u8 *addr4,
                            ssv_type_bool f80211, ssv_type_u8 security, 
                            ssv_type_u8 tx_dscrp_flag, ssv_type_u8 vif_idx, ssv_type_u8 ampdu_ctl, ssv_type_u8 ack_ctl, ssv_type_u8 ack_seq);
    /** rx descriptor **/
    int (* rx_11to3)(void *p);
    int	(* dump_rxinfo)(void *p);
    int	(* get_rxpkt_size)(void);
    int	(* get_rxpkt_ctype)(void *p);
    int	(* get_rxpkt_len)(void *p);
    int	(* get_rxpkt_rssi)(void *p);
    int	(* set_rxpkt_rssi)(void *p, ssv_type_u32 rssi);
    int	(* get_rxpkt_qos)(void *p);
    int	(* get_rxpkt_f80211)(void *p);
    int	(* get_rxpkt_psm)(void *p);
    int	(* get_rxpkt_chl_info)(void *p);
    int	(* get_rxpkt_wsid)(void *p);
    int	(* get_rxpkt_tid)(void *p);
    int	(* get_rxpkt_seqnum)(void *p);
    ssv_type_u8 *(* get_rxpkt_qos_ptr)(void *p);
    ssv_type_u8 *(* get_rxpkt_data_ptr)(void *p);
    int (* get_rxpkt_data_len)(void *p);
    ssv_type_u8  (* get_rxpkt_bssid_idx)(void *p);

    /*HCI TRX Aggregation*/
    ssv_type_u32 (* process_hci_rx_aggr)(void* pdata, ssv_type_u32 len, RxPktHdr cbk_fh);
    ssv_type_u32 (* process_hci_tx_aggr)(void* tFrame, void* aggr_buf, ssv_type_u32* aggr_len);
    int (* hci_aggr_en)(HCI_AGGR_HW trx, ssv_type_bool en);
     /** load fw **/
    int (* download_fw)(ssv_type_u8 *fw_bin, ssv_type_u32 len);

    /**decision table**/
    int (* apply_sniffer_setting)(ssv_type_u32 mask);
    int (* accept_none_wsid)(void);
    int (* drop_none_wsid)(void);
    int (* drop_porbe_request)(ssv_type_bool IsDrop);
    int (* sta_rcv_all_bcn)(void);
    int (* sta_rcv_specific_bcn)(void);
    int (* sta_reject_bcn)(void);

    /**data flow**/
    int (* acs_rx_mgmt_flow)(void);
    int (* ap_rx_mgmt_flow)(void);
    int (* sniffer_rx_data_flow)(void);
    int (* sta_rx_data_flow)(void);

	/** station add/delete **/
	int (* sta_add)(ssv_type_u8 wsid, ssv_type_u8 *mac);
	int (* sta_remove)(ssv_type_u8 wsid);
	/** ampdu session **/
	int (* ampdu_rx_start)(ssv_type_u32 rxbuf_size);
	/** bssid set/get  **/
	ssv_type_s32 (* set_bssid)(ssv_type_u8 bsssid_idx, ssv_type_u8 *bssid);
	ssv_type_s32 (* get_bssid)(ssv_type_u8 bsssid_idx, ssv_type_u8 *bssid);
    /* power saving */
    void (* ps_hold_on3)(ssv_type_u32 value);
    void (* save_clear_trap_reason)(void);
    void (* restore_trap_reason)(void);
    void (* check_rx_page_empty)(void);
    void (* pmu_awake)(ssv_type_u32 aid);
    void (*get_temp)(ssv_type_s32 *temp);
};

extern struct ssv_hal_ops	g_hal_ssv6020;

#endif /* _SSV_HAL_IF_H_ */

