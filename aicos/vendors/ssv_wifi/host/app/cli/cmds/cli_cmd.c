/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/



/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0583f0
#define LOG_MODULE    AMOD_VENDOR

#include <ssv_types.h>
#include <msgevt.h>
#include <drv/ssv_drv.h>
#include <log.h>
#include <ssv_lib.h>
#include <host_apis.h>
#include <porting.h>
#include <ssv_devinfo.h>
#include <ssv_hal.h>
#include <ssv_frame.h>
#include <pbuf.h>
#include <wifi.h>
//#include <txrx_task.h>
//#include <sta_mlme.h>


#if (CLI_ENABLE==1)
#include <cli/cli.h>
#include "cli_cmd.h"
#include "cli_cmd_soc.h"
#include "cli_cmd_net.h"
#include "cli_cmd_wifi.h"
#include "cli_cmd_sys.h"
#include <ssv_dev.h>
#if CONFIG_MEMP_DEBUG
#include "lwip/memp.h"
#endif
#include <net_mgr.h>
#if NETAPP_SUPPORT
#include <net_app.h>
#endif
//#include <recover.h>

void show_invalid_para(char * cmd_name)
{
   LOG_PRINTF("%s: invalid parameter \r\n",cmd_name);
}

/*---------------------------------- CMDs -----------------------------------*/
#ifdef __SSV_UNIX_SIM__
static void cmd_abort( ssv_type_s32 argc, char *argv[] )
{
  //  abort(); ==sentton==
}

extern char SysProfBuf[256];
extern signed char SysProfSt;
static void cmd_exit( ssv_type_s32 argc, char *argv[] )
{
    if(SysProfSt == 1)
    {
        ssv_type_s16 i;

        LOG_PRINTF("*************System profiling result*************\n");
        for(i=0; i<256; i++)
            LOG_PRINTF("%c",SysProfBuf[i]);

        LOG_PRINTF("*************End of profiling result*************\n");
    }
    LOG_PRINTF("cmd_exit");
    reset_term();
}
#endif

#if CONFIG_STATUS_CHECK

extern void stats_display(void);
extern void stats_p(void);
extern void stats_m(void);
extern void SSV_PBUF_Status(void);

extern ssv_type_u32 g_l2_tx_packets;
extern ssv_type_u32 g_l2_tx_copy;
extern ssv_type_u32 g_l2_tx_late;
extern ssv_type_u32 g_notpool;

extern ssv_type_u32 g_heap_used;
extern ssv_type_u32 g_heap_max;
extern ssv_type_u32 g_tx_allocate_fail;

#if USE_ICOMM_LWIP

void dump_protocol(void){
#if USE_ICOMM_LWIP
    stats_p();
#endif    
    LOG_PRINTF("\tl2 tx pakets[%d]\r\n\tl2_tx_copy[%d]\r\n\tl2_tx_late[%d]\r\n\tbuf_not_f_pool[%d]\r\n\ttx_allocate_fail[%d]\r\n\t",
                    g_l2_tx_packets, g_l2_tx_copy, g_l2_tx_late, g_notpool, g_tx_allocate_fail);
}



void dump_mem(void){
#if USE_ICOMM_LWIP
    stats_m();
#endif    
#if (CONFIG_USE_LWIP_PBUF == 0)
    SSV_PBUF_Status();
#endif
    LOG_PRINTF("\tg_heap_used[%u] g_heap_max[%u]\r\n", g_heap_used, g_heap_max);

}

#if CONFIG_MEMP_DEBUG
extern void dump_mem_pool(memp_t type);
extern void dump_mem_pool_pbuf(void);
extern void dump_active_tcp_pcb(void);
#endif
void cmd_tcpip_status (ssv_type_s32 argc, char *argv[]){

    if(argc >=2){

        if (ssv6xxx_strcmp(argv[1], "p") == 0) {
            dump_protocol();
            return;
        }
        else if (ssv6xxx_strcmp(argv[1], "m") == 0) {
            dump_mem();
            return;
        }
#if CONFIG_MEMP_DEBUG
        else if(ssv6xxx_strcmp(argv[1], "mp") == 0){

            dump_mem_pool(ssv6xxx_atoi(argv[2]));
            return;
        }
        else if(ssv6xxx_strcmp(argv[1], "mpp") == 0){
            dump_mem_pool_pbuf();

             LOG_PRINTF("\tl2 tx pakets[%d]\r\n\tl2_tx_copy[%d]\r\n\tl2_tx_late[%d]\r\n\tbuf_not_f_pool[%d]\r\n\ttx_allocate_fail[%d]\r\n",
                    g_l2_tx_packets, g_l2_tx_copy, g_l2_tx_late, g_notpool, g_tx_allocate_fail);
            return;
        }
        else if (ssv6xxx_strcmp(argv[1],"pcb") == 0){
            dump_active_tcp_pcb();
            return;
        }

#endif//#if CONFIG_MEMP_DEBUG



    }

    dump_protocol();
    dump_mem();
}
#endif//CONFIG_STATUS_CHECK
#endif

