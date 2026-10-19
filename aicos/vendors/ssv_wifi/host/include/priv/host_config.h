/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _SIM_CONFIG_H_
#define _SIM_CONFIG_H_

//#include <ssv_types.h>
#include <porting.h>
#include <msgevt.h>
// ------------------------- debug ---------------------------

#define CONFIG_STATUS_CHECK         1

//-----------------------------------------------------------
#define Sleep                       OS_MsDelay
#define AUTO_INIT_STATION           1


#define CONFIG_CHECKSUM_DCHECK      0
#define TARGET						prj1_config

// ------------------------- log -------------------------------
#define SSV_LOG_DEBUG           1


/** lower two bits indicate debug level
 * - 0 all
 * - 1 warning
 * - 2 serious
 * - 3 severe
 */
#define LOG_LEVEL_ALL     0x00
#define LOG_LEVEL_WARNING 0x01 	/* bad checksums, dropped packets, ... */
#define LOG_LEVEL_SERIOUS 0x02 	/* memory allocation failures, ... */
#define LOG_LEVEL_SEVERE  0x03
#define LOG_LEVEL_MASK_LEVEL 0x03


#define LOG_NONE	       			   (0)
#define LOG_MEM	       			       (1<<16)
#define LOG_L3_SOCKET				   (1<<17)
#define LOG_L3_API				   	   (1<<18)
#define LOG_L3_TCP					   (1<<19)
#define LOG_L3_UDP					   (1<<20)
#define LOG_L3_IP					   (1<<21)
#define LOG_L3_OTHER_PROTO			   (1<<22)
#define LOG_L2_DATA			           (1<<23)
#define LOG_L2_AP			           (1<<24)
#define LOG_L2_STA			           (1<<25)
#define LOG_CMDENG			           (1<<26)
#define LOG_TXRX			           (1<<27)
#define LOG_SNIFFER               (1<<28)

#define LOG_L4_HTTPD                   (1<<29)
#define LOG_L4_NETMGR   	           (1<<30)
#define LOG_L4_DHCPD			       ((unsigned int)1<<31)

#define LOG_ALL_MODULES                (0xffffffff)

//***********************************************//
//Modify default log config
#define CONFIG_LOG_MODULE				(LOG_ALL_MODULES)
#define CONFIG_LOG_LEVEL				(LOG_LEVEL_SERIOUS)

// ------------------------PHY mode ----------------------------
#define PHY_GB_MODE    0
#define PHY_NB_MODE    1

#define PHY_SUPPORT_HT PHY_NB_MODE

// ------------------------RF Band ----------------------------
#define RF_2G_BAND    0
#define RF_5G_BAND    1

#if(PHY_SUPPORT_HT==PHY_GB_MODE)
#define RF_BAND RF_2G_BAND
#else
#define RF_BAND RF_5G_BAND
#endif

#define RF_BAND_GAIN_OFFSET 0
#define EFUSE_RATE_GAIN_MASK 0xF
#define RF_B_RATE_OFFSET 0
#define RF_G_RATE_OFFSET 0
#define RF_HT20_RATE_OFFSET 0
#define RF_HT40_RATE_OFFSET 0
#define RF_TX_LOW_POWER 1
#define RF_VOLT_REGULATOR 1
#define RF_PADPD 1

// ------------------------- rtos -------------------------------
#define OS_MUTEX_SIZE				10              // Mutex
#define OS_COND_SIZE				5               // Conditional variable
#define OS_TIMER_SIZE				5               // OS Timer
#define OS_MSGQ_SIZE				5               // Message Queue

// ------------------------- cli -------------------------------
#define CLI_ENABLE					1               // Enable/Disable CLI
#if (CLI_ENABLE==1)
#define CLI_HISTORY_ENABLE			1               // Enable/Disable CLI history log. only work in SIM platofrm for now.
#define CLI_HISTORY_NUM				10


#define CLI_BUFFER_SIZE				80              // CLI Buffer size
#define CLI_ARG_SIZE				22             // The number of arguments
#define CLI_PROMPT					"wifi-host> "
#endif

