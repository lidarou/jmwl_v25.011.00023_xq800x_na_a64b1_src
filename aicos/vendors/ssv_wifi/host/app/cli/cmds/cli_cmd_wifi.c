/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x19ace0
#define LOG_MODULE    AMOD_VENDOR

#include <cmd_def.h>
#include <log.h>
#include <rtos.h>
#include <ssv_lib.h>
#include <netmgr/net_mgr.h>
#include <ssv_devinfo.h>
#include <ssv_ether.h>
#include <ssv_frame.h>
#include <ssv_timer.h>
#include <ssv_drv.h>

#if (CLI_ENABLE==1)
#include <cli.h>
#include "cli_cmd.h"
#include "cli_cmd_wifi.h"
#include "cli_cmd_net.h"

#include <netstack.h>

#define WPA_AUTH_ALG_OPEN BIT(0)
#define WPA_AUTH_ALG_SHARED BIT(1)
#define WPA_AUTH_ALG_LEAP BIT(2)
#define WPA_AUTH_ALG_FT BIT(3)

//#define SEC_USE_NONE
//#define SEC_USE_WEP40_PSK
//#define SEC_USE_WEP40_OPEN
//#define SEC_USE_WEP104_PSK
//#define SEC_USE_WEP104_OPEN
//#define SEC_USE_WPA_TKIP
#define SEC_USE_WPA2_CCMP


// iw command executioner
void _cmd_wifi_scan (ssv_type_s32 argc, char *argv[]);
void _sconfig_usage(void);
void _cmd_wifi_sconfig (ssv_type_s32 argc, char *argv[]);
static void _cmd_wifi_join (ssv_type_s32 argc, char *argv[]);
void _cmd_wifi_join_other (ssv_type_s32 argc, char *argv[]);
static void _cmd_wifi_leave (ssv_type_s32 argc, char *argv[]);
static void _cmd_wifi_list (ssv_type_s32 argc, char *argv[]);
#if (AP_MODE_ENABLE == 1)
void _cmd_wifi_ap (ssv_type_s32 argc, char *argv[]);
extern void APStaInfo_PrintStaInfo(ssv_type_u32 idx);
#endif

// CB for host event
// Individual event handler
//static void _get_soc_reg_response(ssv_type_u32 eid, void *data, ssv_type_s32 len);
void _soc_evt_handler_ssv6xxx_log(ssv_type_u32 eid, void *data, ssv_type_s32 len);
#ifndef CONFIG_NO_WPA2
extern void ssv6xxx_eapol_data_reg_cb(void);
#endif
typedef struct ba_session_st {
	ssv_type_u16 tid;
	ssv_type_u16 buf_size;
	ssv_type_u16 policy;
	ssv_type_u16 timeout;
} BA_SESSION ;

ssv_type_bool g_cli_joining=false;
//static BA_SESSION g_ba_session;

#define ON_OFF_LAG_INTERVAL 1000


/*===================== Start of Get Command Handlers ====================*/


void _soc_evt_get_soc_status(void *data)
{
    ssv_type_u8 i;
    char *sta_status[]={ "STA_STATE_UNAUTH_UNASSOC",/*STA_STATE_UNAUTH_UNASSOC*/
                        "STA_STATE_AUTHED_UNASSOC",/*STA_STATE_AUTHED_UNASSOC*/
                        "STA_STATE_AUTHED_ASSOCED",/*STA_STATE_AUTHED_ASSOCED*/
                        "STA_STATE_ASSOCED_4way",/*STA_STATE_ASSOCED_4way*/
                        };
    char *sta_action[]={
                        "STA_ACTION_INIT",/*STA_ACTION_INIT*/
                        "STA_ACTION_IDLE",/*STA_ACTION_IDLE*/
                        "STA_ACTION_READY",/*STA_ACTION_READY*/
                        "STA_ACTION_RUNNING",/*STA_ACTION_RUNNING*/
                        "STA_ACTION_SCANING",/*STA_ACTION_SCANING*/
                        "STA_ACTION_JOING",/*STA_ACTION_JOING*/
                        "STA_ACTION_JOING_4WAY",/*STA_ACTION_JOING_4WAY*/
                        "STA_ACTION_LEAVING" /*STA_ACTION_LEAVING*/
                        };

    struct ST_SOC_STATUS{
        ssv_type_u8 u8vif_State[MAX_VIF_NUM];
        ssv_type_u32 u32SocAction;
    }*ps1=NULL;

    ps1=(struct ST_SOC_STATUS *)data;
    //LOG_PRINTF("u8SocState=%d, u32SocAction=%d\r\n",ps1->u8SocState,ps1->u32SocAction);
    for(i=0;i<MAX_VIF_NUM;i++)
        LOG_PRINTF("\n  >> vif[%d] status:%s\r\n",i,sta_status[ps1->u8vif_State[i]]);
    LOG_PRINTF("\n  >> soc action:%s\r\n",sta_action[ps1->u32SocAction]);
}

/* ====================== End of Get Command Handlers ====================*/



//-------------------------------------------------------------------------------------
extern void cmd_loop_pattern(void);

extern struct task_info_st g_cli_task_info[];
#if 0
void _get_soc_reg_response(ssv_type_u32 eid, void *data, ssv_type_s32 len)
{
    LOG_PRINTF("%s(): HOST_EVENT=%d: len=%d\n", __FUNCTION__, eid, len);
//    memcpy((void *)g_soc_cmd_rx_buffer, (void *)data, len);
//    g_soc_cmd_rx_ready = 1;
}
#endif
extern void ssv_cmd_ifconfig(ssv_type_s32 argc, char *argv[]);
void cli_ps_resume(void)
{
    ssv_cmd_ifconfig(1, NULL);
}

void cmd_iw(ssv_type_s32 argc, char *argv[])
{
//    ssv6xxx_wifi_reg_evt_cb(_host_event_handler);
//gHCmdEngInfo

	if (argc<2)
		return;

    if (ssv6xxx_strcmp(argv[1], "scan")==0) {
        if (argc >= 2)
		    _cmd_wifi_scan(argc - 2, &argv[2]);
        else
            LOG_PRINTF("Invalid arguments.\n");
	} else if (ssv6xxx_strcmp(argv[1], "join")==0) {
        if (argc >= 3)
            _cmd_wifi_join(argc - 2, &argv[2]);
        else
            LOG_PRINTF("Invalid arguments.\n");
    }  else if (ssv6xxx_strcmp(argv[1], "join-other")==0) {
        if (argc >= 3)
            _cmd_wifi_join_other(argc - 2, &argv[2]);
        else
            LOG_PRINTF("Invalid arguments.\n");
    } else if (ssv6xxx_strcmp(argv[1], "leave")==0) {
        if (argc<4)
            _cmd_wifi_leave(argc - 2, &argv[2]);
        else
            LOG_PRINTF("Invalid arguments.\n");
    } else if (ssv6xxx_strcmp(argv[1], "list")==0) {
        if (argc == 2)
        {
            _cmd_wifi_list(argc - 2, &argv[2]);
        }
        else
            LOG_PRINTF("Invalid arguments.\n");
        /*
    } else if (ssv6xxx_strcmp(argv[1], "ap")==0) {
        if (argc >= 3)
            _cmd_wifi_ap(argc - 2, &argv[2]);
		else
            LOG_PRINTF("Invalid arguments.\n");
        */
    }else if (ssv6xxx_strcmp(argv[1], "reg")==0){
        ssv6xxx_wifi_show_reg_list();
    }else if (ssv6xxx_strcmp(argv[1], "greg")==0){
        char reg[3]={0};
        ssv6xxx_wifi_get_reg(reg,sizeof(reg));
        LOG_PRINTF("current reg=%c%c\r\n",reg[0],reg[1]);
    }else if (ssv6xxx_strcmp(argv[1], "sreg")==0){
        if(0==ssv6xxx_wifi_set_reg(argv[2]))
        {
            LOG_PRINTF("set reg=%c%c\r\n",argv[2][0],argv[2][1]);
        }
    }else if (ssv6xxx_strcmp(argv[1], "clist")==0){
        ssv6xxx_wifi_show_available_channel();
    }else {
        LOG_PRINTF("Invalid iw command.\n");
    }
} // end of - cmd_iw -



void _cmd_wifi_scan (ssv_type_s32 argc, char *argv[])
{
    /**
     *  Scan Command Usage:
     *  iw scan <chan_mask> <chan_5g_mask>
     */
    ssv_type_u16 channel_mask = (ssv_type_u16)strtol(argv[0],NULL,16);
    ssv_type_u32 channel_5g_mask = (ssv_type_u32)strtol(argv[1],NULL,16);

    if(argc!=0)
    {
        if(argc==1)
        {
            //only scan 2.4G band
            netmgr_wifi_scan_async(channel_mask, 0, 0);
        }
        else
        {
            //scan 2.4G and 5G band
            netmgr_wifi_scan_ex_async(channel_mask, channel_5g_mask, 0, 0);
        }
    }
    else
    {
        //scan 2.4G and 5G band(if 5G band is enabled, and chip support 5G band)
        netmgr_wifi_scan_ex_async(SCAN_ALL_2G_CHANNEL, SCAN_ALL_5G_CHANNEL, 0, 0);    
    }
    return;
} // end of - _cmd_wifi_scan -


//iw join ap_name passwd vif 0/1
void _cmd_wifi_join (ssv_type_s32 argc, char *argv[])
{
    /**
	 * Join Command Usage:
	 *	iw join ssid
     */
	int ret = -1;
    wifi_sta_join_cfg *join_cfg = NULL;
    ssv_type_s32 arg_cnt=0;
    //ssv_type_u8 vif_idx;

    join_cfg = (wifi_sta_join_cfg *)OS_MemAlloc(sizeof(wifi_sta_join_cfg));
    if(NULL==join_cfg)
    {
        LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
        return;
    }
    OS_MemSET((void *)join_cfg,0,sizeof(wifi_sta_join_cfg));

    if (argc > arg_cnt)
    {
        ssv6xxx_strcpy((char *)join_cfg->ssid.ssid, argv[arg_cnt]);
        join_cfg->ssid.ssid_len=ssv6xxx_strlen(argv[arg_cnt]);
        if(argc > (arg_cnt+1))
        {
            arg_cnt++;
            if(0==ssv6xxx_strcmp(argv[arg_cnt],"vif"))
            {
                arg_cnt++;                
            }
            else
            {
                ssv6xxx_strcpy((char *)join_cfg->password, argv[arg_cnt]);
                if(argc > (arg_cnt+1))
                {
                    arg_cnt++;
                    if(0==ssv6xxx_strcmp(argv[arg_cnt],"vif"))
                    {
                        arg_cnt++;
                    }

                }
            }
        }
    }
    else
    {
        OS_MemFree(join_cfg);
        return;
    }

    ret = netmgr_wifi_join_async(join_cfg);
    if (ret != 0)
    {
	    LOG_PRINTF("netmgr_wifi_join_async failed !!\r\n");
    }

    OS_MemFree(join_cfg);
} // end of - _cmd_wifi_join -

