/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x173710
#define LOG_MODULE    AMOD_VENDOR

#include <porting.h>
#include <host_apis.h>
#include <log.h>

#if CLI_ENABLE
#include <cli/cli.h>
#include <cli/cmds/cli_cmd_wifi.h>
#endif
#if NETMGR_SUPPORT
#include <net_mgr.h>
#endif
#include <netstack.h>
#include <ssv_devinfo.h>
#include <ssv_drv.h>
#include <ssv_lib.h>
#include <pbuf.h>
#if NETAPP_SUPPORT
#include <net_app.h>
#endif
#if SSV_LOG_DEBUG
ssv_type_u32 g_log_module;
ssv_type_u32 g_log_min_level;
#endif
#include <rtos.h>
//Configurations
// ----------------------------------------------------
//Mac address
ssv_type_u8 config_mac[] ={0x60,0x11,0x22,0x33,0x44,0x77};

//Max number of AP list
ssv_type_u8 g_max_num_of_ap_list=NUM_AP_INFO;

//Auto channel selection in AP mode
ssv_type_u16 g_acs_channel_mask=ACS_CHANNEL_MASK;
ssv_type_u32 g_acs_5g_channel_mask=ACS_5G_CHANNEL_MASK;
ssv_type_u16 g_acs_channel_scanning_interval=ACS_SCANNING_INTERVAL;
ssv_type_u8  g_acs_channel_scanning_loop=ACS_SCANNING_LOOP;

ssv_type_u32 g_sta_timeout_retry_count = STA_TIMEOUT_RETRY_COUNT;

// ------------------------- rate control ---------------------------
struct Host_cfg g_host_cfg;
// ------------- User mode SmartConfig ...............................................
// ----------------------------------------------------
ssv_type_u8 g_lwip_tcp_ignore_cwnd=LWIP_TCP_IGNORE_CWND;

#ifdef NET_MGR_AUTO_RETRY
int  g_auto_retry_times_delay = NET_MGR_AUTO_RETRY_DELAY;
int  g_auto_retry_times_max = NET_MGR_AUTO_RETRY_TIMES;
#endif

AP_SETTING_ACL_TYPE g_ap_acl_mode = AP_ACL_DISABLE;
ssv_type_u16 g_ap_acl_mac_count = 0;
AP_SETTING_ACL_MAC g_ap_acl_mac_list[AP_ACL_MAX_COUNT];

#if DHCPD_SUPPORT
extern int udhcpd_init(void);
#endif

void stop_and_halt (void)
{
    //while (1) {}
    /*lint -restore */
} // end of - stop_and_halt -

//=====================Task parameter setting========================
extern struct task_info_st g_txrx_task_info[];
extern struct task_info_st g_host_task_info[];
extern struct task_info_st g_timer_task_info[];
#if NETMGR_SUPPORT
extern struct task_info_st st_netmgr_task[];
#endif
#if DHCPD_SUPPORT
extern struct task_info_st st_dhcpd_task[];
#endif
void ssv6xxx_init_task_para(void)
{
    g_txrx_task_info[0].prio = OS_TX_TASK_PRIO;
    g_txrx_task_info[1].prio = OS_RX_TASK_PRIO;
    g_host_task_info[0].prio = OS_CMD_ENG_PRIO;
    g_timer_task_info[0].prio = OS_TMR_TASK_PRIO;
#if NETMGR_SUPPORT
    st_netmgr_task[0].prio = OS_NETMGR_TASK_PRIO;
#endif
#if DHCPD_SUPPORT    
    st_dhcpd_task[0].prio = OS_DHCPD_TASK_PRIO;

#endif
}
//=============================================================

int ssv6xxx_start(ssv_vif* vif)
{
    //u32 wifi_mode;
    ssv6xxx_drv_irq_disable(false);
    SSV_OS_TickDelay(1);
    ssv6xxx_drv_irq_enable(false);
    ssv6xxx_drv_start();
    
    ssv6xxx_send_host_cmd_ignore(TRUE);
    // disable mcu
    ssv_hal_mcu_disable();
    /* Reset MAC & Re-Init */

    /* Initialize ssv6200 mac */
    LOG_PRINTF("##sentton entry ssv6xxx_init_mac \r\n");
    if(-1==ssv6xxx_init_mac(vif))
    {
         LOG_PRINTF("##sentton## ssv6xxx_init_mac fail\n");
    	return SSV6XXX_FAILED;
    }
    
    LOG_PRINTF("##sentton ssv6xxx_start success\r\n");

    return SSV6XXX_SUCCESS;
}