// ------------------------- misc ------------------------------
#define _INTERNAL_MCU_SUPPLICANT_SUPPORT_
#define CONFIG_HOST_PLATFORM         1
/*test for sdio/spi drv*/
#define SSV_TMR_MAX   24

//STA mode change the RX sensitive dynamically
#define ENABLE_DYNAMIC_RX_SENSITIVE 0

//TX power mode , only workable for 6030P
#define TX_POWER_NORMAL          0
#define TX_POWER_B_HIGH_ONLY     1
#define TX_POWER_ENHANCE_ALL     2

#define CONFIG_TX_PWR_MODE     TX_POWER_NORMAL

//Voltage setting: LDO or DCDC
#define VOLT_LDO_REGULATOR  0
#define VOLT_DCDC_CONVERT   1

#define SSV_VOLT_REGULATOR  VOLT_LDO_REGULATOR

// ------------------------- mlme ------------------------------
/*Regular iw scan or regulare ap list update*/
#define CONFIG_AUTO_SCAN            0               // 1=auto scan, 0=auto flush ap_list table

#define TIMEOUT_TASK                MBOX_CMD_ENGINE               // MLME function 0=close,1=open

#define NUM_AP_INFO                 50
// ------------------------- txrx ------------------------------
#define TX_TASK_SLEEP               1               // Tx task go to sleep 1 tick when get no tx resource 200 time
#define TX_TASK_SLEEP_TICK          1               // how long does the tx task sleep? The default time is 1 tick
#define TX_TASK_RETRY_CNT           200             // Tx task retry count when tx resource of wifi-chip is not enough

#define TX_RESOURCE_PAGE            (128+60) //82 //164 //128
#define RX_RESOURCE_PAGE            (118-60)  //164 //82 //118, 4 pages for secure, 6 pages for beacon
#define RXINTGPIO      14 //20
#define HCI_TX_AGGR    1
#define HCI_RX_AGGR    1

#define HCI_TX_AGGR_BUF_SIZE (1500*15)

#define HCI_RX_AGGR_CNT 3
#define HCI_RX_AGGR_TIMER_RLOD 20

#define SW_Q_FORCE_UNLOCK_TIME   200 //unit: 10ms. 0 means never force unlock, 100=1000ms

// ------------------------- network ---------------------------
#define CONFIG_MAX_NETIF        1
#define AP_MODE_ENABLE          1
#define AP_SUPPORT_CLIENT_NUM     8
#define CONFIG_PLATFORM_CHECK   0

// ------------------------- mac address  ------------------------------
#define CONFIG_EFUSE_MAC            1
#define	CONFIG_RANDOM_MAC           1

// ------------------------- Deafult channel in STA mode ---------------------------
#define STA_DEFAULT_CHANNEL 6

// ------------------------- STA channel mask  ------------------------------
//This macro is for STA mode, evey bit corrsebonds to a channel, for example: bit[0] -> channel 1, bit[1] ->cahnnel 2.
//If user assign 0 to channel mask to netmgr_wifi_scan or netmgr_wifi_sconfig, we use the default value.
//If youe set ch15, we will filter it automatically
#define DEFAULT_STA_CHANNEL_MASK 0x3FFF //from 1~14

#if 0
BIT[0]=CHAN5G(5180, 36)
BIT[1]=CHAN5G(5200, 40)
BIT[2]=CHAN5G(5220, 44)
BIT[3]=CHAN5G(5240, 48)

BIT[4]=CHAN5G(5260, 52)
BIT[5]=CHAN5G(5280, 56)
BIT[6]=CHAN5G(5300, 60)
BIT[7]=CHAN5G(5320, 64)

BIT[8]=CHAN5G(5500, 100)
BIT[9]=CHAN5G(5520, 104)
BIT[10]=CHAN5G(5540, 108)
BIT[11]=CHAN5G(5560, 112)

BIT[12]=CHAN5G(5660, 132)
BIT[13]=CHAN5G(5680, 136)
BIT[14]=CHAN5G(5700, 140)
BIT[15]=CHAN5G(5745, 149)