void _cmd_wifi_join_other (ssv_type_s32 argc, char *argv[])
{
    /**
	 * Join Command Usage:
	 *	iw join ssid
     */
	int ret = -1;
    wifi_sta_join_cfg *join_cfg = NULL;
    ssv_type_s32 arg_cnt=0;

    join_cfg = (wifi_sta_join_cfg *)OS_MemAlloc(sizeof(wifi_sta_join_cfg));
    if(NULL==join_cfg)
    {
        LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
        return;
    }
    OS_MemSET((void *)join_cfg,0,sizeof(wifi_sta_join_cfg));

    if (argc > arg_cnt)
    {
        ssv6xxx_strcpy((char *)join_cfg->ssid.ssid, argv[arg_cnt]);
        join_cfg->ssid.ssid_len=ssv6xxx_strlen(argv[arg_cnt]);
        if(argc > (arg_cnt+1))
        {
            arg_cnt++;
            if(0==ssv6xxx_strcmp(argv[arg_cnt],"vif"))
            {
                arg_cnt++;
            }
            else
            {
                ssv6xxx_strcpy((char *)join_cfg->password, argv[arg_cnt]);
                if(argc > (arg_cnt+1))
                {
                    arg_cnt++;
                    if(0==ssv6xxx_strcmp(argv[arg_cnt],"vif"))
                    {
                        arg_cnt++;
                    }

                }
            }
        }
    }
    else
    {
        OS_MemFree(join_cfg);
        return;
    }

    ret = netmgr_wifi_join_other_async(join_cfg);
    if (ret != 0)
    {
	    LOG_PRINTF("netmgr_wifi_join_other_async failed !!\r\n");
    }

    OS_MemFree(join_cfg);
} // end of - _cmd_wifi_join -

void _cmd_wifi_leave(ssv_type_s32 argc, char *argv[])
{
    /**
	 *	Leave Command Usage:
	 *	host leave ... ...
	 */
	int ret = -1;

    LOG_PRINTF("_cmd_wifi_leave\r\n");
    ret = netmgr_wifi_leave_async();
    if (ret != 0)
    {
	    LOG_PRINTF("netmgr_wifi_leave failed !!\r\n");
    }
} // end of - _cmd_wifi_leave -

void _cmd_wifi_list(ssv_type_s32 argc, char *argv[])
{
    ssv_type_u32 i=0,AP_cnt;
    ssv_type_s32     pairwise_cipher_index=0,group_cipher_index=0;
    ssv_type_u8      sec_str[][7]={"OPEN","WEP40","WEP104","TKIP","CCMP"};
    ssv_type_u8      ch_offset_str[][10]={"No_HT","HT20","HT40MINUS","HT40PLUS"};
    ssv_type_u8  ssid_buf[MAX_SSID_LEN+1]={0};
    Ap_sta_status connected_info;

    struct ssv6xxx_ieee80211_bss *ap_list = NULL;
    AP_cnt = ssv6xxx_get_aplist_info((void **)&ap_list);

    OS_MemSET(&connected_info , 0 , sizeof(Ap_sta_status));
    ssv6xxx_wifi_status(&connected_info);

    if((ap_list==NULL) || (AP_cnt==0))
    {
        LOG_PRINTF("AP list empty!\r\n");
        return;
    }
    for (i=0; i<AP_cnt; i++)
    {
        if(ap_list[i].channel_id!= 0)
		{
            if((!ssv6xxx_wifi_support_5g_band())&&IS_5G_BAND(ap_list[i].channel_id))
                continue;
            
		    LOG_PRINTF("BSSID: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
            ap_list[i].bssid.addr[0],  ap_list[i].bssid.addr[1], ap_list[i].bssid.addr[2],  ap_list[i].bssid.addr[3],  ap_list[i].bssid.addr[4],  ap_list[i].bssid.addr[5]);
            OS_MemSET((void*)ssid_buf,0,sizeof(ssid_buf));
            OS_MemCPY((void*)ssid_buf,(void*)ap_list[i].ssid.ssid,ap_list[i].ssid.ssid_len);
            LOG_PRINTF("SSID: %s\t", ssid_buf);
			LOG_PRINTF("@Channel Idx: %d  %s\r\n", ap_list[i].channel_id,ch_offset_str[ap_list[i].channel_offset]);
            if(ap_list[i].capab_info&BIT(4)){
                LOG_PRINTF("Secure Type=[%s]\r\n",
                ap_list[i].proto&WPA_PROTO_WPA?"WPA":
                ap_list[i].proto&WPA_PROTO_RSN?"WPA2":"WEP");

                if(ap_list[i].pairwise_cipher[0]){
                    pairwise_cipher_index=0;
                    LOG_PRINTF("Pairwise cipher=");
                    for(pairwise_cipher_index=0;pairwise_cipher_index<8;pairwise_cipher_index++){
                        if(ap_list[i].pairwise_cipher[0]&BIT(pairwise_cipher_index)){
                            LOG_PRINTF("[%s] ",sec_str[pairwise_cipher_index]);
                        }
                    }
                    LOG_PRINTF("\r\n");
                }
                if(ap_list[i].group_cipher){
                    group_cipher_index=0;
                    LOG_PRINTF("Group cipher=");
                    for(group_cipher_index=0;group_cipher_index<8;group_cipher_index++){
                        if(ap_list[i].group_cipher&BIT(group_cipher_index)){
                            LOG_PRINTF("[%s] ",sec_str[group_cipher_index]);
                        }
                    }
                    LOG_PRINTF("\r\n");
                }
            }else{
                LOG_PRINTF("Secure Type=[NONE]\r\n");
            }
          
            if((connected_info.vif_operate[0]==SSV6XXX_HWM_STA)&&
              (!ssv6xxx_memcmp((void *)ap_list[i].bssid.addr,(void *)connected_info.vif_u[0].station_status.apinfo.Mac,ETHER_ADDR_LEN)))
            {
                    LOG_PRINTF("RSSI=-%d (dBm)\r\n",ssv6xxx_get_rssi_by_mac((ssv_type_u8 *)ap_list[i].bssid.addr,0));
            }
            else if((connected_info.vif_operate[1]==SSV6XXX_HWM_STA)&&
                   (!ssv6xxx_memcmp((void *)ap_list[i].bssid.addr,(void *)connected_info.vif_u[1].station_status.apinfo.Mac,ETHER_ADDR_LEN)))
            {
                    LOG_PRINTF("RSSI=-%d (dBm)\r\n",ssv6xxx_get_rssi_by_mac((ssv_type_u8 *)ap_list[i].bssid.addr,1));
            }
            else
            {
                LOG_PRINTF("RSSI=-%d (dBm)\r\n",ap_list[i].rxphypad.rssi);
            }
#if(ENABLE_WPS ==1)
            if(ap_list[i].WPSPBCActive){
                LOG_PRINTF("WPS PBC is active\r\n");
            }
#endif
            LOG_PRINTF("\r\n");
		}

    }
    OS_MemFree((void *)ap_list);
} // end of - _cmd_wifi_list -



#if (AP_MODE_ENABLE == 1)

// iw ap ssid sectype password
// iw ap ssv  password
void _cmd_wifi_ap (ssv_type_s32 argc, char *argv[])
{
    /**
	 * Join Command Usage:
	 *	iw ap ssid
     */
    const char *sec_name;
    ssv_type_u8 ssid_buf[MAX_SSID_LEN+1]={0};
	struct cfg_set_ap_cfg ApCfg;
    OS_MemSET(&ApCfg, 0, sizeof(struct cfg_set_ap_cfg));

//Fill SSID
    ApCfg.ssid.ssid_len = ssv6xxx_strlen(argv[0]);
    OS_MemCPY( (void*)&ApCfg.ssid.ssid, (void*)argv[0], ssv6xxx_strlen(argv[0]));

//Fill PASSWD
	if (argc == 3)
		OS_MemCPY(&ApCfg.password, argv[2], ssv6xxx_strlen(argv[2]));


    if (argc == 1){
        sec_name = "open";
        ApCfg.sec_type = SSV6XXX_SEC_NONE;
    }
	else if (ssv6xxx_strcmp(argv[1], "wep40") == 0){
        sec_name = "wep40";
    	ApCfg.sec_type = SSV6XXX_SEC_WEP_40;
		if (argc != 3)
		{
	        ApCfg.password[0]= 0x31;
			ApCfg.password[1]= 0x32;
    		ApCfg.password[2]= 0x33;
	    	ApCfg.password[3]= 0x34;
			ApCfg.password[4]= 0x35;
    		ApCfg.password[5]= '\0';
		}


	}
	else if (ssv6xxx_strcmp(argv[1], "wep104") == 0){
			sec_name = "wep104";

			ApCfg.sec_type = SSV6XXX_SEC_WEP_104;
			if (argc != 3)
			{
				ApCfg.password[0]= '0';
				ApCfg.password[1]= '1';
				ApCfg.password[2]= '2';
				ApCfg.password[3]= '3';
				ApCfg.password[4]= '4';
				ApCfg.password[5]= '5';
				ApCfg.password[6]= '6';
				ApCfg.password[7]= '7';
				ApCfg.password[8]= '8';
				ApCfg.password[9]= '9';
				ApCfg.password[10]= '0';
				ApCfg.password[11]= '1';
				ApCfg.password[12]= '2';
				ApCfg.password[13]= '\0';
			}
    }
#ifndef CONFIG_NO_WPA2
	else if (ssv6xxx_strcmp(argv[1], "wpa2") == 0){
        sec_name = "wpa2";
	  	ApCfg.sec_type = SSV6XXX_SEC_WPA2_PSK;

		if (argc != 3)
		{
	        ApCfg.password[0]= 's';
		    ApCfg.password[1]= 'e';
	       	ApCfg.password[2]= 'c';
		    ApCfg.password[3]= 'r';
			ApCfg.password[4]= 'e';
			ApCfg.password[5]= 't';
	        ApCfg.password[6]= '0';
		    ApCfg.password[7]= '0';
			ApCfg.password[8]= '\0';
		}

    }
#endif
	else{
        LOG_PRINTF("ERROR: unkown security type: %s\n", argv[1]);
        sec_name = "open";
        //ApCfg.auth_alg = WPA_AUTH_ALG_OPEN;
        ApCfg.sec_type = SSV6XXX_SEC_NONE;
    }

    OS_MemCPY((void*)ssid_buf,(void*)ApCfg.ssid.ssid,ApCfg.ssid.ssid_len);
    LOG_PRINTF("AP configuration==>\nSSID:\"%s\" \nSEC Type:\"%s\" \nPASSWD:\"%s\"\n",
		ssid_buf, sec_name, ApCfg.password);



    if (ssv6xxx_send_msg_to_hcmd_eng(CMD_SET_AP_CFG, 0, &ApCfg, sizeof(ApCfg),FALSE,TRUE) < 0)
	    LOG_PRINTF("Command failed !!\n");


} // end of - _cmd_wifi_join -
#endif