#if 0
static void cmd_recovery(ssv_type_s32 argc, char *argv[])
{
    ssv_type_u8 vif_idx;
    if(netmgr_wifi_check_sta(&vif_idx)){
        LOG_PRINTF("STA recover\r\n");
        ssv6xxx_wifi_sta_recover(vif_idx);
    }else if(netmgr_wifi_check_ap(&vif_idx)){
        LOG_PRINTF("AP recover\r\n");
        ssv6xxx_wifi_ap_recover(vif_idx);
    }else{
        LOG_PRINTF("Wrong mode\r\n");
    }
}
#endif

extern struct Host_cfg g_host_cfg;
extern ssv_type_u32 g_dump_tx;

static int __string2mac(ssv_type_u8 *mac_str, void *val, ssv_type_u32 arg)
{
    int s, macaddr[6];
    u8 *mac=(u8 *)val;

    s = sscanf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", 
        &macaddr[0], &macaddr[1], &macaddr[2],
        &macaddr[3], &macaddr[4], &macaddr[5]);
    if (s != 6)
        return -1;
    mac[0] = (u8)macaddr[0], mac[1] = (u8)macaddr[1];
    mac[2] = (u8)macaddr[2], mac[3] = (u8)macaddr[3];
    mac[4] = (u8)macaddr[4], mac[5] = (u8)macaddr[5];
    return 0;
}

