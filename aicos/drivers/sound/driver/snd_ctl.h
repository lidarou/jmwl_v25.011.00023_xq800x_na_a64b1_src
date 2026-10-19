/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is about sound control, interface and related struct, MACRO
 *    and enum are defined here.
 *    Application or HAL can control audio route, set parameter by these
 *    interface.
 */

#ifndef __SND_CTL_H__
#define __SND_CTL_H__

#include "snd_compat.h"

#define MAX_CTL_VOLATILE 5

#define snd_kcontrol_chip(kcontrol)             ((kcontrol)->private_data)

#define SNDRV_CTL_ELEM_IFACE_MIXER              (2)       /* virtual mixer device */

#define SNDRV_CTL_ELEM_ACCESS_TLV_READ          (1<<4)    /* TLV read is possible */
#define SNDRV_CTL_ELEM_ACCESS_TLV_WRITE         (1<<5)    /* TLV write is possible */
#define SNDRV_CTL_ELEM_ACCESS_READ              (1<<0)
#define SNDRV_CTL_ELEM_ACCESS_WRITE             (1<<1)
#define SNDRV_CTL_ELEM_ACCESS_READWRITE         (SNDRV_CTL_ELEM_ACCESS_READ|SNDRV_CTL_ELEM_ACCESS_WRITE)
#define SNDRV_CTL_ELEM_ACCESS_VOLATILE          (1<<2)    /* control value may be changed without a notification */
#define SNDRV_CTL_ELEM_ACCESS_TIMESTAMP         (1<<3)    /* when was control changed */
#define SNDRV_CTL_ELEM_ACCESS_TLV_READ          (1<<4)    /* TLV read is possible */
#define SNDRV_CTL_ELEM_ACCESS_TLV_WRITE         (1<<5)    /* TLV write is possible */
#define SNDRV_CTL_ELEM_ACCESS_TLV_READWRITE     (SNDRV_CTL_ELEM_ACCESS_TLV_READ|SNDRV_CTL_ELEM_ACCESS_TLV_WRITE)
#define SNDRV_CTL_ELEM_ACCESS_TLV_COMMAND       (1<<6)    /* TLV command is possible */
#define SNDRV_CTL_ELEM_ACCESS_INACTIVE          (1<<8)    /* control does actually nothing, but may be updated */
#define SNDRV_CTL_ELEM_ACCESS_LOCK              (1<<9)    /* write lock */
#define SNDRV_CTL_ELEM_ACCESS_OWNER             (1<<10)    /* write lock owner */
#define SNDRV_CTL_ELEM_ACCESS_TLV_CALLBACK      (1<<28)    /* kernel use a TLV callback */
#define SNDRV_CTL_ELEM_ACCESS_USER              (1<<29)    /* user space element */


typedef int snd_ctl_elem_type_t;
#define SNDRV_CTL_ELEM_TYPE_NONE         ((__force snd_ctl_elem_type_t) 0) /* invalid */
#define SNDRV_CTL_ELEM_TYPE_BOOLEAN      ((__force snd_ctl_elem_type_t) 1) /* boolean type */
#define SNDRV_CTL_ELEM_TYPE_INTEGER      ((__force snd_ctl_elem_type_t) 2) /* integer type */
#define SNDRV_CTL_ELEM_TYPE_ENUMERATED   ((__force snd_ctl_elem_type_t) 3) /* enumerated type */
#define SNDRV_CTL_ELEM_TYPE_BYTES        ((__force snd_ctl_elem_type_t) 4) /* byte array */
#define SNDRV_CTL_ELEM_TYPE_IEC958       ((__force snd_ctl_elem_type_t) 5) /* IEC958 (S/PDIF) setup */
#define SNDRV_CTL_ELEM_TYPE_INTEGER64    ((__force snd_ctl_elem_type_t) 6) /* 64-bit integer type */
#define SNDRV_CTL_ELEM_TYPE_LAST         SNDRV_CTL_ELEM_TYPE_INTEGER64

