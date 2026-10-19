#ifndef __AIC_FIXED_APNS_H__
#define __AIC_FIXED_APNS_H__

#pragma pack(push,1)

typedef struct _APN_INFO{
    unsigned char imsi[16];    /* imsi */
    unsigned char apn[40];    /* apn */
}APN_INFO;

typedef struct _APN_INFO_FIXED{
    APN_INFO apn_info[1];
}APN_INFO_FIXED;


#define AIC_FIXED_APNS_STRUCT_SIZE    56

#pragma pack(pop)

#endif /* __AIC_FIXED_APNS_H__ */