static void cmd_cfg(ssv_type_s32 argc, char *argv[])
{
    
    if (ssv6xxx_strcmp(argv[1], "ts")==0) {
        if (argc >= 3)
        {
            if(0==ssv6xxx_strcmp("1",argv[2])){
                ssv6xxx_wifi_set_tx_task_sleep(1);
            }
            if(0==ssv6xxx_strcmp("0",argv[2])){
                ssv6xxx_wifi_set_tx_task_sleep(0);
            }
            return ;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg ts 1\r\n");
            LOG_PRINTF("     cfg ts 0\r\n");
        }
        
    } else if (ssv6xxx_strcmp(argv[1], "ts_tick")==0) {
        if (argc >= 3)
        {
           ssv6xxx_wifi_set_tx_task_sleep_tick(ssv6xxx_atoi(argv[2]));
           return ;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg ts_tick [tick]\r\n");
        }
        
    } else if (ssv6xxx_strcmp(argv[1], "tx_retry_cnt")==0) {
        if (argc >= 3)
        {
           ssv6xxx_wifi_set_tx_task_retry_cnt(ssv6xxx_atoi(argv[2]));
           return ;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg tx_retry_cnt [cnt]\r\n");
        }
        
    }else if (ssv6xxx_strcmp(argv[1], "erp")==0) {

        if (argc >= 3)
        {
            if(0==ssv6xxx_strcmp("1",argv[2])){
                ssv6xxx_wifi_set_ap_erp(1);
            }
            if(0==ssv6xxx_strcmp("0",argv[2])){
                ssv6xxx_wifi_set_ap_erp(0);
            }
            return ;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg erp 1\r\n");
            LOG_PRINTF("     cfg erp 0\r\n");
        }
    }else if (ssv6xxx_strcmp(argv[1], "resp")==0) {

        if (argc >= 4)
        {
            ssv_type_u8 txp = ssv6xxx_atoi(argv[2]);
            ssv_type_u8 rxp = ssv6xxx_atoi(argv[3]);            

            ssv6xxx_wifi_set_trx_res_page(txp,rxp);

            return ;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg respage txp rxp\r\n");
        }
    }else if (ssv6xxx_strcmp(argv[1], "spmb")==0) {
    
        if (argc >= 3)
        {
            if(0==ssv6xxx_strcmp("1",argv[2])){
                ssv6xxx_wifi_set_ap_short_preamble(1);
            }
            if(0==ssv6xxx_strcmp("0",argv[2])){
                ssv6xxx_wifi_set_ap_short_preamble(0);
            }
    
            return ;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg spmb 0\r\n");
            LOG_PRINTF("     cfg spmb 1\r\n");
        }
    }else if (ssv6xxx_strcmp(argv[1], "rxsrate")==0) {
    
        if (argc >= 4)
        {
            ssv_type_u16 rx_bas_srate = (ssv_type_u16)ssv6xxx_atoi(argv[2]);
            ssv_type_u16 rx_mcs_srate = (ssv_type_u16)ssv6xxx_atoi(argv[3]);
            ssv6xxx_wifi_set_ap_rx_support_rate(rx_bas_srate,rx_mcs_srate);
            return ;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg rxsrate basrate(0xfff) mcsrate(0xfff)\r\n");
        }
    
    }else if (ssv6xxx_strcmp(argv[1], "volt")==0) {
            
        if (argc >= 3)
        {
            ssv_type_u32 volt_mode = ssv6xxx_atoi(argv[2]);
            ssv6xxx_set_voltage_mode(volt_mode);
            return ;
        }
        else
        {
            LOG_PRINTF("Usage:0=LDO, 1=DCDC\r\n");
            LOG_PRINTF("     cfg volt 0/1\r\n");
        }
    }else if(ssv6xxx_strcmp(argv[1], "5G")==0){
        if (argc >= 3)
        {
            ssv_type_bool rf_5g_band = ssv6xxx_atoi(argv[2]);
            ssv6xxx_wifi_enable_5g_band(rf_5g_band);
            return ;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg 5G 0\r\n");
            LOG_PRINTF("     cfg 5G 1\r\n");            
        }                
    }else if(ssv6xxx_strcmp(argv[1], "ap_no_dfs")==0){
        if (argc >= 3)
        {
            ssv_type_u8 ap_no_dfs = ssv6xxx_atoi(argv[2]);
            ssv6xxx_wifi_set_ap_no_dfs(ap_no_dfs);
            return;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg ap_no_dfs 0\r\n");
            LOG_PRINTF("     cfg ap_no_dfs 1\r\n");                    
        }
        
    }else if (ssv6xxx_strcmp(argv[1], "no_bcn_to")==0){
        ssv6xxx_wifi_set_sta_no_bcn_timeout(ssv6xxx_atoi(argv[2]));
    }else if (ssv6xxx_strcmp(argv[1], "rc_mask")==0)
    {
        if (argc >= 3)
        {
            ssv_type_u32 rate_mask = (ssv_type_u16)ssv6xxx_atoi(argv[2]);
            ssv6xxx_set_rc_value(RC_RATEMASK, rate_mask);
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg rc_mask [value]\r\n");
        }        
    }
    else if (ssv6xxx_strcmp(argv[1], "ampdu")==0) {
        if (argc >= 4)
        {
            ssv_type_u8 value = (ssv_type_u8)ssv6xxx_atoi(argv[3]);
            if (ssv6xxx_strcmp(argv[3], "tx")==0)
            {
                value = (ssv_type_u8)ssv6xxx_atoi(argv[4]);
                if (ssv6xxx_strcmp(argv[2], "sta")==0)
                {
                    g_host_cfg.sta_ampdu_tx_enable = value;
                    LOG_PRINTF("STA AMPDU tx = %d\r\n", value);
                }
                else
                {
                    g_host_cfg.ap_ampdu_tx_enable = value;
                    LOG_PRINTF("AP AMPDU tx = %d\r\n", value);
                }
            }               
            else if (ssv6xxx_strcmp(argv[3], "rx")==0)
            {
                value = (ssv_type_u8)ssv6xxx_atoi(argv[4]);

                if (ssv6xxx_strcmp(argv[2], "sta")==0)
                {
                    g_host_cfg.sta_ampdu_rx_enable = value;
                    LOG_PRINTF("STA AMPDU rx = %d\r\n", value);
                }
                else
                {
                    g_host_cfg.ap_ampdu_rx_enable = value;
                    LOG_PRINTF("AP AMPDU rx = %d\r\n", value);
                }
            }
            else if (ssv6xxx_strcmp(argv[1], "rxbufsize")==0)
            {
                value = (ssv_type_u8)ssv6xxx_atoi(argv[3]);
                g_host_cfg.ampdu_rx_buf_size = value;
                LOG_PRINTF("AMPDU rxbufsize = %d\r\n", value);
            }

        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg ampdu ap tx [0/1]\r\n");
            LOG_PRINTF("     cfg ampdu ap rx [0/1]\r\n");
            LOG_PRINTF("     cfg ampdu sta tx [0/1]\r\n");
            LOG_PRINTF("     cfg ampdu sta rx [0/1]\r\n");
            LOG_PRINTF("     cfg ampdu rxbufsize [value]\r\n");
        }
    }
    else if (ssv6xxx_strcmp(argv[1], "fwtxops")==0) {
        ssv_type_u32 tx_ops = 0;

        if (argc == 4) {
            if (ssv6xxx_strcmp(argv[2], "wait_num")==0) {
                g_host_cfg.fw_tx_waitnum = ssv6xxx_atoi(argv[3]);
                tx_ops = g_host_cfg.fw_tx_waitnum;
                ssv6xxx_send_host_cmd((void *)&tx_ops, (int)(sizeof(ssv_type_u32)),
                        SSV6XXX_HOST_CMD_TX_OPS, (ssv_type_u32)SSV6XXX_TX_CMD_WAIT_NUM, TRUE);
            } else if (ssv6xxx_strcmp(argv[2], "wait_time")==0) {
                tx_ops = ssv6xxx_atoi(argv[3]);
                ssv6xxx_send_host_cmd((void *)&tx_ops, (int)(sizeof(ssv_type_u32)),
                        SSV6XXX_HOST_CMD_TX_OPS, (ssv_type_u32)SSV6XXX_TX_CMD_WAIT_TIME, TRUE);
            } else if (ssv6xxx_strcmp(argv[2], "chk_point")==0) {
                tx_ops = ssv6xxx_atoi(argv[3]);
                ssv6xxx_send_host_cmd((void *)&tx_ops, (int)(sizeof(ssv_type_u32)),
                        SSV6XXX_HOST_CMD_TX_OPS, (ssv_type_u32)SSV6XXX_TX_CMD_CHECK_HWQ_NUM, TRUE);
            } else if (ssv6xxx_strcmp(argv[2], "wait_time_ht40")==0) {
                tx_ops = ssv6xxx_atoi(argv[3]);
                ssv6xxx_send_host_cmd((void *)&tx_ops, (int)(sizeof(ssv_type_u32)),
                        SSV6XXX_HOST_CMD_TX_OPS, (ssv_type_u32)SSV6XXX_TX_CMD_WAIT_TIME_HT40, TRUE);
            } else if (ssv6xxx_strcmp(argv[2], "chk_hwq_num")==0) {
                g_host_cfg.fw_tx_chkhwqnum = ssv6xxx_atoi(argv[3]);
                tx_ops = g_host_cfg.fw_tx_chkhwqnum;
                ssv6xxx_send_host_cmd((void *)&tx_ops, (int)(sizeof(ssv_type_u32)),
                        SSV6XXX_HOST_CMD_TX_OPS, (ssv_type_u32)SSV6XXX_TX_CMD_CHECK_HWQ_NUM, TRUE);
            } else {
                LOG_PRINTF("fwtxops wait_num <num>");
                LOG_PRINTF("fwtxops wait_time <time_in_us>");
                LOG_PRINTF("fwtxops wait_time_ht40 <time_in_us>");
                LOG_PRINTF("fwtxops chk_point <val>");
                LOG_PRINTF("fwtxops chk_hwq_num <num>");
            }
        } else {
            LOG_PRINTF("fwtxops [wait_num|wait_time|wait_time_ht40|chk_point|chk_hwq_num]");
        }
    }
    else if (ssv6xxx_strcmp(argv[1], "dump") == 0){
        if(argc!=4){
            if(ssv6xxx_strcmp(argv[2], "tx")==0){
                if (ssv6xxx_strcmp(argv[3], "on") == 0){
                    g_dump_tx = 1;
                    LOG_PRINTF("tx on\r\n");
                }else{
                    g_dump_tx = 0;
                    LOG_PRINTF("tx off\r\n");
                }
            }
        }
        else{
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg dump tx [on/off]\r\n");
        }
    }
    else if (ssv6xxx_strcmp(argv[1], "force_rf")==0){
        if (argc >= 3)
        {
            g_host_cfg.force_custom_rftable = ssv6xxx_atoi(argv[2]);
            LOG_PRINTF("force_custom_rftable %d\r\n", g_host_cfg.force_custom_rftable);
        }
    }
    else if (ssv6xxx_strcmp(argv[1], "acl")==0){
        if (argc >= 3)
        {
            if (ssv6xxx_strcmp(argv[2], "clear")==0)
            {
                ssv6xxx_wifi_clear_ap_acl();
                LOG_PRINTF("ssv6xxx_wifi_clear_ap_acl\r\n");
            }
            
            if (ssv6xxx_strcmp(argv[2], "show")==0)
            {
                AP_SETTING_ACL_TYPE ap_acl_mode;
                AP_SETTING_ACL_MAC mac_list[AP_ACL_MAX_COUNT];
                ssv_type_u16 ap_acl_mac_count;
                int i = 0;
                ssv6xxx_wifi_get_ap_acl(&ap_acl_mode, &mac_list[0], &ap_acl_mac_count);
                LOG_PRINTF("ACL MODE: %d\r\n", ap_acl_mode);
                LOG_PRINTF("ACL MAC COUNT: %d\r\n", ap_acl_mac_count);
                for (i = 0;i < AP_ACL_MAX_COUNT; i++)
                {
                    LOG_PRINTF("MAC[%02d]: %02x:%02x:%02x:%02x:%02x:%02x\r\n", i, mac_list[i].mac[0], mac_list[i].mac[1], mac_list[i].mac[2], 
                        mac_list[i].mac[3], mac_list[i].mac[4], mac_list[i].mac[5]);
                }
            }
        }
        
        if (argc >= 4)
        {
            if (ssv6xxx_strcmp(argv[2], "type")==0)
            {
                ssv6xxx_wifi_set_ap_acl_type(ssv6xxx_atoi(argv[3]));
                LOG_PRINTF("ssv6xxx_wifi_set_ap_acl_type %d\r\n", ssv6xxx_atoi(argv[3]));
            }
            
            if (ssv6xxx_strcmp(argv[2], "add")==0)
            {
                ssv_type_u8           mac[ETHER_ADDR_LEN]; 
                //int i = 0;
                __string2mac(argv[3],  mac, 0);
                ssv6xxx_wifi_add_ap_acl(mac);
                LOG_PRINTF("ssv6xxx_wifi_add_ap_acl: %02x:%02x:%02x:%02x:%02x:%02x\r\n", mac[0], mac[1], mac[2], 
                    mac[3], mac[4], mac[5]);
            }
            if (ssv6xxx_strcmp(argv[2], "del")==0)
            {
                ssv_type_u8           mac[ETHER_ADDR_LEN]; 
                //int i = 0;
                __string2mac(argv[3],  mac, 0);
                ssv6xxx_wifi_del_ap_acl(mac);
                LOG_PRINTF("ssv6xxx_wifi_del_ap_acl: %02x:%02x:%02x:%02x:%02x:%02x\r\n", mac[0], mac[1], mac[2], 
                    mac[3], mac[4], mac[5]);
            }
        }
        
    }
    else if (ssv6xxx_strcmp(argv[1], "show")==0) {
            LOG_PRINTF("TX rate mask=0x%x\r\n",g_host_cfg.rate_mask);
            LOG_PRINTF("TX task sleep = %d\r\n",g_host_cfg.tx_sleep);
            LOG_PRINTF("TX task sleep tick = %d\r\n",g_host_cfg.tx_sleep_tick);            
            LOG_PRINTF("TX task retry cnt = %d\r\n",g_host_cfg.tx_retry_cnt);
            LOG_PRINTF("AP erp = %d\r\n",g_host_cfg.erp);
            LOG_PRINTF("AP b mode short preamble = %d\r\n",g_host_cfg.b_short_preamble);
            LOG_PRINTF("Resource page T=%d, R=%d\r\n",g_host_cfg.tx_res_page,g_host_cfg.rx_res_page);            
            LOG_PRINTF("Host pbuf num = %d\r\n",g_host_cfg.pool_size);
            LOG_PRINTF("Support ht = %d\r\n",g_host_cfg.support_ht);
            LOG_PRINTF("Support RF band = %s\r\n",(ssv6xxx_wifi_support_5g_band()?"5G":"2G"));
            LOG_PRINTF("AP no DFS = %d\r\n",g_host_cfg.ap_no_dfs); 
            LOG_PRINTF("STA no bcn timeout = %d(ms)\r\n",g_host_cfg.sta_no_bcn_timeout*500);            
            LOG_PRINTF("ap  mode amdpu tx = %d\r\n",g_host_cfg.ap_ampdu_tx_enable);        
            LOG_PRINTF("ap  mode amdpu rx = %d\r\n",g_host_cfg.ap_ampdu_rx_enable);        
            LOG_PRINTF("sta mode amdpu tx = %d\r\n",g_host_cfg.sta_ampdu_tx_enable);        
            LOG_PRINTF("sta mode amdpu rx = %d\r\n",g_host_cfg.sta_ampdu_rx_enable);        
            LOG_PRINTF("fw_tx_chkhwqnum = %d\r\n",g_host_cfg.fw_tx_chkhwqnum);        
            LOG_PRINTF("fw_tx_waitnum = %d\r\n",g_host_cfg.fw_tx_waitnum);        
            LOG_PRINTF("dump tx = %d\r\n",g_dump_tx);
            LOG_PRINTF("force_custom_rftable = %d\r\n",g_host_cfg.force_custom_rftable); 
    }
    return ;
}

