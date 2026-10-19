
#ifndef __SND_DAI_H__
#define __SND_DAI_H__

#define DAI_TYPE_PCM    1
#define DAI_TYPE_COMPR  2

#define SND_DAI_SAFE_RUN(d, f) \
    if (d && d->ops && d->ops->f)  \
        d->ops->f(d);

struct snd_dai_cfg {
    u32 phy_addr;
    u32 dma_hw_id;
};

struct snd_dai {
    struct list_head list;
    char *name;
    char *stream_name;
    u32 type;
    u32 dev_id;
    u32 direct;
    struct snd_dai_ops *ops;
    u32 cfg;
    u32 private;
};

struct snd_dai_ops {
    int (*open)(struct snd_dai *dai);
    int (*close)(struct snd_dai *dai);
    int (*set_params)(struct snd_dai *dai, u32 width, u32 chn, u32 srate);
    int (*start)(struct snd_dai *dai);
    int (*stop)(struct snd_dai *dai);
};

int snd_dai_register(struct snd_dai *dai);
struct snd_dai *snd_dai_get(char *stream_name, u32 type, u32 reserve);
struct snd_dai_cfg *snd_dai_get_cfg(struct snd_dai *dai);

#endif /* __SND_DAI_H__ */
