#ifndef _SSV_HAL_H_
#define _SSV_HAL_H_

#include <host_config.h>
#include <ssv_types.h>
#include <cmd_def.h>


#ifdef _PORTING_C_
#include <hal/ssv6020/firmware/ssv6020C/ssv6x5x-sw.h>

#define SSV_HAL_FW_BIN ssv6x5x_sw_bin

#else //#ifdef _PORTING_C_
#define SSV_HAL_FW_BIN
#endif //#ifdef _PORTING_C_


//#define SSV6XXX_TX_DESC                                     void
//#define CFG_HOST_TXREQ                                      void
//#define CFG_HOST_RXPKT                                      void


typedef     void    CFG_HOST_RXPKT;
typedef     void    CFG_HOST_TXREQ;
typedef     void    SSV6XXX_TX_DESC;

typedef enum _SSV_FRM_TYPE{
    SSV_FRM_M0_TXREQ=0,
    SSV_FRM_M1_TXREQ,
    SSV_FRM_M2_TXREQ, 
    SSV_FRM_M0_RXEVENT,
    SSV_FRM_M2_RXEVENT,
    SSV_FRM_HOST_CMD,
    SSV_FRM_HOST_EVENT,
    SSV_FRM_RATE_RPT
}SSV_FRM_TYPE;

enum
{
    SW_RESET_MAC,
    SW_RESET_MCU,
    SW_RESET_SDIO,
    SW_RESET_SPI_SLV,
    SW_RESET_SYS_ALL     
};
/*
ssv_hal_init
return value:
    TRUE: Success
    FALSE Fail
*/
extern ssv_type_bool ssv_hal_init(void);

/*
ssv_hal_chip_init
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_chip_init(cali_result_info* pCliRes_info);

/*
ssv_hal_init_mac
self_mac: input
hmode: input
data1: input
return value:
    1: Success
    0: Fail
*/
extern int ssv_hal_init_mac(ssv_type_u8 *self_mac, ssv6xxx_hw_mode hmode, ssv_type_u32 data1);

/*
ssv_hal_init_sta_mac
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_init_sta_mac(ssv_type_u32 wifi_mode);

/*
ssv_hal_init_ap_mac
bssid: input
channel: input
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_init_ap_mac(ssv_type_u8 *bssid, ssv_type_u8 channel);

/*
ssv_hal_init_ap_mac
sec_type: input
password: input
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_ap_wep_setting(ssv6xxx_sec_type sec_type, ssv_type_u8 *password, ssv_type_u8 vif_idx, 
            ssv_type_u8 wsid);

/*
ssv_hal_tx_loopback_done
dat: input
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_tx_loopback_done(ssv_type_u8 *dat);


extern int ssv_hal_add_interface(ssv_type_u8 itf_idx,ssv6xxx_hw_mode hmode,ssv_type_u8 * selfmac,ssv_type_u8 channel);
extern int ssv_hal_remove_interface(ssv_type_u8 itf_idx);
/*
ssv_hal_setup_ampdu_wmm
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_setup_ampdu_wmm(ssv_type_bool IsAMPDU);

/*
ssv_hal_rf_enable
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_rf_enable(void);

/*
ssv_hal_rf_disable
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_rf_disable(void);

/*
ssv_hal_rf_load_default_setting
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_rf_load_default_setting(void);

/*
ssv_hal_watchdog_enable
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_watchdog_enable(void);

/*
ssv_hal_watchdog_disable
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_watchdog_disable(void);


/*
ssv_hal_mcu_enable
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_mcu_enable(void);

/*
ssv_hal_mcu_disable
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_mcu_disable(void);

/*
ssv_hal_gen_rand
data: output
len: input
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_gen_rand(ssv_type_u8 *data, ssv_type_u32 len);

/*
ssv_hal_promiscuous_enable
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_promiscuous_enable(void);

/*
ssv_hal_promiscuous_disable
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_promiscuous_disable(void);

/*
ssv_hal_read_chip_id
return value:
    xxx: chip ID
    -1: Fail
*/
extern ssv_type_u32 ssv_hal_read_chip_id(void);