static void cmd_read_hw_queue(ssv_type_s32 argc, char *argv[])
{
    ssv_hal_read_hw_queue();
    return;
}
#if 1
extern int ssv6xxx_dev_init(ssv6xxx_hw_mode hmode);
extern void ssv6xxx_dev_deinit(void);
static void cmd_deinit(ssv_type_s32 argc, char *argv[])
{
    ssv_type_u16 mode = 0;
    mode=ssv6xxx_atoi(argv[1]);
    LOG_PRINTF("\33[32m  ssv_hal start to deinit driver \33[0m\r\n");
    ssv6xxx_dev_deinit();
    LOG_PRINTF("\33[32m  \n Finish.. \33[0m\r\n");
    return;
}
static void cmd_init(ssv_type_s32 argc, char *argv[])
{
	ssv_type_u16 mode = 0;
    mode=ssv6xxx_atoi(argv[1]);
    LOG_PRINTF("\33[32m ssv_hal switch to mode(%d) \33[0m\r\n",mode);
    ssv6xxx_dev_init(mode);
    return;
}
static void cmd_ldo_en_low(ssv_type_s32 argc, char *argv[])
{
	LOG_PRINTF("\33[32m  ssv_hal cmd_ldo_en_low \33[0m\r\n");
	platform_ldo_en_pin_init();
	platform_ldo_en(0);
}
static void cmd_ldo_en_high(ssv_type_s32 argc, char *argv[])
{
	LOG_PRINTF("\33[32m  ssv_hal cmd_ldo_en_high \33[0m\r\n");
	platform_ldo_en_pin_init();
	platform_ldo_en(1);
}
static void cmd_ssv_init(ssv_type_s32 argc, char *argv[])
{
	ssv_type_u16 mode = 0;
    mode=ssv6xxx_atoi(argv[1]);
    LOG_PRINTF("\33[32m ssv_hal start to cmd_ssv_init \33[0m\r\n");
    //ssv6xxx_dev_init(mode);
	ssv_wifi_init();
    return;
}
static void cmd_ssv_deinit(ssv_type_s32 argc, char *argv[])
{
	ssv_type_u16 mode = 0;
    mode=ssv6xxx_atoi(argv[1]);
    LOG_PRINTF("\33[32m  ssv_hal start to cmd_ssv_deinit \33[0m\r\n");
    //ssv6xxx_dev_deinit();
    ssv_wifi_deinit();
    LOG_PRINTF("\33[32m  \n Finish.. \33[0m\r\n");
    return;

}

