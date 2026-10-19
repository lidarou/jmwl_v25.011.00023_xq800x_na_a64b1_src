#ifndef __AIC_DEBUG_H__
#define __AIC_DEBUG_H__

#pragma pack(push,1)

typedef struct _AIC_AT{
    unsigned int baudrate;    /* The baudrate of the at uart com. */
}AIC_AT;

typedef struct _AIC_DIAG{
    unsigned char device;    /* bit4~bit7: device, bit0~bit3: com. */
    unsigned char pad1;    /* pad1. */
    unsigned short pad2;    /* pad2. */
    unsigned int baudrate;    /* The baudrate of the com. */
}AIC_DIAG;

typedef struct _AIC_LOG{
    unsigned char device;    /* bit4~bit7: device, bit0~bit3: com. */
    unsigned char gate;    /* bit0: trace, bit1: msg, bit2: data. */
    unsigned char bak_dev;    /* bak dev. */
    unsigned char pad;    /* pad. */
    unsigned int baudrate;    /* The baudrate of the com. */
    unsigned long level_bitmap[32];    /* bitmap:bit0~bit3: the level of the module 1. */
}AIC_LOG;

typedef struct _AIC_DEBUG{
    unsigned short magic;    /* magic, 0x5a5a means has been inited. */
    unsigned char sysdump;    /* bit0: ap, bit1: cp, bit2: adsp. */
    unsigned char autoreset;    /* bit0: ap, bit1: cp, bit2: adsp. */
    unsigned int debug_gate;    /* bit0~bit32, can used as 32 debug function switch. */
    unsigned char sleep_mode;    /* bit1: sleep_mode off/sleep_mode on; bit2: buck0 deepsleep off/on; bit3: buck1 deepsleep off/on */
    unsigned char status_light;    /* bit0: net status light, bit1:devcie status light, bit2:module work status */
    AIC_AT aic_at;
    AIC_DIAG aic_diag;
    AIC_LOG aic_log;
    unsigned int uart_datadump[6];    /* 0: no data dump, 1: Dump as char stream, 2: Dump as hex stream. */
    unsigned char reserved[2];    /* 4byte alignment */
}AIC_DEBUG;

#define AIC_DEBUG_STRUCT_SIZE    184

#pragma pack(pop)

#endif /* __AIC_DEBUG_H__ */