void ssv6xxx_wifi_cfg(void)
{
    //ssv6xxx_wifi_reg_evt_cb(_host_event_handler,NULL);
#ifndef CONFIG_NO_WPA2
#if (AP_MODE_ENABLE == 1)
    ssv6xxx_eapol_data_reg_cb();
#endif
#endif
}

void cmd_rf(ssv_type_s32 argc, char *argv[])
{
    ssv_type_u32 regval = 0;
    ssv_type_s32 ch = 0;
    enum ssv_80211_channel_type ch_type;
    struct ssv_rf_tool_param rf_tool_param;

    unsigned char rate_tbl[] = {
        0x00,0x01,0x02,0x03,                        // B mode long preamble [0~3]
        0x00,0x12,0x13,                             // B mode short preamble [4~6], no 2M short preamble
        0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,    // G mode [7~14]
        0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,    // N mode HT20 long GI mixed format [15~22]
        0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,    // N mode HT20 short GI mixed format  [23~30]
        0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,    // N mode HT40 long GI mixed format [31~38]
        0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,    // N mode HT40 short GI mixed format  [39~46]
    };
#define SIZE_RATE_TBL   (sizeof(rate_tbl) / sizeof((rate_tbl)[0]))
    
    LOG_PRINTF(" cmd_rf %s\r\n", argv[1]);
    if (argc < 2)
        goto out;

    if (!ssv6xxx_strcmp(argv[1], "tx")) {

        if (argc == 3)
            regval = ssv6xxx_atoi(argv[2]);
        else
            regval = 0;

        OS_MemSET((void *)&rf_tool_param, 0x0, sizeof(struct ssv_rf_tool_param));
        rf_tool_param.count = 0xffffffff;
        rf_tool_param.interval = regval;

        if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&rf_tool_param, (int)sizeof(struct ssv_rf_tool_param), SSV6XXX_HOST_CMD_RFPHY_OPS, SSV6XXX_RFPHY_CMD_RF_TOOL_TX, FALSE))
        {
            LOG_PRINTF("\r\n   RF TX\r\n");
        } else{
            LOG_PRINTF("\r\n send host cmd fail\r\n");
        }
    
       return;

    } else if (!ssv6xxx_strcmp(argv[1], "rx")) {
        #if 0
        sc->sc_flags |= SC_OP_BLOCK_CNTL;
        sc->sc_flags |= SC_OP_CHAN_FIXED; // fixed channel
        #endif
        LOG_PRINTF("\r\n   rf rx\r\n");
        return ;

    } else if (!ssv6xxx_strcmp(argv[1], "rxreset")) {
        if (argc == 2) {
            OS_MemSET((void *)&rf_tool_param, 0x0, sizeof(struct ssv_rf_tool_param));
            rf_tool_param.rx_reset = 1;
            
            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&rf_tool_param, (int)sizeof(struct ssv_rf_tool_param), SSV6XXX_HOST_CMD_RFPHY_OPS, SSV6XXX_RFPHY_CMD_RF_TOOL_RX_RESET, FALSE))
            {            
                LOG_PRINTF("\r\n   RF RX Reset\r\n");
            } else {
                LOG_PRINTF("\r\n send host cmd fail\r\n");
            }
        }
        return;
        
    } else if(!ssv6xxx_strcmp(argv[1], "stop")) {
        OS_MemSET((void *)&rf_tool_param, 0x0, sizeof(struct ssv_rf_tool_param));
                           
        if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&rf_tool_param, (int)sizeof(struct ssv_rf_tool_param), SSV6XXX_HOST_CMD_RFPHY_OPS, SSV6XXX_RFPHY_CMD_RF_TOOL_STOP, FALSE))
        {
            LOG_PRINTF("\r\n   RF STOP\r\n");
        } else{
            LOG_PRINTF("\r\n send host cmd fail\r\n");
        }
        return;
    
    } else if (!ssv6xxx_strcmp(argv[1], "ch")) {
        
        if ((argc == 3) || (argc == 4)) {
            ch = ssv6xxx_atoi(argv[2]);
            ch_type = SSV_80211_CHAN_HT20;

            if (argc == 4) {
                if (!ssv6xxx_strcmp(argv[3], "bw40")) {
                    if ((ch == 3) || (ch == 4) || (ch == 5) || (ch == 6) ||
                        (ch == 7) || (ch == 8) || (ch == 9) || (ch == 10) ||
                        (ch == 11)) {
                    
                        ch_type = SSV_80211_CHAN_HT40PLUS;
                        ch = ch - 2; // find center chan
                    }
                }
            }
            OS_MemSET((void *)&rf_tool_param, 0x0, sizeof(struct ssv_rf_tool_param));
            rf_tool_param.ch = ch;
            rf_tool_param.ch_type = ch_type;                                       
            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&rf_tool_param, (int)sizeof(struct ssv_rf_tool_param), SSV6XXX_HOST_CMD_RFPHY_OPS, SSV6XXX_RFPHY_CMD_RF_TOOL_CH, FALSE))
            {

                LOG_PRINTF("\r\n RF ch %d, ch_type %d\r\n", ch, (int)ch_type);
            } else {
                LOG_PRINTF("\r\n send host cmd fail\r\n");
            }
        }
        return;

    } else if(!ssv6xxx_strcmp(argv[1], "rate")){
        if (argc == 3){
            regval = ssv6xxx_atoi(argv[2]);
            if ((regval != 4) && (regval < SIZE_RATE_TBL)) {
                OS_MemSET((void *)&rf_tool_param, 0x0, sizeof(struct ssv_rf_tool_param));
                rf_tool_param.rate = regval;
                if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&rf_tool_param, (int)sizeof(struct ssv_rf_tool_param), SSV6XXX_HOST_CMD_RFPHY_OPS, SSV6XXX_RFPHY_CMD_RF_TOOL_RATE, FALSE))
                {
                    LOG_PRINTF("\r\n   rf rate index %d\r\n", regval);
                } else{
                    LOG_PRINTF("\r\n send host cmd fail\r\n");
				}

            } else {
                LOG_PRINTF("Not support rf rate index %d\r\n", regval);
            }
            return;

        } else {
            LOG_PRINTF("\r\n\t Incorrect rf rate set format\r\n");
            return;
        }

    } else if(!ssv6xxx_strcmp(argv[1], "disablethermal")){
        if (argc == 3){
            regval = ssv6xxx_atoi(argv[2]);
            regval = (0 == regval) ? 0 : 1;
            OS_MemSET((void *)&rf_tool_param, 0x0, sizeof(struct ssv_rf_tool_param));
            rf_tool_param.disable_thermal = regval;            
            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&rf_tool_param, (int)sizeof(struct ssv_rf_tool_param), SSV6XXX_HOST_CMD_RFPHY_OPS, SSV6XXX_RFPHY_CMD_RF_TOOL_THERMAL, FALSE))
            {
                LOG_PRINTF("\r\n   rf thermal %d\r\n", regval);
            } else {
                LOG_PRINTF("\r\n send host cmd fail\r\n");
		    }	

            return;

        } else {
            LOG_PRINTF("\r\n\t Incorrect rf rate set format\r\n");
            return;
        }
    
    } else if(!ssv6xxx_strcmp(argv[1], "freq")){
        if (argc == 3){
            regval = ssv6xxx_atoi(argv[2]);
            OS_MemSET((void *)&rf_tool_param, 0x0, sizeof(struct ssv_rf_tool_param));
            rf_tool_param.freq = regval;
            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&rf_tool_param, (int)sizeof(struct ssv_rf_tool_param), SSV6XXX_HOST_CMD_RFPHY_OPS, SSV6XXX_RFPHY_CMD_RF_TOOL_FREQ, FALSE))
            {
                LOG_PRINTF("Set cbanki/cbanko to 0x%x\r\n", regval);
            } else
                LOG_PRINTF("\r\n send host cmd fail\r\n");
        } else{
            LOG_PRINTF("./cli rf freq [value]\r\n");
        }
        return;

    } else if(!ssv6xxx_strcmp(argv[1], "rfreq")){
        if (argc == 2){
            ssv_type_u32 CBANKI=0,CBANKO;
            CBANKI=CBANKO=ssv6xxx_drv_read_reg(0xccb0b000);
            LOG_PRINTF("Get freq 0x%x/0x%x\r\n", (CBANKI&0x0000ff00>>8), (CBANKO&0x00ff0000>>16));
        } else {
            LOG_PRINTF("./cli rf rfreq\r\n");
        }
        return;

    } else if(!ssv6xxx_strcmp(argv[1], "dcdc")){
        if (argc == 3){
            regval = ssv6xxx_atoi(argv[2]);
            regval = (regval == 0) ? 0 : 1;
            OS_MemSET((void *)&rf_tool_param, 0x0, sizeof(struct ssv_rf_tool_param));
            rf_tool_param.dcdc = regval;
            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&rf_tool_param, (int)sizeof(struct ssv_rf_tool_param), SSV6XXX_HOST_CMD_RFPHY_OPS, SSV6XXX_RFPHY_CMD_RF_TOOL_DCDC, FALSE))
            {
                LOG_PRINTF("Set dcdc to 0x%x\r\n", regval);
            } else
                LOG_PRINTF("\r\n send host cmd fail\r\n");
        } else{
            LOG_PRINTF("./cli rf dcdc [value]\r\n");
        }
        return;

    } else if(!ssv6xxx_strcmp(argv[1], "dacgain")){
        if (argc == 3){
            regval = ssv6xxx_atoi(argv[2]);
            if (regval > 15)
                regval = 15;
            OS_MemSET((void *)&rf_tool_param, 0x0, sizeof(struct ssv_rf_tool_param));
            rf_tool_param.dcdc = regval;
            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&rf_tool_param, (int)sizeof(struct ssv_rf_tool_param), SSV6XXX_HOST_CMD_RFPHY_OPS, SSV6XXX_RFPHY_CMD_RF_TOOL_DACGAIN, FALSE))
            {
                rf_tool_param.dacgain = regval;
                LOG_PRINTF("Set dgcgain to 0x%x\n", regval);
            } else
                LOG_PRINTF("\r\n send host cmd fail\r\n");
        } else{
            LOG_PRINTF("./cli rf dacgain [value]\n");
        }
        return;
    
    } else if(!ssv6xxx_strcmp(argv[1], "ratebgain")){
        if (argc == 3){
            regval = ssv6xxx_atoi(argv[2]);
            if (regval > 15)
                regval = 15;
            OS_MemSET((void *)&rf_tool_param, 0x0, sizeof(struct ssv_rf_tool_param));
            rf_tool_param.ratebgain = regval;
            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&rf_tool_param, (int)sizeof(struct ssv_rf_tool_param), SSV6XXX_HOST_CMD_RFPHY_OPS, SSV6XXX_RFPHY_CMD_RF_TOOL_RATEBGAIN, FALSE))
            {
                LOG_PRINTF("Set rate b gain to 0x%x\r\n", regval);
            } else
                LOG_PRINTF("\r\n send host cmd fail\r\n");
        } else{
            LOG_PRINTF("./cli rf ratebgain [value]\r\n");
        }
        return;
    
    } else if(!ssv6xxx_strcmp(argv[1], "padpd")){
        if (argc == 3){
            regval = ssv6xxx_atoi(argv[2]);
            regval = (regval == 0) ? 0 : 3;
            OS_MemSET((void *)&rf_tool_param, 0x0, sizeof(struct ssv_rf_tool_param));
            rf_tool_param.padpd = regval;
            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&rf_tool_param, (int)sizeof(struct ssv_rf_tool_param), SSV6XXX_HOST_CMD_RFPHY_OPS, SSV6XXX_RFPHY_CMD_RF_TOOL_PADPD, FALSE))
            {
                LOG_PRINTF("Set padpd to 0x%x\r\n", regval);
            } else
                LOG_PRINTF("\r\n send host cmd fail\r\n");
        } else{
            LOG_PRINTF("./cli rf padpd [value]\r\n");
        }
        return;
    } else if(!ssv6xxx_strcmp(argv[1], "count")){
        if  (argc == 3) {
            OS_MemSET((void *)&rf_tool_param, 0x0, sizeof(struct ssv_rf_tool_param));
            if ((!ssv6xxx_strcmp(argv[2], "0")) || (!ssv6xxx_strcmp(argv[2], "2") ))
                rf_tool_param.rx_mib = 1;
            else
                rf_tool_param.rx_mib = 0;            

            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&rf_tool_param, (int)sizeof(struct ssv_rf_tool_param), SSV6XXX_HOST_CMD_RFPHY_OPS, SSV6XXX_RFPHY_CMD_RF_TOOL_RX_MIB, FALSE))
            {
                // show result
                //LOG_PRINTF("count = %d\r\n", sc->rf_tool_rx_count);
                //LOG_PRINTF("err = %d\r\n", sc->rf_tool_rx_err_count);
                return;
            } else {
                LOG_PRINTF("\r\n send host cmd fail\r\n");
                return;
            }
        }
        LOG_PRINTF("\r\n\t./cli rf count 0|1\r\n");
        return;
    
    } else if(!ssv6xxx_strcmp(argv[1], "tonegen")){
        if (argc == 3){
            regval = ssv6xxx_atoi(argv[2]);
            OS_MemSET((void *)&rf_tool_param, 0x0, sizeof(struct ssv_rf_tool_param));
            rf_tool_param.tone_gen = regval;

            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&rf_tool_param, (int)sizeof(struct ssv_rf_tool_param), SSV6XXX_HOST_CMD_RFPHY_OPS, SSV6XXX_RFPHY_CMD_RF_TONE_GEN, FALSE))
            {
                LOG_PRINTF("Set tone gen to 0x%x\r\n", regval);
            } else
                LOG_PRINTF("\r\n send host cmd fail\r\n");
        } else{
            LOG_PRINTF("./cli rf tonegen [value]\r\n");
        }
        return;

    } else if(!ssv6xxx_strcmp(argv[1], "phy_txgen")) {
        OS_MemSET((void *)&rf_tool_param, 0x0, sizeof(struct ssv_rf_tool_param));
       	rf_tool_param.count = 0xffffffff;
	    rf_tool_param.interval = 1;
        if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&rf_tool_param, (int)sizeof(struct ssv_rf_tool_param), SSV6XXX_HOST_CMD_RFPHY_OPS, SSV6XXX_RFPHY_CMD_RF_TOOL_TX, FALSE))
        {
            #if 0
            ssv6020_cmd_turismoE_rfphy_ops_start(sc, 1);

            sc->sc_flags |= SC_OP_BLOCK_CNTL;
            sc->sc_flags |= SC_OP_CHAN_FIXED; // fixed channel
            #endif
            LOG_PRINTF("\r\n   RF TX\r\n");
        }
        else
        {
            LOG_PRINTF("\r\n send host cmd fail\r\n");        
        }
        return;
	}else if(!ssv6xxx_strcmp(argv[1], "block")){
    
        #if 0
		sc->sc_flags |= SC_OP_BLOCK_CNTL;
        sc->sc_flags |= SC_OP_CHAN_FIXED;
        #endif
        LOG_PRINTF("\r\n\t block control form system\r\n");
        return;
    } else if(!ssv6xxx_strcmp(argv[1], "unblock")){
        OS_MemSET((void *)&rf_tool_param, 0x0, sizeof(struct ssv_rf_tool_param));
        if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&rf_tool_param, (int)sizeof(struct ssv_rf_tool_param), SSV6XXX_HOST_CMD_RFPHY_OPS, SSV6XXX_RFPHY_CMD_RF_TOOL_STOP, FALSE))
        {
            LOG_PRINTF("\r\n\t unblock control form system\r\n");        
        }
        else
        {
            LOG_PRINTF("\r\n send host cmd fail\r\n");        
        }
        #if 0
   		sc->sc_flags &= ~SC_OP_BLOCK_CNTL;
        sc->sc_flags &= ~SC_OP_CHAN_FIXED;
        ssv6020_cmd_turismoE_rfphy_ops_stop(sc);
        #endif    
        return;
	}else {
        LOG_PRINTF("\r\n\t./cli rf phy_txgen|block|unblock|count|freq|rfreq|rate|ch|thermal|stop|rxreset|rx|tx\r\n");
        return;
    }