static void cmd_ssv_sleep(ssv_type_s32 argc, char *argv[])
{
	ssv_type_u16 mode = 0;
    mode=ssv6xxx_atoi(argv[1]);
    LOG_PRINTF("\33[32m  ssv_hal cmd_ssv_sleep mode:(%d) \33[0m\r\n",mode);
    ssv_wifi_sleep(mode);
    LOG_PRINTF("\33[32m  \n Finish.. \33[0m\r\n");
    return;

}

static void cmd_ssv_connect(ssv_type_s32 argc, char *argv[])
{
	ssv_type_u16 mode = 0;
    mode=ssv6xxx_atoi(argv[1]);
    LOG_PRINTF("\33[32m  ssv_hal cmd_ssv_connect mode:(%d) \33[0m\r\n",mode);
    ssv_wifi_start_station_ext(argv[1],argv[2],11,0);
    LOG_PRINTF("\33[32m  \n Finish.. \33[0m\r\n");
    return;

}
static void cmd_ssv_connect2(ssv_type_s32 argc, char *argv[])
{
	ssv_type_u16 mode = 0;
    mode=ssv6xxx_atoi(argv[1]);
    LOG_PRINTF("\33[32m  ssv_hal cmd_ssv_conect2 mode:(%d) \33[0m\r\n",mode);
    ssv_wifi_station_connect_2(argv[1],argv[2]);
    LOG_PRINTF("\33[32m  \n Finish.. \33[0m\r\n");
    return;

}
static void cmd_ssv_connect_no_scan(ssv_type_s32 argc, char *argv[])
{
	ssv_type_u16 mode = 0;
    mode=ssv6xxx_atoi(argv[1]);
    LOG_PRINTF("\33[32m  ssv_hal ssv_wifi_station_connect_no_scan mode:(%d) \33[0m\r\n",mode);
    ssv_wifi_station_connect_no_scan(argv[1],argv[2]);
    LOG_PRINTF("\33[32m  \n Finish.. \33[0m\r\n");
    return;

}
static void cmd_ssv_taskcount(ssv_type_s32 argc, char *argv[])
{
	ssv_type_u16 mode = 0;
    mode=ssv6xxx_atoi(argv[1]);
    LOG_PRINTF("\33[32m  cmd_ssv_taskcount  \33[0m\r\n");
    ssv_wifi_taskcount();
    LOG_PRINTF("\33[32m  \n Finish.. \33[0m\r\n");
    return;

}
static void cmd_ssv_scan(ssv_type_s32 argc, char *argv[])
{
	//ssv_type_u16 mode = 0;
    //mode=ssv6xxx_atoi(argv[1]);
    LOG_PRINTF("\33[32m  cmd_ssv_scan  \33[0m\r\n");
    //ssv_wifi_scan_ssid(SCAN_ALL_2G_CHANNEL, SCAN_ALL_5G_CHANNEL, argv[1]);
    ssv_wifi_scan_ssid_atcmd(argv[1]);
    LOG_PRINTF("\33[32m  \n Finish.. \33[0m\r\n");
    return;

}
/*
static void cmd_ssv_setmode(ssv_type_s32 argc, char *argv[])
{

    LOG_PRINTF("\33[32m  cmd_ssv_setmode  \33[0m\r\n");
    ssv_wifi_vif_set();
    LOG_PRINTF("\33[32m  \n Finish.. \33[0m\r\n");
    return;

}

static void cmd_ssv_dhcpc(ssv_type_s32 argc, char *argv[])
{
	LOG_PRINTF("\33[32m  cmd_ssv_setmode  \33[0m\r\n");
	wifi_adap_report_dhcpc_start_atcmd();
	LOG_PRINTF("\33[32m  \n Finish.. \33[0m\r\n");
	return;
}
*/
#endif
#define MAX_FRM_SIZE 2304
static void cmd_rxtput(ssv_type_s32 argc, char *argv[])
{
    struct sdio_rxtput_cfg   cmd_rxtput_cfg;
    
    if (3 == argc) {
        cmd_rxtput_cfg.size_per_frame = ssv6xxx_atoi(argv[1]);    
        cmd_rxtput_cfg.total_frames = ssv6xxx_atoi(argv[2]); 
        
        if (cmd_rxtput_cfg.size_per_frame > MAX_FRM_SIZE) {
            LOG_PRINTF("Frame size too large!!\r\n");
            return; 
        }

        ssv6xxx_send_host_cmd((void *)&cmd_rxtput_cfg, (int)(sizeof(struct sdio_rxtput_cfg)),
                        SSV6XXX_HOST_CMD_RX_TPUT, 0, TRUE);
    } else {
        LOG_PRINTF("rxtput [size] [frames]\r\n");
    }
}

