/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defined sound card interface and related struct, codec driver
 *    can regist to this sound card during boot time.
 *    Normaly it's used for driver module.
 */

#ifndef __SND_CARD_H__
#define __SND_CARD_H__

#include "snd_compat.h"
#include "snd_dpm.h"

struct snd_codec;
struct snd_codec_driver;

enum snd_bias_level {
    SND_BIAS_OFF = 0,
    SND_BIAS_STANDBY = 1,
    SND_BIAS_PREPARE = 2,
    SND_BIAS_ON = 3,
};

/* DPM context */
struct snd_dpm_context {
    int n_widgets; /* number of widgets in this context */
    enum snd_bias_level bias_level;
    enum snd_bias_level suspend_bias_level;
    unsigned int idle_bias_off:1; /* Use BIAS_OFF instead of STANDBY */

    struct snd_dpm_update *update;

    void (*seq_notifier)(struct snd_dpm_context *, int, int);

    struct device *dev; /* from parent - for debug */
    struct snd_codec *codec; /* parent codec */
    struct snd_card *card; /* parent card */

    /* used during DPM updates */
    enum snd_bias_level target_bias_level;
    struct list_head list;

    int (*stream_event)(struct snd_dpm_context *dpm, int event);
};

struct snd_card {
    const char *name;

    struct list_head list;
    /* struct mutex mutex; */

    /*
     * Card-specific routes and widgets.
     */
    const struct snd_dpm_widget *dpm_widgets;
    int num_dpm_widgets;
    const struct snd_dpm_route *dpm_routes;
    int num_dpm_routes;

    struct list_head widgets;
    struct list_head paths;
    struct list_head dpm_list;
    struct list_head dpm_dirty;
    struct list_head codecs;
    struct snd_codec *primary_codec;

    /* change to here */
    struct list_head ctls;
    int last_numid;
    /* Generic DPM context for the card */
    struct snd_dpm_context dpm;

    void *drvdata;
};

struct snd_codec {
    const char *name;
    int id;

    struct snd_card *card;
    struct list_head list;
    const struct snd_codec_driver *driver;
    MUTEX_PTR mutex;

    /* codec IO */
    unsigned int (*read)(struct snd_codec *, unsigned int);
    int (*write)(struct snd_codec *, unsigned int, unsigned int);

    /* dpm */
    struct snd_dpm_context dpm;
    unsigned int ignore_pmdown_time:1; /* pmdown_time is ignored at stop */

    u32 private;
};

struct snd_codec_driver {
    /* driver ops */
    int (*probe)(struct snd_codec *);
    int (*remove)(struct snd_codec *);
    int (*suspend)(struct snd_codec *);
    int (*resume)(struct snd_codec *);

    /* Default control and setup, added after probe() is run */
    const struct snd_kcontrol_new *controls;
    int num_controls;
    const struct snd_dpm_widget *dpm_widgets;
    int num_dpm_widgets;
    const struct snd_dpm_route *dpm_routes;
    int num_dpm_routes;

    /* codec wide operations */
    int (*set_sysclk)(struct snd_codec *codec,
                      int clk_id, int source, unsigned int freq, int dir);
    int (*set_pll)(struct snd_codec *codec, int pll_id, int source,
                   unsigned int freq_in, unsigned int freq_out);

    /* codec IO */
    unsigned int (*read)(struct snd_codec *, unsigned int);
    int (*write)(struct snd_codec *, unsigned int, unsigned int);

    /* codec bias level */
    int (*set_bias_level)(struct snd_codec *, enum snd_bias_level level);
    bool idle_bias_off;

    void (*seq_notifier)(struct snd_dpm_context *, int, int);

    int (*set_params)(struct snd_dpm_context *dpm, u32 wid, u32 chn, u32 srate);

    /* codec stream completion event */
    int (*stream_event)(struct snd_dpm_context *dpm, int event);

    /* probe ordering - for components with runtime dependencies */
    int probe_order;
    int remove_order;
};

int snd_card_add_controls(const struct snd_kcontrol_new *controls,
                          int num_controls, const char *prefix, void *data);
int snd_card_add_widgets_routes(const struct snd_dpm_widget *widget, int wnum,
                               const struct snd_dpm_route *route, int rnum);
int snd_card_set_param(char *name, u32 wid, u32 chn, u32 srate);
int snd_card_stream_event(char *stream, int event);
int snd_card_enable_pin(char *name);
int snd_card_register_codec(char *name,
                            const struct snd_codec_driver *codec_drv);
int snd_card_register_aux_codec(char *name,
                                const struct snd_codec_driver *codec_drv);
int snd_card_set_codec_data_by_name(char *name, u32 private_data);
int snd_card_set_codec_data(struct snd_codec *codec, u32 private_data);
u32 snd_card_get_codec_data(struct snd_codec *codec);
struct snd_card *snd_card_get(void);

#endif /* __SND_CARD_H__ */

