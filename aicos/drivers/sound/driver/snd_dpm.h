/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the basic interface about DPM for sound device driver,
 *    some service prototypes and data structure definitions are defined in
 *    this file, sound HAL maybe include it also.
 */

#ifndef __SND_DPM_H__
#define __SND_DPM_H__

#include "snd_card.h"
#include "snd_ctl.h"

/* dpm stream operations */
#define SND_DPM_STREAM_NOP              0x0
#define SND_DPM_STREAM_START            0x1
#define SND_DPM_STREAM_STOP             0x2
#define SND_DPM_STREAM_SUSPEND          0x4
#define SND_DPM_STREAM_RESUME           0x8
#define SND_DPM_STREAM_PAUSE_PUSH       0x10
#define SND_DPM_STREAM_PAUSE_RELEASE    0x20

/* dpm event types */
#define SND_DPM_PRE_PMU     0x1     /* before widget power up */
#define SND_DPM_POST_PMU    0x2     /* after widget power up */
#define SND_DPM_PRE_PMD     0x4     /* before widget power down */
#define SND_DPM_POST_PMD    0x8     /* after widget power down */
#define SND_DPM_PRE_REG     0x10    /* before audio path setup */
#define SND_DPM_POST_REG    0x20    /* after audio path setup */
#define SND_DPM_PRE_POST_PMD \
    (SND_DPM_PRE_PMD | SND_DPM_POST_PMD)

/* convenience event type detection */
#define SND_DPM_EVENT_ON(e)    \
    (e & (SND_DPM_PRE_PMU | SND_DPM_POST_PMU))
#define SND_DPM_EVENT_OFF(e)    \
    (e & (SND_DPM_PRE_PMD | SND_DPM_POST_PMD))

struct snd_dpm_widget;
enum snd_dpm_type;
struct snd_dpm_path;
struct snd_dpm_route;
struct snd_dpm_context;

int dpm_reg_event(struct snd_dpm_widget *w,
                  struct snd_kcontrol *kcontrol, int event);

/* dpm controls */
int snd_dpm_put_volsw(struct snd_kcontrol *kcontrol,
                      struct snd_ctl_elem_value *ucontrol);
int snd_dpm_get_volsw(struct snd_kcontrol *kcontrol,
                      struct snd_ctl_elem_value *ucontrol);
int snd_dpm_get_enum_double(struct snd_kcontrol *kcontrol,
                            struct snd_ctl_elem_value *ucontrol);
int snd_dpm_put_enum_double(struct snd_kcontrol *kcontrol,
                            struct snd_ctl_elem_value *ucontrol);
int snd_dpm_get_enum_virt(struct snd_kcontrol *kcontrol,
                          struct snd_ctl_elem_value *ucontrol);
int snd_dpm_put_enum_virt(struct snd_kcontrol *kcontrol,
                          struct snd_ctl_elem_value *ucontrol);
int snd_dpm_get_value_enum_double(struct snd_kcontrol *kcontrol,
                                  struct snd_ctl_elem_value *ucontrol);
int snd_dpm_put_value_enum_double(struct snd_kcontrol *kcontrol,
                                  struct snd_ctl_elem_value *ucontrol);
int snd_dpm_info_pin_switch(struct snd_kcontrol *kcontrol,
                            struct snd_ctl_elem_info *uinfo);
int snd_dpm_get_pin_switch(struct snd_kcontrol *kcontrol,
                           struct snd_ctl_elem_value *uncontrol);
int snd_dpm_put_pin_switch(struct snd_kcontrol *kcontrol,
                           struct snd_ctl_elem_value *uncontrol);
int snd_dpm_new_control(struct snd_dpm_context *dpm,
                        const struct snd_dpm_widget *widget);
int snd_dpm_new_controls(struct snd_dpm_context *dpm,
                         const struct snd_dpm_widget *widget, int num);

/* dpm path setup */
int snd_dpm_new_widgets(struct snd_dpm_context *dpm);
void snd_dpm_free(struct snd_dpm_context *dpm);
int snd_dpm_add_routes(struct snd_dpm_context *dpm,
                       const struct snd_dpm_route *route, int num);
int snd_dpm_weak_routes(struct snd_dpm_context *dpm,
                        const struct snd_dpm_route *route, int num);

/* dpm audio pin control and status */
int snd_dpm_enable_pin(struct snd_dpm_context *dpm,
                       const char *pin);
int snd_dpm_disable_pin(struct snd_dpm_context *dpm,
                        const char *pin);
int snd_dpm_nc_pin(struct snd_dpm_context *dpm, const char *pin);
int snd_dpm_get_pin_status(struct snd_dpm_context *dpm,
                           const char *pin);
int snd_dpm_sync(struct snd_dpm_context *dpm);
int snd_dpm_force_enable_pin(struct snd_dpm_context *dpm,
                             const char *pin);
int snd_dpm_enable_pin_sync(char *name);
int snd_dpm_ignore_suspend(struct snd_dpm_context *dpm,
                           const char *pin);

/* Mostly internal - should not normally be used */
void dpm_mark_dirty(struct snd_dpm_widget *w, const char *reason);