ssv6xxx_result ssv6xxx_check_ready(ssv6xxx_hw_mode hmode)
{
    ssv_type_u32 start_tick = 0;
    ssv_type_u32 timeout = 0;
    ssv_type_u32 total_5g_channel = 0;
    ssv_type_u32 total_2g_channel = 0;
    ssv_type_u8  i = 0;
    Ap_sta_status *info = NULL;
    ssv_type_bool bRet = false;

    info = (Ap_sta_status *)OS_MemAlloc(sizeof(Ap_sta_status));
    if (info == NULL)
    {
        return SSV6XXX_FAILED;
    }

    OS_MemSET((void *)info, 0, sizeof(Ap_sta_status));

    if(hmode == SSV6XXX_HWM_AP)
    {
        for(i=0;i<MAX_2G_CHANNEL_NUM;i++){
            if(g_acs_channel_mask&(0x01<<i)){
                total_2g_channel++;
            }
        }

        if(TRUE==ssv6xxx_wifi_support_5g_band())
        {
            for(i=0;i<MAX_5G_CHANNEL_NUM;i++){
                if(g_acs_5g_channel_mask&(0x01<<i)){
                    total_5g_channel++;
                }
            }
        }
        timeout = g_acs_channel_scanning_interval*10*(total_2g_channel+total_5g_channel)+100;
    }
    else
    {
        timeout = 3000;
    }

    start_tick = OS_GetSysTick();

    while(1)
    {
        ssv6xxx_wifi_status(info);
        switch(hmode)
        {
            case SSV6XXX_HWM_STA:
            {
                for(i=0;i<MAX_VIF_NUM;i++)
                {
                    if ((info->vif_operate[i]== SSV6XXX_HWM_STA) && (info->status))
                    {
                        bRet = true;
                        break;
                    }
                }            
                break;
            }
            case SSV6XXX_HWM_AP:
            {
                for(i=0;i<MAX_VIF_NUM;i++)
                {
                    if ((info->vif_operate[i] == SSV6XXX_HWM_AP) && (info->status))
                    {
                        if(info->vif_u[i].ap_status.current_st == AP_STATE_READY)
                        {
                            bRet = true;
                            break;
                        }
                    }
                }            
                break;
            }
            case SSV6XXX_HWM_SNIFFER:
            {
                for(i=0;i<MAX_VIF_NUM;i++)
                {
                    if ((info->vif_operate[i] == SSV6XXX_HWM_SNIFFER) && (info->status))
                    {
                         bRet = true;
                         //LOG_PRINTF("netmgr check sniffer vif=%d\r\n",i);
                         break; //for
                    }
                }
                break;//switch
            }
            default:
                break;
        }

        if((bRet == false)&&(((OS_GetSysTick() - start_tick) * OS_TICK_RATE_MS) < timeout))
        {
            SSV_OS_TickDelay(1);
        }
        else
        {
            break; //while
        }
    }

    OS_MemFree(info);

    if (!bRet)
    {
        return SSV6XXX_FAILED;
    }

    return SSV6XXX_SUCCESS;
}

ssv6xxx_result ssv6xxx_check_stop(ssv6xxx_hw_mode hmode)
{
    ssv_type_u32 start_tick = 0;
    ssv_type_u32 timeout = 0;
    Ap_sta_status *info = NULL;
    ssv_type_bool bRet = false;

    info = (Ap_sta_status *)OS_MemAlloc(sizeof(Ap_sta_status));
    if (info == NULL)
    {
        return SSV6XXX_FAILED;
    }

    OS_MemSET((void *)info, 0, sizeof(Ap_sta_status));

    timeout = 3000;

    start_tick = OS_GetSysTick();

    while(1)
    {
        bRet = false;
        ssv6xxx_wifi_status(info);
        switch(hmode)
        {
            case SSV6XXX_HWM_STA:
            {
                if ((info->vif_operate[0]== SSV6XXX_HWM_STA) && (info->status))
                {
                    bRet = true;
                }
                break;
            }
            case SSV6XXX_HWM_AP:
            {
                if ((info->vif_operate[1] == SSV6XXX_HWM_AP) && (info->status))
                {
                    if(info->vif_u[1].ap_status.current_st == AP_STATE_READY)
                    {
                        bRet = true;
                    }
                }
                break;
            }
            case SSV6XXX_HWM_SNIFFER:
            {
                if ((info->vif_operate[0] == SSV6XXX_HWM_SNIFFER) && (info->status))
                {
                     bRet = true;
                }
                break;//switch
            }
            default:
                break;
        }
        
        if((bRet == true)&&(((OS_GetSysTick() - start_tick) * OS_TICK_RATE_MS) < timeout))
        {
            SSV_OS_TickDelay(1);
        }
        else
        {
            break; //while
        }
    }
    OS_MemFree(info);

    if (bRet)
    {
        return SSV6XXX_FAILED;
    }

    return SSV6XXX_SUCCESS;
}

#if 0
#define JOIN_DEFAULT_SSID    "ap-Ian-WPA2" //"china"
#define JOIN_DEFAULT_PSK     "12345678" //"12345678"
ssv_type_u8 join_default_pmk[32]={0xB6, 0x0B, 0x01, 0xDC, 0x84, 0x09, 0xFA, 0x41, 0x55, 0xD3, 0x59, 0x81, 0xDA, 0xAC, 0xCE, 0x6E,
                         0x3F, 0x78, 0x32, 0x7B, 0x74, 0xDC, 0xC3, 0xB3, 0x4C, 0x66, 0xE6, 0xC7, 0x88, 0x83, 0x59, 0xE0};
#define JOIN_DEFAULT_2P4G_CH_MASK    0x02
#define JOIN_DEFAULT_5G_CH_MASK    0x0
#endif

