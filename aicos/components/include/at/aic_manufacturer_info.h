#ifndef __AIC_MANUFACTURER_INFO_H__
#define __AIC_MANUFACTURER_INFO_H__

#pragma pack(push,1)

typedef struct _MANUFACTURER_INFO{
    unsigned char manufacturer_name[40];    /* manufacturer_info */
}MANUFACTURER_INFO;


#define AIC_MANUFACTURER_INFO_STRUCT_SIZE    40

#pragma pack(pop)

#endif /* __AIC_MANUFACTURER_INFO_H__ */