#define SNDRV_CTL_TLVT_CONTAINER        0    /* one level down - group of TLVs */
#define SNDRV_CTL_TLVT_DB_SCALE         1    /* dB scale */
#define SNDRV_CTL_TLVT_DB_LINEAR        2    /* linear volume */
#define SNDRV_CTL_TLVT_DB_RANGE         3    /* dB range container */
#define SNDRV_CTL_TLVT_DB_MINMAX        4    /* dB scale with min/max */
#define SNDRV_CTL_TLVT_DB_MINMAX_MUTE   5    /* dB scale with min/max with mute */

#define TLV_DB_SCALE_MASK    0xffff
#define TLV_DB_SCALE_MUTE    0x10000
#define TLV_DB_SCALE_ITEM(min, step, mute)            \
    SNDRV_CTL_TLVT_DB_SCALE, 2 * sizeof(unsigned int),    \
    (min), ((step) & TLV_DB_SCALE_MASK) | ((mute) ? TLV_DB_SCALE_MUTE : 0)
#define DECLARE_TLV_DB_SCALE(name, min, step, mute) \
    unsigned int name[] = { TLV_DB_SCALE_ITEM(min, step, mute) }


#define SOC_SINGLE_EXT_TLV(xname, xreg, xshift, xmax, xinvert,\
     xhandler_get, xhandler_put, tlv_array) \
{   .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
    .access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
         SNDRV_CTL_ELEM_ACCESS_READWRITE,\
    .tlv.p = (tlv_array), \
    .info = snd_ctl_info_volsw, \
    .get = xhandler_get, .put = xhandler_put, \
    .private_value = SOC_SINGLE_VALUE(xreg, xshift, xmax, xinvert) }
#define SOC_SINGLE_EXT(xname, xreg, xshift, xmax, xinvert,\
          xhandler_get, xhandler_put) \
{    .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
     .info = snd_ctl_info_volsw, \
     .get = xhandler_get, .put = xhandler_put, \
     .private_value = SOC_SINGLE_VALUE(xreg, xshift, xmax, xinvert) }

#define SOC_DOUBLE_VALUE(xreg, shift_left, shift_right, xmax, xinvert) \
    ((unsigned long)&(struct soc_mixer_control) \
{   .reg = xreg, .rreg = xreg, .shift = shift_left, \
    .rshift = shift_right, .max = xmax, .platform_max = xmax, \
    .invert = xinvert})
#define SOC_SINGLE_VALUE(xreg, xshift, xmax, xinvert) \
    SOC_DOUBLE_VALUE(xreg, xshift, xshift, xmax, xinvert)
#define SOC_SINGLE(xname, reg, shift, max, invert) \
{   .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
    .info = snd_ctl_info_volsw, .get = snd_ctl_get_volsw,\
    .put = snd_ctl_put_volsw, \
    .private_value = SOC_SINGLE_VALUE(reg, shift, max, invert) }
#define SOC_SINGLE_TLV(xname, reg, shift, max, invert, tlv_array) \
{   .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
    .access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
         SNDRV_CTL_ELEM_ACCESS_READWRITE,\
    .tlv.p = (tlv_array), \
    .info = snd_ctl_info_volsw, .get = snd_ctl_get_volsw,\
    .put = snd_ctl_put_volsw, \
    .private_value = SOC_SINGLE_VALUE(reg, shift, max, invert) }

#define SOC_ENUM_DOUBLE(xreg, xshift_l, xshift_r, xmax, xtexts) \
{   .reg = xreg, .shift_l = xshift_l, .shift_r = xshift_r, \
    .max = xmax, .texts = xtexts }
#define SOC_VALUE_ENUM_DOUBLE(xreg, xshift_l, xshift_r, xmask, xmax, xtexts, xvalues) \
{   .reg = xreg, .shift_l = xshift_l, .shift_r = xshift_r, \
    .mask = xmask, .max = xmax, .texts = xtexts, .values = xvalues}
#define SOC_ENUM_SINGLE(xreg, xshift, xmax, xtexts) \
    SOC_ENUM_DOUBLE(xreg, xshift, xshift, xmax, xtexts)
#define SOC_ENUM_SINGLE_EXT(xmax, xtexts) \
{    .max = xmax, .texts = xtexts }
#define SOC_VALUE_ENUM_SINGLE(xreg, xshift, xmask, xitems, xtexts, xvalues) \
    SOC_VALUE_ENUM_DOUBLE(xreg, xshift, xshift, xmask, xitems, xtexts, xvalues)