out:

    LOG_PRINTF("\r\n\t Current RF tool settings: ch %d\r\n", ch);
#if 0
    if (sc->sc_flags && SC_OP_BLOCK_CNTL) {
        snprintf_res(cmd_data,"\t system control is blocked\n");
    } else {
        snprintf_res(cmd_data,"\t WARNING system control is not blocked\n");
    }
#endif
}

void cmd_efuse(ssv_type_s32 argc, char *argv[])
{
    //ssv_type_u32 regval = 0;
    struct ssv_efuse_tool_param efuse_tool_param;
    int i  = 0, value1 = 0, value2 = 0;
    unsigned char mac[6];
    
    LOG_PRINTF(" cmd_rf %s\r\n", argv[1]);
    if (argc < 2)
        return;

    if (!ssv6xxx_strcmp(argv[1], "writepsk")) {
        if (argc == 3)
        {
            if (ssv6xxx_strlen(argv[2]) > 32) {
                LOG_PRINTF("write psk range 1 ~ 32\r\n");
                return;
            }

            OS_MemSET((void *)&efuse_tool_param, 0x0, sizeof(struct ssv_efuse_tool_param));
            efuse_tool_param.psk_len = ssv6xxx_strlen(argv[2]);
            for (i= 0; i < efuse_tool_param.psk_len; i++)
                efuse_tool_param.psk[i] = argv[2][i];

            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&efuse_tool_param, (int)sizeof(struct ssv_efuse_tool_param), SSV6XXX_HOST_CMD_EFUSE_OPS, SSV6XXX_EFUSE_CMD_WRITE_PSK, TRUE))
            {
                LOG_PRINTF("\r\nwrite psk = %s, len %d\r\n", argv[2], ssv6xxx_strlen(argv[2]));
            } else{
                LOG_PRINTF("\r\n send host cmd fail\r\n");
            }
           return;
        }
    }else if (!ssv6xxx_strcmp(argv[1], "readpsk")) {
        if (argc == 2)
        {
            OS_MemSET((void *)&efuse_tool_param, 0x0, sizeof(struct ssv_efuse_tool_param));
            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&efuse_tool_param, (int)sizeof(struct ssv_efuse_tool_param), SSV6XXX_HOST_CMD_EFUSE_OPS, SSV6XXX_EFUSE_CMD_READ_PSK, FALSE))
            {
                LOG_PRINTF("\r\npsk = %s, len %d\r\n", gDeviceInfo->efuse_psk, gDeviceInfo->efuse_psk_len);
            } else{
                LOG_PRINTF("\r\n send host cmd fail\r\n");
            }
           return;
        }
        else {
            LOG_PRINTF("Incorrect efuse command format\r\n");
        }
    }else if (!ssv6xxx_strcmp(argv[1], "readchipid")) {
        if (argc == 2)
        {
            OS_MemSET((void *)&efuse_tool_param, 0x0, sizeof(struct ssv_efuse_tool_param));
            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&efuse_tool_param, (int)sizeof(struct ssv_efuse_tool_param), SSV6XXX_HOST_CMD_EFUSE_OPS, SSV6XXX_EFUSE_CMD_READ_CHIPID, FALSE))
            {
                LOG_PRINTF("\r\nchip_id = 0x%08x\r\n", gDeviceInfo->efuse_chip_id);
            } else{
                LOG_PRINTF("\r\n send host cmd fail\r\n");
            }
           return;
        }
        else {
            LOG_PRINTF("Incorrect efuse command format\r\n");
        }
    }else if (!ssv6xxx_strcmp(argv[1], "readmac")) {
        if (argc == 2)
        {
            OS_MemSET((void *)&efuse_tool_param, 0x0, sizeof(struct ssv_efuse_tool_param));
            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&efuse_tool_param, (int)sizeof(struct ssv_efuse_tool_param), SSV6XXX_HOST_CMD_EFUSE_OPS, SSV6XXX_EFUSE_CMD_READ_MAC, FALSE))
            {
                LOG_PRINTF("\r\nefuse mac = %02X:%02X:%02X:%02X:%02X:%02X\r\n", gDeviceInfo->efuse_mac[0], gDeviceInfo->efuse_mac[1],
                    gDeviceInfo->efuse_mac[2],gDeviceInfo->efuse_mac[3],gDeviceInfo->efuse_mac[4],gDeviceInfo->efuse_mac[5]);
            } else{
                LOG_PRINTF("\r\n send host cmd fail\r\n");
            }
           return;
        }
        else {
            LOG_PRINTF("Incorrect efuse command format\r\n");
        }
    }else if (!ssv6xxx_strcmp(argv[1], "writemac")) {

        if (argc == 4)
        {
            if (ssv6xxx_strlen(argv[2]) > 32) {
                LOG_PRINTF("write psk range 1 ~ 32\r\n");
                return;
            }
            
            value1 = sscanf(argv[3], "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX", 
                &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
            
            if (6 != value1) {
                LOG_PRINTF("write incorrect mac format\r\n");
                return;
            }

            OS_MemSET((void *)&efuse_tool_param, 0x0, sizeof(struct ssv_efuse_tool_param));
            OS_MemCPY(efuse_tool_param.mac, mac, 6);
            efuse_tool_param.psk_len = ssv6xxx_strlen(argv[2]);
            for (i= 0; i < efuse_tool_param.psk_len; i++)
                efuse_tool_param.psk[i] = argv[2][i];

            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&efuse_tool_param, (int)sizeof(struct ssv_efuse_tool_param), SSV6XXX_HOST_CMD_EFUSE_OPS, SSV6XXX_EFUSE_CMD_WRITE_MAC, TRUE))
            {
                LOG_PRINTF("\r\nwrite efuse mac = %02X:%02X:%02X:%02X:%02X:%02X\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            } else{
                LOG_PRINTF("\r\n send host cmd fail\r\n");
            }
           return;
        }
        else {
            LOG_PRINTF("Incorrect efuse command format\r\n");
        }
    }else if (!ssv6xxx_strcmp(argv[1], "readtxpower1")) {
        if (argc == 2)
        {
            OS_MemSET((void *)&efuse_tool_param, 0x0, sizeof(struct ssv_efuse_tool_param));
            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&efuse_tool_param, (int)sizeof(struct ssv_efuse_tool_param), SSV6XXX_HOST_CMD_EFUSE_OPS, SSV6XXX_EFUSE_CMD_READ_TX_POWER1, FALSE))
            {
                LOG_PRINTF("\r\nefuse txpower1 = %d\r\n", gDeviceInfo->efuse_txpower1);
            } else{
                LOG_PRINTF("\r\n send host cmd fail\r\n");
            }
           return;
        }
        else {
            LOG_PRINTF("Incorrect efuse command format\r\n");
        }
    }else if (!ssv6xxx_strcmp(argv[1], "writetxpower1")) {

        if (argc == 5)
        {
            if (ssv6xxx_strlen(argv[2]) > 32) {
                LOG_PRINTF("write psk range 1 ~ 32\r\n");
                return;
            }
            
            value1 = ssv6xxx_atoi(argv[3]);
            if ((value1 < 1) || (value1 > 15)) {
                LOG_PRINTF("txpower1 gain2000 range 1 ~ 15\r\n");
                return;
            }
            
            value2 = ssv6xxx_atoi(argv[4]);
            if ((value2 < 1) || (value2 > 15)) {
                LOG_PRINTF("txpower1 gain5200 range 1 ~ 15\r\n");
                return;
            }

            OS_MemSET((void *)&efuse_tool_param, 0x0, sizeof(struct ssv_efuse_tool_param));
            efuse_tool_param.txpower1_gain2000 = value1;
            efuse_tool_param.txpower1_gain5200 = value2;
            efuse_tool_param.psk_len = ssv6xxx_strlen(argv[2]);
            for (i= 0; i < efuse_tool_param.psk_len; i++)
                efuse_tool_param.psk[i] = argv[2][i];

            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&efuse_tool_param, (int)sizeof(struct ssv_efuse_tool_param), SSV6XXX_HOST_CMD_EFUSE_OPS, SSV6XXX_EFUSE_CMD_WRITE_TX_POWER1, TRUE))
            {
                LOG_PRINTF("\r\nwrite efuse mac = %02X:%02X:%02X:%02X:%02X:%02X\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            } else{
                LOG_PRINTF("\r\n send host cmd fail\r\n");
            }
           return;
        }
        else {
            LOG_PRINTF("Incorrect efuse command format\r\n");
        }
    }else if (!ssv6xxx_strcmp(argv[1], "readxtal")) {
        if (argc == 2)
        {
            OS_MemSET((void *)&efuse_tool_param, 0x0, sizeof(struct ssv_efuse_tool_param));
            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&efuse_tool_param, (int)sizeof(struct ssv_efuse_tool_param), SSV6XXX_HOST_CMD_EFUSE_OPS, SSV6XXX_EFUSE_CMD_READ_XTAL, FALSE))
            {
                LOG_PRINTF("\r\nefuse xtal = %d\r\n", gDeviceInfo->efuse_xtal);
            } else{
                LOG_PRINTF("\r\n send host cmd fail\r\n");
            }
           return;
        }
        else {
            LOG_PRINTF("Incorrect efuse command format\r\n");
        }
    }else if (!ssv6xxx_strcmp(argv[1], "writextal")) {

        if (argc == 4)
        {
            if (ssv6xxx_strlen(argv[2]) > 32) {
                LOG_PRINTF("write psk range 1 ~ 32\r\n");
                return;
            }
            
            value1 = ssv6xxx_atoi(argv[3]);
            if ((value1 < 1) || (value1 > 255)) {
                LOG_PRINTF("xtal range 0 ~ 255\r\n");
                return;
            }
            
            OS_MemSET((void *)&efuse_tool_param, 0x0, sizeof(struct ssv_efuse_tool_param));
            efuse_tool_param.xtal = value1;
            efuse_tool_param.psk_len = ssv6xxx_strlen(argv[2]);
            for (i= 0; i < efuse_tool_param.psk_len; i++)
                efuse_tool_param.psk[i] = argv[2][i];

            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&efuse_tool_param, (int)sizeof(struct ssv_efuse_tool_param), SSV6XXX_HOST_CMD_EFUSE_OPS, SSV6XXX_EFUSE_CMD_WRITE_XTAL, TRUE))
            {
                LOG_PRINTF("\r\nwrite efuse xtal = %d\r\n", value1);
            } else{
                LOG_PRINTF("\r\n send host cmd fail\r\n");
            }
           return;
        }
        else {
            LOG_PRINTF("Incorrect efuse command format\r\n");
        }
    }else if (!ssv6xxx_strcmp(argv[1], "readrategainbn40")) {
        if (argc == 2)
        {
            OS_MemSET((void *)&efuse_tool_param, 0x0, sizeof(struct ssv_efuse_tool_param));
            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&efuse_tool_param, (int)sizeof(struct ssv_efuse_tool_param), SSV6XXX_HOST_CMD_EFUSE_OPS, SSV6XXX_EFUSE_CMD_READ_RATE_GAIN_B_N40, FALSE))
            {
                LOG_PRINTF("\r\nefuse rate gain b   = %d\r\n", gDeviceInfo->efuse_rate_gain_b);
                LOG_PRINTF("\r\nefuse rate gain n40 = %d\r\n", gDeviceInfo->efuse_rate_gain_n40);
            } else{
                LOG_PRINTF("\r\n send host cmd fail\r\n");
            }
           return;
        }
        else {
            LOG_PRINTF("Incorrect efuse command format\r\n");
        }
    }else if (!ssv6xxx_strcmp(argv[1], "writerategainbn40")) {

        if (argc == 5)
        {
            if (ssv6xxx_strlen(argv[2]) > 32) {
                LOG_PRINTF("write psk range 1 ~ 32\r\n");
                return;
            }
            
            value1 = ssv6xxx_atoi(argv[3]);
            if ((value1 < 1) || (value1 > 15)) {
                LOG_PRINTF("rate gain b range 1 ~ 15\r\n");
                return;
            }
            
            value2 = ssv6xxx_atoi(argv[4]);
            if ((value2 < 1) || (value2 > 15)) {
                LOG_PRINTF("rate gain n40 range 1 ~ 15\r\n");
                return;
            }
            
            OS_MemSET((void *)&efuse_tool_param, 0x0, sizeof(struct ssv_efuse_tool_param));
            efuse_tool_param.rate_gain_b = value1;
            efuse_tool_param.rate_gain_n40 = value2;
            efuse_tool_param.psk_len = ssv6xxx_strlen(argv[2]);
            for (i= 0; i < efuse_tool_param.psk_len; i++)
                efuse_tool_param.psk[i] = argv[2][i];

            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&efuse_tool_param, (int)sizeof(struct ssv_efuse_tool_param), SSV6XXX_HOST_CMD_EFUSE_OPS, SSV6XXX_EFUSE_CMD_WRITE_XTAL, TRUE))
            {
                LOG_PRINTF("\r\nwrite efuse xtal = %d\r\n", value1);
            } else{
                LOG_PRINTF("\r\n send host cmd fail\r\n");
            }
           return;
        }
        else {
            LOG_PRINTF("Incorrect efuse command format\r\n");
        }
    }else if (!ssv6xxx_strcmp(argv[1], "checkitemfree")) {
        if (argc == 3)
        {
            value1 = ssv6xxx_atoi(argv[2]);
            if ((value1 < 0) || (value1 > 4)) {
                LOG_PRINTF("item range 0 ~ 4\r\n");
                LOG_PRINTF("0: macaddress, 1: freq offset, 2: txpower1, 3: rate gain, 4: ble power\r\n");
                return;
            }
            
            OS_MemSET((void *)&efuse_tool_param, 0x0, sizeof(struct ssv_efuse_tool_param));
            efuse_tool_param.check_item_free = value1;
            if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&efuse_tool_param, (int)sizeof(struct ssv_efuse_tool_param), SSV6XXX_HOST_CMD_EFUSE_OPS, SSV6XXX_EFUSE_CMD_CHECK_ITEM_FREE, FALSE))
            {
                LOG_PRINTF("\r\nfree count = %d\r\n", gDeviceInfo->efuse_free_count);
            } else{
                LOG_PRINTF("\r\n send host cmd fail\r\n");
            }
           return;
        }
        else {
            LOG_PRINTF("Incorrect efuse command format\r\n");
        }
    }else if (!ssv6xxx_strcmp(argv[1], "readblepower")) {
            if (argc == 2)
            {
                OS_MemSET((void *)&efuse_tool_param, 0x0, sizeof(struct ssv_efuse_tool_param));
                if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&efuse_tool_param, (int)sizeof(struct ssv_efuse_tool_param), SSV6XXX_HOST_CMD_EFUSE_OPS, SSV6XXX_EFUSE_CMD_READ_BLE_POWER, FALSE))
                {
                    LOG_PRINTF("\r\nefuse ble gain = %d\r\n", gDeviceInfo->efuse_ble_gain);
                } else{
                    LOG_PRINTF("\r\n send host cmd fail\r\n");
                }
               return;
            }
            else {
                LOG_PRINTF("Incorrect efuse command format\r\n");
            }
        }else if (!ssv6xxx_strcmp(argv[1], "writeblepower")) {
            if (argc == 4)
            {
                if (strlen(argv[2]) > 32) {
                    LOG_PRINTF("write psk range 1 ~ 32\r\n");
                    return;
                }
                
                value1 = ssv6xxx_atoi(argv[3]);
                if ((value1 < 1) || (value1 > 15)) {
                    LOG_PRINTF("ble power range 1 ~ 15\r\n");
                    return;
                }
                
                OS_MemSET((void *)&efuse_tool_param, 0x0, sizeof(struct ssv_efuse_tool_param));
                efuse_tool_param.ble_gain = value1;
                efuse_tool_param.psk_len = ssv6xxx_strlen(argv[2]);
                for (i= 0; i < efuse_tool_param.psk_len; i++)
                    efuse_tool_param.psk[i] = argv[2][i];

                if(SSV6XXX_SUCCESS==ssv6xxx_send_host_cmd((void *)&efuse_tool_param, (int)sizeof(struct ssv_efuse_tool_param), SSV6XXX_HOST_CMD_EFUSE_OPS, SSV6XXX_EFUSE_CMD_WRITE_BLE_POWER, TRUE))
                {
                    LOG_PRINTF("\r\nwrite efuse ble power = %d\r\n", value1);
                } else{
                    LOG_PRINTF("\r\n send host cmd fail\r\n");
                }
               return;
            }
        else {
            LOG_PRINTF("Incorrect efuse command format\r\n");
        }
    }else {
        LOG_PRINTF("\r\n efuse [readchipid|readmac|writemac|readtxpower1|writetxpower1|readxtal|writextal|readrategainbn40|writerategainbn40|raadblepower|writeblepower|checkitemfree]\r\n");
    }

}