#if 1
#define JOIN_DEFAULT_SSID    "CMW-AP" //"china"
#define JOIN_DEFAULT_PSK     "" //"12345678"
ssv_type_u8 join_default_pmk[32]={0x85, 0x87, 0x6B, 0xDE, 0xC7, 0x8E, 0x20, 0x41, 0xA7, 0xE4, 0xF6, 0x58, 0xDF, 0xD4, 0xC3, 0xCB,
                         0x2E, 0xA1, 0x70, 0xB4, 0xA1, 0x3F, 0x6C, 0x0A, 0xEA, 0x63, 0x1A, 0x54, 0x6E, 0xCE, 0x6A, 0x61};
#define JOIN_DEFAULT_2P4G_CH_MASK    0x02
#define JOIN_DEFAULT_5G_CH_MASK    0x0

#endif
//#define JOIN_DEFAULT_SSID    "Default_ap" //"china"
//#define JOIN_DEFAULT_PSK     "12345678" //"12345678"

#define AP_DEFAULT_SSID    "ssv_rtos_ap"
#define AP_DEFAULT_PSK     "12345678"
extern void ssv_netmgr_init_netdev(ssv_type_bool default_cfg);
extern void ssv6xxx_wifi_station_async(void* pcfg);
extern int ssv6xxx_wifi_init_regdom(void);
ssv6xxx_result STAmode_default(ssv_type_bool bJoin, ssv_type_bool static_ip, ssv_type_bool dhcp_server)
{
    int ret;
    ssv6xxx_hw_mode mode;
    Sta_setting sta;

    mode = SSV6XXX_HWM_STA;
    OS_MemSET(&sta, 0 , sizeof(Sta_setting));    
    sta.status = TRUE;
    sta.mode = mode;

#if NETMGR_SUPPORT

#if USE_ICOMM_LWIP
    if (static_ip)
    {
        netmgr_dhcpc_set(false,0);
        {
            ipinfo info;
    
            info.ipv4 = ipaddr_addr("192.168.100.100");
            info.netmask = ipaddr_addr("255.255.255.0");
            info.gateway = ipaddr_addr("192.168.100.1");
            info.dns = ipaddr_addr("192.168.100.1");
    
            netmgr_ipinfo_set(0, &info, false);
        }
    }
    else
    {
        netmgr_dhcpc_set(true,0);
    }
#else
    netmgr_dhcpc_set(true,0);
#endif

    if(bJoin)
    {
        wifi_sta_join_cfg join_cfg;
        OS_MemSET((void * )&join_cfg, 0, sizeof(join_cfg));       
        OS_MemCPY((void *)join_cfg.ssid.ssid,JOIN_DEFAULT_SSID,ssv6xxx_strlen(JOIN_DEFAULT_SSID));
        join_cfg.ssid.ssid_len=ssv6xxx_strlen(JOIN_DEFAULT_SSID);
        ssv6xxx_strcpy((void *)join_cfg.password, JOIN_DEFAULT_PSK);
        //join_cfg.pmk_valid=1;
        //OS_MemCPY((void *)join_cfg.pmk,join_default_pmk,32);
        //join_cfg.def_ch_mask=JOIN_DEFAULT_2P4G_CH_MASK;
        //join_cfg.def_5g_ch_mask=JOIN_DEFAULT_5G_CH_MASK;
        ret = netmgr_wifi_switch_async(mode, NULL, &join_cfg);
    }
    else
    {
        ret = netmgr_wifi_sta_on_async();
    }
#else
    ssv6xxx_wifi_station_async((void*)&sta);
#endif    
    if (ret != 0)
    {
        LOG_PRINTF("STA mode error (%d)\r\n", bJoin);
        return SSV6XXX_FAILED;
    }
    
    if (ssv6xxx_check_ready(SSV6XXX_HWM_STA) != SSV6XXX_SUCCESS)
    {
        LOG_PRINTF("STA mode timeout\r\n");
        return SSV6XXX_FAILED;
    }

    LOG_PRINTF("STA mode success\r\n");
    return SSV6XXX_SUCCESS;
}