/*
ssv_hal_read_efuse_mac
mac: output

return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_read_efuse_mac(ssv_type_u8 *mac);
extern int ssv_hal_write_efuse_mac(ssv_type_u8 *mac);
extern int ssv_hal_dump_efuse_data(void);
extern int ssv_hal_efuse_read_usb_vid(ssv_type_u16 *pvid);
extern int ssv_hal_efuse_read_usb_pid(ssv_type_u16 *ppid);
extern int ssv_hal_efuse_read_tx_power1(ssv_type_u8* pvalue);
extern int ssv_hal_efuse_read_tx_power2(ssv_type_u8* pvalue);
extern int ssv_hal_efuse_read_xtal(ssv_type_u8* pxtal);
extern int ssv_hal_efuse_write_xtal(ssv_type_u8 xtal);
extern int ssv_hal_efuse_read_ble_gain(ssv_type_u8* pxtal);
extern int ssv_hal_efuse_write_ble_gain(ssv_type_u8 xtal);
extern int ssv_hal_efuse_write_rate_gain_b_n40(ssv_type_u8 value);
extern int ssv_hal_efuse_read_rate_gain_b_n40(ssv_type_u8 * pvalue);
extern int ssv_hal_efuse_write_rate_gain_g_n20(ssv_type_u8 value);
extern int ssv_hal_efuse_read_rate_gain_g_n20(ssv_type_u8 * pvalue);

/*
ssv_hal_get_diagnosis
return value:
    TRUE: need to reset
    FALSE: no reset
*/
extern ssv_type_bool ssv_hal_get_diagnosis(void);

/*
ssv_hal_is_heartbeat
return value:
    1: heatbeat
    0: no heatbeat
    -1: Fail
*/
extern int ssv_hal_is_heartbeat(void);

/*
ssv_hal_reset_heartbeat
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_reset_heartbeat(void);

/*
ssv_hal_get_fw_status
val: output
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_get_fw_status(ssv_type_u32 *val);

/*
ssv_hal_set_fw_status
val: input
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_set_fw_status(ssv_type_u32 val);

/*
ssv_hal_reset_soc_irq
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_reset_soc_irq(void);


/*
ssv_hal_set_short_slot_time
en: input
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_set_short_slot_time(ssv_type_bool en);

/*
ssv_hal_beacon_set
beacon_skb: input
dtim_offset:input
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_beacon_set(void* beacon_skb, int dtim_offset, ssv_type_bool block);

/*
ssv_hal_soc_set_bcn
extra_type: input
frame:input
bcn_info:input
dtim_cnt:input
bcn_itv:input
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_soc_set_bcn(enum ssv6xxx_tx_extra_type extra_type, void *frame, struct cfg_bcn_info *bcn_info, ssv_type_u8 dtim_cnt, ssv_type_u16 bcn_itv);

/*
ssv_hal_beacon_enable
bEnable: input

return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_beacon_enable(ssv_type_bool bEnable, ssv_type_bool block);

/*
ssv_hal_is_beacon_enable

return value:
    TRUE: beacon enable
    FALSE: beacon disable
*/
extern ssv_type_bool ssv_hal_is_beacon_enable(void);

/*
ssv_hal_get_tx_resources
pFreePage: output. how many pages can be stored incoming packets in wifi-chip
pFreeID: output. how many ids can be used for incoming packets in the wifi-chip
pFreeSpaces: output. how many space of HCI input queue for incoming packets

return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_get_tx_resources(void * rs_info);


/*
ssv_hal_bytes_to_pages

return value:
    xxx:Page numbers
    -1: Fail
*/
extern int ssv_hal_bytes_to_pages(ssv_type_u32 size);

/*
ssv_hal_get_rssi_from_reg

return value:
    xxx:RSSI
    -1: Fail
*/
extern int ssv_hal_get_rssi_from_reg(ssv_type_u8 vif_idx);