#define SOC_ENUM_DOUBLE_DECL(name, xreg, xshift_l, xshift_r, xtexts) \
    struct soc_enum name = SOC_ENUM_DOUBLE(xreg, xshift_l, xshift_r, \
                        ARRAY_SIZE(xtexts), xtexts)
#define SOC_ENUM_SINGLE_DECL(name, xreg, xshift, xtexts) \
    SOC_ENUM_DOUBLE_DECL(name, xreg, xshift, xshift, xtexts)
#define SOC_ENUM_SINGLE_EXT_DECL(name, xtexts) \
    struct soc_enum name = SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(xtexts), xtexts)
#define SOC_VALUE_ENUM_DOUBLE_DECL(name, xreg, xshift_l, xshift_r, xmask, xtexts, xvalues) \
    struct soc_enum name = SOC_VALUE_ENUM_DOUBLE(xreg, xshift_l, xshift_r, xmask, \
                            ARRAY_SIZE(xtexts), xtexts, xvalues)
#define SOC_VALUE_ENUM_SINGLE_DECL(name, xreg, xshift, xmask, xtexts, xvalues) \
    SOC_VALUE_ENUM_DOUBLE_DECL(name, xreg, xshift, xshift, xmask, xtexts, xvalues)
#define SOC_ENUM_EXT(xname, xenum, xhandler_get, xhandler_put) \
{   .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
    .info = snd_ctl_info_enum_ext, \
    .get = xhandler_get, .put = xhandler_put, \
    .private_value = (unsigned long)&xenum }
#define SOC_ENUM(xname, xenum) \
{   .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname,\
    .info = snd_ctl_info_enum_double, \
    .get = snd_ctl_get_enum_double, .put = snd_ctl_put_enum_double, \
    .private_value = (unsigned long)&xenum }

/* enumerated kcontrol */
struct soc_enum {
    unsigned short reg;
    unsigned short reg2;
    unsigned char shift_l;
    unsigned char shift_r;
    unsigned int max;
    unsigned int mask;
    const char * const *texts;
    const unsigned int *values;
    void *dapm;
};

/* mixer control */
struct soc_mixer_control {
    int min, max, platform_max;
    unsigned int reg, rreg, shift, rshift, invert;
};

struct snd_ctl_elem_id {
    unsigned int numid;     /* numeric identifier, zero = invalid */
    int iface;              /* interface identifier */
    unsigned int device;    /* device/client number */
    unsigned int subdevice; /* subdevice (substream) number */
    char name[44];          /* ASCII name of item */
    unsigned int index;     /* index of item */
};

struct snd_ctl_elem_value {
    struct snd_ctl_elem_id id;  /* W: element ID */
    unsigned int indirect: 1;   /* W: indirect access - obsoleted */
    union {
        union {
            long value[128];
            long *value_ptr;    /* obsoleted */
        } integer;
        union {
            long long value[64];
            long long *value_ptr;       /* obsoleted */
        } integer64;
        union {
            unsigned int item[128];
            unsigned int *item_ptr;     /* obsoleted */
        } enumerated;
        union {
            unsigned char data[512];
            unsigned char *data_ptr;    /* obsoleted */
        } bytes;

    } value;        /* RO */
};

struct snd_ctl_elem_info {
    struct snd_ctl_elem_id id;    /* W: element ID */
    int type;           /* R: value type - SNDRV_CTL_ELEM_TYPE_* */
    unsigned int access;/* R:value access (bitmask) - SNDRV_CTL_ELEM_ACCESS_* */
    unsigned int count; /* count of values */

    union {
        struct {
            long min;        /* R: minimum value */
            long max;        /* R: maximum value */
            long step;       /* R: step (0 variable) */
        } integer;
        struct {
            long long min;        /* R: minimum value */
            long long max;        /* R: maximum value */
            long long step;       /* R: step (0 variable) */
        } integer64;
        struct {
            unsigned int items;     /* R: number of items */
            unsigned int item;      /* W: item number */
            char *name;             /* R: value name, point to name */
            __u64 names_ptr;        /* W: names list (ELEM_ADD only) */
            unsigned int names_length;
        } enumerated;
        unsigned char reserved[128];
    } value;
    union {
        unsigned short d[4];        /* dimensions */
        unsigned short *d_ptr;      /* indirect - obsoleted */
    } dimen;
    unsigned char reserved[64-4*sizeof(unsigned short)];
};