#if (AP_MODE_ENABLE == 1)
ssv6xxx_result APmode_default(ssv_type_bool static_ip, ssv_type_bool dhcp_server)
{
    Ap_setting ap_cfg;
    int ret = 0;
    
    OS_MemSET((void * )&ap_cfg, 0, sizeof(ap_cfg));
#if 1
    ap_cfg.status = TRUE;
    OS_MemCPY((void *)ap_cfg.ssid.ssid,AP_DEFAULT_SSID,ssv6xxx_strlen(AP_DEFAULT_SSID));
    ap_cfg.ssid.ssid_len = ssv6xxx_strlen(AP_DEFAULT_SSID);
    ssv6xxx_strcpy((void *)(ap_cfg.password),AP_DEFAULT_PSK);
    //ap_cfg.is_hidden = 1;
    ap_cfg.max_sta_count = AP_SUPPORT_CLIENT_NUM;
    ap_cfg.security =   SSV6XXX_SEC_WPA2_PSK;
    ap_cfg.proto = WPA_PROTO_RSN;
    ap_cfg.key_mgmt = WPA_KEY_MGMT_PSK ;
    ap_cfg.group_cipher=WPA_CIPHER_CCMP;
    ap_cfg.pairwise_cipher = WPA_CIPHER_CCMP;
    ap_cfg.channel = 1; //EN_CHANNEL_AUTO_SELECT;
    ap_cfg.ch_bw = EN_CH_BW_AUTO;//EN_CH_BW_AUTO;
    ap_cfg.step = 0;
#else
    ap_cfg.status = TRUE;
    OS_MemCPY((void *)ap_cfg.ssid.ssid,AP_DEFAULT_SSID,ssv6xxx_strlen(AP_DEFAULT_SSID));
    ap_cfg.ssid.ssid_len = ssv6xxx_strlen(AP_DEFAULT_SSID);
  
    ap_cfg.max_sta_count = AP_SUPPORT_CLIENT_NUM;
    ap_cfg.security =   SSV6XXX_SEC_NONE;
    ap_cfg.channel = 1;
    ap_cfg.ch_bw = EN_CH_BW_AUTO;
    ap_cfg.step = 0;
#endif

#if NETMGR_SUPPORT

#if USE_ICOMM_LWIP
    if (static_ip)
    {
        ipinfo info;
    
        info.ipv4 = ipaddr_addr("172.16.10.1");
        info.netmask = ipaddr_addr("255.255.255.0");
        info.gateway = ipaddr_addr("172.16.10.1");
        info.dns = ipaddr_addr("172.16.10.1");
    
        netmgr_ipinfo_set(0,&info, true);
    }
    
    if (dhcp_server)
    {
        netmgr_dhcpd_set(true,0);
    }
    else
    {
        netmgr_dhcpd_set(false,0);
    }
#endif
    ret = netmgr_wifi_ap_on_async(&ap_cfg);
#else
    ssv6xxx_wifi_ap_async(&ap_cfg);
#endif
    if (ret != 0)
    {
        LOG_PRINTF("AP mode error\r\n");
        return SSV6XXX_FAILED;
    }
    
    if (ssv6xxx_check_ready(SSV6XXX_HWM_AP) != SSV6XXX_SUCCESS)
    {
        LOG_PRINTF("AP mode timeout\r\n");
        return SSV6XXX_FAILED;
    }
    
    //LOG_PRINTF("AP mode success\r\n");
    return SSV6XXX_SUCCESS;
}
#endif

ssv6xxx_result SNIFFERmode_default(void)
{
	ssv_type_s32 ret=0;
#if NETMGR_SUPPORT
    ret = netmgr_wifi_sniffer_on_async();
#else    
    ssv6xxx_wifi_station_async((void*)&sta);
#endif
    if (ret != 0)
    {
        LOG_PRINTF("SNIFFER mode error\r\n");
    }
    
    if (ssv6xxx_check_ready(SSV6XXX_HWM_SNIFFER))
    {
        LOG_PRINTF("SNIFFER mode timeout\r\n");
        return SSV6XXX_FAILED;
    }
       
    LOG_PRINTF("SNIFFER mode success\r\n");
    return SSV6XXX_SUCCESS;
}

static ssv6xxx_result _wifi_start(ssv6xxx_hw_mode mode, ssv_type_bool static_ip, ssv_type_bool dhcp_server)
{
    ssv6xxx_result res = 0;
    /* station mode */
    if (mode == SSV6XXX_HWM_STA)
    {
        LOG_PRINTF("_wifi_start SSV6XXX_HWM_STA\r\n");
		res = STAmode_default(false, static_ip, dhcp_server); //false: don't auto join AP_DEFAULT_SSDI
    }

    #if (AP_MODE_ENABLE == 1)
    /* ap mode */
    if (mode == SSV6XXX_HWM_AP)
    {
        LOG_PRINTF("_wifi_start SSV6XXX_HWM_AP\r\n");
        res = APmode_default(static_ip, dhcp_server);
    }
    #endif
    /* sniffer mode */
    if (mode == SSV6XXX_HWM_SNIFFER)
    {
        res = SNIFFERmode_default();
    }    

    //LOG_PRINTF("wifi start \r\n");
    return res;
}

ssv6xxx_result wifi_start(ssv6xxx_hw_mode hmode)
{
    ssv6xxx_result res;
    switch (hmode)
    {
        case SSV6XXX_HWM_STA: //AUTO_INIT_STATION	    
        {
            res = _wifi_start(SSV6XXX_HWM_STA,false,false);
            break;
        }
        case SSV6XXX_HWM_AP:
        {
#if (AP_MODE_ENABLE == 1)
           res = _wifi_start(SSV6XXX_HWM_AP,false,true);
#else
           res = SSV6XXX_FAILED;
#endif
           break;
        }
        case SSV6XXX_HWM_SNIFFER:
        {
            res = _wifi_start(SSV6XXX_HWM_SNIFFER,false,true);
            break;
        }
        default:
        {
            res = _wifi_start(SSV6XXX_HWM_STA,false,false);
        }
    }
    return res;
}
/**
 *  Entry point of the firmware code. After system booting, this is the
 *  first function to be called from boot code. This function need to
 *  initialize the chip register, software protoctol, RTOS and create
 *  tasks. Note that, all memory resources needed for each software
 *  modulle shall be pre-allocated in initialization time.
 */