/*
ssv_hal_get_rc_info

tx_cnt: output
retry_cnt:output
phy_rate:output
return value:
    xxx:RSSI
    -1: Fail
*/
extern int ssv_hal_get_rc_info(ssv_type_u16 *tx_cnt, ssv_type_u16 *retry_cnt, ssv_type_u16 *phy_rate);


/*
ssv_hal_get_agc_gain

return value:
    xxx:Value
    -1: Fail
*/
extern int ssv_hal_get_agc_gain(void);

/*
ssv_hal_set_agc_gain
gain: input
return value:
    0:Success
    -1: Fail
*/
extern int ssv_hal_set_agc_gain(ssv_type_u32 gain);


/*
ssv_hal_set_acs_agc_gain
return value:
    0:Success
    -1: Fail
*/
extern int ssv_hal_set_acs_agc_gain(void);
/*
ssv_hal_ap_listen_neighborhood
en: input
return value:
    0:Success
    -1: Fail
*/
extern int ssv_hal_ap_listen_neighborhood(ssv_type_bool en);

/*
ssv_hal_set_ext_rx_int
input_level:input
return value:
    0:Success
    -1: Fail
*/
extern int ssv_hal_set_ext_rx_int(ssv_type_u32 pin);

/*
ssv_hal_rf_mib_reset
return value:
    0:Success
    -1: Fail
*/
int ssv_hal_rf_mib_reset(void);
/*
ssv_hal_get_b_mode_count
total: output
error: output
return value:
    0:Success
    -1: Fail
*/
extern int ssv_hal_get_b_mode_count(ssv_type_u32 *total, ssv_type_u32 *error);

/*
ssv_hal_get_gn_mode_count
total: output
error: output
return value:
    0:Success
    -1: Fail
*/
extern int ssv_hal_get_gn_mode_count(ssv_type_u32 *total, ssv_type_u32 *error);


extern int ssv_hal_hw_mib_reset(void);

extern int ssv_hal_hw_mib_off(void);

extern int ssv_hal_hw_mib_on(void);

extern int ssv_hal_hw_mib_show_tx(void);

extern int ssv_hal_hw_mib_show_rx(void);
/*

ssv_hal_support_5g_band
return value:
    TRUE:Support 5G
    FALSE: Don't support 5G
*/
extern ssv_type_bool ssv_hal_support_5g_band(void);

extern ssv_type_u32 ssv_hal_get_ava_wsid(ssv_type_u32 init_id);
extern ssv_type_bool ssv_hal_check_wsid_free(ssv_type_u32 wsid);

/*
ssv_hal_read_hw_queue
return value:
    0:success
    -1: fail
*/
extern int ssv_hal_read_hw_queue(void);

/*

ssv_hal_l2_off
vif_idx: intervace index
return value:
    -1:fail
     0:ok
*/
extern int ssv_hal_l2_off(ssv_type_u8 vif_idx);

/*

ssv_hal_l2_on
vif_idx: intervace index
return value:
    -1:fail
     0:ok
*/
extern int ssv_hal_l2_on(ssv_type_u8 vif_idx);

/*

ssv_hal_dump_txinfo
p: input
return value:
    0:Success
    -1: Fail
*/
extern int ssv_hal_dump_txinfo(void *p);

/*
ssv_hal_get_valid_txinfo_size
return value:
    xxx: size(unit is bytes)
    -1: Fail
*/

extern int ssv_hal_get_valid_txinfo_size(void);
/*
ssv_hal_get_txreq0_size
return value:
    xxx: size(unit is bytes)
    -1: Fail
*/
extern int ssv_hal_get_txreq0_size(void);

/*
ssv_hal_get_txreq0_ctype
p: input
return value:
    xxx: type
    -1: Fail
*/
extern int ssv_hal_get_txreq0_ctype(CFG_HOST_TXREQ *p);