struct snd_kcontrol;
typedef int (snd_kcontrol_info_t) (struct snd_kcontrol * kcontrol,
                                   struct snd_ctl_elem_info * uinfo);
typedef int (snd_kcontrol_get_t) (struct snd_kcontrol * kcontrol,
                                  struct snd_ctl_elem_value * ucontrol);
typedef int (snd_kcontrol_put_t) (struct snd_kcontrol * kcontrol,
                                  struct snd_ctl_elem_value * ucontrol);
typedef int (snd_kcontrol_tlv_rw_t)(struct snd_kcontrol *kcontrol,
                                    int op_flag, /* 0=read,1=write,-1=command */
                                    unsigned int size,
                                    unsigned int *tlv);

struct snd_kcontrol_new {
    int iface;              /* interface identifier */
    unsigned int device;    /* device/client number */
    unsigned int subdevice; /* subdevice (substream) number */
    char *name;             /* ASCII name of item */
    unsigned int index;     /* index of item */
    unsigned int access;    /* access rights */
    unsigned int count;     /* count of same elements */
    snd_kcontrol_info_t *info;
    snd_kcontrol_get_t *get;
    snd_kcontrol_put_t *put;
    union {
        snd_kcontrol_tlv_rw_t *c;
        const unsigned int *p;
    } tlv;
    unsigned long private_value;
};

struct snd_kcontrol_volatile {
    //struct snd_ctl_file *owner;    /* locked */
    unsigned int access;    /* access rights */
};

struct snd_kcontrol {
    struct list_head list;          /* list of controls */
    struct snd_ctl_elem_id id;
    unsigned int count;             /* count of same elements */
    snd_kcontrol_info_t *info;
    snd_kcontrol_get_t *get;
    snd_kcontrol_put_t *put;
    union {
        snd_kcontrol_tlv_rw_t *c;
        const unsigned int *p;
    } tlv;
    unsigned long private_value;
    void *private_data;
    void (*private_free)(struct snd_kcontrol *kcontrol);
    struct snd_kcontrol_volatile vd[MAX_CTL_VOLATILE];    /* volatile data */
};


int snd_ctl_add(struct snd_kcontrol *kcontrol);
int snd_ctl_read(struct snd_ctl_elem_value *control);
int snd_ctl_write(struct snd_ctl_elem_value *control);
int snd_ctl_get_cnt(void);
int snd_ctl_get_info(struct snd_ctl_elem_value *control,
                     struct snd_ctl_elem_info *info);
int snd_ctl_get_info_numid(int numid, struct snd_ctl_elem_info *info);
struct snd_kcontrol *snd_ctl_cnew(const struct snd_kcontrol_new *_template,
                                  void *data, char *long_name,
                                  const char *prefix);

int snd_ctl_info_volsw(struct snd_kcontrol *kcontrol,
                       struct snd_ctl_elem_info *uinfo);
int snd_ctl_info_enum_ext(struct snd_kcontrol *kcontrol,
                          struct snd_ctl_elem_info *uinfo);
int snd_ctl_info_enum_double(struct snd_kcontrol *kcontrol,
                             struct snd_ctl_elem_info *uinfo);
int snd_ctl_get_enum_double(struct snd_kcontrol *kcontrol,
                            struct snd_ctl_elem_value *ucontrol);
int snd_ctl_put_enum_double(struct snd_kcontrol *kcontrol,
                            struct snd_ctl_elem_value *ucontrol);
int snd_ctl_get_volsw(struct snd_kcontrol *kcontrol,
                      struct snd_ctl_elem_value *ucontrol);
int snd_ctl_put_volsw(struct snd_kcontrol *kcontrol,
                      struct snd_ctl_elem_value *ucontrol);

#endif /* __SND_CTL_H__ */

