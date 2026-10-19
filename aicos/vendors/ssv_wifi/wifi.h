#ifndef _WIFI_H_
#define _WIFI_H_

//#include "lwip/netif.h"
//#include "lwip/netifapi.h"

#define SSV_MAC_ADDR_LEN  	6
#define SSV_MAX_SSID_LEN 	32
#define SSV_MIN_KEY_LEN  	8
#define SSV_MAX_KEY_LEN  	64
#define SSV_MAX_AP_COUNT 	50
#define SSV_MAX_STA_COUNT 	8

#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

typedef enum
{
	WIFI_MODE_STA = 0, 
	WIFI_MODE_AP,
	WIFI_MODE_SMARTCFG,
	WIFI_MODE_AP_STA,
	WIFI_MODE_UNKNOWN
}SSV_WIFI_MODE;

typedef enum
{
	KEY_NONE = 0, KEY_WEP, KEY_WPA, KEY_WPA2, KEY_MAX_VALUE
} SSV_WIFI_SECURITY_TYPE;

typedef struct _stacfg
{
	unsigned char ssid[SSV_MAX_SSID_LEN];
	unsigned int ssid_len;
	unsigned char key[SSV_MAX_KEY_LEN];
	unsigned int key_len;
	unsigned char channel;
    unsigned char bssid[SSV_MAC_ADDR_LEN];
}ssv_wifi_stacfg_t;

typedef struct _apcfg
{
	unsigned char ssid[SSV_MAX_SSID_LEN];
	unsigned int ssid_len;
	unsigned char channel;
	SSV_WIFI_SECURITY_TYPE  authmode; // encryption protocol  eg.wep, wpa, wpa2 
	unsigned char key[SSV_MAX_KEY_LEN];
	unsigned int key_len;
    unsigned char is_hidden;
    unsigned char max_sta_count;
}ssv_wifi_apcfg_t;

typedef struct ssv_wifi_ap_info {
    unsigned char   ssid[SSV_MAX_SSID_LEN];
    unsigned char   bssid[SSV_MAC_ADDR_LEN];
    unsigned int  channel;
    SSV_WIFI_SECURITY_TYPE  security;
    unsigned int  rssi;
} ssv_wifi_ap_info_t;

typedef struct ssv_wifi_ap_list
{
	unsigned short  ap_count;
	ssv_wifi_ap_info_t ap_info[SSV_MAX_AP_COUNT];
}ssv_wifi_ap_list_t;

typedef struct ssv_wifi_sta_info 
{
    unsigned char  mac_address[SSV_MAC_ADDR_LEN];
	unsigned char  rssi;
} ssv_wifi_sta_info_t;

typedef struct ssv_wifi_sta_list
{
	unsigned short  sta_count;
	ssv_wifi_sta_info_t sta_info[SSV_MAX_STA_COUNT];
} ssv_wifi_sta_list_t;

typedef enum _ssv_wifi_dhcp_mode{
    DHCP_MODE_STATIC,           // static every time
    DHCP_MODE_DYNAMIC_FIRST,    // dhcp first at wifi init, get ip from back at wifi resume  
    DHCP_MODE_DYNAMIC_EVERY,    // dhcp every time
}ssv_wifi_dhcp_mode;

#if 0
typedef struct _ssv_wifi_ip_info
{
	ip_addr_t ipaddr;
	ip_addr_t netmask;
	ip_addr_t gw;
}ssv_wifi_ip_info;
#endif

typedef enum
{
	SCAN_RESULT_EVENT = 0, 
    SCAN_DONE_EVENT,
    JOIN_SUCCESS_EVENT,
    JOIN_FAIL_EVENT,
    LEAVE_RESULT_EVENT,
    ASSOC_IND_EVENT,
    DISASSOC_IND_EVENT,
    UNKNOWN_EVENT
} SSV_WIFI_EVENT;


typedef struct _ssv_wifi_scan_result_data
{
	unsigned int  reserved[16];
}ssv_wifi_scan_result_data;

typedef struct _ssv_wifi_scan_done_data
{
	unsigned int  reserved[1];
}ssv_wifi_scan_done_data;

typedef struct _ssv_wifi_join_data
{
	unsigned int  reserved[1];
}ssv_wifi_join_data;

typedef struct _ssv_wifi_leave_data
{
	unsigned int  reserved[1];
}ssv_wifi_leave_data;

typedef struct _ssv_wifi_auth_deauth_data
{
	unsigned int  reserved[6];
}ssv_wifi_auth_deauth_data;

typedef struct _ssv_wifi_event_data
{
    union
    {
        struct _ssv_wifi_scan_result_data  scan_result_data;
        struct _ssv_wifi_scan_done_data  scan_done_data;
        struct _ssv_wifi_join_data  join_data;
        struct _ssv_wifi_leave_data  leave_data;
        struct _ssv_wifi_auth_deauth_data  auth_deauth_data;
    }data;
}ssv_wifi_event_data;

typedef struct _WLAN_CONNECTION_INFO
{
	unsigned char   Ssid_len;
	unsigned char   Ssid[32];
	int   Rssi;
	int   Phy_rate;
	int   channel;
} WLAN_CONNECTION_INFO;