void cmd_ctl(ssv_type_s32 argc, char *argv[])
{

    ssv_type_bool errormsg = FALSE;
    //char *err_str = "";
    ssv_type_u8 ssid_buf[MAX_SSID_LEN+1]={0};
    ssv_type_u32 i;

    if (argc <= 1)
    {
        errormsg = TRUE;
    }
	else if (ssv6xxx_strcmp(argv[1], "status")==0)
    {
        Ap_sta_status info;
        OS_MemSET(&info , 0 , sizeof(Ap_sta_status));
        errormsg = FALSE;
        ssv6xxx_wifi_status(&info);
        if(info.status)
            LOG_PRINTF("status:ON\r\n");
        else
            LOG_PRINTF("status:OFF\r\n");

        i=STA_VIF_IDX; 
        if((SSV6XXX_HWM_STA==info.vif_operate[i])||(SSV6XXX_HWM_SNIFFER==info.vif_operate[i]))
        {
            LOG_PRINTF("\r\nMode:%s, %s\r\n",(SSV6XXX_HWM_STA==info.vif_operate[i])?"STA":"SNIFFER",(info.vif_u[i].station_status.apinfo.status == CONNECT) ? "connected" :"disconnected");
            LOG_PRINTF("self Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                info.vif_u[i].station_status.selfmac[0],
                info.vif_u[i].station_status.selfmac[1],
                info.vif_u[i].station_status.selfmac[2],
                info.vif_u[i].station_status.selfmac[3],
                info.vif_u[i].station_status.selfmac[4],
                info.vif_u[i].station_status.selfmac[5]);
            OS_MemSET(ssid_buf,0,sizeof(ssid_buf));
            OS_MemCPY((void*)ssid_buf,(void*)info.vif_u[i].station_status.ssid.ssid,info.vif_u[i].station_status.ssid.ssid_len);
            LOG_PRINTF("SSID:%s\r\n",ssid_buf);
            LOG_PRINTF("channel:%d\r\n",info.vif_u[i].station_status.channel);
            LOG_PRINTF("AP Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                info.vif_u[i].station_status.apinfo.Mac[0],
                info.vif_u[i].station_status.apinfo.Mac[1],
                info.vif_u[i].station_status.apinfo.Mac[2],
                info.vif_u[i].station_status.apinfo.Mac[3],
                info.vif_u[i].station_status.apinfo.Mac[4],
                info.vif_u[i].station_status.apinfo.Mac[5]);
            if(ssv_is_valid_ether_addr(info.vif_u[i].station_status.apinfo.Mac)){
                LOG_PRINTF("RSSI = -%d (dBm)\r\n",ssv6xxx_get_rssi_by_mac(info.vif_u[i].station_status.apinfo.Mac, i));
            }
            else{
                LOG_PRINTF("RSSI = 0 (dBm)\r\n");
            }
        }
#if (AP_MODE_ENABLE == 1)        
        i=AP_VIF_IDX;
        if(SSV6XXX_HWM_AP==info.vif_operate[i])
        {
            ssv_type_u32 statemp;
            ssv_type_u8 idx;
            ssv_type_u32 ip=0;
            struct apmode_sta_info *sta_info=NULL;
    
            sta_info = (void*)OS_MemAlloc(sizeof(struct apmode_sta_info));
            if(!sta_info)
            {
                LOG_PRINTF("malloc sta_info fail \r\n");
                return ;
            }
            
            LOG_PRINTF("Mode:AP\r\n");
            LOG_PRINTF("self Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                info.vif_u[i].ap_status.selfmac[0],
                info.vif_u[i].ap_status.selfmac[1],
                info.vif_u[i].ap_status.selfmac[2],
                info.vif_u[i].ap_status.selfmac[3],
                info.vif_u[i].ap_status.selfmac[4],
                info.vif_u[i].ap_status.selfmac[5]);
            OS_MemSET(ssid_buf,0,sizeof(ssid_buf));
            OS_MemCPY((void*)ssid_buf,(void*)info.vif_u[i].ap_status.ssid.ssid,info.vif_u[i].ap_status.ssid.ssid_len);
            LOG_PRINTF("SSID:%s\r\n",ssid_buf);
            LOG_PRINTF("channel:%d\r\n",info.vif_u[i].ap_status.channel);
            LOG_PRINTF("Station number:%d\r\n",info.vif_u[i].ap_status.stanum);
            for(statemp=0; statemp < info.vif_u[i].ap_status.stanum ;statemp ++ )
            {
                LOG_PRINTF("station Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                    info.vif_u[i].ap_status.stainfo[statemp].Mac[0],
                    info.vif_u[i].ap_status.stainfo[statemp].Mac[1],
                    info.vif_u[i].ap_status.stainfo[statemp].Mac[2],
                    info.vif_u[i].ap_status.stainfo[statemp].Mac[3],
                    info.vif_u[i].ap_status.stainfo[statemp].Mac[4],
                    info.vif_u[i].ap_status.stainfo[statemp].Mac[5]);
            }

            for(idx=0;idx<AP_SUPPORT_CLIENT_NUM;idx++)
            {
                OS_MemSET(sta_info , 0 , sizeof(struct apmode_sta_info));

                LOG_PRINTF("==================STA:%d=================\r\n",idx);
                APStaInfo_PrintStaInfo(idx);
                
                if(ssv6xxx_get_sta_info_by_aid(sta_info,idx)==SSV6XXX_SUCCESS)
                {
                    if(ssv_is_valid_ether_addr(sta_info->addr))
                    {
                        if(sta_info->arp_retry_count>g_sta_timeout_retry_count)
                        {
                            LOG_PRINTF("Inactive\r\n");
                        }
                        else
                        {
                            LOG_PRINTF("Active\r\n");
                        }
                    
                        LOG_PRINTF("addr:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
                            sta_info->addr[0],
                            sta_info->addr[1],
                            sta_info->addr[2],
                            sta_info->addr[3],
                            sta_info->addr[4],
                            sta_info->addr[5]);

                        netstack_dhcp_ip_get_by_mac(sta_info->addr,&ip);

                        if(ip)
                            LOG_PRINTF("IP = [%d.%d.%d.%d]\r\n", 
                                netstack_ip4_addr1_16(&ip),
                                netstack_ip4_addr2_16(&ip),
                                netstack_ip4_addr3_16(&ip),
                                netstack_ip4_addr4_16(&ip));
                        
                        LOG_PRINTF("RSSI = -%d (dBm) \r\n", sta_info->rssi);
                   }
                }
                LOG_PRINTF("========================================\r\n");
            }
            OS_MemFree(sta_info);
         }
#endif        
    }
#if (AP_MODE_ENABLE == 1)    
    else if (ssv6xxx_strcmp(argv[1], "ap")==0&&argc >= 3)
    {
        ssv_wifi_ap_cfg ap;
        ssv_type_bool bApOn=FALSE;
        OS_MemSET(&ap , 0 , sizeof(Ap_setting));
        //instruction dispatch
        // ctl ap on [ap_name] [channel] [bw] [security] [password]
        switch(argc)
        {
            case 4: // only ssid , security open , //ctl ap on [ap_name]
                //ctl ap on ssid
                if((ssv6xxx_strcmp(argv[2], "on" )== 0)&&(ssv6xxx_strlen(argv[3])<=MAX_SSID_LEN))
                {
					bApOn =TRUE;
                    ap.status = TRUE;
                    ap.security = SSV6XXX_SEC_NONE;
                    ap.ssid.ssid_len = ssv6xxx_strlen(argv[3]);
					ap.channel =EN_CHANNEL_AUTO_SELECT;
					ap.ch_bw = EN_CH_BW_AUTO; 
                    OS_MemCPY( (void*)ap.ssid.ssid, (void*)argv[3], ssv6xxx_strlen(argv[3]));
                }
                else
                {
                 
                    LOG_PRINTF("SSID is too long\r\n");
                }
                break;
			case 6: 
                //ctl ap on ssid ch bw
                if((ssv6xxx_strcmp(argv[2], "on" )== 0)&&(0<=strtol(argv[4],NULL,10))&&
                        /*(strtol(argv[4],NULL,10)<=14)&&*/(ssv6xxx_strlen(argv[3])<=MAX_SSID_LEN) &&
                        ((0==ssv6xxx_strcmp("AUTO",argv[5])) || (0==ssv6xxx_strcmp("HT20",argv[5])) 
                                    || (0==ssv6xxx_strcmp("HT40",argv[5]))))
                {
                    //ctl ap on [ap_name] [channel] [bandwidth]
                    bApOn =TRUE;
                    ap.status = TRUE;
                    ap.security = SSV6XXX_SEC_NONE;
                    ap.ssid.ssid_len = ssv6xxx_strlen(argv[3]);
                    ap.channel = (ssv_type_u8)strtol(argv[4],NULL,10);
                    if(ap.channel==0)
                    {
                        ap.channel =EN_CHANNEL_AUTO_SELECT;
                    }
                    if(0==ssv6xxx_strcmp("AUTO",argv[5]))
                    {
                        ap.ch_bw = EN_CH_BW_AUTO; 
                    }
                    else if(0==ssv6xxx_strcmp("HT20",argv[5]))
                    {
                        ap.ch_bw = EN_CH_BW_HT20; 
                    }
                    else if(0==ssv6xxx_strcmp("HT40",argv[5]))
                    {
                        ap.ch_bw = EN_CH_BW_HT40; 
                    }
                    OS_MemCPY( (void*)ap.ssid.ssid, (void*)argv[3], ssv6xxx_strlen(argv[3]));
                    LOG_PRINTF("SSID:%s, ch:%d. \r\n",ap.ssid.ssid,ap.channel);
                }
                else
                {
			
					//ctl ap on ssid sec pwd
					if(ssv6xxx_strcmp(argv[2], "on") == 0)
					{
						bApOn =TRUE;
						ap.channel =EN_CHANNEL_AUTO_SELECT;
						ap.ch_bw = EN_CH_BW_AUTO; 
						if(ssv6xxx_strcmp(argv[4], "wep") == 0)
						{
							ap.status = TRUE;
							if(ssv6xxx_strlen(argv[5]) == 5)
								ap.security = 	SSV6XXX_SEC_WEP_40;
							else if (ssv6xxx_strlen(argv[5]) == 13)
								ap.security = 	SSV6XXX_SEC_WEP_104;
							else
							{
								LOG_PRINTF("WEP key length must be 5 or 13 character. \r\n");
								errormsg =TRUE;
								break;
							}
						}
	#ifndef CONFIG_NO_WPA2
						else if(ssv6xxx_strcmp(argv[4], "wpa2") == 0)
						{
							ap.status = TRUE;
							ap.security = SSV6XXX_SEC_WPA2_PSK;
							ap.proto = WPA_PROTO_RSN;
							ap.key_mgmt = WPA_KEY_MGMT_PSK ;
							ap.group_cipher=WPA_CIPHER_CCMP;
							ap.pairwise_cipher = WPA_CIPHER_CCMP;
							if((ssv6xxx_strlen(argv[5]) <8) || (ssv6xxx_strlen(argv[5])>63))

							{
								LOG_PRINTF("WPA2 key length must be 8~63 character. \r\n");
								break;
							}

						}
	#endif
						else
						{
							LOG_PRINTF("SSID:%s, Security type:%s, Password:%s. \r\n",argv[3],argv[4],argv[5] );
							break;

						}
						ssv6xxx_strcpy((char *)ap.password, argv[5]);
						ap.ssid.ssid_len = ssv6xxx_strlen(argv[3]);
						OS_MemCPY( (void*)ap.ssid.ssid, (void*)argv[3], ssv6xxx_strlen(argv[3]));

					}
				}
                break;

            case 8:                 
                // ctl ap on [ap_name] [channel] [bandwidth] [security] [password]                
                if(ssv6xxx_strcmp(argv[2], "on") == 0&&(0<=strtol(argv[4],NULL,10))&&
                        /*(strtol(argv[4],NULL,10)<=14)&&*/(ssv6xxx_strlen(argv[3])<=MAX_SSID_LEN))
                {
                
                    ap.ssid.ssid_len = ssv6xxx_strlen(argv[3]);
                    OS_MemCPY( (void*)ap.ssid.ssid, (void*)argv[3], ssv6xxx_strlen(argv[3]));
                    ap.channel = (ssv_type_u8)strtol(argv[4],NULL,10);
 
                    if(ap.channel==0)
                    {
                        ap.channel =EN_CHANNEL_AUTO_SELECT;
                    }
  
                    if(0==ssv6xxx_strcmp("AUTO",argv[5]))
                    {
                        ap.ch_bw = EN_CH_BW_AUTO; 
                    }
                    else if(0==ssv6xxx_strcmp("HT20",argv[5]))
                    {
                        ap.ch_bw = EN_CH_BW_HT20; 
                    }
                    else if(0==ssv6xxx_strcmp("HT40",argv[5]))
                    {
                        ap.ch_bw = EN_CH_BW_HT40; 
                    }
                }
                else
                {
                    break;
                }
                if(ssv6xxx_strcmp(argv[6], "wep") == 0)                 
                {
                    bApOn =TRUE;
                    ap.status = TRUE;
                    if(ssv6xxx_strlen(argv[7]) == 5)
                        ap.security = 	SSV6XXX_SEC_WEP_40;
                    else if (ssv6xxx_strlen(argv[7]) == 13)
                        ap.security = 	SSV6XXX_SEC_WEP_104;
                    else
                    {
                        LOG_PRINTF("WEP key length must be 5 or 13 character. \r\n");
                    }
                }
#ifndef CONFIG_NO_WPA2
                else if(ssv6xxx_strcmp(argv[6], "wpa2") == 0)
                {
                    bApOn =TRUE;
                    ap.status = TRUE;
                    ap.security = SSV6XXX_SEC_WPA2_PSK;
                    ap.proto = WPA_PROTO_RSN;
                    ap.key_mgmt = WPA_KEY_MGMT_PSK ;
                    ap.group_cipher=WPA_CIPHER_CCMP;
                    ap.pairwise_cipher = WPA_CIPHER_CCMP;
                    LOG_PRINTF("SSID:%s, channel:%d, Security type:%s, Password:%s. \r\n",argv[3],strtol(argv[4],NULL,10),argv[6],argv[7] );
                    LOG_PRINTF("ap.pairwise_cipher=%d+++++++\n\r",ap.pairwise_cipher);
                    if((ssv6xxx_strlen(argv[7]) <8) || (ssv6xxx_strlen(argv[7])>63))

                    {
                        LOG_PRINTF("s key length must be 8~63 character. \r\n");
                        break;
                    }

                }
#endif
                else
                {
                    LOG_PRINTF("SSID:%s, channel:%d, Security type:%s, Password:%s. \r\n",argv[3],strtol(argv[4],NULL,10),argv[6],argv[7] );
                    break;

                }
                ssv6xxx_strcpy((char *)ap.password, argv[7]);
                break;
             default:
                bApOn =FALSE;
                break;


        }
        if(TRUE==bApOn)
        {
            if (netmgr_wifi_ap_on_async(&ap) == SSV6XXX_FAILED)
                errormsg =  TRUE;
            else
                errormsg = FALSE;
        }
        else
        {
            if (netmgr_wifi_ap_off_async() == SSV6XXX_FAILED)
                errormsg =  TRUE;
            else
                errormsg = FALSE;

        }
	}
#endif    
    else if (ssv6xxx_strcmp(argv[1], "sta")==0&&argc >= 3)
	{   
	    //ctl sta on
	    //ctl sta on static
        ssv_type_u8 arg_cnt = 2;

        if(ssv6xxx_strcmp(argv[arg_cnt], "on") == 0)
        {
            LOG_PRINTF("sta on \r\n");

#if USE_ICOMM_LWIP
            if(argc > (arg_cnt+1))
            {
                arg_cnt++;
                if(ssv6xxx_strcmp(argv[arg_cnt], "static")==0)
                {
                    LOG_PRINTF("static ip\r\n");
                    netmgr_dhcpc_set(false,STA_VIF_IDX);
                }
            }            
            else
            {
                LOG_PRINTF("dynamic IP\r\n");
                netmgr_dhcpc_set(true,STA_VIF_IDX);
            }
#endif
            if (netmgr_wifi_sta_on_async() == SSV6XXX_FAILED)
                errormsg =  TRUE;
            else
                errormsg = FALSE;

        }
        else
        {
            if (netmgr_wifi_sta_off_async() == SSV6XXX_FAILED)
                errormsg =  TRUE;
            else
                errormsg = FALSE;

        }
    }
    else if ((0 == ssv6xxx_strcmp(argv[1], "sniffer")) &&(3 <= argc))
    {
        if(ssv6xxx_strcmp(argv[2], "on") == 0)
        {
            if (netmgr_wifi_sniffer_on_async() == SSV6XXX_FAILED)
                errormsg =  TRUE;
            else
                errormsg = FALSE;

        }
        else
        {
        
            if (netmgr_wifi_sniffer_off_async() == SSV6XXX_FAILED)
                errormsg =  TRUE;
            else
                errormsg = FALSE;
        }
    }
    else{
        LOG_PRINTF("Invalid wifictl command.\r\n");
    }

	if(FALSE==errormsg)
    {
        LOG_PRINTF("OK.\r\n");
    }
    else
    {
        char * cmd_name = "ctl";
        show_invalid_para(cmd_name);
    }
} // end of - cmd_iw -