BIT[16]=CHAN5G(5765, 153)
BIT[17]=CHAN5G(5785, 157)
BIT[18]=CHAN5G(5805, 161)
BIT[19]=CHAN5G(5825, 165)
#endif
#define DEFAULT_STA_5G_CHANNEL_MASK 0x1FFFFFF 
#define SCANNING_CH_INTERVAL 150 //ms
#define SCANNING_OFF_CH_INTERVAL 50 //ms
#define INFO_SCAN_RESULT_TO_APP 0

// ------------------------- auto channel selection ---------------------------
//The ACS_CHANNEL_MASK is for auto channel selection in AP  mode, evey bit corrsebonds to a channel, for example: bit[0] -> channel 1, bit[1] ->cahnnel 1.
//Now, we set 0xFFE, this value means we do auto channel selection from channel 1 to channel 11.
//If you just want to choose a channel from 1, 6,11, you must set ACS_CHANNEL_MASK  to 0x421
//If youe set ch12, ch13, ch14, or ch15, we will filter it automatically
#define ACS_CHANNEL_MASK 0x7FF
#define ACS_5G_CHANNEL_MASK 0x1FFFFFF

//The g_acs_channel_scanning_interval is for auto channel secltion in AP mode, if you set 10, it means AP will stay in one channel for 10x10ms,
//and then change to the next channel.
#define ACS_SCANNING_INTERVAL 100 //100ms

//This macro is used to set the number of times that you want to do the channel scanning.
//If this macro is 1, we do channel scanning one times, if this macro is 2, we do channel scanning two times, and then we choose a channel by all datas
#define ACS_SCANNING_LOOP 1

// ------------------------- Smart Config ---------------------------
#define ENABLE_SMART_CONFIG 0

#define AP_ACL_MAX_COUNT 12

// ------------------------- ---------------------------------------
#if (AP_MODE_ENABLE == 1)

	//#define __TEST__
	//#define __TEST_DATA__  //Test data flow
	//#define __TEST_BEACON__

#else
	#define __STA__

	//#define __TCPIP_TEST__
#endif


//#define __AP_DEBUG__

// ------------------------- rate control ---------------------------

#define RC_DEFAULT_RATE_MSK 0x0fff //0x0FFD
#define RC_DEFAULT_RESENT_REPORT 1  // feature, 0 or 1
#define RC_DEFAULT_UP_PF 10         // percentage, smaller than RC_DEFAULT_DOWN_PF
#define RC_DEFAULT_DOWN_PF 27       // percentage, bigger than RC_DEFAULT_UP_PF
#define RC_DEFAULT_PREPRBFRM 0      // feature, 0 or 1
#define RC_DIRECT_DOWN 0            // 1: TRUE, 0:False
#define RC_DEFAULT_DRATE_ENDIAN 0 //1: The default data rate is from the lowest rate of rate mask. 0:from highest index of rate mask
#define RC_DEFAULT_TX_SGI 0         // 1: TRUE, 0:False
// ------------------------- AMPDU ---------------------------
#define STA_AMPDU_TX_ENABLE 1
#define STA_AMPDU_RX_ENABLE 1
#define AP_AMPDU_TX_ENABLE 1
//VIF 1 can not support AMPDU RX on current mode, so default disable
#define AP_AMPDU_RX_ENABLE 1
#define AMPDU_RX_MAX_STATION AP_SUPPORT_CLIENT_NUM
#define AMPDU_RX_BUF_SIZE 64
#define AMPDU_TX_BUF_SIZE 64

//==============EDCA===============
//#define EDCA_PATTERN_TEST
#ifdef EDCA_PATTERN_TEST
#define EDCA_DBG						1		//Enable to test edca function
#define MACTXRX_CONTROL_ENABLE			1		//To simulate MAC TX operation. It's also enable ASIC queue empty interrupt.
#define MACTXRX_CONTROL_DURATION_SIM	1		//TX control use softmain edca handler to test MAC TX EDCA function
//#define __EDCA_INT_TEST__						//
//#define __EDCA_NOTIFY_HOST__					//When TX done send an event to nofity host to know
#define BEACON_DBG						1
#else
#define EDCA_DBG						0		//Enable to test edca function
#define MACTXRX_CONTROL_ENABLE			0		//To simulate MAC TX operation. It's also enable ASIC queue empty interrupt.
#define MACTXRX_CONTROL_DURATION_SIM	0		//TX control use softmain edca handler to test MAC TX EDCA function
//#define __EDCA_INT_TEST__						//
//#define __EDCA_NOTIFY_HOST__					//When TX done send an event to nofity host to know
#define BEACON_DBG						0
#endif


