#ifndef __MAC_H__
#define __MAC_H__

#pragma pack(push,1)

typedef struct _MAC_T{
    unsigned char MAC[6];    /* mac address */
    unsigned char reserved[2];    /* 4byte alignment */
}MAC_T;

#define MAC_STRUCT_SIZE    8

#pragma pack(pop)

#endif /* __MAC_H__ */
