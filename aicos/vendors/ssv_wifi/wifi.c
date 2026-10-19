/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x00caa9
#define LOG_MODULE    AMOD_VENDOR

#include "wifi.h"
#include <porting.h>
#include <host_config.h>
#include <log.h>
#include <host_apis.h>
#include <cli/cmds/cli_cmd_wifi.h>
#include <net_mgr.h>
#include <netstack.h>

#define SSV_NETIF_NUM 2

#define SSV_WIFI_EVENT_ENABLE

extern const char *ssv_version;
extern const char *ssv_date;
extern const char *rlsversion;

#ifdef SSV_WIFI_EVENT_ENABLE
static int ssv_wifi_scan_status = 0;
#endif

ssv_wifi_event_cb g_ssv_wifi_event_cb;

#define WIFI_LIB_VERSION	ssv_version
extern int ssv6xxx_dev_init(ssv6xxx_hw_mode hmode);
extern void netstack_wifi_link_register_cb(netdev_link_callback_t link_up_cb, netdev_link_callback_t link_down_cb);
extern void customer_setting_rftable(int is_5g);
extern ssv6xxx_result ssv6xxx_check_ready(ssv6xxx_hw_mode hmode);
extern ssv6xxx_result ssv6xxx_check_stop(ssv6xxx_hw_mode hmode);
//void wifi_adap_report_sta_connect_status(unsigned char *ssid, unsigned char *bssid, int status);
//void wifi_adap_report_sta_disconnect(unsigned char *ssid);
//void wifi_adap_report_dhcpc_start(int id_, unsigned int taskid, int timeout);

static int ssv_wifi_link_get_status(void);

int g_wifi_init = 0;
int g_wifi_vif_idx = -1;


