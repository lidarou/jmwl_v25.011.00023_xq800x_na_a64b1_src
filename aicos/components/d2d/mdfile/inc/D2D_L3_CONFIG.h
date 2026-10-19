#ifndef __D2D_L3_CONFIG_H__
#define __D2D_L3_CONFIG_H__

#pragma pack(push,1)

typedef struct _PBCH_DTX_CONFIG{
    unsigned char usr_num_threshold_1;    /* PBCH DTX user number threshold 1 */
    unsigned char usr_num_threshold_2;    /* PBCH DTX user number threshold 2 */
    unsigned char usr_num_threshold_3;    /* PBCH DTX user number threshold 3 */
    unsigned char dtx_level_1;    /* PBCH DTX level 1 */
    unsigned char dtx_level_2;    /* PBCH DTX level 2 */
    unsigned char dtx_level_3;    /* PBCH DTX level 3 */
}PBCH_DTX_CONFIG;

typedef struct _LCID_PARAM{
    short lcid;    /* lcid */
    short priority;    /* priority of lcid 0 */
    short l2_mode;    /* l2 mode: 0:UM, 1:AM */
    short t_reordering;    /* t-reordering time */
    short t_discard;    /* t-discard time */
    short pollingNum;    /* polling number of PDU */
    short status_prohibit_time;    /* polling number of PDU */
}LCID_PARAM;

typedef struct _PBCH_CONFIG{
    PBCH_DTX_CONFIG pbch_dtx_config;
    short pbch_tx_power;    /* PBCH initial TX power */
    short max_tx_power;    /* Maximum TX power */
}PBCH_CONFIG;

typedef struct _LCID_CONFIG{
    unsigned short lcid_num;    /* number of valid lcid */
    LCID_PARAM lcid_param[9];
}LCID_CONFIG;

typedef struct _D2D_L3_CONFIG{
    PBCH_CONFIG pbch_config;
    LCID_CONFIG lcid_config;
    unsigned char reserved[2];    /* 4byte alignment */
}D2D_L3_CONFIG;

#define D2D_L3_CONFIG_STRUCT_SIZE    140

#pragma pack(pop)

#endif /* __D2D_L3_CONFIG_H__ */