/*
ssv_hal_set_txreq0_ctype
p: input
ctype: input
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_set_txreq0_ctype(CFG_HOST_TXREQ *p,ssv_type_u8 ctype);

/*
ssv_hal_get_txreq0_len
p: input
return value:
    xxx: size
    -1: Fail
*/
extern int ssv_hal_get_txreq0_len(CFG_HOST_TXREQ *p);

/*
ssv_hal_set_txreq0_len
p: input
len: input
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_set_txreq0_len(CFG_HOST_TXREQ *p,ssv_type_u32 len);

/*
ssv_hal_get_txreq0_rsvd0
p: input
return value:
    xxx: Value
    -1: Fail
*/
extern int ssv_hal_get_txreq0_rsvd0(CFG_HOST_TXREQ *p);

/*
ssv_hal_set_txreq0_rsvd0
p: input
val: input
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_set_txreq0_rsvd0(CFG_HOST_TXREQ *p,ssv_type_u32 val);

/*
ssv_hal_get_txreq0_padding
p: input
return value:
    xxx: value
    -1: Fail
*/
extern int ssv_hal_get_txreq0_padding(CFG_HOST_TXREQ *p);

/*
ssv_hal_set_txreq0_padding
p: input
val: input
return value:
    xxx: value
    -1: Fail
*/
extern int ssv_hal_set_txreq0_padding(CFG_HOST_TXREQ *p, ssv_type_u32 val);
/*
ssv_hal_get_txreq0_qos
p: input
return value:
    1: qos packet
    0: not qos packet
    -1: Fail
*/
extern int ssv_hal_get_txreq0_qos(CFG_HOST_TXREQ *p);

/*
ssv_hal_get_txreq0_ht
p: input
return value:
    1: ht packet
    0: non-ht packet
    -1: Fail
*/
extern int ssv_hal_get_txreq0_ht(CFG_HOST_TXREQ *p);

/*
ssv_hal_get_txreq0_4addr
p: input
return value:
    1: use 4addr
    0: no 4addr
    -1: Fail
*/
extern int ssv_hal_get_txreq0_4addr(CFG_HOST_TXREQ *p);

/*
ssv_hal_set_txreq0_f80211
p: input
f80211: input
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_set_txreq0_f80211(CFG_HOST_TXREQ *p, ssv_type_u8 f80211);

/*
ssv_hal_get_txreq0_f80211
p: input
return value:
    1: f80211 frame
    0: non-f80211 frame
    -1: Fail
*/
extern int ssv_hal_get_txreq0_f80211(CFG_HOST_TXREQ *p);

extern int ssv_hal_set_txreq0_bssid_idx(CFG_HOST_TXREQ *p, ssv_type_u8 idx);

extern int ssv_hal_get_txreq0_bssid_idx(CFG_HOST_TXREQ *p);

/*
ssv_hal_get_txreq0_more_data
p: input
return value:
    xxx: value
    -1: Fail
*/
extern int ssv_hal_get_txreq0_more_data(CFG_HOST_TXREQ *p);

/*
ssv_hal_set_txreq0_more_data
p: input
more_data:input
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_set_txreq0_more_data(CFG_HOST_TXREQ *p, ssv_type_u8 more_data);

/*
ssv_hal_get_txreq0_wsid
p: input
return value:
    xxx: value
    -1: Fail
*/
extern int ssv_hal_get_txreq0_wsid(CFG_HOST_TXREQ *p);

/*
ssv_hal_set_txreq0_txtput_desc
p: input, len: input
*/
extern void ssv_hal_set_txreq0_txtput_desc(CFG_HOST_TXREQ *p, int len);

/*
ssv_hal_get_txreq0_qos_ptr
p: input
return value:
    xxx: qos ptr
    Null: Fail
*/
extern ssv_type_u8 * ssv_hal_get_txreq0_qos_ptr(CFG_HOST_TXREQ *p);

/*
ssv_hal_get_txreq0_data_ptr
p: input
return value:
    xxx: data ptr
    Null: Fail
*/
extern ssv_type_u8 * ssv_hal_get_txreq0_data_ptr(CFG_HOST_TXREQ *p);