/* dpm widget types */
enum snd_dpm_type {
    snd_dpm_input = 0,      /* input pin */
    snd_dpm_output,         /* output pin */
    snd_dpm_mux,            /* selects 1 analog signal from many inputs */
    snd_dpm_virt_mux,       /* virtual version of snd_dpm_mux */
    snd_dpm_value_mux,      /* selects 1 analog signal from many inputs */
    snd_dpm_mixer,          /* mixes several analog signals together */
    snd_dpm_mixer_named_ctl,/* mixer with named controls */
    snd_dpm_pga,            /* programmable gain/attenuation (volume) */
    snd_dpm_out_drv,        /* output driver */
    snd_dpm_adc,            /* analog to digital converter */
    snd_dpm_dac,            /* digital to analog converter */
    snd_dpm_micbias,        /* microphone bias (power) */
    snd_dpm_mic,            /* microphone */
    snd_dpm_hp,             /* headphones */
    snd_dpm_spk,            /* speaker */
    snd_dpm_line,           /* line input/output */
    snd_dpm_switch,         /* analog switch */
    snd_dpm_vmid,           /* codec bias/vmid - to minimise pops */
    snd_dpm_pre,            /* machine specific pre widget - exec first */
    snd_dpm_post,           /* machine specific post widget - exec last */
    snd_dpm_supply,         /* power/clock supply */
    snd_dpm_aif_in,         /* audio interface input */
    snd_dpm_aif_out,        /* audio interface output */
    snd_dpm_siggen,         /* signal generator */
    snd_dpm_max
};

void snd_dpm_list_all(void);
int snd_dpm_get_widget_status(char *name, char *buf, size_t count);

/*
 * DPM audio route definition.
 *
 * Defines an audio route originating at source via control and finishing
 * at sink.
 */
struct snd_dpm_route {
    const char *sink;
    const char *control;
    const char *source;

    /* Note: currently only supported for links where source is a supply */
    int (*connected)(struct snd_dpm_widget *source,
                     struct snd_dpm_widget *sink);
};

/* dpm audio path between two widgets */
struct snd_dpm_path {
    char *name;
    char *long_name;

    /* source (input) and sink (output) widgets */
    struct snd_dpm_widget *source;
    struct snd_dpm_widget *sink;
    struct snd_kcontrol *kcontrol;

    /* status */
    u32 connect:1;    /* source and sink widgets are connected */
    u32 walked:1;    /* path has been walked */
    u32 weak:1;    /* path ignored for power management */

    int (*connected)(struct snd_dpm_widget *source,
                     struct snd_dpm_widget *sink);

    struct list_head list_source;
    struct list_head list_sink;
    struct list_head list;
};

/* dpm widget */
struct snd_dpm_widget {
    enum snd_dpm_type id;
    char *name;        /* widget name */
    char *sname;    /* stream name */
    struct snd_codec *codec;
    struct snd_soc_platform *platform;
    struct list_head list;
    struct snd_dpm_context *dpm;

    /* dpm control */
    short reg;                          /* negative reg = no direct dpm */
    unsigned char shift;                /* bits to shift */
    unsigned int saved_value;           /* widget saved value */
    unsigned int value;                 /* widget current value */
    unsigned int mask;                  /* non-shifted mask */
    unsigned int on_val;                /* on state value */
    unsigned int off_val;               /* off state value */
    unsigned char power:1;              /* block power status */
    unsigned char invert:1;             /* invert the power bit */
    unsigned char active:1;             /* active stream on DAC, ADC's */
    unsigned char connected:1;          /* connected codec pin */
    unsigned char new:1;                /* cnew complete */
    unsigned char ext:1;                /* has external widgets */
    unsigned char force:1;              /* force state */
    unsigned char ignore_suspend:1;     /* kept enabled over suspend */
    unsigned char new_power:1;          /* power from this run */
    unsigned char power_checked:1;      /* power checked this run */
    int subseq;                         /* sort within widget type */

    int (*power_check)(struct snd_dpm_widget *w);

    /* external events */
    unsigned short event_flags;        /* flags to specify event types */
    int (*event)(struct snd_dpm_widget*, struct snd_kcontrol *, int);

    /* kcontrols that relate to this widget */
    int num_kcontrols;
    const struct snd_kcontrol_new *kcontrol_news;
    struct snd_kcontrol **kcontrols;

    /* widget input and outputs */
    struct list_head sources;
    struct list_head sinks;

    /* used during DPM updates */
    struct list_head power_list;
    struct list_head dirty;
    int inputs;
    int outputs;
};

struct snd_dpm_update {
    struct snd_dpm_widget *widget;
    struct snd_kcontrol *kcontrol;
    int reg;
    int mask;
    int val;
};

/* A list of widgets associated with an object, typically a snd_kcontrol */
struct snd_dpm_widget_list {
    int num_widgets;
    struct snd_dpm_widget *widgets[5];
};

struct snd_dpm_stats {
    int power_checks;
    int path_checks;
    int neighbour_checks;
};

int snd_dpm_get_widget_status(char *name, char *buf, size_t count);
void snd_dpm_list_all(void);

#endif /* __SND_DPM_H__ */

