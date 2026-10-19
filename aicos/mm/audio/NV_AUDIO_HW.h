#ifndef _NV_AUIDIO_HW_H
#define _NV_AUIDIO_HW_H

#pragma pack(push,1)

typedef struct _NV_AUDIO_HW_T{
	char name;	/* nv name */
	unsigned short gain[6];	/* gain */
}NV_AUDIO_HW_T;

typedef struct _NV_PARAM_TYPE_HW_T{
	NV_AUDIO_HW_T nv_audio_hw_t[5];
	unsigned char reserved[3];	/* 4byte alignment */
}NV_PARAM_TYPE_HW_T;

#define NV_AUIDIO_HW_STRUCT_SIZE	68

#pragma pack(pop)

#endif