//=================================

//#define PACKED

/* default ip */
#define DEFAULT_IPADDR   "192.168.25.1"
#define DEFAULT_SUBNET   "255.255.255.0"
#define DEFAULT_GATEWAY  "192.168.25.1"
#define DEFAULT_DNS      "192.168.25.1"

/* default dhcp server info */
#define DEFAULT_DHCP_START_IP    "192.168.25.101"
#define DEFAULT_DHCP_END_IP      "192.168.25.110"
#define DEFAULT_DHCP_MAX_LEASES  10

#define DEFAULT_DHCP_AUTO_TIME       (7200)
#define DEFAULT_DHCP_DECLINE_TIME    (3600)
#define DEFAULT_DHCP_CONFLICT_TIME   (3600)
#define DEFAULT_DHCP_OFFER_TIME      (60)
#define DEFAULT_DHCP_MIN_LEASE_SEC   (60)
#define DEFAULT_DHCP_MAX_LEASE_SEC   (60*60*24 * 10)


/* watchdog */
#define RECOVER_ENABLE                  0
#ifndef __SSV_UNIX_SIM__
#define RECOVER_MECHANISM               1 //0:Used GPIO interrupt, 1:Used Timer interrupt
#else
#define RECOVER_MECHANISM               0 //0:Used GPIO interrupt, 1:Used Timer interrupt
#endif
#define IPC_CHECK_TIMER                 1000 //For IPC interrupt

// ------------------------- Power Saving ---------------------------
#define BUS_IDLE_TIME 0 //5000 //mswpw

/* netmgr auto retry default times and delay value */
#define NET_MGR_AUTO_RETRY_TIMES 0xFFFF
#define NET_MGR_AUTO_RETRY_DELAY 5 // unit s

//------------------------wpa2--------------------
//#define CONFIG_NO_WPA2

//------------------------AP mode--------------------
#define AP_BEACON_INT	(100)
#define AP_ERP                   0
#define AP_B_SHORT_PREAMBLE      0
#define AP_RX_SHORT_GI           1
#define AP_RX_SUPPORT_BASIC_RATE     0xFFF
#define AP_RX_SUPPORT_MCS_RATE       0xFF

#define AP_MODE_BEACON_VIRT_BMAP_0XFF         1		// 1: FIX VIRT BMAP 0XFF, 0:DYNAMIC VIRT BMAP
#define AP_SAT_INACTIVE                       20    //sec
#define CONFIG_MIFI                           1
// ------------------------- Update features depends on TCPIP stack ---------------------------
/* TCP/IP Configuration: */
#ifndef USE_ICOMM_LWIP
#define USE_ICOMM_LWIP          0
#endif
/*
    1: LWIP ignore the pcb->cwnd when LWIP output tcp packet
    0: LWIP refer the pcb->cwnd when LWIP output tcp packet
*/
#define LWIP_TCP_IGNORE_CWND     0
/* 0 for minimal resources, 1 for double resource of setting 0 with higher performance, 2 for default, 3 for maximal resources */
/* RAM Usage:
   0: 46KB
   1: 77KB
   2: 99KB
   3: 177KB
*/
#define LWIP_PARAM_SET                  3
#if USE_ICOMM_LWIP
#define DHCPD_SUPPORT 1
#define CONFIG_USE_LWIP_PBUF    0
#define CONFIG_MEMP_DEBUG       1
#define PING_SUPPORT 1
#define IPERF3_ENABLE  1
#define IPERF3_UDP_TEST 1
#else //#if USE_ICOMM_LWIP
#define PING_SUPPORT 0
#undef IPERF3_ENABLE
#define IPERF3_ENABLE  0
#define IPERF3_UDP_TEST 0
#undef  CONFIG_USE_LWIP_PBUF
#define CONFIG_USE_LWIP_PBUF    0
#undef  CONFIG_MEMP_DEBUG
#define CONFIG_MEMP_DEBUG       0
#define DHCPD_SUPPORT           0

