/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file audio engine module file, it contains serial debug command,
 *    communication with audio PC tool, and other function such like
 *    debug, dump, etc.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0699b7
#define LOG_MODULE    AMOD_AUDIO

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <cmsis_compiler.h>

#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_type.h>
#include <aic_cmd_api.h>
#include <aic_init.h>
#include <aic_io.h>
#ifdef CONFIG_RIL_PDP
#include <aic_at_cmd_handle.h>
#endif

#include "audio_util.h"
#include "audio_stream.h"
#include "audio_control.h"
#include "audio_nv.h"
#include "audio_system.h"
#include "audio_client.h"

#define ADPM_BUFF_SIZE               200
#define TLV_HEADER_SIZE              0

#ifdef CONFIG_AUDIO_STREAM_TEST
/* these are for stream test */
#define ASTREAM_PERIOD_SIZE          640
#define ASTREAM_PERIOD_COUNT         4
#define ASTREAM_SRATE                16000
#define ASTREAM_CHN                  2
#define ASTREAM_READ_COUNT           300

#define ASTREAM_TEST_STACK_SIZE      2048
#define ASTREAM_TEST_THREAD_PRI      20

struct astream {
    aos_thread_id thread;
    int is_running;
};
static struct astream g_astream[STREAM_CHN_MAX];

#include "audio_eng_resource.h"
#endif

const char *alist[] = {
    "actl",
    "adpm",
    "aplay",
    "aplay_stop",
    "arec",
    "arec_stop",
    "anv",
    "tone_play",
    "aopen",
    "astart",
    "astop",
    "aclose",
    "astream",
    "asink",
    "asink_stop",
    "arecord",
    "set_mvol",
    "get_mvol",
    "adsp_dump",
    "codec_reg",
    "volte_start",
    "volte_stop",
    "tts",
};

static int g_tabs_only = 0;
static int g_all_values = 1;
#ifdef CONFIG_AUDIO_STREAM_TEST
static char *g_rec_buf;
#endif

extern int snd_dpm_get_widget_status(char *name, char *buf, size_t count);
extern void snd_dpm_list_all(void);

static int actl_detail_control(const char *control,
                               int prefix, int print_all);


int alist_main(int argc, char **argv)
{
    int i;

    for (i = 0; i < sizeof(alist) /sizeof(char *); i++)
        CONS_OUT("%s\n", alist[i]);

    return 0;
}
CONSOLE_EXPORT_CMD(alist, alist_main, audio cmd list);

static void actl_usage (void) {
    CONS_OUT("actl [control name/#] [value to set]\n"
             "    actl      -- list all controls\n"
             "    actl help -- show this text\n"
            );
}

static void actl_list_controls(void)
{
    struct audio_ctl *ctl;
    const char *name, *type;
    unsigned int num_ctls, num_values;
    unsigned int i;

    num_ctls = audio_ctl_get_num();

    CONS_OUT("Number of controls: %u\n", num_ctls);

    if (g_tabs_only)
        CONS_OUT("ctl\ttype\tnum\tname\tvalue");
    else
        CONS_OUT("ctl\ttype\tnum\t%-40s value", "name");
    if (g_all_values)
        CONS_OUT("\trange/values\n");
    else
        CONS_OUT("\n");
    for (i = 0; i < num_ctls; i++) {
        ctl = audio_clt_get(i);
        if (!ctl)
            continue;
        name = (char *)audio_ctl_get_name(ctl);
        if (!name)
            continue;
        type = audio_ctl_get_type_string(ctl);
        num_values = audio_ctl_get_num_values(ctl);
        if (g_tabs_only)
            CONS_OUT("%d\t%s\t%d\t%s\t", i, type, num_values, name);
        else
            CONS_OUT("%d\t%s\t%d\t%-40s ", i, type, num_values, name);
        actl_detail_control(name, 0, g_all_values);
    }
}

static void actl_print_enum(struct audio_ctl *ctl, const char *space,
                            int print_all)
{
    unsigned int num_enums;
    unsigned int i;
    const char *string;
    int control_value = audio_ctl_get_value(ctl, 0);

    if (print_all) {
        num_enums = audio_ctl_get_num_enums(ctl);
        for (i = 0; i < num_enums; i++) {
            string = audio_ctl_get_enum_string(ctl, i);
            CONS_OUT("%s%s%s",
                     control_value == (int)i ? ">" : "", string,
                     (i < num_enums - 1) ? space : "");
        }
    }
    else {
        string = audio_ctl_get_enum_string(ctl, control_value);
        CONS_OUT("%s", string);
    }
}

static int actl_detail_control(const char *control,
                               int prefix, int print_all)
{
    struct audio_ctl *ctl;
    enum audio_ctl_type type;
    unsigned int num_values;
    unsigned int i;
    int min, max;
    int ret;
    char *buf = NULL;
    size_t len;
    unsigned int tlv_header_size = 0;
    const char *space = g_tabs_only ? "\t" : " ";

    if (isdigit((int)control[0]))
        ctl = audio_clt_get(atoi(control));
    else
        ctl = audio_ctl_get_by_name(control);

    if (!ctl) {
        CONS_OUT("Invalid mixer control: %s\n", control);
        return AIC_ENOENT;
    }

    type = audio_ctl_get_type(ctl);
    num_values = audio_ctl_get_num_values(ctl);

    if (type == AUDIO_CTL_TYPE_BYTE) {
        buf = audio_malloc(num_values + tlv_header_size);
        if (buf == NULL) {
            CONS_OUT("Failed to alloc mem for bytes %d\n", num_values);
            return AIC_ENOENT;
        }

        len = num_values;
        ret = audio_ctl_get_array(ctl, buf, len + tlv_header_size);
        if (ret < 0) {
            CONS_OUT("Failed to audio_ctl_get_array\n");
            audio_free(buf);
            return AIC_ENOENT;
        }
    }

    if (prefix)
        CONS_OUT("%s:%s", audio_ctl_get_name(ctl), space);

    for (i = 0; i < num_values; i++) {
        switch (type)
        {
        case AUDIO_CTL_TYPE_INT:
            CONS_OUT("%d", audio_ctl_get_value(ctl, i));
            break;
        case AUDIO_CTL_TYPE_BOOL:
            CONS_OUT("%s", audio_ctl_get_value(ctl, i) ? "On" : "Off");
            break;
        case AUDIO_CTL_TYPE_ENUM:
            actl_print_enum(ctl, space, print_all);
            break;
        case AUDIO_CTL_TYPE_BYTE:
            /* skip printing TLV header if exists */
            CONS_OUT(" %02x", buf[i + tlv_header_size]);
            break;
        default:
            CONS_OUT("unknown");
            break;
        }

        if (i < num_values - 1)
            CONS_OUT("%s", space);
    }

    if (print_all) {
        if (type == AUDIO_CTL_TYPE_INT) {
            min = audio_ctl_get_range_min(ctl);
            max = audio_ctl_get_range_max(ctl);
            CONS_OUT("%s(dsrange %d->%d)", space, min, max);
        }
    }

    if (buf)
        audio_free(buf);

    CONS_OUT("\n");

    return AIC_SUCCESS;
}