/* Supported authentication mode. */
/* Values are used to select the authentication mode used to join a network. */
enum {
	WLAN_WPA_AUTH_DISABLED = 0x0000,	/* Legacy (i.e., non-WPA) */
	WLAN_WPA_AUTH_NONE = 0x0001,		/* none (IBSS) */
	WLAN_WPA_AUTH_PSK = 0x0004,		/* Pre-shared key */
	WLAN_WPA2_AUTH_PSK = 0x0080,		/* Pre-shared key */
	WLAN_MIX_AUTH_PSK = 0x0100,	        /* Pre-shared key */
};

typedef int WLAN_AUTH_MODE;


/* WLAN Security Encryption. */
/* Values are used to select the type of encryption used for testing. */
enum {
	WLAN_ENCRYPT_NONE = 0,    /* No encryption. */
	WLAN_ENCRYPT_WEP = 1,     /* WEP encryption. */
	WLAN_ENCRYPT_TKIP = 2,    /* TKIP encryption. */
	WLAN_ENCRYPT_AES = 4,     /* AES encryption. */
	WLAN_ENCRYPT_WSEC = 8,    /* Software WSEC encryption. */
	WLAN_ENCRYPT_WEP_SHARED = 0x11,	  /* WEP shard encryption. */
	WLAN_ENCRYPT_FIPS = 0x80,  /* FIPS encryption. */
};

typedef int WLAN_ENCRYPTION;

typedef int (* wifi_event_handle)(void *enData);

typedef void (* ssv_wifi_event_cb)(SSV_WIFI_EVENT enEvent, ssv_wifi_event_data *enData);

char *ssv_wifi_get_version(void);
SSV_WIFI_MODE ssv_wifi_get_mode(void);
int ssv_wifi_set_mode(SSV_WIFI_MODE wifi_mode);  
int ssv_wifi_isconnected(void); 
int ssv_wifi_start_softap(struct _apcfg *ap_cfg);
int ssv_wifi_stop_softap(void);
int ssv_wifi_start_softap_ext(char *name, char *key, int key_type, int channel, unsigned char is_hidden, int max_sta_count);
int ssv_wifi_get_mac_2(unsigned char *mac_addr ,int index);
int ssv_wifi_softap_list_station(ssv_wifi_sta_list_t *sta_list);
void ssv_wifi_get_ap_list(ssv_wifi_ap_list_t *wifi_ap_list);
int ssv_wifi_get_rssi(void);
int ssv_wifi_scan(unsigned  int channel_mask, unsigned int channel_mask_5g, ssv_wifi_ap_list_t *ap_list);
int ssv_wifi_scan_ext(ssv_wifi_ap_list_t *ap_list);
int ssv_wifi_scan_ssid_atcmd(char* ssid);
int ssv_wifi_scan_ssid_ext(char* ssid, ssv_wifi_ap_list_t *ap_list);
int ssv_wifi_init_mode(int mode);
void ssv_wifi_event_register(ssv_wifi_event_cb cb);
int ssv_wifi_open(int mode);
int ssv_wifi_station_connect(ssv_wifi_stacfg_t *sta_cfg);
int ssv_wifi_station_connect_2(char *ssid, char *password);
int ssv_wifi_station_connect_no_scan( char *ssid, char *key);
int ssv_wifi_station_disconnect(void);
int ssv_wifi_start_station(ssv_wifi_stacfg_t *sta_cfg);
int ssv_wifi_start_station_ext(char *name, char *key, unsigned char channel, unsigned char bssid[SSV_MAC_ADDR_LEN]);
int ssv_wifi_stop_station(void);
void ssv_wifi_get_ap_list(ssv_wifi_ap_list_t *wifi_ap_list);
void ssv_wifi_init(void);
void ssv_wifi_deinit(void);
int ssv_wifi_get_avail_tx_queue_count(void);
void* ssv_wifi_vif_get();
int ssv_wifi_scan_network(char* scan_buf, unsigned int buf_size);
int ssv_wifi_get_connected_info(WLAN_CONNECTION_INFO *wlan_connection_info);
int ssv_wifi_ap_create(char* ssid, int authMode, int encryption, char *key, int channel, int ssidBcst , int max_sta_count);
int ssv_wifi_get_associated_client_list(void *pchBuf, unsigned int uiBufSize);
void ssv_wifi_tx_pkt(void *p, int len);
void ssv_wifi_rx_pkt(void *dev, int len);
int ssv_wifi_ap_acl_set_type(int type);
int ssv_wifi_ap_acl_add_mac(unsigned char *macaddr);
int ssv_wifi_ap_acl_del_mac(unsigned char *macaddr);
int ssv_wifi_ap_acl_get(int *type,  unsigned short *mac_count, unsigned char mac_list[][6]);
void ssv_wifi_ap_mode_open_wps(void);
void ssv_wifi_enter_ps(void);
void ssv_wifi_exit_ps(void);
void ssv_wifi_sleep(unsigned int mSleep);
void ssv_wifi_taskcount();

#endif