/* return int to avoid from compiler warning */

#if (AP_MODE_ENABLE == 1)
extern ssv_type_s32 AP_Init(ssv_type_u32 max_sta_num);
#endif
extern ssv_type_s32 STA_Init(void);
extern void host_global_init(void);
extern void host_global_deinit(void);
extern void os_timer_init(void);
extern void os_timer_deinit(void);
extern ssv6xxx_result ssv6xxx_wifi_deinit(ssv_type_u32 step);
extern char *if_name[];
void ssv6xxx_dev_deinit(void)
{
    ssv_type_u8 i = 0;
    Sta_setting sta;
	ssv_type_u8 times = 100;

    OS_MemSET(&sta, 0 , sizeof(Sta_setting));

    for (i=0;i<MAX_VIF_NUM;i++)
    {
        if(gDeviceInfo->vif[i].hw_mode != SSV6XXX_HWM_INVALID)
        {
            if(gDeviceInfo->vif[i].hw_mode == SSV6XXX_HWM_AP)            
            {
                netmgr_wifi_ap_off_async();

				while (times--)
				{
					if (gDeviceInfo->vif[i].hw_mode != SSV6XXX_HWM_AP)
					{
						break;
					}

					OS_MsDelay(10);
				}
			}
            else
            {
                if(gDeviceInfo->vif[i].hw_mode == SSV6XXX_HWM_STA)            
                {
                    netmgr_wifi_leave_async();
                    netmgr_wifi_sta_off_async();
                	
					while (times--)
					{
						if (gDeviceInfo->vif[i].hw_mode != SSV6XXX_HWM_STA)
						{
							break;
						}

						OS_MsDelay(10);
					}
				}
                else
                {
                    sta.status = FALSE;
                    netmgr_wifi_control_async(gDeviceInfo->vif[i].hw_mode, NULL, &sta);
                }
            }
        }
    }
    
    OS_MsDelay(100);
    ssv6xxx_drv_irq_disable(0);
    //ssv6xxx_drv_write_reg(0xc0000000, 0x200000);
    
    ssv6xxx_drv_close();
}
#if (AP_MODE_ENABLE == 1)
extern void ssv6xxx_eapol_data_reg_cb(void);
#endif

