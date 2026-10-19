#ifndef __SN_H__
#define __SN_H__

#pragma pack(push,1)

typedef struct _SN_T{
    unsigned char SN[64];    /* SN */
}SN_T;


#define SN_STRUCT_SIZE    64

#pragma pack(pop)

#endif /* __SN_H__ */