#endif //#if USE_ICOMM_LWIP

#if CONFIG_USE_LWIP_PBUF
#define POOL_SIZE PBUF_POOL_SIZE
#define RECV_BUF_SIZE MAX_RECV_BUF
#define TRX_HDR_LEN DRV_TRX_HDR_LEN

#else
#define RECV_BUF_SIZE           (1460+156)
#define POOL_SIZE               (512) //128
#define TRX_HDR_LEN             (80+20)

#endif //#if USE_ICOMM_LWIP

//------------------------Net APP--------------------
//Disable NETAPP, and we need to manually set the netapp stack size to zero in porting.h
#define NETAPP_SUPPORT  0 //1
#define NETMGR_SUPPORT  1

//------------------------8023 <-> 80211--------------------
#define SW_8023TO80211 1


//------------------------Deafult Country--------------------
#define DEFAULT_COUNTRY_CODE "00"

//------------------------DFS--------------------
#define AP_NO_DFS_FUN 1

//Set a timeout periord for STA auto disconnect when AP power off or change channel
//The unit is 500ms, the default value is 40, it means STA will disconnect after AP power off for 24s(40*600ms)
#define STA_NO_BCN_TIMEOUT 40

//------------------------XTAL--------------------
#if 0
typedef enum {
    SSV6XXX_IQK_CFG_XTAL_26M = 0,//0
    SSV6XXX_IQK_CFG_XTAL_40M,	//1
    SSV6XXX_IQK_CFG_XTAL_24M,	//2
    SSV6XXX_IQK_CFG_XTAL_25M,	//3
    SSV6XXX_IQK_CFG_XTAL_12M,	//4
    SSV6XXX_IQK_CFG_XTAL_16M,	//5
    SSV6XXX_IQK_CFG_XTAL_20M,	//6
    SSV6XXX_IQK_CFG_XTAL_32M,	//7
    SSV6XXX_IQK_CFG_XTAL_MAX
} ssv6xxx_iqk_cfg_xtal;
#endif

#define XTAL_16M        5
#define XTAL_24M        2
#define XTAL_26M        0
#define XTAL_40M        1
#define XTAL_12M        4
#define XTAL_20M        6
#define XTAL_32M        7


#if (defined(SDRV_INCLUDE_SDIO) && SDRV_INCLUDE_SDIO)
//#define XTAL_SETTING	XTAL_26M
#define XTAL_SETTING	XTAL_24M
#endif

#if (defined(SDRV_INCLUDE_SPI) && SDRV_INCLUDE_SPI)
#define XTAL_SETTING	XTAL_24M
#endif


//------------------------LOW POWER--------------------
#define PS_SLEEP_GPIO_CTRL      0x00028018
#define PS_DORMANT_GPIO_CTRL    0x0002b718
#define PS_KEEPALIVE            30000
#define PS_USERDTIM             0
#define PS_WAKEUPPIN            1
#define PS_IGNOREDISCONNECTION  0
#define PS_DATARETX_GUARD       0
#define PS_BEACONLOSS_GUARD     0
#define PS_BEACONLOSS_MAXCNT    0
#define PS_DRV_SDIO_INTERFACE   1   // 1: sdio interface, 0 usb interface
#define PS_WOW_ANY_METHOD       0   // 1: any packet wakeup, 0: magic packet wakeup  

//------------------------WPS--------------------
//If we set ENABLE_WPS to 0, we should set ELOOP_TASK_STACK_SIZE to 0 in porting.h at the same time
#define ENABLE_WPS       1
#define WPS_WAKE_TIMEOUT 60000 //unit is ms

#define FAST_CONNECT_PROFILE 0
#define FORCE_CUSTOM_RFTABLE  0 // 0 -> update by efuse item, 1 -> no update by efuse item
#define BACKOFF_ENABLE 1
#endif /* _SIM_CONFIG_H_ */
