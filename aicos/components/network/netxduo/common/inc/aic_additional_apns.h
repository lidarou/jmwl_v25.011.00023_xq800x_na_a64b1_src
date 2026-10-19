#ifndef __AIC_ADDITIONAL_APNS_H__
#define __AIC_ADDITIONAL_APNS_H__

#pragma pack(push,1)

typedef struct _CARRIER_APN{
    unsigned char imsi[16];    /* imsi */
    unsigned char apn[40];    /* apn */
    unsigned short iccid_cmp_len;    /* iccid cmp len */
    unsigned char iccid[24];    /* iccid */
    unsigned char auth_prot;    /* auth protocol */
    unsigned char userid[20];    /* userid */
    unsigned char password[20];    /* password */
}CARRIER_APN;

typedef struct _APN_INFO_ADDITIONAL{
    CARRIER_APN carrier_apn[1];
    unsigned char reserved[1];    /* 4byte alignment */
}APN_INFO_ADDITIONAL;

#define AIC_ADDITIONAL_APNS_STRUCT_SIZE    124

#pragma pack(pop)

#endif /* __AIC_ADDITIONAL_APNS_H__ */