extern ssv_type_s32 TXRXTask_FrameEnqueue(void* frame, ssv_type_u32 wsid);
static void cmd_txtput(ssv_type_s32 argc, char *argv[])
{
    ssv_type_u8* frame = NULL;
    CFG_HOST_TXREQ* frame_data = NULL;
    int size = 0, frames = 0;
    //int cnt=0;
    if (3 == argc) {
        size = ssv6xxx_atoi(argv[1]);    
        if(size>1500)
        {
            goto END;
        }
        frames = ssv6xxx_atoi(argv[2]); 
        do {
            frame = os_frame_alloc(size,FALSE);
            if(frame)
            {
                frame_data = (CFG_HOST_TXREQ *)OS_FRAME_GET_DATA(frame);
                ssv_hal_set_txreq0_txtput_desc(frame_data, (int)OS_FRAME_GET_DATA_LEN(frame));
                TXRXTask_FrameEnqueue(frame,MAX_STA_NUM);
                frames--;  
#if 0
                cnt++;
                if(cnt==24)
                {
                    SSV_OS_TickDelay(1);
                    cnt=0;
                }          
#endif
                //LOG_PRINTF("done\r\n");
            }
            else
            {
                SSV_OS_TickDelay(1);
            }
        } while(frames != 0);
    } 
    
    LOG_PRINTF("done\r\n");
    return;
END:    
    LOG_PRINTF("txtput [size] [frames]\r\n");
    return;
    
    //os_frame_free(frame);
}