void cmd_mib(ssv_type_s32 argc, char *argv[])
{

    if(ssv6xxx_strcmp(argv[1], "reset")==0)
    {
        ssv_hal_hw_mib_reset();
    }
    else if(ssv6xxx_strcmp(argv[1], "off")==0)
    {
        ssv_hal_hw_mib_off();    
    }
    else if(ssv6xxx_strcmp(argv[1], "on")==0)
    {
        ssv_hal_hw_mib_on();    
    }    
    else if(ssv6xxx_strcmp(argv[1], "tx")==0)
    {
        ssv_hal_hw_mib_show_tx();    
    }    
    else if(ssv6xxx_strcmp(argv[1], "rx")==0)
    {
        ssv_hal_hw_mib_show_rx();        
    }    
    else if(ssv6xxx_strcmp(argv[1], "rc")==0)
    {
        extern ssv_type_s32 ssv6xxx_wifi_get_recover_cnt(void);
        extern ssv_type_s32 ssv6xxx_wifi_get_fw_interrupt_cnt(void);
        LOG_PRINTF("Recover count:%d\r\n",ssv6xxx_wifi_get_recover_cnt());
        LOG_PRINTF("Fw timer interrupt count:%u\r\n",ssv6xxx_wifi_get_fw_interrupt_cnt());
    }
    return;

}
void cmd_ssv_adapt(ssv_type_s32 argc, char *argv[])
{
	/** 
	*  AT+ADAPT [0|1] 
	*/
	int ret = -1;
	struct ssv6xxx_adaptive_param param = {0};
	LOG_PRINTF("\33[32m  cmd_ssv_adapt  \33[0m\r\n");
	if ((argc == 2) && (!strcmp(argv[1], "1"))) {
		LOG_PRINTF("call cmd AT+ADAPT 1");
		ret = ssv6xxx_send_host_cmd((void *)&param, (int)(sizeof(struct ssv6xxx_adaptive_param)),SSV6XXX_HOST_CMD_ADAPTIVE,SSV6XXX_ADAPTIVE_OPS_ENABLE, TRUE);
		LOG_PRINTF("ssv6xxx_send_host_cmd ret:%d\n",ret);
	} else if ((argc == 2) && (!strcmp(argv[1], "0"))) {
		LOG_PRINTF("call cmd AT+ADAPT 0");
		ret = ssv6xxx_send_host_cmd((void *)&param, (int)(sizeof(struct ssv6xxx_adaptive_param)),SSV6XXX_HOST_CMD_ADAPTIVE,SSV6XXX_ADAPTIVE_OPS_DISABLE, TRUE);
		LOG_PRINTF("ssv6xxx_send_host_cmd ret:%d\n",ret);
	} else {
		LOG_PRINTF(" AT+ADAPT [0|1]\n\n");
		LOG_PRINTF(" - 0: Disable adaptive test\n\n");
		LOG_PRINTF(" - 1: Enable adaptive test\n\n");
	}
	
	LOG_PRINTF("\33[32m  \n Finish.. \33[0m\r\n");
	return;
}
void cmd_ssv_set_adapt(ssv_type_s32 argc, char *argv[])
{
	struct ssv6xxx_adaptive_param param = {0};
	char *endp;
	int ret = -1;
	 /**
        *  AT+SET_ADAP <interval> <hit count> <total count> <threshold>
        */
	LOG_PRINTF("\33[32m  cmd_ssv_set_adapt  \33[0m\r\n");
	if (argc == 5){
		LOG_PRINTF("cmd AT+SET_ADAPT %s %s %s %s\n", argv[1], argv[2], argv[3], argv[4]);
		//param.interval = (u32)simple_strtoul(argv[1], &endp, 0);
        //param.hit_count = (u32)simple_strtoul(argv[2], &endp, 0);
        //param.total_count = (u32)simple_strtoul(argv[3], &endp, 0);
        //param.threshold = (u32)simple_strtoul(argv[4], &endp, 0);
		param.interval = (u32)strtoul(argv[1], &endp, 0);
        param.hit_count = (u32)strtoul(argv[2], &endp, 0);
        param.total_count = (u32)strtoul(argv[3], &endp, 0);
        param.threshold = (u32)strtoul(argv[4], &endp, 0);
		LOG_PRINTF("interval:%d,hit_count:%d,total_count:%d,threshold:%d \n",param.interval,param.hit_count,param.total_count,param.threshold);
		ret = ssv6xxx_send_host_cmd((void *)&param, (int)(sizeof(struct ssv6xxx_adaptive_param)),SSV6XXX_HOST_CMD_ADAPTIVE,SSV6XXX_ADAPTIVE_OPS_SET, TRUE);
		LOG_PRINTF("ssv6xxx_send_host_cmd ret:%d\n",ret);
		} else {
			LOG_PRINTF(" AT+SET_ADAPT <interval> <hit count> <total count> <threshold>\n\n");
		}
		
	LOG_PRINTF("\33[32m  \n Finish.. \33[0m\r\n");
	
	return;
}