static void actl_set_byte_ctl(struct audio_ctl *ctl,
                              char **values, unsigned int num_values)
{
    unsigned int *tlv, tlv_size;
    unsigned int tlv_header_size = TLV_HEADER_SIZE;
    int ret;
    char *buf;
    char *end;
    unsigned int i;
    long n;

    tlv_size = num_values + tlv_header_size;

    buf = audio_malloc(tlv_size);
    if (buf == NULL) {
        CONS_OUT("set_byte_ctl: Failed to alloc mem for bytes %d\n",
                 num_values);
        return;
    }

    tlv = (unsigned int *)buf;
    tlv[0] = 0;
    tlv[1] = num_values;

    for (i = 0; i < num_values; i++) {
        n = strtol(values[i], &end, 0);
        if (*end) {
            CONS_OUT("%s not an integer\n", values[i]);
            goto fail;
        }

        if (n < 0 || n > 0xff) {
            CONS_OUT("%s should be between [0, 0xff]\n",
                values[i]);
            goto fail;
        }
        /* start filling after the TLV header */
        buf[i + tlv_header_size] = n;
    }

    ret = audio_ctl_set_array(ctl, buf, tlv_size);
    if (ret < 0)
        CONS_OUT("Failed to set binary control\n");

fail:
    audio_free(buf);
    return;
}

static int actl_set_value(const char *control,
                          char **values, unsigned int num_values)
{
    struct audio_ctl *ctl;
    enum audio_ctl_type type;
    unsigned int num_ctl_values;
    unsigned int i;

    if (isdigit((int)control[0]))
        ctl = audio_clt_get(atoi(control));
    else
        ctl = audio_ctl_get_by_name(control);

    if (!ctl) {
        CONS_OUT("Invalid mixer control: %s\n", control);
        return AIC_ENOENT;
    }

    type = audio_ctl_get_type(ctl);
    num_ctl_values = audio_ctl_get_num_values(ctl);

    if (type == AUDIO_CTL_TYPE_BYTE) {
        actl_set_byte_ctl(ctl, values, num_values);
        return AIC_ENOENT;
    }

    if (isdigit((int)values[0][0])) {
        if (num_values == 1) {
            /* Set all values the same */
            int value = atoi(values[0]);

            for (i = 0; i < num_ctl_values; i++) {
                if (audio_ctl_set_value(ctl, i, value)) {
                    CONS_OUT("Error: invalid value\n");
                    return AIC_EINVAL;
                }
            }
        } else {
            /* Set multiple values */
            if (num_values > num_ctl_values) {
                CONS_OUT("Error: %u values given, but control only takes %u\n",
                         num_values, num_ctl_values);
                return AIC_EINVAL;
            }
            for (i = 0; i < num_values; i++) {
                if (audio_ctl_set_value(ctl, i, atoi(values[i]))) {
                    CONS_OUT("Error: invalid value for index %d\n", i);
                    return AIC_EINVAL;
                }
            }
        }
    } else {
        if (type == AUDIO_CTL_TYPE_ENUM) {
            if (num_values != 1) {
                CONS_OUT("Enclose strings in quotes and try again\n");
                return AIC_EINVAL;
            }
            if (audio_ctl_set_enum_by_string(ctl, values[0])) {
                CONS_OUT("Error: invalid enum value\n");
                return AIC_EINVAL;
            }
        } else {
            CONS_OUT("Error: only enum types can be set with strings\n");
            return AIC_EINVAL;
        }
    }

    return AIC_SUCCESS;
}

int actl_main(int argc, char **argv)
{
    if(argc == 0) {
        CONS_OUT("Control platfrom name: '%s'\n", "aic-micro");
        actl_list_controls();
        return AIC_SUCCESS;
    }

    if (argc == 1) {
        if (!strcmp(argv[0], "help"))
        {
            actl_usage();
            return AIC_SUCCESS;
        }
        return actl_detail_control(argv[0], 0, 0);
    }

    return actl_set_value(argv[0], &argv[1], argc - 1);
}
CONSOLE_EXPORT_CMD(actl, actl_main, actl list);

#ifdef CONFIG_SOUND_SUPPORT
int adpm_main(int argc, char **argv)
{
    int ret = 0, len;
    char *buf;

    len = ADPM_BUFF_SIZE;
    buf = (char *)audio_malloc(len);
    if (!buf) {
        CONS_OUT("No memory!\n");
        return AIC_ENOMEM;
    }
    if(argc == 0) {
        CONS_OUT("All dpms:\n");
        snd_dpm_list_all();
    } else {/* if (argc == 1) { */
        ret = snd_dpm_get_widget_status(argv[0], buf, len);
        if (ret >= 0)
            CONS_OUT("%s\n", buf);
        else
            CONS_OUT("error! ret=%d\n", ret);

    }
    audio_free(buf);

    return 0;
}
CONSOLE_EXPORT_CMD(adpm, adpm_main, adpm list);
#endif

#ifdef CONFIG_AUDIO_STREAM_TEST
struct input_param {
    u32 stream_chn;
    u32 period_size;
    u32 period_cnt;
    u32 channels;
    u32 srate;
    u32 brate;
    u32 fmt;
    u32 start_threshold;
    u32 stop_threshold;
    u32 flag;   /* useless now */
    u32 rec;    /* 0: music_file; 1: rec buf */
    u32 astream_type;
    u32 aplay_type;
    u32 astream_fmt;
    u32 astream_flag;
    u32 dir;
    u32 source;
    u32 loop_cnt;
    char fname[128];
};

