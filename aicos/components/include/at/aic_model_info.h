#ifndef __AIC_MODEL_INFO_H__
#define __AIC_MODEL_INFO_H__

#pragma pack(push,1)

typedef struct _MODEL_INFO{
    unsigned char model_name[60];    /* model_info */
}MODEL_INFO;


#define AIC_MODEL_INFO_STRUCT_SIZE    60

#pragma pack(pop)

#endif /* __AIC_MODEL_INFO_H__ */