#if (ENABLE_SMART_CONFIG == 1)
extern void smart_config_start(void);
extern void smart_config_stop(void);
void cmd_sconfig(ssv_type_s32 argc, char *argv[])
{
    if((2 == argc) && (0 == ssv6xxx_strcmp(argv[1], "on")))
    {
        smart_config_start();
    }
    else
    {
        smart_config_stop();
    }
    return;
}
#endif
#endif
#if(ENABLE_WPS ==1)
void wps_event_cb_ap(ssv_type_u8 success, int id, ssv_type_u8 macaddr[6], void* priv)
{
    LOG_PRINTF("wps_event_cb_ap 0x%d, id %d, mac %02x:%02x:%02x:%02x:%02x:%02x\r\n", 
            success, id,
            macaddr[0], macaddr[1], macaddr[2],
            macaddr[3], macaddr[4], macaddr[5]);
}

void wps_event_cb(ssv_type_u32 evt_id, void *data, ssv_type_s32 len, void* priv)
{
    struct wps_res *wres=NULL;    
    ssv_type_u8  ssid[MAX_SSID_LEN+1]={0};
    ssv_type_u8  password[MAX_PASSWD_LEN+1]={0};
    wifi_sta_join_cfg *join_cfg = NULL;
    int ret = -1;
    
    if(EVT_WPS_RESULT!=evt_id)
    {
        return;
    }
    
    wres=(struct wps_res *)data;    
    switch (wres->status)
    {
        case EN_WPS_SUCCESS:
            LOG_PRINTF("WPS Success\r\n");                
            ssv6xxx_memset(ssid,0,sizeof(ssid));
            ssv6xxx_memcpy(ssid,wres->ssid, wres->ssid_len);
            ssv6xxx_memset(password,0,sizeof(password));
            ssv6xxx_memcpy(password,wres->key, wres->key_len);    
            LOG_PRINTF("SSID=%s, Password=%s\r\n",ssid,password);    
            aos_sleep(100); //wake that DUT is in disconnect status
            netmgr_wifi_leave_async(); //make sure DUT is back to disconnect status
            join_cfg = (wifi_sta_join_cfg *)OS_MemAlloc(sizeof(wifi_sta_join_cfg));
            if(NULL==join_cfg)
            {
                LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
                return;
            }
            ssv6xxx_memset((void *)join_cfg,0,sizeof(wifi_sta_join_cfg));
            ssv6xxx_memcpy((char *)join_cfg->ssid.ssid,(const void *)wres->ssid,wres->ssid_len);
            join_cfg->ssid.ssid_len=wres->ssid_len;
            ssv6xxx_memcpy((char *)join_cfg->password,(const void *)wres->key,wres->key_len);  
            ret = netmgr_wifi_join_async(join_cfg);
            if (ret != 0)
            {
        	    LOG_PRINTF("netmgr_wifi_join_async failed !!\r\n");
            }
            OS_MemFree(join_cfg);
            break;
        case EN_WPS_WAKE_TIME_TO:
            LOG_PRINTF("WPS Wake Time Out\r\n"); 
            //netmgr_wifi_leave_async();
            break;
        case EN_WPS_PBC_OVERLAP:
            LOG_PRINTF("WPS PBC Overlap\r\n");              
            //netmgr_wifi_leave_async();
            break;
        case EN_WPS_FAIL:
            LOG_PRINTF("WPS Fail\r\n");
            netmgr_wifi_leave_async();
            break;
        default:
            break;
    }
    return;
}
void wps_reg_evt_cb(void)
{
    ssv6xxx_wifi_reg_evt_cb(wps_event_cb,NULL);
}

void cmd_wps(ssv_type_s32 argc, char *argv[])
{
    wps_reg_evt_cb();
    if(argc==0){
       goto USAGE;
    }

    if(argc==2){
        if(0==ssv6xxx_strcmp(argv[1],"pbc")){
            netmgr_wifi_wps_pbc(TRUE);
        } else if (0==ssv6xxx_strcmp(argv[1],"push_button")){
            netmgr_wifi_wps_push_button();
        } else {
            goto USAGE;
        }
    }else{
        goto USAGE;
    }
#if 0    
    else if(param->argc==2){
        if(0==strcmp(param->argv[1],"get")){
            WPSGenPIN(&pin);
        }
        else{
            pin=atoi(param->argv[1]);
            WPSStartPIN(FALSE,pin,atWPScb,atwificbfunc);        
        }
    }
    else{
        goto USAGE;
    }
#endif        
    LOG_PRINTF("\n");   
    return;

USAGE:
    LOG_PRINTF("wrong usage:\n");
    LOG_PRINTF("wps pbc\n");        
    //LOG_PRINTF("wps pin [pin code]\n");      
    //LOG_PRINTF("wps pin get\n");          

    return;
}
#endif