void parse_input_cmd(int argc, char **argv, struct input_param *param)
{
    int i;
    char *p;

    /* set default at first */
    param->stream_chn = 0;
    param->period_size = ASTREAM_PERIOD_SIZE;
    param->period_cnt = ASTREAM_PERIOD_COUNT;
    param->channels = ASTREAM_CHN;
    param->srate = ASTREAM_SRATE;
    param->brate = 0;
    param->fmt = PCM_FORMAT_S16_LE;
    param->start_threshold = 0;
    param->stop_threshold = 0;
    param->flag = PCM_OUT;
    param->rec = 0;
    param->astream_type = ASTREAM_TYPE_MUSIC;
    param->aplay_type = APLAY_TYPE_BUF;
    param->astream_fmt = ASTREAM_FORMAT_PCM;
    param->dir = ASTREAM_DIR_PLAY;
    param->source = ASTREAM_SOURCE_DEFAULT;
    param->astream_flag = AUDIO_OUTPUT_FLAG_DIRECT;
    param->loop_cnt = 1;
    param->fname[0] = 0;

    for (i = 0; i < argc; i++) {
        p = argv[i];

        if (strcmp(p, "-sc") == 0) {
            param->stream_chn = atoi(argv[++i]);
        } else if (strcmp(p, "-p") == 0) {
            param->period_size = atoi(argv[++i]);
        } else if(strcmp(p, "-n") == 0) {
            param->period_cnt = atoi(argv[++i]);
        } else if(strcmp(p, "-c") == 0) {
            param->channels = atoi(argv[++i]);
        } else if(strcmp(p, "-r") == 0) {
            param->srate = atoi(argv[++i]);
        } else if(strcmp(p, "-br") == 0) {
            param->brate = atoi(argv[++i]);
        } else if(strcmp(p, "-f") == 0) {
            param->fmt = atoi(argv[++i]);
        } else if(strcmp(p, "-stt") == 0) {
            param->start_threshold = atoi(argv[++i]);
        } else if(strcmp(p, "-stp") == 0) {
            param->stop_threshold = atoi(argv[++i]);
        } else if(strcmp(p, "-flag") == 0) {
            param->flag = atoi(argv[++i]);
        } else if(strcmp(p, "-rec") == 0) {
            param->rec = atoi(argv[++i]);
        } else if(strcmp(p, "-st") == 0) {
            param->astream_type = atoi(argv[++i]);
        } else if(strcmp(p, "-pt") == 0) {
            param->aplay_type = atoi(argv[++i]);
        } else if(strcmp(p, "-sf") == 0) {
            param->astream_fmt = atoi(argv[++i]);
        } else if(strcmp(p, "-sflag") == 0) {
            param->astream_flag = atoi(argv[++i]);
        } else if(strcmp(p, "-dir") == 0) {
            param->dir = atoi(argv[++i]);
        } else if(strcmp(p, "-src") == 0) {
            param->source = atoi(argv[++i]);
        } else if(strcmp(p, "-lc") == 0) {
            param->loop_cnt = atoi(argv[++i]);
        } else if(strcmp(p, "-fname") == 0) {
            strcpy(param->fname, argv[++i]);
        }
    }
}

void aplay_thread_loop(void *data)
{
    struct stream_config config;
    struct input_param *param;
    int stream, flag, loop_cnt = 0;
    int ret, send_cnt, music_size, len;
    int stream_chn;
    char *pmusic;

    if (!data)
        return;

    param = (struct input_param *)data;
    stream_chn = param->stream_chn;
    g_astream[stream_chn].is_running = TRUE;
    config.period_cnt = param->period_cnt;
    config.period_size = param->period_size;
    config.start_threshold = param->start_threshold;
    config.stop_threshold = param->stop_threshold;
    config.chn = param->channels;
    config.fmt = param->fmt;
    config.sample_rate = param->srate;
    config.bit_rate = param->brate;
    if (stream_chn <= STREAM_CHN_PCM_MAX)
        flag = PCM_OUT;
    else
        flag = COMPR_OUT;
    if (param->rec) {
        pmusic = g_rec_buf;
        music_size = config.period_size * ASTREAM_READ_COUNT;
    } else {
        pmusic = (char *)music_file;
        music_size = sizeof(music_file);
    }
    stream = audio_stream_open(stream_chn, &config, flag);
    if (!stream) {
        audio_free(param);
        CONS_OUT("open file failed!\n");
        return;
    }

    ret = audio_stream_write(stream, pmusic, config.period_size);
    if (ret < 0) {
        CONS_OUT("write error, ret=%d\n", ret);
        goto out;
    }
    send_cnt = 0;

    ret = audio_stream_start(stream);
    if (ret) {
        CONS_OUT("start error, ret=%d\n", ret);
        goto out;
    }
#ifdef CONFIG_ADSP_EDA_MP3
    AIC_RAW_WRITE32(0x41090080, 0xAD);
#endif

    while (g_astream[stream_chn].is_running && loop_cnt < param->loop_cnt) {

        if (send_cnt + config.period_size > music_size)
            len = music_size - send_cnt;
        else
            len = config.period_size;

        ret = audio_stream_write(stream, pmusic + send_cnt, len);
        if (ret < 0) {
            CONS_OUT("write error, ret=%d\n", ret);
            break;
        }
        if (send_cnt + len >= music_size) {
            send_cnt = 0;
            loop_cnt++;
            if (loop_cnt >= param->loop_cnt) {
                if (param->astream_fmt != ASTREAM_FORMAT_PCM)
                    audio_stream_drain(stream);
            }
        } else {
            send_cnt += config.period_size;
        }
        aicx_debug(0x11a217, "sd", "%s, after write, send_cnt=%d\n", __func__, send_cnt);
    }

    ret = audio_stream_stop(stream);
    aicx_debug(0x160318, "sd", "%s, stop ret=%d\n", __func__, ret);
    CONS_OUT("Done!\n");
out:
    ret = audio_stream_close(stream);
    g_astream[stream_chn].is_running = 0xff;
    audio_free(param);
}

int aplay_main(int argc, char **argv)
{
    struct input_param *param;
    u32 stream_chn;

    param = audio_malloc(sizeof(*param));
    if (!param) {
        CONS_OUT("no mem!\n");
        return AIC_ENOMEM;
    }
    parse_input_cmd(argc, argv, param);

    stream_chn = param->stream_chn;
    if (stream_chn >= STREAM_CHN_MAX) {
        CONS_OUT("error chn!\n");
        return AIC_SUCCESS;
    }

    g_astream[stream_chn].thread = aos_create_thread("aplay test",
                                                     aplay_thread_loop,
                                                     (void *)param,
                                                     ASTREAM_TEST_STACK_SIZE, 0,
                                                     NULL,
                                                     ASTREAM_TEST_THREAD_PRI,
                                                     AOS_AUTO_START);
    return 0;
}
CONSOLE_EXPORT_CMD(aplay, aplay_main, aplay test);

