#ifndef _NV_AUDIO_CONFIG_H
#define _NV_AUDIO_CONFIG_H

#pragma pack(push,1)

typedef struct _NV_AUDIO_CONFIG_T{
	char name;	/* nv name */
	unsigned short gain[6];	/* gain */
}NV_AUDIO_CONFIG_T;

typedef struct _NV_PARAM_TYPE_AUDIO_T{
	NV_AUDIO_CONFIG_T nv_audio_config_t[5];
	unsigned char reserved[3];	/* 4byte alignment */
}NV_PARAM_TYPE_AUDIO_T;

#define NV_AUDIO_CONFIG_STRUCT_SIZE	68

#pragma pack(pop)

#endif