extern ssv_type_u32 loopback_total_count;
extern ssv_type_u32 loopback_recv_count;
static void cmd_loopback(ssv_type_s32 argc, char *argv[])
{
    ssv_type_u8* frame = NULL;
    ssv_type_u8* data = NULL;
    struct cfg_host_cmd *host_cmd = NULL;
    int size = 0, frames = 0;
    int i = 0;

    if (3 == argc) {
        size = ssv6xxx_atoi(argv[1]);    
        frames = ssv6xxx_atoi(argv[2]); 
        
        if (size > MAX_FRM_SIZE) {
            LOG_PRINTF("Frame size too large!!\r\n");
            return; 
        }
        
        if (size < 256) {
            LOG_PRINTF("Frame size too small!!\r\n");
            return; 
        }
        
        
        loopback_total_count = frames;
        loopback_recv_count = 0;
        
        do {
            frame = os_frame_alloc(size, FALSE);
            if (frame)
            {
                host_cmd = (struct cfg_host_cmd *)OS_FRAME_GET_DATA(frame);
                // fill host cmd header and data
                OS_MemSET((void *)host_cmd, 0x0, (int)OS_FRAME_GET_DATA_LEN(frame));
                host_cmd->c_type = HOST_CMD;
                host_cmd->len = (int)OS_FRAME_GET_DATA_LEN(frame);
                host_cmd->cmd_seq_no = frames % 0xFF;
                host_cmd->h_cmd = SSV6XXX_HOST_CMD_LOOPBACK;
                //LOG_PRINTF("host_cmd->cmd_seq_no %d, frames %d\r\n", host_cmd->cmd_seq_no, frames);
                data = (ssv_type_u8 *)(((ssv_type_u8 *)host_cmd) + HOST_CMD_HDR_LEN);
                for (i = 0; i < (int)((int)OS_FRAME_GET_DATA_LEN(frame) - HOST_CMD_HDR_LEN); i++) {
                    data[i] = host_cmd->cmd_seq_no;
                    //LOG_PRINTF("h,data[%d]=0x%02x\r\n", i, data[i]);
                }
            #ifndef __SSV_UNIX_SIM__
                ssv6xxx_drv_wait_tx_resource(OS_FRAME_GET_DATA_LEN(frame));
            #endif
                if(FALSE==TXRXTask_FrameEnqueue(frame, MAX_STA_NUM))
                {                 
                    os_frame_free(frame);
                }
                frames--;            
            }
            else
            {
                SSV_OS_TickDelay(1);
            }
        } while(frames != 0);
    } else {
        LOG_PRINTF("loopback [size] [frames]\r\n");
    }
}

static void cmd_powermode(ssv_type_s32 argc, char *argv[])
{
    int powermode = 0;
    ssv_type_u32 ipaddr = 0, gw = 0, netmask = 0;

    if (2 == argc) {
        powermode = ssv6xxx_atoi(argv[1]);    
        if (!netmgr_wifi_check_sta()) {
            LOG_PRINTF("AP mode cannot support lowpower\r\n");
            return;
        }
        
        switch (powermode) {
            case 0:
                netmgr_wifi_resume(); 
                break;
            case 2:
            case 3:
            case 4:
                netdev_getipv4info(WLAN_IFNAME, &ipaddr, &gw, &netmask);
                LOG_PRINTF("STA powermode %d, ipaddr 0x%08x\r\n", powermode, ipaddr);
                netmgr_wifi_suspend(ipaddr, powermode); 
                break;
            default:
                LOG_PRINTF("unknown powermode %d\r\n", powermode);
                break;
        }
    } else {
        LOG_PRINTF("powermode [val]\r\n");
    }
}

static void cmd_gettemp(ssv_type_s32 argc, char *argv[])
{
    ssv_type_s32 temp = 0;
    ssv_hal_get_temp(&temp);
    LOG_PRINTF("current temp: %d \r\n", temp);
}

