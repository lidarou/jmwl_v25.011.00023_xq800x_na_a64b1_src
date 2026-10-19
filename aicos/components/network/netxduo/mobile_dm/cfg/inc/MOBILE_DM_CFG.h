#ifndef __MOBILE_DM_CFG_H__
#define __MOBILE_DM_CFG_H__

#pragma pack(push,1)

typedef struct _MOBILE_DM_FILED_CFG{
    unsigned char api_ver[8];    /* api version */
    unsigned char app_key[16];    /* app key */
    unsigned char platform[2];    /* platform */
    unsigned char mac[18];    /* mac address */
    unsigned char rom[6];    /* rom size */
    unsigned char ram[6];    /* ram size */
    unsigned char cpu[10];    /* cpu type */
    unsigned char sys_ver[16];    /* system version */
    unsigned char sw_ver[16];    /* software version */
    unsigned char sw_name[16];    /* software name */
    int volte;    /* volte support flag, 0: enable, 1: disable, -1: not support */
    unsigned char net_type[6];    /* net type: wifi, 4G, 5G */
    unsigned char router_mac[18];    /* router mac address */
    unsigned char bt_mac[18];    /* bluetooth mac address */
    unsigned char brand[10];    /* brand name */
    unsigned char model[10];    /* model */
    unsigned char gpu[16];    /* gpu type */
    unsigned char board[16];    /* board information */
    unsigned char resolution[10];    /* resolution */
    unsigned int battery_capacity;    /* battery capacity (mah) */
    unsigned char screen_size[8];    /* screen size */
    int network_status;    /* network status. 1: connect, 0: not connect, -1: not support */
    int wearing_status;    /* wearing status. 1: wearing, 0: not wearing, -1: not support */
    unsigned char app_info[16];    /* app information */
}MOBILE_DM_FILED_CFG;

typedef struct _MOBILE_DM_REPORT_INFO{
    unsigned int report_num;    /* statistics of report number. */
    unsigned int report_num_max;    /* maximum report number from config. */
    unsigned int retry_num;    /* statistics of retry number. */
    unsigned int retry_num_max;    /* maximum retry number from config */
    unsigned long long report_start_time;    /* start time of first report. */
    unsigned int report_time;    /* maximum report time. */
    unsigned int retry_interval;    /* retry interval. */
}MOBILE_DM_REPORT_INFO;

typedef struct _MOBILE_DM_HTTP_INFO{
    unsigned char url[64];    /* http url */
}MOBILE_DM_HTTP_INFO;

typedef struct _MOBILE_DM_ENCRYPT_INFO{
    unsigned char key[34];    /* aes key */
}MOBILE_DM_ENCRYPT_INFO;

typedef struct _MOBILE_DM_CFG{
    MOBILE_DM_FILED_CFG mobile_dm_filed_cfg;
    MOBILE_DM_REPORT_INFO mobile_dm_report_info;
    MOBILE_DM_HTTP_INFO mobile_dm_http_info;
    MOBILE_DM_ENCRYPT_INFO mobile_dm_encrypt_info;
}MOBILE_DM_CFG;


#define MOBILE_DM_CFG_STRUCT_SIZE    388

#pragma pack(pop)

#endif /* __MOBILE_DM_CFG_H__ */