void init_host_cfg(void)
{
    //host config default value
    OS_MemSET((void*)&g_host_cfg,0,sizeof(g_host_cfg));
    g_host_cfg.rate_mask= RC_DEFAULT_RATE_MSK;
    g_host_cfg.force_rtscts = 0;
    g_host_cfg.force_cts2self = 0;
    g_host_cfg.pool_size = POOL_SIZE;
    g_host_cfg.recv_buf_size = RECV_BUF_SIZE;
    g_host_cfg.bcn_interval = AP_BEACON_INT;
    g_host_cfg.trx_hdr_len = TRX_HDR_LEN;
    g_host_cfg.erp = AP_ERP;
    g_host_cfg.b_short_preamble= AP_B_SHORT_PREAMBLE;
    g_host_cfg.tx_sleep = TX_TASK_SLEEP;
    g_host_cfg.tx_sleep_tick = TX_TASK_SLEEP_TICK;    
    g_host_cfg.tx_retry_cnt= TX_TASK_RETRY_CNT;    
    g_host_cfg.tx_res_page = TX_RESOURCE_PAGE;
    g_host_cfg.rx_res_page = RX_RESOURCE_PAGE;
    g_host_cfg.ap_rx_short_GI = AP_RX_SHORT_GI;
    g_host_cfg.ap_rx_support_legacy_rate_msk = AP_RX_SUPPORT_BASIC_RATE;
    g_host_cfg.ap_rx_support_mcs_rate_msk = AP_RX_SUPPORT_MCS_RATE;
    g_host_cfg.volt_mode = SSV_VOLT_REGULATOR;
    g_host_cfg.ampdu_rx_buf_size= AMPDU_RX_BUF_SIZE;
    g_host_cfg.ampdu_tx_buf_size= AMPDU_TX_BUF_SIZE;
    g_host_cfg.support_ht = PHY_SUPPORT_HT;
    if(g_host_cfg.support_ht)
    {
        g_host_cfg.sta_ampdu_tx_enable= STA_AMPDU_TX_ENABLE;
        g_host_cfg.sta_ampdu_rx_enable= STA_AMPDU_RX_ENABLE;
        g_host_cfg.ap_ampdu_tx_enable= AP_AMPDU_TX_ENABLE;
        g_host_cfg.ap_ampdu_rx_enable= AP_AMPDU_RX_ENABLE;
    }
    else
    {
        g_host_cfg.sta_ampdu_tx_enable= 0;
        g_host_cfg.sta_ampdu_rx_enable= 0;
        g_host_cfg.ap_ampdu_tx_enable= 0;
        g_host_cfg.ap_ampdu_rx_enable= 0;
    }
    g_host_cfg.support_rf_band = RF_BAND;
    g_host_cfg.support_tx_SGI = RC_DEFAULT_TX_SGI;
    g_host_cfg.ap_no_dfs = AP_NO_DFS_FUN;
    g_host_cfg.sta_no_bcn_timeout = STA_NO_BCN_TIMEOUT;
    g_host_cfg.rxIntGPIO   = RXINTGPIO;
    g_host_cfg.AP_TimAllOne = AP_MODE_BEACON_VIRT_BMAP_0XFF;
    g_host_cfg.ApStaInactiveTime = AP_SAT_INACTIVE;
    // Red Bull support dual interfaces, if dual interfaces are working, and one interfaces as AP mode, and the ohter interfaces as STA mode,
    // ap mode just can support 7 clients, because we need to reserve a client for STA mode
    g_host_cfg.ap_supported_sta_num = AP_SUPPORT_CLIENT_NUM; //Reserve one clinet for STA mode
    // fw tx operation
    g_host_cfg.fw_tx_waitnum = 0;
    g_host_cfg.fw_tx_chkhwqnum = 0;
    g_host_cfg.backoff_enable = BACKOFF_ENABLE;

    if(ssv6xxx_strcmp(INTERFACE, "spi") == 0)
    {
        g_host_cfg.extRxInt = 1;
    }
    else
    {
#ifdef EXT_RX_INT
        g_host_cfg.extRxInt = 1;
#else
        g_host_cfg.extRxInt = 0;
#endif
    }

    g_host_cfg.xtal=XTAL_SETTING;
    
    //Default channel mask in sta and smart config mode
    g_host_cfg.sta_scan_ch_interval=SCANNING_CH_INTERVAL;
    g_host_cfg.sta_scan_off_ch_interval=SCANNING_OFF_CH_INTERVAL;
    g_host_cfg.sta_channel_mask = DEFAULT_STA_CHANNEL_MASK;
    g_host_cfg.sta_5g_channel_mask = DEFAULT_STA_5G_CHANNEL_MASK;

    // init lowpower param
    g_host_cfg.ps_wakeuppin = PS_WAKEUPPIN;
    g_host_cfg.ps_userdtim = PS_USERDTIM;
    g_host_cfg.ps_ignore_disconnection = PS_IGNOREDISCONNECTION;
    g_host_cfg.ps_wow_any_method = PS_WOW_ANY_METHOD;
    g_host_cfg.ps_sleep_gpio_ctrl = PS_SLEEP_GPIO_CTRL;
    g_host_cfg.ps_dormant_gpio_ctrl = PS_DORMANT_GPIO_CTRL;
    g_host_cfg.ps_keepalive = PS_KEEPALIVE;
    g_host_cfg.ps_dataretx_guard = PS_DATARETX_GUARD;
    g_host_cfg.ps_beaconloss_guard = PS_BEACONLOSS_GUARD;
    g_host_cfg.ps_beaconloss_maxcnt = PS_BEACONLOSS_MAXCNT;
    g_host_cfg.ps_sdio_interface = PS_DRV_SDIO_INTERFACE;

    g_host_cfg.force_custom_rftable = FORCE_CUSTOM_RFTABLE;
    g_host_cfg.hci_tx_aggr = HCI_TX_AGGR;
    g_host_cfg.hci_rx_aggr = HCI_RX_AGGR;
    g_host_cfg.hci_tx_aggr_buf_size = HCI_TX_AGGR_BUF_SIZE;

    if(g_host_cfg.hci_rx_aggr)
    {
        g_host_cfg.hw_rx_agg_cnt = HCI_RX_AGGR_CNT;
        g_host_cfg.hw_rx_agg_timer_reload = HCI_RX_AGGR_TIMER_RLOD;
    }
#if 0 
    g_host_cfg.tx_trigger_en = 1;
    g_host_cfg.tx_trigger_qlen = 3;
    g_host_cfg.tx_trigger_pkt_size = 250;
    
    g_host_cfg.tx_wait_period = 3000;
#else
    g_host_cfg.tx_trigger_en = 0;
    g_host_cfg.tx_wait_period = 0;
#endif

    g_host_cfg.mifi_idle_timeout = 3;

    g_host_cfg.rf_band_gain_offset = RF_BAND_GAIN_OFFSET;
    g_host_cfg.efuse_rate_gain_mask = EFUSE_RATE_GAIN_MASK;
    g_host_cfg.rf_b_rate_offset = RF_B_RATE_OFFSET;
    g_host_cfg.rf_g_rate_offset =RF_G_RATE_OFFSET;
    g_host_cfg.rf_ht20_rate_offset = RF_HT20_RATE_OFFSET;
    g_host_cfg.rf_ht40_rate_offset = RF_HT40_RATE_OFFSET;
    g_host_cfg.rf_volt_regulator = RF_VOLT_REGULATOR;
    g_host_cfg.rf_tx_lowpower = RF_TX_LOW_POWER;
    g_host_cfg.rf_padpd = RF_PADPD;

    g_ap_acl_mode = AP_ACL_DISABLE;
    g_ap_acl_mac_count = 0;
    OS_MemSET(&g_ap_acl_mac_list[0], 0, sizeof(AP_SETTING_ACL_MAC) * AP_ACL_MAX_COUNT);
#if(ENABLE_WPS==1)
    g_host_cfg.WPSWakeTimeout=WPS_WAKE_TIMEOUT;
#endif

}

extern int customer_update_rf_table(void);