static void _ssv_wifi_set_eth_mac_config(void)
{
    unsigned char mac_addr[SSV_MAC_ADDR_LEN] = {0};
    int ret = -1;
    netdev_getmacaddr_vif(g_wifi_vif_idx, (ssv_type_u8 *)mac_addr);
    // LOG_PRINTF("mac_addr = %02x:%02x:%02x:%02x:%02x:%02x\r\n"
    //     , mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    //ret = ecos_ssv6x5x_set_mac_addr(mac_addr);
    if(0 != ret)
    {
        LOG_PRINTF("Set mac address(%02x:%02x:%02x:%02x:%02x:%02x) failed.\r\n"
            , mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    }
}

char *ssv_wifi_get_version(void)
{
	return (char *)WIFI_LIB_VERSION;
}

SSV_WIFI_MODE ssv_wifi_get_mode(void)
{
    wifi_mode mode;
    ssv_type_bool status = 0;
    ssv_type_bool sta_status = 0, smartcfg_status = 0, ap_status = 0;
    if (netmgr_wifi_mode_get(&mode, &status, 0) != 0)
    {
        return WIFI_MODE_UNKNOWN;
    }

    if ((mode == SSV6XXX_HWM_STA) && (status))
    {
        sta_status = TRUE;
    }
    else
    {
        if ((mode == SSV6XXX_HWM_SNIFFER) && (status))
        {
            smartcfg_status = TRUE;
        }
    }

    if (netmgr_wifi_mode_get(&mode, &status, 1) != 0)
    {
        return WIFI_MODE_UNKNOWN;
    }

    if ((mode == SSV6XXX_HWM_AP) && (status))
    {
        ap_status = TRUE;
    }

    if ((ap_status) && (sta_status || smartcfg_status))
    {
        return WIFI_MODE_AP_STA;
    }
    else if (ap_status)
    {
        return WIFI_MODE_AP;
    }
    else if (sta_status)
    {
        return WIFI_MODE_STA;
    }
    else if (smartcfg_status)
    {
        return WIFI_MODE_SMARTCFG;
    }

    return WIFI_MODE_UNKNOWN;
}

int ssv_wifi_set_mode(SSV_WIFI_MODE ssv_wifi_mode)
{
    LOG_PRINTF("wifi_set_mode %d\r\n", ssv_wifi_mode);
    if (ssv_wifi_mode == WIFI_MODE_STA)
    {
        SSV_WIFI_MODE mode = ssv_wifi_get_mode();

        if (mode == WIFI_MODE_STA)
        {
            LOG_PRINTF("Is STA mode\r\n");
            return SSV6XXX_SUCCESS;
        }
        else if (mode == WIFI_MODE_AP)
        {
            if (netmgr_wifi_ap_off_async() == SSV6XXX_FAILED)
            {
               LOG_PRINTF("ap mode off fail\r\n");
               return SSV6XXX_FAILED;
            }

            if (ssv6xxx_check_stop(SSV6XXX_HWM_AP) != SSV6XXX_SUCCESS)
            {
                LOG_PRINTF("ap mode off timeout\r\n");
            }

            LOG_PRINTF("ap mode off ok\r\n");
        }
        else if (mode == WIFI_MODE_SMARTCFG)
        {
            if (netmgr_wifi_sniffer_off_async()== SSV6XXX_FAILED)
            {
               LOG_PRINTF("sniffer mode off fail\r\n");
               return SSV6XXX_FAILED;
            }

            if (ssv6xxx_check_stop(SSV6XXX_HWM_SNIFFER) != SSV6XXX_SUCCESS)
            {
                LOG_PRINTF("sniffer mode off timeout\r\n");
            }

            LOG_PRINTF("sniffer mode off ok\r\n");
        }

        if (netmgr_wifi_sta_on_async()== SSV6XXX_FAILED)
        {
           LOG_PRINTF("sta mode on fail\r\n");
           return SSV6XXX_FAILED;
        }

        if (ssv6xxx_check_ready(SSV6XXX_HWM_STA) != SSV6XXX_SUCCESS)
        {
            LOG_PRINTF("STA mode timeout\r\n");
            return SSV6XXX_FAILED;
        }

        g_wifi_vif_idx = 0;
        _ssv_wifi_set_eth_mac_config();

        LOG_PRINTF("STA mode success\r\n");
        return 0;

    }
    else if (ssv_wifi_mode == WIFI_MODE_AP)
    {
        SSV_WIFI_MODE mode = ssv_wifi_get_mode();

        if (mode == WIFI_MODE_AP)
        {
            if (netmgr_wifi_ap_off_async() == SSV6XXX_FAILED)
            {
               LOG_PRINTF("ap mode off fail\r\n");
               return SSV6XXX_FAILED;
            }

            if (ssv6xxx_check_stop(SSV6XXX_HWM_AP) != SSV6XXX_SUCCESS)
            {
                LOG_PRINTF("ap mode off timeout\r\n");
            }

            LOG_PRINTF("ap mode off ok\r\n");
            return SSV6XXX_SUCCESS;
        }
        else if (mode == WIFI_MODE_STA)
        {
            if (netmgr_wifi_sta_off_async() == SSV6XXX_FAILED)
            {
               LOG_PRINTF("sta mode off fail\r\n");
               return SSV6XXX_FAILED;
            }

            if (ssv6xxx_check_stop(SSV6XXX_HWM_STA) != SSV6XXX_SUCCESS)
            {
                LOG_PRINTF("sta mode off timeout\r\n");
            }

            LOG_PRINTF("sta mode off ok\r\n");
        }
        else if (mode == WIFI_MODE_SMARTCFG)
        {
            if (netmgr_wifi_sniffer_off_async()== SSV6XXX_FAILED)
            {
               LOG_PRINTF("sniffer mode off fail\r\n");
               return SSV6XXX_FAILED;
            }

            if (ssv6xxx_check_stop(SSV6XXX_HWM_SNIFFER) != SSV6XXX_SUCCESS)
            {
                LOG_PRINTF("sniffer mode off timeout\r\n");
            }

            LOG_PRINTF("sniffer mode off ok\r\n");
        }

        LOG_PRINTF("AP mode success\r\n");
        return 0;
    }

    return 0;
}

int ssv_wifi_stop_softap(void)
{
    SSV_WIFI_MODE mode = ssv_wifi_get_mode();
    if (mode == WIFI_MODE_AP)
    {
        if (netmgr_wifi_ap_off_async() == SSV6XXX_FAILED)
        {
           LOG_PRINTF("ap mode off fail\r\n");
           return SSV6XXX_FAILED;
        }
    }
    else
    {
        LOG_PRINTF("Not AP mode\r\n");
        return SSV6XXX_SUCCESS;
    }

    return SSV6XXX_SUCCESS;
}

int ssv_wifi_start_softap(ssv_wifi_apcfg_t *ap_cfg)
{
    LOG_PRINTF("ssv_wifi_start_softap\r\n");
	return ssv_wifi_start_softap_ext((char *)ap_cfg->ssid, (char *)ap_cfg->key, 
        ap_cfg->authmode, ap_cfg->channel, ap_cfg->is_hidden, ap_cfg->max_sta_count);
}

int ssv_wifi_start_softap_ext(char *name, char *key, int key_type, int channel, unsigned char is_hidden, int max_sta_count)
{
    wifi_mode mode;
    ssv_wifi_ap_cfg ap_cfg;
    ssv_wifi_set_mode(WIFI_MODE_AP);
    
    LOG_PRINTF("ssv_wifi_start_softap ssid=%s, channel=%d, is_hidden=%d, max_sta_count=%d\r\n",name,
        channel, is_hidden, max_sta_count);

    OS_MemSET((void * )&ap_cfg, 0, sizeof(ap_cfg));
    mode = SSV6XXX_HWM_AP;
    ap_cfg.status = TRUE;
    OS_MemCPY((void *)ap_cfg.ssid.ssid,name,ssv6xxx_strlen(name));
    ap_cfg.ssid.ssid_len = ssv6xxx_strlen(name);
    ssv6xxx_strcpy((void *)(ap_cfg.password),key);
    ap_cfg.is_hidden = is_hidden;
    ap_cfg.max_sta_count = SSV_MAX_STA_COUNT;
    if ((max_sta_count > 0) && (max_sta_count < SSV_MAX_STA_COUNT))
    {
        ap_cfg.max_sta_count = max_sta_count;
    }
    
    if (key_type == KEY_NONE)
    {
        ap_cfg.security = SSV6XXX_SEC_NONE;
        ssv6xxx_strcpy((void *)(ap_cfg.password),"");
    }
    else if(key_type == KEY_WEP)
    {
        if(ssv6xxx_strlen((const char *)ap_cfg.password) == 5)
            ap_cfg.security =   SSV6XXX_SEC_WEP_40;
        else if (ssv6xxx_strlen((const char *)ap_cfg.password) == 13)
            ap_cfg.security =   SSV6XXX_SEC_WEP_104;
    }
    else if(key_type == KEY_WPA)
    {
        LOG_PRINTF("not support\r\n");
        return SSV6XXX_FAILED;
    }
    else if(key_type == KEY_WPA2)
    {
        ap_cfg.security =   SSV6XXX_SEC_WPA2_PSK;
        ap_cfg.proto = WPA_PROTO_RSN;
        ap_cfg.key_mgmt = WPA_KEY_MGMT_PSK ;
        ap_cfg.group_cipher=WPA_CIPHER_CCMP;
        ap_cfg.pairwise_cipher = WPA_CIPHER_CCMP;
    }
    else
    {
        ap_cfg.security = SSV6XXX_SEC_NONE;
        ssv6xxx_strcpy((void *)(ap_cfg.password),"");
    }

    ap_cfg.ch_bw = EN_CH_BW_AUTO;
    ap_cfg.channel = channel; //EN_CHANNEL_AUTO_SELECT;
    if (channel > 13)
    {
        customer_setting_rftable(1);
    }
    else
    {
        customer_setting_rftable(0);
    }

    if(netmgr_wifi_ap_on_async(&ap_cfg) == SSV6XXX_FAILED)
    {
        LOG_PRINTF("AP mode error\r\n");
        return SSV6XXX_FAILED;
    }

    if (ssv6xxx_check_ready(SSV6XXX_HWM_AP) != SSV6XXX_SUCCESS)
    {
        LOG_PRINTF("AP mode timeout\r\n");
        return -1;
    }

    g_wifi_vif_idx = 1;
    _ssv_wifi_set_eth_mac_config();

    LOG_PRINTF("AP mode success\r\n");
    return 0;
}


int ssv_wifi_ap_create(char* ssid, int authMode, int encryption, char *key, int channel, int ssidBcst , int max_sta_count)
{
    int key_type = KEY_WPA;

    if(authMode == WLAN_WPA_AUTH_NONE && encryption == WLAN_ENCRYPT_NONE)
        key_type = KEY_NONE;
    else if(authMode == WLAN_WPA2_AUTH_PSK && encryption == WLAN_ENCRYPT_AES)
        key_type = KEY_WPA2;
    else if(authMode == WLAN_WPA_AUTH_DISABLED && encryption == WLAN_ENCRYPT_WEP)
        key_type = KEY_WEP;

    ssv_wifi_start_softap_ext(ssid, key, key_type, channel, !ssidBcst, max_sta_count);
    return 0;
}

int ssv_wifi_get_mac_2(unsigned char *mac_addr ,int index)
{
    netdev_getmacaddr_vif(index, mac_addr);
	return 0;
}

int ssv_wifi_softap_list_station(ssv_wifi_sta_list_t *sta_list)
{
    Ap_sta_status info;
    OS_MemSET(&info , 0 , sizeof(Ap_sta_status));
    ssv6xxx_wifi_status(&info);

    if(!info.status)
    {
        sta_list->sta_count = 0;
        return 0;
    }

    if(SSV6XXX_HWM_AP==info.vif_operate[1])
    {
        ssv_type_u8 idx;
        struct apmode_sta_info *sta_info=NULL;
        ssv_type_u8 ssid_buf[MAX_SSID_LEN+1]={0};

        OS_MemCPY((void*)ssid_buf,(void*)info.vif_u[1].station_status.ssid.ssid,info.vif_u[1].station_status.ssid.ssid_len);
        LOG_PRINTF("SSID:%s\r\n",ssid_buf);
        LOG_PRINTF("channel:%d\r\n",info.vif_u[1].station_status.channel);

        sta_info = (void*)OS_MemAlloc(sizeof(struct apmode_sta_info));
        if(!sta_info)
        {
            LOG_PRINTF("malloc sta_info fail \r\n");
            return -1;
        }
        for(idx=0;idx<AP_SUPPORT_CLIENT_NUM;idx++)
        {
            OS_MemSET(sta_info , 0 , sizeof(struct apmode_sta_info));
            if(ssv6xxx_get_sta_info_by_aid(sta_info,idx)==SSV6XXX_SUCCESS)
            {
                LOG_PRINTF("ssv_wifi_softap_list_station: sta_info->addr %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                    sta_info->addr[0], sta_info->addr[1],sta_info->addr[2],
                    sta_info->addr[3],sta_info->addr[4],sta_info->addr[5]);
                if(ssv_is_valid_ether_addr(sta_info->addr))
                {
                    if(sta_info->arp_retry_count>g_sta_timeout_retry_count)
                    {
                        //LOG_PRINTF("Inactive\r\n");
                    }
                    else
                    {
                        OS_MemCPY((unsigned char *)sta_list->sta_info[sta_list->sta_count].mac_address,  sta_info->addr, SSV_MAC_ADDR_LEN);
                        sta_list->sta_info[sta_list->sta_count].rssi = sta_info->rssi;
                        sta_list->sta_count++;
                        //LOG_PRINTF("Active\r\n");
                    }
               }
            }
        }
        OS_MemFree(sta_info);
    }
    else
    {
        sta_list->sta_count = 0;
        return 0;
    }

    return 0;
}

int ssv_wifi_get_associated_client_list(void *pchBuf, unsigned int uiBufSize)
{
    return 0;
}

int ssv_wifi_start_station(ssv_wifi_stacfg_t *sta_cfg)
{

    LOG_PRINTF("ssv_wifi_start_station\r\n");
    if (sta_cfg)
    {
	    return ssv_wifi_start_station_ext((char *)sta_cfg->ssid,(char *) sta_cfg->key, sta_cfg->channel, sta_cfg->bssid);
    }
    else
    {
        ssv_wifi_set_mode(WIFI_MODE_STA);
    }

    return SSV6XXX_SUCCESS;
}

int ssv_wifi_start_station_ext(char *name, char *key, unsigned char channel, unsigned char bssid[SSV_MAC_ADDR_LEN])
{
    int ret;
    Sta_setting sta;
    wifi_sta_join_cfg join_cfg;
	ssv_type_bool timeout = false;
	ssv_type_u32 start_tick = 0;

    if (!name || !key || (channel > 165))
    {
        LOG_PRINTF("ssv_wifi_start_station_ext parameter err\r\n");
        return 0;
    }

    ssv_wifi_set_mode(WIFI_MODE_STA);

    OS_MemSET(&sta, 0 , sizeof(Sta_setting));
    sta.status = TRUE;
    sta.mode = SSV6XXX_HWM_STA;

    netmgr_dhcpc_set(true,0);
    OS_MemSET((void * )&join_cfg, 0, sizeof(join_cfg));
    OS_MemCPY((void *)join_cfg.ssid.ssid,name,ssv6xxx_strlen(name));
    join_cfg.ssid.ssid_len=ssv6xxx_strlen(name);
    ssv6xxx_strcpy((void *)join_cfg.password, key);
    if (ssv_wifi_get_mode() == WIFI_MODE_STA)
    {
        ret = netmgr_wifi_join_other_async(&join_cfg);
    }
    else
    {
        ret = netmgr_wifi_switch_async(sta.mode, NULL, &join_cfg);
    }
    if (ret != 0)
    {
        LOG_PRINTF("STA mode error r\n");
        return SSV6XXX_FAILED;
    }

    if (ssv6xxx_check_ready(SSV6XXX_HWM_STA) != SSV6XXX_SUCCESS)
    {
        LOG_PRINTF("STA mode timeout\r\n");
        return SSV6XXX_FAILED;
    }
#if 0
	if (ssv_wifi_isconnected() == false)
    {
    	start_tick = OS_GetSysTick();
		while ((ssv_wifi_isconnected() == false)&&(((OS_GetSysTick() - start_tick) * OS_TICK_RATE_MS) < 15000))
        {
            SSV_OS_TickDelay(1);
        }

    }

	if (ssv_wifi_isconnected() == false)
	{
		LOG_PRINTF("join timeout\r\n");
	}
	else
	{
		LOG_PRINTF("join ok\r\n");
	}
#endif
	start_tick = OS_GetSysTick();
    timeout = false;
    while (!netmgr_wifi_check_sta_connected())
    {
        // wait 8 second timeout
        if (((OS_GetSysTick() - start_tick) * OS_TICK_RATE_MS) >= (ssv6xxx_wifi_support_5g_band()? 15000: 8000))
        {
            timeout = true;
            break;
        }
        SSV_OS_TickDelay(1);
    }

    if (timeout)
    {
        LOG_PRINTF("ssv_wifi_start_station_ext  timeout\r\n");
        return -1;
    }

    LOG_PRINTF("ssv_wifi_start_station_ext success\r\n");
    return SSV6XXX_SUCCESS;

}

int ssv_wifi_stop_station(void)
{
    wifi_mode mode;
    ssv_wifi_sta_cfg sta_cfg;
    int ret = 0;
    ssv_type_u32 start_tick = 0;

    LOG_PRINTF("ssv_wifi_stop_station\r\n");

    if (ssv_wifi_get_mode() == WIFI_MODE_STA)
    {
        //OS_MsDelay(300);
		if (ssv_wifi_isconnected() == true)
		{
			ret = netmgr_wifi_leave_async();
			start_tick = OS_GetSysTick();
    		while ((ssv_wifi_isconnected() == true)&&(((OS_GetSysTick() - start_tick) * OS_TICK_RATE_MS) < 3000))
            {
                SSV_OS_TickDelay(1);
            }
		}

        OS_MemSET((void * )&sta_cfg, 0, sizeof(sta_cfg));
        mode = SSV6XXX_HWM_STA;
        sta_cfg.status = FALSE;

        ret = netmgr_wifi_control_async(mode, NULL, &sta_cfg);
        if (ret != 0)
        {
            LOG_PRINTF("station stop err\r\n");
            return SSV6XXX_FAILED;
        }

        if (ssv6xxx_check_stop(SSV6XXX_HWM_STA) != SSV6XXX_SUCCESS)
        {
            LOG_PRINTF("station stop timeout\r\n");
            return SSV6XXX_FAILED;
        }
    }

    LOG_PRINTF("ssv_wifi_stop_station success\r\n");
    return SSV6XXX_SUCCESS;
}

int ssv_wifi_station_connect(ssv_wifi_stacfg_t *sta_cfg)
{
	return ssv_wifi_start_station_ext((char *)sta_cfg->ssid, (char *)sta_cfg->key, sta_cfg->channel, sta_cfg->bssid);
}

int ssv_wifi_station_connect_2( char *ssid, char *key)
{
    int ret;
    wifi_mode mode;
    wifi_sta_join_cfg join_cfg;
    ssv_type_u32 start_tick = 0;
    ssv_type_bool timeout = false;
    ssv_type_u8 vif_index = 0;

    if (!ssid || !key)
    {
        LOG_PRINTF("ssv_wifi_station_connect_2 parameter err\r\n");
        return 0;
    }

    ssv_wifi_set_mode(WIFI_MODE_STA);

    OS_MemSET((void * )&join_cfg, 0, sizeof(join_cfg));
    netmgr_dhcpc_set(true, vif_index);

    OS_MemCPY((void *)join_cfg.ssid.ssid,ssid,ssv6xxx_strlen(ssid));
    join_cfg.ssid.ssid_len=ssv6xxx_strlen(ssid);
    ssv6xxx_strcpy((void *)join_cfg.password, key);
    if (ssv_wifi_get_mode() == WIFI_MODE_STA)
    {
        ret = netmgr_wifi_join_other_async(&join_cfg);
    }
    else
    {
        mode = SSV6XXX_HWM_STA;
        ret = netmgr_wifi_switch_async(mode, NULL, &join_cfg);
    }

    if (ret != 0)
    {
        LOG_PRINTF("STA mode error (%d)\r\n", ret);
        return -1;
    }

    if (ssv6xxx_check_ready(SSV6XXX_HWM_STA) != SSV6XXX_SUCCESS)
    {
        LOG_PRINTF("STA mode timeout\r\n");
        return SSV6XXX_FAILED;
    }

    start_tick = OS_GetSysTick();
    timeout = false;
    while (!netmgr_wifi_check_sta_connected())
    {
        // wait 8 second timeout
        if (((OS_GetSysTick() - start_tick) * OS_TICK_RATE_MS) >= (ssv6xxx_wifi_support_5g_band()? 15000: 8000))
        {
            timeout = true;
            break;
        }
        SSV_OS_TickDelay(1);
    }

    if (timeout)
    {
        LOG_PRINTF("ssv_wifi_station_connect_2 2. timeout\r\n");
        return -1;
    }

    LOG_PRINTF("ssv_wifi_station_connect_2 success\r\n");
    return 0;
}

int ssv_wifi_station_connect_no_scan( char *ssid, char *key)
{
    int ret;
    // wifi_mode mode;
    wifi_sta_join_cfg join_cfg;
    ssv_type_u32 start_tick = 0;
    ssv_type_bool timeout = false;
    ssv_type_u8 vif_index = 0;

    if (!ssid || !key)
    {
        LOG_PRINTF("ssv_wifi_station_connect_2 parameter err\r\n");
        return 0;
    }

    OS_MemSET((void * )&join_cfg, 0, sizeof(join_cfg));
    netmgr_dhcpc_set(true, vif_index);

    OS_MemCPY((void *)join_cfg.ssid.ssid,ssid,ssv6xxx_strlen(ssid));
    join_cfg.ssid.ssid_len=ssv6xxx_strlen(ssid);
    ssv6xxx_strcpy((void *)join_cfg.password, key);
    ret = netmgr_wifi_join_async(&join_cfg);

    if (ret != 0)
    {
        LOG_PRINTF("join3 error (%d)\r\n", ret);
        return -1;
    }

    start_tick = OS_GetSysTick();
    timeout = false;
    while (!netmgr_wifi_check_sta_connected())
    {
        // wait 3 second timeout
        if (((OS_GetSysTick() - start_tick) * OS_TICK_RATE_MS) >= 3000)
        {
            timeout = true;
            break;
        }
        SSV_OS_TickDelay(1);
    }

    if (timeout)
    {
        LOG_PRINTF("ssv_wifi_station_connect_3 timeout\r\n");
        return -1;
    }

    LOG_PRINTF("ssv_wifi_station_connect_3 success\r\n");
    return 0;
}


int ssv_wifi_station_disconnect(void)
{
    int ret;
    ssv_type_u32 start_tick = 0;
    ssv_type_bool timeout = false;

    if (netmgr_wifi_check_sta_connected())
    {
        ret = netmgr_wifi_leave_async();
    }
    else
    {
        return SSV6XXX_SUCCESS;
    }

    if (ret != 0)
    {
        LOG_PRINTF("netmgr_wifi_leave_async error\r\n");
        return SSV6XXX_FAILED;
    }

    start_tick = OS_GetSysTick();
    timeout = false;
    while (netmgr_wifi_check_sta_connected())
    {
        // wait 3 second timeout
        if (((OS_GetSysTick() - start_tick) * OS_TICK_RATE_MS) >= 3000)
        {
            timeout = true;
            break;
        }
        SSV_OS_TickDelay(1);
    }

    if (timeout)
    {
        LOG_PRINTF("ssv_wifi_disconnect 1. timeout\r\n");
        return SSV6XXX_FAILED;
    }


    LOG_PRINTF("ssv_wifi_disconnect success\r\n");
    return SSV6XXX_SUCCESS;
}

int ssv_wifi_isconnected()
{
    return ssv_wifi_link_get_status();
}

int ssv_wifi_get_rssi()
{
    Ap_sta_status info;
    int rssi = 0;
    SSV_WIFI_MODE mode = ssv_wifi_get_mode();

    OS_MemSET(&info , 0 , sizeof(Ap_sta_status));
    ssv6xxx_wifi_status(&info);
    if(info.status)
    {
        if (((ssv6xxx_hw_mode)mode == SSV6XXX_HWM_STA) ||
            (SSV6XXX_HWM_SNIFFER == (ssv6xxx_hw_mode)mode))
        {
            if(ssv_is_valid_ether_addr(info.vif_u[0].station_status.apinfo.Mac)){
                rssi = - ssv6xxx_get_rssi_by_mac(info.vif_u[0].station_status.apinfo.Mac, 0);
            }
            else{
                rssi = 0;
            }
        }
    }

    return rssi;
}


void ssv_wifi_get_ap_list(ssv_wifi_ap_list_t *wifi_ap_list)
{
    ssv_type_u32 i=0, j = 0, AP_cnt = 0;
    Ap_sta_status connected_info;
    struct ssv6xxx_ieee80211_bss *ap_list = NULL;

    AP_cnt = ssv6xxx_get_aplist_info((void **)&ap_list);

    OS_MemSET(&connected_info , 0 , sizeof(Ap_sta_status));
    ssv6xxx_wifi_status(&connected_info);

    if((ap_list==NULL) || (AP_cnt==0))
    {
        //LOG_PRINTF("AP list empty!\r\n");
        wifi_ap_list->ap_count = 0;
        return;
    }

    for (i=0; i<AP_cnt; i++)
    {
        if(ap_list[i].channel_id!= 0)
		{
            if((!ssv6xxx_wifi_support_5g_band())&&IS_5G_BAND(ap_list[i].channel_id))
                continue;
            OS_MemCPY((void*)wifi_ap_list->ap_info[j].bssid,(void*)ap_list[i].bssid.addr, SSV_MAC_ADDR_LEN);
            OS_MemCPY((void*)wifi_ap_list->ap_info[j].ssid,(void*)ap_list[i].ssid.ssid,ap_list[i].ssid.ssid_len);
            wifi_ap_list->ap_info[j].ssid[ap_list[i].ssid.ssid_len] = 0;
            wifi_ap_list->ap_info[j].channel = ap_list[i].channel_id;
            if (ap_list[i].capab_info&BIT(4))
            {
                if (ap_list[i].proto & WPA_PROTO_WPA)
                {
                    wifi_ap_list->ap_info[j].security = KEY_WPA;
                }
                else if (ap_list[i].proto & WPA_PROTO_RSN)
                {
                    wifi_ap_list->ap_info[j].security = KEY_WPA2;
                }
                else
                {
                    wifi_ap_list->ap_info[j].security = KEY_WEP;
                }
            }
            else
            {
                wifi_ap_list->ap_info[j].security = KEY_NONE;
            }

            if ((connected_info.vif_operate[0]==SSV6XXX_HWM_STA)&&
              (!ssv6xxx_memcmp((void *)ap_list[i].bssid.addr,(void *)connected_info.vif_u[0].station_status.apinfo.Mac,ETHER_ADDR_LEN)))
            {
                    wifi_ap_list->ap_info[j].rssi = ssv6xxx_get_rssi_by_mac((ssv_type_u8 *)ap_list[i].bssid.addr,0);
            }
            else if((connected_info.vif_operate[1]==SSV6XXX_HWM_STA)&&
                   (!ssv6xxx_memcmp((void *)ap_list[i].bssid.addr,(void *)connected_info.vif_u[1].station_status.apinfo.Mac,ETHER_ADDR_LEN)))
            {
                    wifi_ap_list->ap_info[j].rssi = ssv6xxx_get_rssi_by_mac((ssv_type_u8 *)ap_list[i].bssid.addr,1);
            }
            else
            {
                wifi_ap_list->ap_info[j].rssi = ap_list[i].rxphypad.rssi;
            }
            j++;
    	}
    }

    wifi_ap_list->ap_count = j;

    OS_MemFree((void *)ap_list);
    return;
}

int ssv_wifi_scan(ssv_type_u32 channel_mask, ssv_type_u32 channel_mask_5g, ssv_wifi_ap_list_t *ap_list)
{
    #ifdef SSV_WIFI_EVENT_ENABLE
    ssv_type_u32 start_tick = 0;
    ssv_type_bool timeout = false;
    ssv_wifi_scan_status = 1;
    #endif
    netmgr_wifi_scan_ex_async(channel_mask, channel_mask_5g, 0, 0);
    #ifdef SSV_WIFI_EVENT_ENABLE
    start_tick = OS_GetSysTick();
    timeout = false;
    while (ssv_wifi_scan_status == 1)
    {
        // wait 5 second timeout
        if (((OS_GetSysTick() - start_tick) * OS_TICK_RATE_MS) >=10000)
        {
            timeout = true;
            break;
        }
        SSV_OS_TickDelay(1);
    }

    if (timeout)
    {
        LOG_PRINTF("ssv_wifi_scan timeout\r\n");
        return -1;
    }
    #else
    OS_MsDelay(10000);
    #endif

    if (ap_list)
    {
        ssv_wifi_get_ap_list(ap_list);
    }

    return 0;
}

int ssv_wifi_scan_ext(ssv_wifi_ap_list_t *ap_list)
{
    ssv_wifi_scan(SCAN_ALL_2G_CHANNEL, SCAN_ALL_5G_CHANNEL, ap_list);
    return 0;
}
int ssv_wifi_scan_ssid_atcmd(char* ssid)
{
	ssv_wifi_ap_list_t *ap_list;
	ap_list = OS_MemAlloc(sizeof(ssv_wifi_ap_list_t));
    if(ap_list == NULL){
        LOG_PRINTF("alloc scan buffer fail\r\n");
        return -1;
    }
	LOG_PRINTF("\33[32m  ssv_wifi_scan_ssid_atcmd  ssid:%s \33[0m\r\n",ssid);
    ssv_wifi_scan_ssid_ext(ssid, ap_list);
    OS_MemFree(ap_list);
    return 0;
}

int ssv_wifi_scan_ssid(ssv_type_u32 channel_mask, ssv_type_u32 channel_mask_5g, char* ssid ,ssv_wifi_ap_list_t *ap_list)
{
    char* scan_ssid[1];
	//ssv_wifi_ap_list_t *ap_list;
    #ifdef SSV_WIFI_EVENT_ENABLE
    ssv_type_u32 start_tick = 0;
    ssv_type_bool timeout = false;
    ssv_wifi_scan_status = 1;
    #endif
	LOG_PRINTF("ssv_wifi_scan_ssid(): ssid:%s\r\n",ssid);
	/*
	ap_list = OS_MemAlloc(sizeof(ssv_wifi_ap_list_t));
    	if(ap_list == NULL){
        LOG_PRINTF("alloc scan buffer fail\r\n");
        return -1;
    }
    */
    scan_ssid[0] = ssid;
    netmgr_wifi_scan_ex_async(channel_mask, channel_mask_5g, scan_ssid, 1);
    #ifdef SSV_WIFI_EVENT_ENABLE
    start_tick = OS_GetSysTick();
    timeout = false;
    while (ssv_wifi_scan_status == 1)
    {
        // wait 5 second timeout
        if (((OS_GetSysTick() - start_tick) * OS_TICK_RATE_MS) >=10000)
        {
            timeout = true;
            break;
        }
        SSV_OS_TickDelay(1);
    }

    if (timeout)
    {
        LOG_PRINTF("ssv_wifi_scan timeout\r\n");
        return -1;
    }
    #else
    OS_MsDelay(10000);
    #endif

    if (ap_list)
    {
        ssv_wifi_get_ap_list(ap_list);
    }

    return 0;
}

int ssv_wifi_scan_ssid_ext(char* ssid, ssv_wifi_ap_list_t *ap_list)
{
	LOG_PRINTF("\33[32m  ssv_wifi_scan_ssid_ext  ssid:%s \33[0m\r\n",ssid);
    ssv_wifi_scan_ssid(SCAN_ALL_2G_CHANNEL, SCAN_ALL_5G_CHANNEL,ssid, ap_list);
    return 0;
}

int ssv_wifi_scan_network(char* scan_buf, unsigned int buf_size)
{
    #if 0
    ssv_wifi_ap_list_t *ap_list = NULL;
    WLAN_SCAN_RESULT *list = (WLAN_SCAN_RESULT *)scan_buf;
    int i = 0;
    //LOG_PRINTF("ssv_wifi_scan_network\r\n");

    ap_list = OS_MemAlloc(sizeof(ssv_wifi_ap_list_t));
    if(ap_list == NULL){
        LOG_PRINTF("alloc scan buffer fail\r\n");
        return -1;
    }
    ssv_wifi_scan_ext(ap_list);
    list->count = ap_list->ap_count;

    for (i = 0;i < list->count; i++)
    {
        list->bss_info[i].length = sizeof(WLAN_BSS_INFO);
        MEMCPY(list->bss_info[i].BSSID, ap_list->ap_info[i].bssid, 6);
        list->bss_info[i].chanspec =  ap_list->ap_info[i].channel;
        list->bss_info[i].RSSI =  ap_list->ap_info[i].rssi;
        ssv6xxx_strcpy((char *)list->bss_info[i].SSID, (char *)ap_list->ap_info[i].ssid);
        list->bss_info[i].SSID_len =  (unsigned char)ssv6xxx_strlen((char *)(list->bss_info[i].SSID));
        //LOG_PRINTF("list->bss_info[i].SSID = %s, list->bss_info[i].SSID_len=%d\r\n", list->bss_info[i].SSID, list->bss_info[i].SSID_len);
        list->buflen += list->bss_info[i].length;
        list->bss_info[i].capability |= WLAN_DOT11_CAP_ESS;
        if (ap_list->ap_info[i].security != KEY_NONE)
        {
            list->bss_info[i].capability |= WLAN_DOT11_CAP_PRIVACY;
        }
    }

    OS_MemFree(ap_list);
    #endif

    //LOG_PRINTF("ssv_wifi_scan_network list->count = %d\r\n", list->count);
    return 0;
}


static ssv_type_bool netdev_wifi_link_status = 0;

void ssv_wifi_link_callback_link_up(void *dat)
{
	LOG_PRINTF("netdev_link_callback_link_up\r\n");
    netdev_wifi_link_status = 1;
}

void ssv_wifi_link_callback_link_down(void *dat)
{
	LOG_PRINTF("netdev_link_callback_link_down\r\n");
    netdev_wifi_link_status = 0;
}

static int ssv_wifi_link_get_status(void)
{
    if (ssv_wifi_get_mode() == WIFI_MODE_AP)
    {
        return 0;
    }
    return netdev_wifi_link_status;
}

#ifdef SSV_WIFI_EVENT_ENABLE

static void ssv_wifi_scan_done_handler (void *data)
{
    ssv_type_u8 *result_code = (ssv_type_u8 *)data;
    if(*result_code==0)
    {

    }
    else
    {

    }
    return;
}

static void ssv_wifi_join_result_handler (void *data)
{
    ssv_type_u8 *join_res = (ssv_type_u8 *)data;
    if (*join_res != 0)
    {
        LOG_PRINTF("Join failure!!\r\n");
        return;
    }

    LOG_PRINTF("Join success!!\r\n");

}

static void ssv_wifi_leave_result_handler (void *data)
{
    struct resp_evt_result *leave_res = (struct resp_evt_result *)data;
    LOG_PRINTF("Leave received deauth from AP!!, Reason Code=%d\r\n", leave_res->u.leave.reason_code);
}
static void ssv_wifi_host_event_handler(ssv_type_u32 evt_id, void *data, ssv_type_s32 len, void* priv)
{
    ssv_wifi_event_data enData;

    switch (evt_id) {
    case EVT_SCAN_DONE:
        ssv_wifi_scan_status = 0;
        ssv_wifi_scan_done_handler(data);
        if (g_ssv_wifi_event_cb)
        {
            g_ssv_wifi_event_cb(SCAN_DONE_EVENT, &enData);
        }
        break;
    case EVT_JOIN_RESULT:
        ssv_wifi_join_result_handler(data);
        if (g_ssv_wifi_event_cb)
        {
            ssv_type_u8 *join_res = (ssv_type_u8 *)data;
            if (*join_res != 0)
            {
                g_ssv_wifi_event_cb(JOIN_FAIL_EVENT, &enData);
            }
            else
            {
                g_ssv_wifi_event_cb(JOIN_SUCCESS_EVENT, &enData);
            }
        }
        break;
    case EVT_LEAVE_RESULT:
        ssv_wifi_leave_result_handler(data);
        if (g_ssv_wifi_event_cb)
        {
             g_ssv_wifi_event_cb(LEAVE_RESULT_EVENT, &enData);
        }
        break;
     case EVT_STA_STATUS: // Station add or remove
        {
            struct cfg_set_sta *cfg_sta =data;

            if(cfg_sta->sta_oper == CFG_STA_ADD)
            {
                LOG_PRINTF("ADD station, MAC:[%02x:%02x:%02x:%02x:%02x:%02x] \r\n",
                cfg_sta->wsid_info.addr.addr[0],
                cfg_sta->wsid_info.addr.addr[1],
                cfg_sta->wsid_info.addr.addr[2],
                cfg_sta->wsid_info.addr.addr[3],
                cfg_sta->wsid_info.addr.addr[4],
                cfg_sta->wsid_info.addr.addr[5] );

                if (g_ssv_wifi_event_cb)
                {
                    enData.data.auth_deauth_data.reserved[0] = cfg_sta->wsid_info.addr.addr[0];
                    enData.data.auth_deauth_data.reserved[1] = cfg_sta->wsid_info.addr.addr[1];
                    enData.data.auth_deauth_data.reserved[2] = cfg_sta->wsid_info.addr.addr[2];
                    enData.data.auth_deauth_data.reserved[3] = cfg_sta->wsid_info.addr.addr[3];
                    enData.data.auth_deauth_data.reserved[4] = cfg_sta->wsid_info.addr.addr[4];
                    enData.data.auth_deauth_data.reserved[5] = cfg_sta->wsid_info.addr.addr[5];
                    g_ssv_wifi_event_cb(ASSOC_IND_EVENT, &enData);
                }
            }
            else if(cfg_sta->sta_oper == CFG_STA_DEL)
            {
                LOG_PRINTF("DEL station, MAC:[%02x:%02x:%02x:%02x:%02x:%02x] \r\n",
                cfg_sta->wsid_info.addr.addr[0],
                cfg_sta->wsid_info.addr.addr[1],
                cfg_sta->wsid_info.addr.addr[2],
                cfg_sta->wsid_info.addr.addr[3],
                cfg_sta->wsid_info.addr.addr[4],
                cfg_sta->wsid_info.addr.addr[5] );
                if (g_ssv_wifi_event_cb)
                {
                    enData.data.auth_deauth_data.reserved[0] = cfg_sta->wsid_info.addr.addr[0];
                    enData.data.auth_deauth_data.reserved[1] = cfg_sta->wsid_info.addr.addr[1];
                    enData.data.auth_deauth_data.reserved[2] = cfg_sta->wsid_info.addr.addr[2];
                    enData.data.auth_deauth_data.reserved[3] = cfg_sta->wsid_info.addr.addr[3];
                    enData.data.auth_deauth_data.reserved[4] = cfg_sta->wsid_info.addr.addr[4];
                    enData.data.auth_deauth_data.reserved[5] = cfg_sta->wsid_info.addr.addr[5];
                    g_ssv_wifi_event_cb(DISASSOC_IND_EVENT, &enData);
                }
            }

            break;
        }


    default:
        break;
    }
}
#endif

//extern ssv_type_u32 usb_if_get_free_urb_cnt(void);

extern ssv_type_u32 ssv6xxx_wifi_get_tx_cnt(void);

int ssv_wifi_get_avail_tx_queue_count(void)
{
    #if 1
    //int usb_cnt = usb_if_get_free_urb_cnt();
    int tx_queue_cnt = ssv6xxx_wifi_get_tx_cnt();
    if ((POOL_SIZE - tx_queue_cnt) < 8)
    {
	    //LOG_PRINTF("tx_queue_cnt:%d, usb_cnt=%d\r\n", tx_queue_cnt, usb_if_get_free_urb_cnt());
    }

    return  (POOL_SIZE - tx_queue_cnt);
    #else
    return 100;
    #endif
}

extern int ssv6xxx_dev_init(ssv6xxx_hw_mode hmode);
extern void ssv6xxx_dev_deinit(void);
/**
 * @brief initializing wifi
 * @author
 * @date
 * @param [in] pParam a pointer to T_WIFI_INITPARAM type
 * @return int
 * @retval   0  initializing sucessful
 * @retval  -1 initializing fail
 */
int ssv_wifi_init_mode(int mode)
{
	int ret;

	LOG_PRINTF("Wifilib version:%s\r\n", ssv_wifi_get_version());

    if (g_wifi_init == 1)
    {
        LOG_PRINTF("ssv_wifi_init already init\r\n");
        return 0;
    }
    
	if (mode == 0)
    {
        g_wifi_vif_idx = 0;
	    ret = ssv6xxx_dev_init(SSV6XXX_HWM_STA);
    }
    else
    {
        g_wifi_vif_idx = 1;
        ret = ssv6xxx_dev_init(SSV6XXX_HWM_AP);
    }
	if (ret != 0)
	{
        g_wifi_vif_idx = -1;
		LOG_PRINTF("ssv6xxx_dev_init faild\r\n");
		return -1;
	}

    ssv6xxx_wifi_reg_evt_cb(ssv_wifi_host_event_handler,NULL);

    if (mode == 0)
    {
        if (ssv6xxx_check_ready(SSV6XXX_HWM_STA) != SSV6XXX_SUCCESS)
        {
            LOG_PRINTF("ssv_wifi_init_mode: STA mode timeout\r\n");
            return SSV6XXX_FAILED;
        }
        else
        {
            LOG_PRINTF("ssv_wifi_init_mode: STA mode OK\r\n");
        }
    }
    else
    {
        if (ssv6xxx_check_ready(SSV6XXX_HWM_AP) != SSV6XXX_SUCCESS)
        {
            LOG_PRINTF("ssv_wifi_init_mode: AP mode timeout\r\n");
            return SSV6XXX_FAILED;
        }
        else
        {
            LOG_PRINTF("ssv_wifi_init_mode: AP mode OK\r\n");
        }
    }
    
    //_ssv_wifi_set_eth_mac_config();
    g_wifi_init = 1;
    
	LOG_PRINTF("wifi init ok\r\n");
	return ret;
}


void ssv_wifi_event_register(ssv_wifi_event_cb cb)
{
    g_ssv_wifi_event_cb = (ssv_wifi_event_cb)cb;
}

int ssv_wifi_open(int mode)
{
    // int ret = 0;
    // int index = 0, i = 0;
    ssv_wifi_init_mode(mode);
    LOG_PRINTF("ssv_wifi_open success\r\n");
    return 0;
}

int  ssv_wifi_close(void)
{
	ssv6xxx_dev_deinit();
    g_wifi_vif_idx = -1;
    g_wifi_init = 0;
    LOG_PRINTF("ssv_wifi_close success\r\n");
    return 0;
}

static wifi_event_handle m_scan_result_handle = NULL;
static wifi_event_handle m_scan_done_handle = NULL;
static wifi_event_handle m_join_success_handle = NULL;
static wifi_event_handle m_join_fail_handle = NULL;
static wifi_event_handle m_leave_handle = NULL;

void ssv_wifi_event_callback(SSV_WIFI_EVENT enEvent, ssv_wifi_event_data *enData)
{
    switch(enEvent)
    {
        case SCAN_RESULT_EVENT:
            {
                //MLOGE("func:%s, SCAN_RESULT_EVENT received\n",__FUNCTION__);
                if (m_scan_result_handle)
                {
                    m_scan_result_handle(enData);
                }
                break;
            }
        case SCAN_DONE_EVENT:
            {
                if (m_scan_done_handle)
                {
                    m_scan_done_handle(enData);
                }
                #if 0
                if (1)
                {
                    wlan_event_msg_t event;
                    //make a fake wlan event
                    event.event_type = WLAN_E_SCAN_COMPLETE;
                    WLAN_SYS_StatusCallback(&event);
                }
                #endif
                break;
            }
        case JOIN_SUCCESS_EVENT:
            {
                //char mac[6];
                if (m_join_success_handle)
                {
                    m_join_success_handle(enData);
                }
                ssv_wifi_link_callback_link_up(NULL);
                //wifi_adap_report_sta_connect_status("jinsongtest", mac, 0);
             //   wifi_adap_report_dhcpc_start(netdev_get_wifi_netid(0), OS_Task_GetSelf(0), 0000);
                #if 0
                if (1)
                {
                    wlan_event_msg_t event;
                    //make a fake wlan event
                    event.event_type = WLAN_E_LINK;
                    event.flags = 1;
                    WLAN_SYS_StatusCallback(&event);
                }
                #endif
                break;
            }
        case JOIN_FAIL_EVENT:
            {
                //char mac[6];
                if (m_join_fail_handle)
                {
                    m_join_fail_handle(enData);
                }
                ssv_wifi_link_callback_link_down(NULL);
              //  wifi_adap_report_sta_connect_status("jinsongtest", mac, 1);
                #if 0
                if (1)
                {
                    struct resp_evt_result *join_res = (struct resp_evt_result *)enData;
                    wlan_event_msg_t event;
                    event.event_type = WLAN_E_LINK;
                    event.flags = 0;
                    event.reason = join_res->u.join.status_code;
                    WLAN_SYS_StatusCallback(&event);
                }
                #endif
                break;
            }
        case LEAVE_RESULT_EVENT:
            {
                if (m_leave_handle)
                {
                    m_leave_handle(enData);
                }
                ssv_wifi_link_callback_link_down(NULL);
                //wifi_adap_report_sta_disconnect("jinsongtest");
                #if 0
                if (1)
                {
                    struct resp_evt_result *leave_res = (struct resp_evt_result *)enData;
                    wlan_event_msg_t event;
                    event.event_type = WLAN_E_LINK;
                    event.flags = 0;
                    event.reason = leave_res->u.leave.reason_code;
                    WLAN_SYS_StatusCallback(&event);
                }

                #endif
                break;
            }
        case ASSOC_IND_EVENT:
            {
                #if 0
                if (1)
                {
                    wlan_event_msg_t event;
                    //make a fake wlan event
                    event.event_type = WLAN_E_ASSOC_IND;
                    event.addr.mac[0] = enData->data.auth_deauth_data.reserved[0];
                    event.addr.mac[1] = enData->data.auth_deauth_data.reserved[1];
                    event.addr.mac[2] = enData->data.auth_deauth_data.reserved[2];
                    event.addr.mac[3] = enData->data.auth_deauth_data.reserved[3];
                    event.addr.mac[4] = enData->data.auth_deauth_data.reserved[4];
                    event.addr.mac[5] = enData->data.auth_deauth_data.reserved[5];
                    WLAN_SYS_StatusCallback(&event);
                }
                #endif
                break;
            }
        case DISASSOC_IND_EVENT:
            {
                netdev_wifi_process_disassoc_ind_event(enData);
                break;
            }
        default:
            {
            }
    }
}

void ssv_register_netif()
{
    unsigned char ssv_mac_addr[6] = {0};
    int vif_index = 0;
    for (vif_index = 0; vif_index < MAX_VIF_NUM; vif_index++)
    {
        ssv_wifi_get_mac_2(ssv_mac_addr, vif_index);
        LOG_PRINTF("ssv_register_netif: ssv_mac_addr[%d]=%02x:%02x:%02x:%02x:%02x:%02x\r\n", vif_index, ssv_mac_addr[0], ssv_mac_addr[1], ssv_mac_addr[2], 
             ssv_mac_addr[3],  ssv_mac_addr[4],  ssv_mac_addr[5]);
        netdev_register_net_dev(ssv_mac_addr, (vif_index == 0) ? 0 : 1); //sentton
    }
}

void ssv_deregister_netif()
{
    int vif_index = 0;
    for (vif_index = 0; vif_index < MAX_VIF_NUM; vif_index++)
    {
        LOG_PRINTF("ssv_deregister_netif\r\n", vif_index);
        netdev_unregister_net_dev(vif_index);
    }  
    //netdev_wifi_evt_stack_pm(0);
}
extern void ssv_wifi_init(void);
extern void ssv_wifi_deinit(void);
extern void ssv_wifi_sleep(uint32 sleep_flag);

void ssv_wifi_init(void)
{
    //int vif_index = 0;
    //unsigned char ssv_mac_addr[6] = {0};
    LOG_PRINTF("ssv_wifi_init\r\n");
    if (g_wifi_init == 1)
    {
        LOG_PRINTF("ssv_wifi_init already init\r\n");
        return;
    }

    //init sta or ap mode
    ssv_wifi_open(1);

    g_wifi_init = 1;

    ssv_wifi_event_register(ssv_wifi_event_callback);
    ssv_register_netif();


    LOG_PRINTF("ssv_wifi_init ok\r\n");
    
}

void ssv_wifi_deinit(void)
{
	LOG_PRINTF("ssv_wifi_deinit\r\n");
    if (g_wifi_init == 1)
    {
       ssv_wifi_close();

       ssv_wifi_event_register(NULL);
    }

    g_wifi_init = 0;

    LOG_PRINTF("ssv_wifi_deinit ok\r\n");
}
void ssv_wifi_sleep(uint32 mSleep)
{
	log_printf("ssv_wifi_sleep\r\n");
	//ssv_enable_sleep(mSleep);
}

void ssv_wifi_reset(void)
{
    // unsigned int status = 0;
    ssv_wifi_deinit();
    //status = OS_EnterCritical();
    LOG_PRINTF("ssv_wifi_reset ok\r\n");
}

int ssv_wifi_get_connected_info(WLAN_CONNECTION_INFO *wlan_connection_info)
{
    Ap_sta_status connected_info;

    ssv6xxx_wifi_status(&connected_info);
    if((SSV6XXX_HWM_STA==connected_info.vif_operate[0]))
    {
        OS_MemCPY((void*)wlan_connection_info->Ssid,(void*)connected_info.vif_u[0].station_status.ssid.ssid,connected_info.vif_u[0].station_status.ssid.ssid_len);
        wlan_connection_info->Ssid_len = connected_info.vif_u[0].station_status.ssid.ssid_len;
        wlan_connection_info->channel = connected_info.vif_u[0].station_status.channel;
        if(ssv_is_valid_ether_addr(connected_info.vif_u[0].station_status.apinfo.Mac)){
            wlan_connection_info->Rssi = ssv6xxx_get_rssi_by_mac(connected_info.vif_u[0].station_status.apinfo.Mac, 0);
        }
        else{
           wlan_connection_info->Rssi = 0;
        }
    }

    return 0;
}

void* ssv_wifi_vif_get(void)
{
    //return &g_wifi_init;
    return &g_wifi_vif_idx;
}

void ssv_wifi_tx_pkt(void *p, int len)
{
    netstack_output((void *)&g_wifi_vif_idx, (void *)p, len);
	return;
}

void ssv_wifi_rx_pkt(void *p, int len)
{
}

int ssv_check_ready(void)
{
	Ap_sta_status info;

	if(1 != g_wifi_init)
	{
		return -1;
	}

	ssv6xxx_wifi_status(&info);
	if(info.status)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int ssv_wifi_ap_acl_set_type(int type)
{
    ssv6xxx_wifi_set_ap_acl_type(type);
    return 0;
}

int ssv_wifi_ap_acl_add_mac(unsigned char *macaddr)
{
    ssv6xxx_wifi_add_ap_acl(macaddr);
    return 0;
}

int ssv_wifi_ap_acl_del_mac(unsigned char *macaddr)
{
    ssv6xxx_wifi_del_ap_acl(macaddr);
    return 0;
}

int ssv_wifi_ap_acl_get(int *type,  ssv_type_u16 *mac_count, ssv_type_u8 mac_list[][6])
{
    AP_SETTING_ACL_TYPE ap_acl_type;
    AP_SETTING_ACL_MAC acl_mac_list[AP_ACL_MAX_COUNT];
    ssv_type_u16 ap_acl_mac_count;
    int i = 0;
    
    ssv6xxx_wifi_get_ap_acl(&ap_acl_type, &acl_mac_list[0], &ap_acl_mac_count);
    if (type)
    {
        *type = ap_acl_type;
    }

    if (mac_count)
    {
        *mac_count = ap_acl_mac_count;
    }

    if (mac_list)
    {
        for (i = 0; i < AP_ACL_MAX_COUNT;i ++)
        {
            memcpy(mac_list[i], acl_mac_list[i].mac, 6);
        }
    }
    
    return 0;
}

extern ssv_type_s32 netmgr_wifi_wps_pbc(ssv_type_bool bEnrollee);
extern void MlmeEAPServerTimeoutHandler(void);

void ssv_wifi_ap_mode_open_wps()
{
    netmgr_wifi_wps_pbc(true);
}

void ssv_wifi_ap_mode_close_wps()
{
    if (TRUE == netmgr_wifi_check_ap()) 
    {
        MlmeEAPServerTimeoutHandler();
    }
}

void ssv_wifi_enter_ps()
{
    int powermode = 4;
    ssv_type_u32 ip = 0;
    ssv_type_u8 powerstatus = 0;
    ssv_type_u8 times = 0;
    LOG_PRINTF("ssv_wifi_enter_ps\r\n");
    if (!netmgr_wifi_check_sta()) {
        LOG_PRINTF("AP mode cannot support lowpower\r\n");
        return;
    }
    
    ip = netdev_wifi_get_ipaddr(0);
    LOG_PRINTF("STA powermode %d, ipaddr 0x%08x\r\n", powermode, ip);
    netmgr_wifi_suspend(netdev_wifi_get_ipaddr(0), powermode); 
    do
    {
        ssv6xxx_wifi_get_pwr_suspend(&powerstatus);
        if (powerstatus == 1)
        {
            break;
        }
        times++;
        OS_Sleep(10);
    }while (times < 100);
    
    netdev_wifi_evt_stack_pm(0);
    
    LOG_PRINTF("ssv_wifi_enter_ps powerstatus(%d) exit\r\n", powerstatus);
}

void ssv_wifi_exit_ps()
{
    ssv_type_u8 powerstatus = 0;
    ssv_type_u8 times = 0;
    
    LOG_PRINTF("ssv_wifi_exit_ps\r\n");
    if (!netmgr_wifi_check_sta()) {
        LOG_PRINTF("AP mode cannot support lowpower\r\n");
        return;
    }
    
    netmgr_wifi_resume();
    do
    {
        ssv6xxx_wifi_get_pwr_suspend(&powerstatus);
        if (powerstatus == 0)
        {
            break;
        }
        times++;
        OS_Sleep(10);
    }while (times < 100);
    
    LOG_PRINTF("ssv_wifi_exit_ps powerstatus(%d) exit\r\n", powerstatus);
}

extern ssv_type_u32 cmdeng_task_count;
extern ssv_type_u32 txTask_count;
extern ssv_type_u32 rxTask_count;
extern ssv_type_u32 timerTask_count;
extern ssv_type_u32 sdio_host_isr_count;

extern ssv_type_s32 rxTask_HOST_EVENT_count;
extern ssv_type_s32 rxTask_MT_RUNNING_count1;
extern ssv_type_s32 rxTask_MT_RUNNING_count2;
extern ssv_type_s32 rxTask_package_count;

extern ssv_type_s32 RXTask_RxFrame_count;

void ssv_wifi_taskcount()
{
	LOG_PRINTF("ssv_wifi_taskcount debug\r\n");
	LOG_PRINTF("cmdeng_task_count: %d\r\n",cmdeng_task_count);
	LOG_PRINTF("tx_task_count: %d\r\n",txTask_count);
	LOG_PRINTF("rx_task_count: %d\r\n",rxTask_count);
	LOG_PRINTF("timerTask_count: %d\r\n",timerTask_count);
	LOG_PRINTF("sdio_host_isr_count: %d\r\n",sdio_host_isr_count);
	
	LOG_PRINTF("rxTask_HOST_EVENT_count: %d\r\n",rxTask_HOST_EVENT_count);
	LOG_PRINTF("rxTask_MT_RUNNING_count1: %d\r\n",rxTask_MT_RUNNING_count1);
	LOG_PRINTF("rxTask_MT_RUNNING_count2: %d\r\n",rxTask_MT_RUNNING_count2);
	LOG_PRINTF("rxTask_package_count: %d\r\n",rxTask_package_count);
	LOG_PRINTF("RXTask_RxFrame_count: %d\r\n",RXTask_RxFrame_count);
	
}