static void cmd_test_wifi(ssv_type_s32 argc, char *argv[])
{
    if (argc < 2)
    {
        return;
    }
    
    if ((ssv6xxx_strcmp(argv[1], "ap")==0)&& (argc == 3))
    {
        u32 sta_count = 0;
        sta_count = ssv6xxx_atoi(argv[2]);    
        ssv_wifi_ap_create(argv[1],1, 0, "", 1, 1, sta_count);
    }
    else if ((ssv6xxx_strcmp(argv[1], "ps")==0) && (argc == 3))
    {
        u32 ps = 0;
        void ssv_wifi_enter_ps();
        void ssv_wifi_exit_ps();
        ps = ssv6xxx_atoi(argv[2]);
        if (ps)
        {
            ssv_wifi_enter_ps();
        }
        else
        {
            ssv_wifi_exit_ps();
        }
    }
    else if ((ssv6xxx_strcmp(argv[1], "deauth")==0) && (argc == 2))
    {
       send_deauth_and_remove_all();
    }
}

CLICmds gCliCmdTable[] =
{
    { "ifconfig",       ssv_cmd_ifconfig,       "Network interface configuration"   },
#if(PING_SUPPORT==1)
    { "ping",           cmd_ping,           "ping"                              },
#endif
#ifdef __APP_TTCP__
    { "ttcp",           cmd_ttcp,           "ttcp"                              },
#endif
#if (IPERF3_ENABLE == 1)
      { "iperf3",       cmd_iperf3,         "throughput testing via tcp or udp"},
#endif
#if (NETAPP_SUPPORT == 1)
    { "netapp",         cmd_net_app,        "Network application utilities"     },
#endif        
    { "netmgr",         cmd_net_mgr,        "Network Management utilities"     },
    { "iw",             cmd_iw,             "Wireless utility"                  },
    { "ctl",            cmd_ctl,            "wi-fi interface control (AP/station on or off)"       },
    { "r",              cmd_read,           "Read SoC"                          },
    { "w",              cmd_write,          "Write SoC"                         },
    { "regr",           cmd_regr,           "Read SoC(IOT style)"               },
    { "regw",           cmd_regw,           "Write SoC(IOT style)"              },
#if USE_ICOMM_LWIP
#if CONFIG_STATUS_CHECK
    { "tcpip",          cmd_tcpip_status,   "dump tcp/ip status"                },
#endif
#endif
    { "sys",            cmd_sys,            "Component info"       },
#if 0//((defined(SDRV_INCLUDE_SDIO)) && (SDRV_INCLUDE_SDIO))
    {"sdio", cmd_sdio_tune,"tune sdio parameter"},
#endif
#if 0
    {"recover",cmd_recovery,"Do sta recover"},
#endif
    {"mib",             cmd_mib,            "Show HW MIB counter"},
    {"cfg",             cmd_cfg,            "change cfg setting"},
    {"q",               cmd_read_hw_queue,  "read hw queue"},
#if 1
    {"deinit",cmd_deinit,"set set_mcc_slice"},
    {"init",cmd_init,	"set set_mcc_slice_init"},
    {"ldo_en_low",cmd_ldo_en_low,"ldo_en_low"},
    {"ldo_en_high",cmd_ldo_en_high,"ldo_en_high"},
    {"ssv_deinit",cmd_ssv_deinit,"ssv deinit"},
    {"ssv_init",cmd_ssv_init,"ssv init"},
    {"ssv_sleep",cmd_ssv_sleep,"ssv sdio sleep"},
    {"ssv_connect",cmd_ssv_connect,"ssv wifi connect"},
    {"ssv_connect2",cmd_ssv_connect2,"ssv wifi connect2"},
    {"ssv_connect_no_scan",cmd_ssv_connect_no_scan,"ssv wifi connect_no_scan"},
    {"ssv_task",cmd_ssv_taskcount,"ssv wifi cmd_ssv_taskcount"},
    {"ssv_scan",cmd_ssv_scan,"ssv wifi ssv_scan"},
    {"ssv_adapt",cmd_ssv_adapt,"ssv wifi adapt test"},   
    {"ssv_set_adapt",cmd_ssv_set_adapt,"ssv wifi set adapt test"},
    //{"ssv_setmode",cmd_ssv_setmode,"ssv wifi ssv_scan"},
    //{"ssv_dhcpc",cmd_ssv_dhcpc,"ssv cmd_ssv_dhcpc "},
#endif
    {"rxtput",          cmd_rxtput,         "test rx sdio throughput"},
    {"txtput",          cmd_txtput,         "test tx sdio throughput"},
    {"loopback",        cmd_loopback,       "test loopback"},
    {"powermode",       cmd_powermode,      "power saving command"},
    {"gettemp",          cmd_gettemp,      "get temp command"},
#if (ENABLE_SMART_CONFIG == 1)
    {"sconfig",         cmd_sconfig,        "Smart config."},
#endif
    {"rf",              cmd_rf,             "rf tool"},
    {"efuse",           cmd_efuse,       "efuse tool"},
#if(ENABLE_WPS ==1)
    {"wps",             cmd_wps,             "wps"},
#endif
    {"test_wifi",             cmd_test_wifi,             "test_wifi"},
    { NULL, NULL, NULL },
};
#endif