int ssv6xxx_dev_init(ssv6xxx_hw_mode hmode)
{
    static ssv_type_bool bDevInit=FALSE;
    ssv6xxx_result res;
    LOG_PRINTF("\33[32mSSV dev init %d\33[0m\r\n",OS_GetSysTick());

#ifndef __SSV_UNIX_SIM_
#if 1 //(defined(SSV_LDO_EN_PIN_) && SSV_LDO_EN_PIN_)
    platform_ldo_en_pin_init();
    //ldo_en 0 -> 1 equal to HW reset.
    platform_ldo_en(0);
    OS_MsDelay(10);
    platform_ldo_en(1);
    OS_MsDelay(50);
#endif
#endif


    if(TRUE==bDevInit)
    {
        //ssv6xxx_drv_module_init();
        ssv6xxx_drv_select(INTERFACE,TRUE);
        ssv6xxx_drv_detect_card();
        wifi_start(hmode);
        return 0;
    }

#if SSV_LOG_DEBUG
	g_log_module = CONFIG_LOG_MODULE;
	g_log_min_level = CONFIG_LOG_LEVEL;//LOG_LEVEL_ALL;//CONFIG_LOG_LEVEL;
#endif    

    init_host_cfg();

	/**
        * On simulation/emulation platform, initialize RTOS (simulation OS)
        * first. We use this simulation RTOS to create the whole simulation
        * and emulation platform.
        */
    SSV_ASSERT( PLATFORM_OS_Init() == OS_SUCCESS );
    host_global_init();
    ssv6xxx_init_task_para();

	LOG_init(true, true, LOG_LEVEL_ON, LOG_MODULE_MASK(LOG_MODULE_EMPTY), false);
#ifdef __SSV_UNIX_SIM__
	//LOG_out_dst_open(LOG_OUT_HOST_TERM, NULL);
	//LOG_out_dst_turn_on(LOG_OUT_HOST_TERM);
#endif

    SSV_ASSERT( msg_evt_init(g_host_cfg.pool_size+SSV_TMR_MAX) == OS_SUCCESS );

#if (CONFIG_USE_LWIP_PBUF==0)
    SSV_ASSERT( PBUF_Init(POOL_SIZE) == OS_SUCCESS );
#endif//#if CONFIG_USE_LWIP_PBUF

    netstack_init(NULL);
    /**
        * Initialize Host simulation platform. The Host initialization sequence
        * shall be the same as the sequence on the real host platform.
        *   @ Initialize host device drivers (SDIO/SIM/UART/SPI ...)
        */
#ifdef __SSV_UNIX_SIM__
    if(TRUE==ssv6xxx_drv_module_init())
    {
        ssv6xxx_drv_select(INTERFACE,FALSE);
    }
    Cli_Init(0, NULL);
    return 0;
#else
    SSV_ASSERT(ssv6xxx_drv_module_init() == true);
#endif
    //LOG_PRINTF("Try to connecting CABRIO via %s...\n\r",INTERFACE);
#if FAST_CONNECT_PROFILE
    LOG_PRINTF("\33[32msdio s %d\33[0m\n",OS_GetSysTick());
#endif    
    if (ssv6xxx_drv_select(INTERFACE,FALSE) == false)
    {

        {
        LOG_PRINTF("==============================\r\n");
    	LOG_PRINTF("Please Insert %s wifi device\n",INTERFACE);
		LOG_PRINTF("==============================\r\n");
    	}
       aic_assert(0);
		return -1;
    }
#ifdef __SSV_UNIX_SIM__
    Cli_Init(0, NULL);
    return -1;
#endif
#if FAST_CONNECT_PROFILE    
    LOG_PRINTF("\33[32msdio e %d\33[0m\n",OS_GetSysTick());
#endif
    ssv_assert(ssv_hal_init() == true);

    if(ssv6xxx_wifi_init()!=SSV6XXX_SUCCESS){
        LOG_PRINTF("ssv6xxx_wifi_init fail\r\n");
		return -1;
    }

    if (1) // for sdio read reg test
    {
        ssv_type_u32 value = 0;
        //OS_MsDelay(10000);
        value = ssv6xxx_drv_read_reg(0xc0000008);
        LOG_PRINTF("0xc0000008 = 0x%08x ?= 0x20202020\r\n", value);
        //while(1);
        value = ssv6xxx_drv_read_reg(0xc000000c);
        LOG_PRINTF("0xc000000c = 0x%08x ?= 0x30202020\r\n", value);

        value = ssv6xxx_drv_read_reg(0xc0000010);
        LOG_PRINTF("0xc0000010 = 0x%08x ?= 0x30303643\r\n", value);
        
        value = ssv6xxx_drv_read_reg(0xc0000014);
        LOG_PRINTF("0xc0000014 = 0x%08x ?= 0x53535636\r\n", value);
        
        //return 0;
    }

    os_timer_init();
    
    if(ssv6xxx_platform_check() != SSV6XXX_SUCCESS)
        return SSV6XXX_FAILED;

    ssv6xxx_wifi_init_regdom(); 
    ssv6xxx_wifi_set_reg(DEFAULT_COUNTRY_CODE);
    
#if (AP_MODE_ENABLE == 1)
    AP_Init(g_host_cfg.ap_supported_sta_num);
#endif
    STA_Init();
    /**
    * Initialize TCP/IP Protocol Stack. If tcpip is init, the net_app_ask()
    * also need to be init.
    */

    
#if DHCPD_SUPPORT
    if(udhcpd_init() != 0)
    {
        LOG_PRINTF("udhcpd_init fail\r\n");
        return SSV6XXX_FAILED;
    }
#endif

#if NETAPP_SUPPORT
    if(net_app_init()== 1)
    {
        LOG_PRINTF("net_app_init fail\n\r");
        return SSV6XXX_FAILED;
    }
#endif

#if NETMGR_SUPPORT
    /* netmgr int */
    ssv_netmgr_init(true);
    //if (1)
    {        
#if 0
            /* we can set default ip address and default dhcpd server config before netmgr netdev init */
            netmgr_cfg cfg;
            cfg.ipaddr = ipaddr_addr("192.168.100.1");
            cfg.netmask = ipaddr_addr("255.255.255.0");
            cfg.gw = ipaddr_addr("192.168.100.1");
            cfg.dns = ipaddr_addr("0.0.0.0");
            
            cfg.dhcps.start_ip = ipaddr_addr("192.168.100.101");
            cfg.dhcps.end_ip = ipaddr_addr("192.168.100.110");
            cfg.dhcps.max_leases = 10;
            cfg.dhcps.subnet = ipaddr_addr("255.255.255.0");
            cfg.dhcps.gw = ipaddr_addr("192.168.100.1");       
            cfg.dhcps.dns = ipaddr_addr("192.168.100.1");
            cfg.dhcps.auto_time = DEFAULT_DHCP_AUTO_TIME;
            cfg.dhcps.decline_time = DEFAULT_DHCP_DECLINE_TIME;
            cfg.dhcps.conflict_time = DEFAULT_DHCP_CONFLICT_TIME;
            cfg.dhcps.offer_time = DEFAULT_DHCP_OFFER_TIME;
            cfg.dhcps.max_lease_sec = DEFAULT_DHCP_MAX_LEASE_SEC;
            cfg.dhcps.min_lease_sec = DEFAULT_DHCP_MIN_LEASE_SEC;
            cfg.s_dhcpc_enable = 0;
            if(hmode == SSV6XXX_HWM_AP)
                cfg.s_dhcpd_enable = 1;
            cfg.s_dhcpc_status = 0;
            cfg.s_dhcpd_status = 0;
            netmgr_cfg_set(&cfg,0);
            if(MAX_VIF_NUM>1)
            {
                cfg.ipaddr = ipaddr_addr("192.168.200.1");
                cfg.netmask = ipaddr_addr("255.255.255.0");
                cfg.gw = ipaddr_addr("192.168.200.1");
                cfg.dns = ipaddr_addr("0.0.0.0");
                
                cfg.dhcps.start_ip = ipaddr_addr("192.168.200.101");
                cfg.dhcps.end_ip = ipaddr_addr("192.168.200.110");
                cfg.dhcps.max_leases = 10;
                cfg.dhcps.subnet = ipaddr_addr("255.255.255.0");
                cfg.dhcps.gw = ipaddr_addr("192.168.200.1");       
                cfg.dhcps.dns = ipaddr_addr("192.168.200.1");
                cfg.s_dhcpd_enable = 0;
                netmgr_cfg_set(&cfg,1);
            }    
            ssv_netmgr_init_netdev(false);
#else
            /* we can set default ip address and default dhcpd server config */
            ssv_netmgr_init_netdev(true);
#endif
        }
#endif
    if (g_host_cfg.force_custom_rftable == 0)
    {
        LOG_PRINTF("force_custom_rftable %d\r\n", g_host_cfg.force_custom_rftable);
        customer_update_rf_table();
    }
    else
    {
        LOG_PRINTF("force_custom_rftable %d\r\n", g_host_cfg.force_custom_rftable);
    }
#if 1
    res = wifi_start(hmode);
#else    
    switch (hmode)
    {
        case SSV6XXX_HWM_STA: //AUTO_INIT_STATION	    
        {
            res = wifi_start(SSV6XXX_HWM_STA,false,false);
            break;
        }
        case SSV6XXX_HWM_AP:
        {
#if (AP_MODE_ENABLE == 1)
           res = wifi_start(SSV6XXX_HWM_AP,false,true);
#else
           res = SSV6XXX_FAILED;
#endif
           break;
        }
        case SSV6XXX_HWM_SNIFFER:
        {
            res = wifi_start(SSV6XXX_HWM_SNIFFER,false,true);
            break;
        }
        default:
        {
            res = wifi_start(SSV6XXX_HWM_STA,false,false);
        }
    }
#endif    
        
#if (CLI_ENABLE==1)
    //Cli_Init(0, NULL);
    ssv6xxx_wifi_cfg();
#else
#ifndef CONFIG_NO_WPA2
#if (AP_MODE_ENABLE == 1)
    ssv6xxx_eapol_data_reg_cb();
#endif
#endif

#endif

    bDevInit = TRUE;
    return res;
}

void ssv_init(void)
{
	LOG_PRINTF("#####################ssv_init\r\n");
	ssv6xxx_dev_init(0);
}