#ifdef CONFIG_ADSP_EDA_MP3
int aplay_start_auto(void)
{
    char *argv[10];

    argv[0] = "-sc";
    argv[1] = "7";
    argv[2] = "-p";
    argv[3] = "2048";
    argv[4] = "-n";
    argv[5] = "2";
    argv[6] = "-c";
    argv[7] = "1";
    argv[8] = "-r";
    argv[9] = "8000";
    aicx_error(0x1e264a, "s", "%s, int!\n", __func__);
    aplay_main(10, argv);
    aicx_error(0x02dbd5, "s", "%s, out!\n", __func__);

    return 0;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(aplay_start_auto);
#endif

int aplay_stop(int argc, char **argv)
{
    int stream_chn = 0;

    if(argc == 1) {
        if (isdigit((int)argv[0][0]))
            stream_chn = atoi(argv[0]);
        else {
            CONS_OUT("must be digital!\n");
            return AIC_SUCCESS;
        }
    }

    if (stream_chn >= STREAM_CHN_MAX) {
        CONS_OUT("error chn!\n");
        return AIC_SUCCESS;
    }

    if (!g_astream[stream_chn].is_running) {
        CONS_OUT("already stoped or not start!\n");
        return AIC_SUCCESS;
    }
    if (g_astream[stream_chn].is_running == TRUE)
        g_astream[stream_chn].is_running = FALSE;
    while (g_astream[stream_chn].is_running != 0xff)
        aos_sleep(100);

    /* force terminate */
    aos_terminate_thread(g_astream[stream_chn].thread);
    aos_delete_thread(g_astream[stream_chn].thread);

    return AIC_SUCCESS;
}
CONSOLE_EXPORT_CMD(aplay_stop, aplay_stop, aplay_stop test);

void arec_thread_loop(void *data)
{
    struct stream_config config;
    struct input_param *param;
    int stream, stream_chn;
    int ret, read_cnt = 0, flag;
    char *buf;

    param = (struct input_param *)data;
    stream_chn = param->stream_chn;
    if (stream_chn >= STREAM_CHN_MAX) {
        CONS_OUT("error chn!\n");
        return;
    }

    if (!g_rec_buf) {
        buf = (char *)audio_malloc(param->period_size * ASTREAM_READ_COUNT);
        g_rec_buf = buf;
    }
    else
        buf = g_rec_buf;
    if (!buf) {
        CONS_OUT("No memory!\n");
        return;
    }
    memset(buf, 0, param->period_size * ASTREAM_READ_COUNT);
    g_astream[stream_chn].is_running = TRUE;
    config.period_cnt = param->period_cnt;
    config.period_size = param->period_size;
    config.start_threshold = param->start_threshold;
    config.stop_threshold = param->stop_threshold;
    config.sample_rate = param->srate;
    config.fmt = param->fmt;
    config.chn = param->channels;
    if (stream_chn <= STREAM_CHN_PCM_MAX)
        flag = PCM_IN;
    else
        flag = COMPR_IN;
    stream = audio_stream_open(stream_chn, &config, flag);
    if (!stream) {
        CONS_OUT("open file failed!\n");
        return;
    }

    ret = audio_stream_start(stream);
    if (ret) {
        CONS_OUT("start error, ret=%d\n", ret);
        goto out;
    }
    while (read_cnt < ASTREAM_READ_COUNT && g_astream[stream_chn].is_running) {
        ret = audio_stream_read(stream, buf + read_cnt * param->period_size,
                                param->period_size);
        if (ret < 0) {
            aicx_error(0x0072b4, "d", "arec read error, ret=%d\n", ret);
            break;
        }
        read_cnt++;
        aicx_debug(0x079031, "sd", "%s, after read, size=%d\n", __func__, ret);
    }

    ret = audio_stream_stop(stream);
    aicx_debug(0x160318, "sd", "%s, stop ret=%d\n", __func__, ret);
    CONS_OUT("Done!\n");
out:
    ret = audio_stream_close(stream);
    g_astream[stream_chn].is_running = FALSE;
    aicx_debug(0x10177a, "sd", "%s, close ret=%d\n", __func__, ret);
}

int arec_main(int argc, char **argv)
{
    struct input_param *param;
    int stream_chn = 0;

    param = audio_malloc(sizeof(*param));
    if (!param) {
        CONS_OUT("no mem!\n");
        return AIC_ENOMEM;
    }
    parse_input_cmd(argc, argv, param);

    stream_chn = param->stream_chn;
    if (stream_chn >= STREAM_CHN_MAX) {
        CONS_OUT("error chn!\n");
        return AIC_SUCCESS;
    }
    g_astream[stream_chn].thread = aos_create_thread("arec test",
                                                     arec_thread_loop,
                                                     (void *)param,
                                                     ASTREAM_TEST_STACK_SIZE,
                                                     0, NULL,
                                                     ASTREAM_TEST_THREAD_PRI,
                                                     AOS_AUTO_START);

    return 0;

}
CONSOLE_EXPORT_CMD(arec, arec_main, arec test);

int arec_stop(int argc, char **argv)
{
    return aplay_stop(argc, argv);
}
CONSOLE_EXPORT_CMD(arec_stop, arec_stop, arec_stop test);

#endif

#ifdef CONFIG_NV_SUPPORT
#ifdef CONFIG_AUDIO_NV_TEST
static const char *nv_name_config[] = {
    "A", "B", "C", "D", "E", "F", "G", "H"
};

static const char *nv_name_dsp[] = {
    "O", "P", "Q", "R", "S", "F", "G", "H"
};

static const char *nv_name_voice_proc[] = {
    "A", "B", "C", "D", "E", "F", "G", "H"
};
#else
static const char *nv_name_config[] = {
    "audio config"
};

static const char *nv_name_dsp[] = {
    "handset",
    "headset",
    "handfree",
    "bt",
    "bt-nrec"
};

static const char *nv_name_voice_proc[] = {
    "handset",
    "headset",
    "handfree",
    "bt",
    "bt-nrec"
};
#endif

#include "NV_AUDIO_CONFIG.h"
#include "NV_AUDIO_HW.h"

int anv_main(int argc, char **argv)
{
    int i, ret, nvid, cnt;
    char **name;
    u32 nv_buff[100];
    NV_AUDIO_CONFIG_T *nv_config;
    NV_AUDIO_HW_T *nv_hw;

    if(argc == 1) {
        nvid = atoi(argv[0]);
        cnt = audio_nv_get_num(nvid);
        if (cnt <= 0) {
            CONS_OUT("error nv id\n");
            return 0;
        }
        if (nvid == AUDIO_NVID_CONFIG)
            name = (char **)nv_name_config;
        else if (nvid == AUDIO_NVID_DSP)
            name = (char **)nv_name_dsp;
        else
            name = (char **)nv_name_voice_proc;
        for(i = 0; i < cnt; i++) {
            ret = audio_nv_read(nvid, name[i], (void *)&nv_buff);
            if (ret < 0) {
                CONS_OUT("read erro\n");
                continue;
            }
            switch(nvid) {
            case AUDIO_NVID_CONFIG:
                nv_config = (NV_AUDIO_CONFIG_T *)nv_buff;
                CONS_OUT("name=%c, gain0=0x%x, gain1=0x%x\n",
                         nv_config->name, nv_config->gain[0],
                         nv_config->gain[1]);
                break;
            case AUDIO_NVID_DSP:
                nv_hw = (NV_AUDIO_HW_T *)nv_buff;
                CONS_OUT("name=%c, gain0=0x%x, gain1=0x%x\n",
                         nv_hw->name, nv_hw->gain[0], nv_hw->gain[1]);
                break;
            default:
                continue;
            }
        }
    } else if (argc == 2) {
        /* TODO */
    }
    return 0;
}
CONSOLE_EXPORT_CMD(anv, anv_main, audio nv test);
#endif

#ifdef CONFIG_AUDIO_STREAM_TEST

int tone_play(int argc, char **argv)
{
    int tone_id = 0, duration = 1000, flag = AUDIO_OUTPUT_FLAG_COMPRESS;

    if(argc > 0)
        tone_id = atoi(argv[0]);

    if (argc > 1)
        duration = atoi(argv[1]);

    if (argc > 2)
        flag = atoi(argv[2]);
    audio_play_tone_ex((enum tone_type)tone_id, duration, flag);

    return 0;
}
CONSOLE_EXPORT_CMD(tone_play, tone_play, tone play test);

int tone_stop(int argc, char **argv)
{
    audio_stop_tone();

    return 0;
}
CONSOLE_EXPORT_CMD(tone_stop, tone_stop, tone stop test);

static ACLIENT g_ahandle;
static ACLIENT g_ahandle_rec;
static int aplay_callback(u32 data, enum astream_cb_event event)
{
    if (event == ASTREAM_CB_EVENT_FINISH) {
        audio_client_delete(data);
        g_ahandle = 0;
    }
    return 0;
}

static int aopen(int argc, char **argv)
{
    struct input_param param;

    parse_input_cmd(argc, argv, &param);

    g_ahandle = audio_open(param.astream_type, param.aplay_type,
                           param.astream_flag, param.srate, param.brate,
                           param.astream_fmt, param.channels, param.dir,
                           param.source);

    CONS_OUT("handle=0x%x\n", g_ahandle);

    return 0;
}
CONSOLE_EXPORT_CMD(aopen, aopen, aopen test);

static int astart(int argc, char **argv)
{
    int ret;

    ret = audio_play_buf(g_ahandle, (char *)music_file,
                         sizeof(music_file), aplay_callback, 0);
    CONS_OUT("start %s\n", (ret) ? "failed":"ok");

    return 0;
}
CONSOLE_EXPORT_CMD(astart, astart, astart test);

static int astop(int argc, char **argv)
{
    int ret;

    ret = audio_stop(g_ahandle);
    CONS_OUT("stop %s\n", (ret) ? "failed":"ok");

    return 0;
}
CONSOLE_EXPORT_CMD(astop, astop, astop test);

static int apause(int argc, char **argv)
{
    int ret;

    ret = audio_pause(g_ahandle);
    CONS_OUT("pause %s\n", (ret) ? "failed":"ok");

    return 0;
}
CONSOLE_EXPORT_CMD(apause, apause, apause test);

static int aresume(int argc, char **argv)
{
    int ret;

    ret = audio_resume(g_ahandle);
    CONS_OUT("resume %s\n", (ret) ? "failed":"ok");

    return 0;
}
CONSOLE_EXPORT_CMD(aresume, aresume, aresume test);

static int aclose(int argc, char **argv)
{
    int ret = 0;

    if (g_ahandle) {
        ret = audio_close(g_ahandle);
        g_ahandle = 0;
        CONS_OUT("close %s\n", (ret) ? "failed":"ok");
    } else
        CONS_OUT("already closed or not open\n");

    return 0;
}

CONSOLE_EXPORT_CMD(aclose, aclose, aclose test);

struct astream_write_info
{
    u32 client_handle;
    void * param;
};

static void astream_write_func(void *data)
{
    struct astream_write_info* info = (struct astream_write_info*)data;
    struct input_param * param = info->param;
    u32 copy = param->period_size;
    ACLIENT handle = info->client_handle;
    char *file_buf = NULL;
    FILE *fp = NULL;
    u32 play_type;// = info->play_type;
    u32 total_len, write_len = 0 , fread_size;

    if (!handle) {
        AIDXE(0x100efe, "", "astream write func error handle\n");
        return;
    }
    if (strlen(param->fname) == 0) {
        total_len = sizeof(music_file);
        play_type = 0;
    }
    else {
        play_type = 1;
        file_buf = (char*)malloc(param->period_size);
        if(file_buf == NULL)
            goto _err_exit;
        fp = fopen(param->fname , "rb");
        if(fp == NULL) {
            goto _err_exit;
        }
    }


    if(play_type == 0) {
        write_len = 0;
        while(write_len < total_len) {
            copy = param->period_size;
            copy = audio_write(handle, (char *)music_file + write_len, copy, 3000);
            aicx_info(0x0cf206, "d", "astrem write, copy=%d\n", copy);
            if (copy == 0) {
                CONS_OUT("astream write return 0, break!\n");
                break;
            }
            write_len += copy;
        };
    }
    else {
        while(1) {
            copy = param->period_size;
            fread_size = fread(file_buf , 1 , copy , fp);
            if(fread_size > 0) {
                write_len = 0;
                total_len = fread_size;
                while(write_len < total_len) {
                    copy = total_len - write_len;
                    copy = audio_write(handle, (char *)file_buf + write_len, copy, 3000);
                    aicx_info(0x0cf206, "d", "astrem write, copy=%d\n", copy);
                    if (copy == 0) {
                        CONS_OUT("astream write return 0, break!\n");
                        break;
                    }
                    write_len += copy;
                }
            }
            else {
                CONS_OUT("astream write file done break!\n");
                break;
            }
        }
    }
    audio_drain(handle);

_err_exit:
    if(file_buf)
        free(file_buf);
    if(fp)
        fclose(fp);
}

static struct input_param g_param;
static struct astream_write_info g_write_info;

static int astream(int argc, char **argv)
{

    ACLIENT handle;
    int ret;

    parse_input_cmd(argc, argv, &g_param);

    handle = audio_open(ASTREAM_TYPE_MUSIC, APLAY_TYPE_STREAM, g_param.astream_flag, g_param.srate, 0,
                        g_param.astream_fmt, g_param.channels,
                        ASTREAM_DIR_PLAY, ASTREAM_SOURCE_DEFAULT);

    CONS_OUT("handle=0x%x\n", handle);

    if (!handle)
        return 0;

    g_write_info.client_handle = handle;
    g_write_info.param = &g_param;
    aos_create_thread("astream_write" , astream_write_func , (void*)&g_write_info , 2048 , 0 , NULL , 10 , 1);

    ret = audio_play_stream(handle, NULL, 0);

    return 0;
}

CONSOLE_EXPORT_CMD(astream, astream, astream test);

#define STS_NONE    0
#define STS_PLAY    1

static int asink_status;
static int asink_callback(u32 data, enum astream_cb_event event)
{
    CONS_OUT("cb event=%d\n", event);
    if (event != 0) {
        asink_status = STS_NONE;
    }

    return 0;
}

static int asink(int argc, char **argv)
{
    struct input_param param;
    int ret;

    if (g_ahandle) {
        CONS_OUT("close first!\n");
        return 0;
    }
    parse_input_cmd(argc, argv, &param);

    if (param.astream_type == ASTREAM_TYPE_FM)
        g_ahandle = audio_open(ASTREAM_TYPE_MUSIC, param.aplay_type,
                               param.astream_flag, param.srate, 0,
                               param.astream_fmt,
                               param.channels, ASTREAM_DIR_PLAY, 0);
    else
        g_ahandle = audio_open_output(ASTREAM_TYPE_MUSIC, param.aplay_type,
                                  param.astream_fmt, param.astream_flag);
    if(!g_ahandle) {
        CONS_OUT("open failed!\n");
        return 0;
    }

    if (strlen(param.fname) == 0)
        ret = audio_play_buf(g_ahandle, (char *)music_file,
                             sizeof(music_file), asink_callback, 0);
    else
        ret = audio_play_file(g_ahandle, param.fname,
                              asink_callback, 0);
    if (ret) {
        audio_close(g_ahandle);
        g_ahandle = 0;
        CONS_OUT("start failed!\n");

        return 0;
    }

    CONS_OUT("started!\n");

    return 0;
}
CONSOLE_EXPORT_CMD(asink, asink, asink);

static int asink_stop(int argc, char **argv)
{
    if (g_ahandle) {
        audio_stop(g_ahandle);
        audio_close(g_ahandle);
        g_ahandle = 0;

        CONS_OUT("stopped!\n");
    } else
        CONS_OUT("not in play!\n");

    return 0;
}

CONSOLE_EXPORT_CMD(asink_stop, asink_stop, asink stop);

static int aseek(int argc, char **argv) {
    struct input_param param;
    int ret;

    parse_input_cmd(argc, argv, &param);
    if(g_ahandle) {
        u32 timestamp;

        timestamp = atoi(argv[0]);

        audio_stop(g_ahandle);
        audio_seek(g_ahandle, timestamp);
        if (strlen(param.fname) == 0)
            ret = audio_play_buf(g_ahandle, (char *)music_file,
                                 sizeof(music_file), asink_callback, 0);
        else
            ret = audio_play_file(g_ahandle, param.fname,
                                  asink_callback, 0);
        CONS_OUT("timestamp is:%d\n", audio_get_timestamp(g_ahandle));
    } else
        CONS_OUT("not in play!\n");

    return 0;
}
CONSOLE_EXPORT_CMD(aseek, aseek, asink seek);

static int asgettime(int argc, char **argv) {
    if(g_ahandle) {
        CONS_OUT("timestamp is:%d\n", audio_get_timestamp(g_ahandle));
    } else
        CONS_OUT("not in play!\n");

    return 0;
}
CONSOLE_EXPORT_CMD(asgettime, asgettime, asink gettime);


static int arecord(int argc, char **argv)
{
    struct input_param param;
    int ret;

    if (g_ahandle_rec) {
        CONS_OUT("close first!\n");
        return 0;
    }
    parse_input_cmd(argc, argv, &param);

    if (param.aplay_type != APLAY_TYPE_FILE && !g_rec_buf) {
        g_rec_buf = (char *)audio_malloc(param.period_size * ASTREAM_READ_COUNT);
        if (!g_rec_buf) {
            CONS_OUT("no enough momory!\n");
            return 0;
        }
    }

    if (param.astream_fmt >= ASTREAM_FORMAT_OFFLD_BEGIN &&
            param.astream_fmt <= ASTREAM_FORMAT_OFFLD_END)
        param.astream_flag |= AUDIO_INPUT_FLAG_COMPRESS;

    g_ahandle_rec =  audio_open(ASTREAM_TYPE_RECORD, param.aplay_type, param.astream_flag, param.srate, param.brate,
                      param.astream_fmt, param.channels, ASTREAM_DIR_RECORD,  param.source);

    if(!g_ahandle_rec) {
        CONS_OUT("open failed!\n");
        return 0;
    }

    if (param.aplay_type == APLAY_TYPE_FILE) {
        ret = audio_play_file(g_ahandle_rec, param.fname,
                              asink_callback, 0);
    } else {
        ret = audio_play_buf(g_ahandle_rec, (char *)g_rec_buf,
                             param.period_size * ASTREAM_READ_COUNT,
                             asink_callback, 0);
    }
    if (ret) {
        audio_close(g_ahandle_rec);
        g_ahandle_rec = 0;
        CONS_OUT("start failed!\n");

        return 0;
    }

    CONS_OUT("started!\n");

    return 0;

}
CONSOLE_EXPORT_CMD(arecord, arecord, arecord test);

static int arecord_pause(int argc, char **argv)
{
    if (g_ahandle_rec) {
        audio_pause(g_ahandle_rec);
        CONS_OUT("arecord paused!\n");
    } else
        CONS_OUT("not in record!\n");

    return 0;
}
CONSOLE_EXPORT_CMD(arecord_pause, arecord_pause, arecord_stop test);

static int arecord_resume(int argc, char **argv)
{
    if (g_ahandle_rec) {
        audio_resume(g_ahandle_rec);
        CONS_OUT("arecord resumed!\n");
    } else
        CONS_OUT("not in record!\n");

    return 0;
}
CONSOLE_EXPORT_CMD(arecord_resume, arecord_resume, arecord_stop test);



static int arecord_stop(int argc, char **argv)
{
    if (g_ahandle_rec) {
        audio_stop(g_ahandle_rec);
        audio_close(g_ahandle_rec);
        g_ahandle_rec = 0;

        CONS_OUT("stopped!\n");
    } else
        CONS_OUT("not in record!\n");

    return 0;
}
CONSOLE_EXPORT_CMD(arecord_stop, arecord_stop, arecord_stop test);

u32 g_astream_record_state;
u32 g_astream_record_handle;
static int arecords_stop(int argc, char **argv)
{
    g_astream_record_state = 0;
    if (g_astream_record_handle) {
        aos_sleep(100);
        aos_delete_thread(g_astream_record_handle);
        g_astream_record_handle = 0;
    }
    return 0;
}
CONSOLE_EXPORT_CMD(arecords_stop, arecords_stop, arecords_stop test);

static void astream_record_func(void *data)
{
    struct input_param param = *(struct input_param *)data;
    u32 handle, period, total = 0;
    int ret, len;
    u8 *buf;

    if (param.astream_fmt >= ASTREAM_FORMAT_OFFLD_BEGIN &&
            param.astream_fmt <= ASTREAM_FORMAT_OFFLD_END)
        param.astream_flag |= AUDIO_INPUT_FLAG_COMPRESS;

    if (param.astream_fmt == ASTREAM_FORMAT_AMR_NB ||
        param.astream_fmt == ASTREAM_FORMAT_AMR_WB )
        period = 64;
    else if (param.astream_fmt == ASTREAM_FORMAT_PCM ||
             param.astream_fmt == ASTREAM_FORMAT_WAV)
        period = 320;
    else {
        CONS_OUT("do not support this format!\n");
        return;
    }
    buf = (char *)audio_malloc(2048);
    if (!buf) {
        CONS_OUT("no memory!\n");
        return;
    }
    handle = audio_open_input(APLAY_TYPE_STREAM, param.astream_fmt, param.srate,
                              param.channels, 0, param.astream_flag);
    if (!handle) {
        CONS_OUT("record open error!\n");
        goto free_mem;
    }
    ret = audio_play_stream(handle, 0, 0);
    if (ret) {
        CONS_OUT("record start error!");
        goto start_err;
    }

    CONS_OUT("arecords started!\n");
    g_astream_record_state = 1;
    while (g_astream_record_state) {
        len = audio_read(handle, buf, period, 40);
        if (len > 0) {
            audio_data(buf, len, 2);
            total += len;
        }
    }

    audio_stop(handle);

start_err:
    audio_close(handle);

free_mem:
    audio_free(buf);

    return;
}

static int arecords(int argc, char **argv)
{
    parse_input_cmd(argc, argv, &g_param);
    g_astream_record_handle = aos_create_thread("arecords", astream_record_func,
                                                (void*)&g_param, 2048, 0,
                                                NULL, 10, 1);
    return 0;
}
CONSOLE_EXPORT_CMD(arecords, arecords, arecords test);

static int set_mvol(int argc, char **argv)
{
    u32 vol;

    if (argc <= 0)
        return AIC_EPERM;

    vol = atoi(argv[0]);

    if (!audio_set_master_volume(vol)) {
        CONS_OUT("set success, mvol=%d\n", vol);
        return 0;
    }

    return AIC_EPERM;
}
CONSOLE_EXPORT_CMD(set_mvol, set_mvol, set_mvol test);

static int get_mvol(int argc, char **argv)
{
    CONS_OUT("mvol=%d\n", audio_get_master_volume());

    return 0;
}
CONSOLE_EXPORT_CMD(get_mvol, get_mvol, get_mvol test);

static int atime(int argc, char **argv)
{
    struct input_param param;
    u32 t;

    parse_input_cmd(argc, argv, &param);

    if (param.dir == 0)
        t = audio_get_timestamp(g_ahandle);
    else
        t = audio_get_timestamp(g_ahandle_rec);

    CONS_OUT("t=%d\n", t);

    return 0;
}
CONSOLE_EXPORT_CMD(atime, atime, atime test);

static int adev(int argc, char **argv)
{
    u32 dev, dir;
    int ret;

    if (argc <= 1)
        return AIC_EPERM;

    dir = atoi(argv[0]);
    dev = atoi(argv[1]);
    if (dir) {
        dev |= 0x80000000u;
        ret = audio_force_in_device(dev);
    } else
        ret = audio_force_out_device(dev);

    return ret;
}
CONSOLE_EXPORT_CMD(adev, adev, adev test);

static int mute_ul(int argc, char **argv)
{
    if (argc < 1)
        return AIC_EPERM;

    return audio_mute_ul(atoi(argv[0]));
}
CONSOLE_EXPORT_CMD(mute_ul, mute_ul, mute_ul test);

#endif

#ifdef CONFIG_ADSP_DUMP
extern int adsp_dump_set(u32 en, u32 pos);
static int adsp_dump(int argc, char **argv)
{
    u32 dump_en = 0, pos = 0;

    if (argc > 0)
        dump_en = atoi(argv[0]);

    if (argc > 1)
        pos = atoi(argv[1]);

    return adsp_dump_set(dump_en, pos);
}

CONSOLE_EXPORT_CMD(adsp_dump, adsp_dump, adsp dump);
#endif

#ifdef CONFIG_SOUND_CODEC_ES8311
u32 es8311_show(u32 reg);
u32 es8311_store(u32 reg, u32 val);

static int codec_reg(int argc, char **argv)
{
    u32 reg = 0, val = 0;

    if (argc == 0)
        es8311_show((u32)(-1));
    else if (argc == 1)
        es8311_show(atoi(argv[0]));
    else {
        reg = atoi(argv[0]);
        val = atoi(argv[1]);
        es8311_store(reg, val);
    }

    return 0;
}
CONSOLE_EXPORT_CMD(codec_reg, codec_reg, codec reg read_write);
#endif

#ifdef CONFIG_SOUND_PA_AW87390
extern int aw_write(unsigned int reg, unsigned int value);
extern unsigned int aw_read(unsigned int reg);
static int aw_reg(int argc, char **argv)
{
    u32 reg = 0, val = 0;
    int i;

    if (argc == 0) {
        for (i = 0; i <= 10; i++) {
            val = aw_read(i);
            CONS_OUT("%02x: 0x%02x\n", i, val);
        }
    } else if (argc == 1) {
        reg = atoi(argv[0]);
        val = aw_read(reg);
        CONS_OUT("%02x: 0x%02x\n", reg, val);
    } else {
        reg = atoi(argv[0]);
        val = atoi(argv[1]);
        aw_write(reg, val);
    }

    return 0;
}
CONSOLE_EXPORT_CMD(aw_reg, aw_reg, aw reg read write);
#endif

#ifdef CONFIG_VOLTE_SIMU_TEST
int volte_dl_stream, volte_ul_stream;

int volte_start(int argc, char **argv)
{
    struct input_param param;
    struct stream_config config;
    int ret, flag;
    u32 stream_chn;

    parse_input_cmd(argc, argv, &param);

    config.period_cnt = param.period_cnt;
    config.period_size = param.period_size;
    config.start_threshold = param.start_threshold;
    config.stop_threshold = param.stop_threshold;
    config.chn = param.channels;
    config.fmt = param.fmt;
    config.sample_rate = param.srate;
    config.bit_rate = param.brate;

    stream_chn = STREAM_CHN_VOICE_DL;
    flag = COMPR_OUT;
    volte_dl_stream = audio_stream_open(stream_chn, &config, flag);
    if (!volte_dl_stream) {
        CONS_OUT("open file failed!\n");
        return 0;
    }

    ret = audio_stream_start(volte_dl_stream);
    if (ret) {
        CONS_OUT("start error, ret=%d\n", ret);
        return 0;
    }

    stream_chn = STREAM_CHN_VOICE_UL;
    flag = COMPR_IN;
    volte_ul_stream = audio_stream_open(stream_chn, &config, flag);
    if (!volte_ul_stream) {
        CONS_OUT("open file failed!\n");
        return 0;
    }

    ret = audio_stream_start(volte_ul_stream);
    if (ret)
        CONS_OUT("start error, ret=%d\n", ret);

    CONS_OUT("volte started!\n");

    return 0;
}
CONSOLE_EXPORT_CMD(volte_start, volte_start, volte_start test);

int volte_stop(int argc, char **argv)
{
    if (volte_dl_stream) {
        audio_stream_stop(volte_dl_stream);
        audio_stream_close(volte_dl_stream);
        volte_dl_stream = 0;
    }
    if (volte_ul_stream) {
        audio_stream_stop(volte_ul_stream);
        audio_stream_close(volte_ul_stream);
        volte_ul_stream = 0;
    }

    return AIC_SUCCESS;
}
CONSOLE_EXPORT_CMD(volte_stop, volte_stop, volte_stop test);

static int sendat(int argc, char **argv)
{
    int ret = 0;
    char *cstr;
#ifdef CONFIG_RIL_PDP
    ulong at_req_resp;
#endif
    if (argc == 0) {
        CONS_OUT("Nothing to do\n");
        return 0;
    }

    cstr = argv[0];
    if (strstr(cstr, "ATA")) {
        ret = audio_start_voice(TRUE, TRUE);

        if (ret)
            CONS_OUT("start voice failed, ret=%d\n", ret);
#ifdef CONFIG_RIL_PDP
        /* notify CP */
        ret = aic_assemble_and_handle_at_cmd("ATA",
                                             &at_req_resp,
                                             AT_FROM_INTERNAL,
                                             AT_NO_SERIAL_PORT_ID,
                                             AT_RETRY_COUNT0);
        if (ret)
            CONS_OUT("ata fail, ret=%d\n", ret);
#endif
    } else if (strstr(cstr, "ATH")) {
#ifdef CONFIG_RIL_PDP
        /* notify CP */
        ret = aic_assemble_and_handle_at_cmd("ATH",
                                            &at_req_resp,
                                             AT_FROM_INTERNAL,
                                             AT_NO_SERIAL_PORT_ID,
                                             AT_RETRY_COUNT0);
        if (ret)
            CONS_OUT("ath fail, ret=%d\n", ret);
#endif
        ret = audio_stop_voice();
        if (ret)
            CONS_OUT("ath fail, ret=%d\n", ret);
    } else {
#ifdef CONFIG_RIL_PDP
        ret = aic_assemble_and_handle_at_cmd(cstr,
                                             &at_req_resp,
                                             AT_FROM_INTERNAL,
                                             AT_NO_SERIAL_PORT_ID,
                                             AT_RETRY_COUNT0);
        if (ret)
            CONS_OUT("atd fail, ret=%d\n", ret);
#endif
    }

    return 0;
}
CONSOLE_EXPORT_CMD(sendat, sendat, sendat test);

#endif

#ifdef CONFIG_AUDIO_TTS
static int tts(int argc, char **argv)
{
    struct input_param param;
    ACLIENT handle;
    int ret;

    if (argc < 1) {
        CONS_OUT("please input the text you want to play\n");
        return 0;
    }

    parse_input_cmd(argc, argv, &param);

    handle = audio_open(ASTREAM_TYPE_TTS, APLAY_TYPE_TTS,
                        AUDIO_INPUT_FLAG_DIRECT, param.srate, 0,
                        ASTREAM_FORMAT_PCM,
                        ASTREAM_CHN_MONO,
                        ASTREAM_DIR_PLAY,
                        ASTREAM_SOURCE_DEFAULT);

    ret = audio_play_tts(handle, argv[0], strlen(argv[0]), NULL, 0);

    CONS_OUT("tts play %s!\n", ret ? "failed" : "success");

    return 0;
}
CONSOLE_EXPORT_CMD(tts, tts, tts test);

#endif

#ifdef CONFIG_AUDIO_STREAM_TEST
u8 voip_test_buf[640 + 4];
u8 voip_test_state;

void voip_test_thread_loop(void *data)
{
    struct input_param *param;
    enum astream_format fmt  = ASTREAM_FORMAT_AMR_WB;
    u32  srate = 16000, brate = 0, rd_len;
    ACLIENT ul_handle = 0, dl_handle = 0;
    int ret;

    if (!data)
        return;
    param = (struct input_param *)data;
    if (param->brate)
        brate = param->brate;
    if (param->astream_fmt == ASTREAM_FORMAT_PCM) {
        fmt = ASTREAM_FORMAT_PCM;
        if (param->srate)
            srate = param->srate;
    } else if (param->astream_fmt == ASTREAM_FORMAT_AMR_NB) {
        fmt = ASTREAM_FORMAT_AMR_NB;
        srate = 8000;
    } else {
        fmt = ASTREAM_FORMAT_AMR_WB;
        srate = 16000;
    }
    /* TODO */
    audio_free(param);

    ul_handle = audio_open(ASTREAM_TYPE_VOIP_UL, APLAY_TYPE_STREAM,
                           AUDIO_INPUT_FLAG_COMPRESS, srate, brate,
                           fmt, ASTREAM_CHN_MONO,
                           ASTREAM_DIR_RECORD,
                           ASTREAM_SOURCE_DEFAULT);
    if (!ul_handle) {
        AIDXE(0x176665, "", "voip ul open failed\n");
        ret = AIC_EIO;

        goto err_proc;
    }

    ret = audio_play_stream(ul_handle, 0, 0);
    if (ret) {
        audio_close(ul_handle);
        ul_handle = 0;

        AIDXE(0x064a9f, "d", "voip ul play failed, ret=%d\n", ret);
        goto err_proc;
    }

    dl_handle = audio_open(ASTREAM_TYPE_VOIP, APLAY_TYPE_STREAM,
                           AUDIO_OUTPUT_FLAG_COMPRESS, srate, brate,
                           fmt, ASTREAM_CHN_MONO,
                           ASTREAM_DIR_PLAY,
                           ASTREAM_SOURCE_DEFAULT);
    if (!dl_handle) {
        AIDXE(0x012d2d, "", "voip dl open failed\n");
        ret = AIC_EIO;
        goto err_proc;
    }

    ret = audio_play_stream(dl_handle, 0, 0);
    if (ret) {
        audio_close(dl_handle);
        dl_handle = 0;

        AIDXE(0x04676d, "d", "voip dl play failed, ret=%d\n", ret);

        goto err_proc;
    }

    if (fmt == ASTREAM_FORMAT_PCM) {
        if (srate == 8000)
            rd_len = 320;
        else
            rd_len = 640;
        while (voip_test_state == 1) {
            ret = audio_read(ul_handle, voip_test_buf, rd_len, 40);
            if (ret < 0)
                CONS_OUT("read error, ret=%d\n", ret);
            audio_data(voip_test_buf, rd_len, 0);

            ret = audio_write(dl_handle, voip_test_buf, rd_len, 21);
            if (ret < 0)
                CONS_OUT("write error, ret=%d\n", ret);
        }
    } else {
        audio_data("#!AMR-WB\n", 9, 0);
        while (voip_test_state == 1) {
            ret = audio_read(ul_handle, voip_test_buf, 64, 40);
            if (ret < 0)
                CONS_OUT("read error, ret=%d\n", ret);
            audio_data(voip_test_buf + 1, voip_test_buf[0], 0);

            /* totol 64, the first is frame len */
            ret = audio_write(dl_handle, voip_test_buf + 1, 64, 21);
            if (ret < 0)
                CONS_OUT("write error, ret=%d\n", ret);
        }
    }
    audio_stop(ul_handle);
    audio_close(ul_handle);

    audio_stop(dl_handle);
    audio_close(dl_handle);

    CONS_OUT("Voip exit!\n");

    voip_test_state = 0;
err_proc:
    return;
}

aos_thread_id voip_test_th;

int voip_test_main(int argc, char **argv)
{
    struct input_param *param;

    if (voip_test_state != 0) {
        CONS_OUT("Active or stopping!\n");
        return 0;
    }
    voip_test_state = 1;

    param = audio_malloc(sizeof(*param));
    if (!param) {
        CONS_OUT("no mem!\n");
        return AIC_ENOMEM;
    }
    parse_input_cmd(argc, argv, param);

    if (voip_test_th)
        aos_delete_thread(voip_test_th);
    voip_test_th = aos_create_thread("voip test",
                                     voip_test_thread_loop,
                                     (void *)param,
                                     ASTREAM_TEST_STACK_SIZE, 0,
                                     NULL,
                                     ASTREAM_TEST_THREAD_PRI,
                                     AOS_AUTO_START);
    return 0;
}
CONSOLE_EXPORT_CMD(voip_start, voip_test_main, voip start test);

int voip_test_stop(int argc, char **argv)
{
    if (voip_test_state == 1)
        voip_test_state = 2;
    else if (voip_test_state != 2)
        voip_test_state = 0;

    return 0;
}
CONSOLE_EXPORT_CMD(voip_stop, voip_test_stop, voip stop test);
#endif