/*
ssv_hal_fill_txreq0
frame: input
len:input
f80211:input
priority:input
tx_dscrp_flag:input
return value:
    xxx: frame address
    Null: Fail
*/
extern void * ssv_hal_fill_txreq0(void *frame, ssv_type_u32 len, ssv_type_u32 priority,
                                                ssv_type_u16 *qos, ssv_type_u32 *ht, ssv_type_u8 *addr4,
                                                ssv_type_bool f80211, ssv_type_u8 security, 
                                                ssv_type_u8 tx_dscrp_flag,ssv_type_u8 vif_idx, ssv_type_u8 ampdu_ctl);




extern void * ssv_hal_fill_txreq0_ext(void *frame, ssv_type_u32 len, ssv_type_u32 priority,
                                       ssv_type_u16 *qos, ssv_type_u32 *ht, ssv_type_u8 *addr4,
                                       ssv_type_bool f80211, ssv_type_u8 security, 
                                       ssv_type_u8 tx_dscrp_flag, ssv_type_u8 vif_idx, ssv_type_u8 ampdu_ctl, ssv_type_u8 ack_ctl, ssv_type_u8 ack_seq);


/*
ssv_hal_dump_rxinfo
p: input
return value:
    0:Success
    -1: Fail
*/
extern int ssv_hal_dump_rxinfo(void *p);

/*
ssv_hal_get_rxpkt_size

return value:
    xxx: size(unit is bytes)
    -1: Fail
*/
extern int ssv_hal_get_rxpkt_size(void);

/*
ssv_hal_get_rxpkt_ctype
p: input
return value:
    xxx: type
    -1: Fail
*/
extern int ssv_hal_get_rxpkt_ctype(CFG_HOST_RXPKT *p);

/*
ssv_hal_get_rxpkt_len
p: input
return value:
    xxx: size
    -1: Fail
*/
extern int ssv_hal_get_rxpkt_len(CFG_HOST_RXPKT *p);

/*
ssv_hal_get_rxpkt_channel_info
p: input
return value:
    xxx: value
    -1: Fail
*/
extern int ssv_hal_get_rxpkt_channel_info(CFG_HOST_RXPKT *p);

/*
ssv_hal_get_rxpkt_rssi
p: input
return value:
    xxx: value
    -1: Fail
*/
extern int ssv_hal_get_rxpkt_rssi(CFG_HOST_RXPKT *p);

/*
ssv_hal_set_rxpkt_rssi
p: input
rssi: input
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_set_rxpkt_rssi(CFG_HOST_RXPKT *p, ssv_type_u32 rssi);

/*
ssv_hal_get_rxpkt_qos
p: input
return value:
    1: qos packet
    0: not qos packet
    -1: Fail
*/
extern int ssv_hal_get_rxpkt_qos(CFG_HOST_RXPKT *p);

/*
ssv_hal_get_rxpkt_f80211
p: input
return value:
    1: f80211 frame
    0: non-f80211 frame
    -1: Fail
*/
extern int ssv_hal_get_rxpkt_f80211(CFG_HOST_RXPKT *p);

extern int ssv_hal_get_rxpkt_wsid(CFG_HOST_RXPKT *p);
extern int ssv_hal_get_rxpkt_seqnum(CFG_HOST_RXPKT *p);
extern int ssv_hal_get_rxpkt_tid(CFG_HOST_RXPKT *p);

/*
ssv_hal_get_rxpkt_psm
p: input
return value:
    1: psm
    0: not psm
    -1: Fail
*/
extern int ssv_hal_get_rxpkt_psm(CFG_HOST_RXPKT *p);

/*
ssv_hal_get_rxpkt_qos_ptr
p: input
return value:
    xxx: qos ptr
    Null: Fail
*/
extern ssv_type_u8 * ssv_hal_get_rxpkt_qos_ptr(CFG_HOST_RXPKT *p);
/*
ssv_hal_get_rxpkt_data_ptr
p: input
return value:
    xxx: data ptr
    Null: Fail
*/
extern ssv_type_u8 * ssv_hal_get_rxpkt_data_ptr(CFG_HOST_RXPKT *p);

