#ifndef __TTS_H__
#define __TTS_H__

#pragma pack(push,1)

typedef struct _TTS_T{
    unsigned char TTS[256];    /* TTS */
}TTS_T;


#define TTS_STRUCT_SIZE    256

#pragma pack(pop)

#endif /* __TTS_H__ */
