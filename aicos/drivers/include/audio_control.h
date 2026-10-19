/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is audio HAL, interface prototypes and structs are defined
 *    here, by which, application can control audio route and set parameter.
 */

#ifndef __AUDIO_CONTROL_H__
#define __AUDIO_CONTROL_H__

enum audio_ctl_type {
    AUDIO_CTL_TYPE_BOOL = 0,
    AUDIO_CTL_TYPE_INT,
    AUDIO_CTL_TYPE_ENUM,
    AUDIO_CTL_TYPE_BYTE,
    AUDIO_CTL_TYPE_INT64,
    AUDIO_CTL_TYPE_UNKNOWN,

    AUDIO_CTL_TYPE_MAX,
};

struct audio_ctl;

unsigned int audio_ctl_get_num(void);
const char *audio_ctl_get_name(struct audio_ctl *ctl);
const char *audio_ctl_get_type_string(struct audio_ctl *ctl);
enum audio_ctl_type audio_ctl_get_type(struct audio_ctl *ctl);

struct audio_ctl *audio_clt_get(unsigned int id);
struct audio_ctl *audio_ctl_get_by_name(const char *name);

int audio_ctl_get_value(struct audio_ctl *ctl, unsigned int id);
int audio_ctl_set_value(struct audio_ctl *ctl, unsigned int id, int value);
int audio_ctl_set_array(struct audio_ctl *ctl, const void *array, size_t count);
int audio_ctl_get_array(struct audio_ctl *ctl, void *array, size_t count);
int audio_ctl_get_range_min(struct audio_ctl *ctl);
int audio_ctl_get_range_max(struct audio_ctl *ctl);

unsigned int audio_ctl_get_num_enums(struct audio_ctl *ctl);
int audio_ctl_set_enum_by_string(struct audio_ctl *ctl, const char *string);
unsigned int audio_ctl_get_num_values(struct audio_ctl *ctl);
const char *audio_ctl_get_enum_string(struct audio_ctl *ctl,
                                      unsigned int enum_id);

#endif /* __AUDIO_CONTROL_H__ */