/*
ssv_hal_get_rxpkt_data_len
p: input
return value:
*/
extern int ssv_hal_get_rxpkt_data_len(CFG_HOST_RXPKT *p);

extern ssv_type_u8 ssv_hal_get_rxpkt_bssid_idx(CFG_HOST_RXPKT *p);

/*
ssv_hal_download_fw
fw_bin: input
len: input
return value:
    0: Success
    -1l: Fail
*/
extern int ssv_hal_download_fw(ssv_type_u8 *fw_bin, ssv_type_u32 len);

/*
ssv_hal_process_hci_rx_aggr

pdata: input, agggr data start address
len: input, aggr data len
cbk_fh: input, rx mpdu handler
return value: next aggr data len
*/
extern ssv_type_u32 ssv_hal_process_hci_rx_aggr(void* pdata, ssv_type_u32 len, RxPktHdr cbk_fh);

/*
ssv_hal_process_hci_tx_aggr

tFrame: input, tx frame to be aggregated
aggr_buf: input, target buffer for aggregated data
aggr_len: output, Total aggregated data length
return value: aggregated pkt number
*/
extern ssv_type_u32 ssv_hal_process_hci_tx_aggr(void* tFrame, void* aggr_buf, ssv_type_u32* aggr_len);

/*
ssv_hal_hci_aggr_en

trx: input, tx frame to be aggregated
en: input, target buffer for aggregated data
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_hci_aggr_en(HCI_AGGR_HW trx, ssv_type_bool en);

/*
ssv_hal_accept_none_wsid_frame
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_accept_none_wsid_frame(void);

/*
ssv_hal_drop_none_wsid_frame
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_drop_none_wsid_frame(void);

/*
ssv_hal_drop_probe_request
return value:
    0: Success
    -1: Fail
*/

extern int ssv_hal_drop_probe_request(ssv_type_bool IsDrop);
/*
ssv_hal_sta_rcv_all_bcn
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_sta_rcv_all_bcn(void);

/*
ssv_hal_sta_rcv_specific_bcn
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_sta_rcv_specific_bcn(void);

/*
ssv_hal_sta_reject_bcn
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_sta_reject_bcn(void);

/*
ssv_hal_acs_rx_mgmt_flow
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_acs_rx_mgmt_flow(void);

/*
ssv_hal_ap_rx_mgmt_flow
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_ap_rx_mgmt_flow(void);

/*
ssv_hal_sniffer_rx_data_flow
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_sniffer_rx_data_flow(void);

/*
ssv_hal_sta_rx_data_flow
return value:
    0: Success
    -1: Fail
*/
extern int ssv_hal_sta_rx_data_flow(void);

/*
ssv_hal_sta_add
return value:
     0: Success
    -1: Fail
*/
extern int ssv_hal_sta_add(ssv_type_u8 wsid, ssv_type_u8 *mac);

/*
ssv_hal_sta_remove
return value:
     0: Success
    -1: Fail
*/
extern int ssv_hal_sta_remove(ssv_type_u8 wsid);

/*
ssv_hal_ampdu_rx_start
return value:
     0: Success
    -1: Fail
*/
extern int ssv_hal_ampdu_rx_start(ssv_type_u32 rxbuf_size);

extern int ssv_hal_pause_resuem_recovery_int(ssv_type_bool resume);
extern ssv_type_u8 ssv_hal_get_frame_type(SSV_FRM_TYPE tpy);
extern void ssv_hal_ps_hold_on3(ssv_type_u32 value);
extern void ssv_hal_save_clear_trap_reason(void);
extern void ssv_hal_restore_trap_reason(void);
extern void ssv_hal_check_rx_page_empty(void);
extern void ssv_hal_pmu_awake(ssv_type_u32 aid);
extern void ssv_hal_get_temp(ssv_type_s32 *temp);

extern void ssv_set_phy_mode(ssv_type_bool val);

#endif /* _SSV_HAL_H_ */

