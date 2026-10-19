/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is used to control sound stream, each stream have it own buffer
 *    with the model producer-consumer, by calling API we can start/stop this
 *    stream, and suspend/resume, open/close support as well.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x19be21
#define LOG_MODULE    AMOD_SOUND

#include <stdio.h>
#include <stdlib.h>
#include "snd_stream_driver.h"
#include "snd_stream.h"
#include "snd_dai.h"

#ifndef STREAM_DRIVER_MAX
#define STREAM_DRIVER_MAX 20
#endif

struct snd_stream_platfrom {
    struct snd_stream_vdev vdev[STREAM_DRIVER_MAX];
    int cnt;
};

struct snd_stream_platfrom *g_snd_stream_plaform;

static struct snd_stream_vdev *get_stream_by_name(char *stream_name);

int snd_stream_open(char *stream_name, int flag, u32 *file)
{
    struct snd_stream_vdev *vdev;
    struct snd_stream *stream;
    struct snd_stream_runtime *runtime;
    enum snd_stream_direction dirn;
    int ret;

    if (flag & (STREAM_PCM_IN | STREAM_COMPR_IN))
        dirn = SND_STREAM_CAPTURE;
    else if (flag & (STREAM_PCM_OUT | STREAM_COMPR_OUT))
        dirn = SND_STREAM_PLAYBACK;
    else {
        aicx_error(0x0e8830, "", "snd stream open, no stream direction\n");
        return AIC_EINVAL;
    }
    *file = 0;
    vdev = get_stream_by_name(stream_name);
    if (vdev == NULL) {
        aicx_error(0x199223, "s", "%s, no device data!!!\n", stream_name);
        return AIC_ENODEV;
    }
    if (vdev->active) {
        aicx_error(0x0aa956, "s", "%s, stream already opened!!!\n", stream_name);
        return AIC_EBUSY;
    }
    if (dirn != vdev->direction) {
        aicx_error(0x0193cf, "sd", "device(%s) doesn't support this direction(%d)\n",
                   vdev->name, dirn);

        return AIC_EINVAL;
    }

    stream = snd_zalloc(sizeof(*stream));
    if (!stream) {
        return AIC_ENOMEM;
    }

    stream->name = vdev->name;
    stream->ops = vdev->ops;
    stream->direction = dirn;
    stream->private_data = vdev->private_data;
    stream->device = vdev;
    runtime = snd_zalloc(sizeof(*runtime));
    if (!runtime) {
        snd_free(stream);
        return AIC_ENOMEM;
    }
    runtime->state = SNDRV_STREAM_STATE_OPEN;
    runtime->dai = snd_dai_get(stream_name, (u32)dirn, 0);
    init_waitqueue_head(&runtime->sleep);
    stream->runtime = runtime;

    /* mutex is not need */
    /* mutex_lock(vdev->lock); */
    ret = vdev->ops->open(stream);
    /* mutex_unlock(vdev->lock); */
    if (ret) {
        cancel_waitqueue_head(runtime->sleep);
        snd_free(runtime);
        snd_free(stream);
        stream = NULL;
        return ret;
    }
    vdev->active = TRUE;
    SND_DAI_SAFE_RUN(stream->runtime->dai, open)
    *file = (u32)stream;

    return 0;
}

int snd_stream_close(u32 file)
{
    struct snd_stream *stream = (struct snd_stream *)file;
    struct snd_stream_runtime *runtime;

    if (!stream)
        return AIC_EINVAL;
    runtime = stream->runtime;

    SND_DAI_SAFE_RUN(runtime->dai, close)

    switch (runtime->state) {
    case SNDRV_STREAM_STATE_RUNNING:
    case SNDRV_STREAM_STATE_DRAINING:
    case SNDRV_STREAM_STATE_PAUSED:
        stream->ops->trigger(stream, SNDRV_STREAM_TRIGGER_STOP);
        break;
    default:
        break;
    }

    stream->ops->free(stream);
    stream->device->active = FALSE;
    cancel_waitqueue_head(runtime->sleep);
    if (stream->runtime->buffer)
        aos_dma_free(stream->runtime->buffer);
    snd_free(stream->runtime);
    snd_free(stream);

    return 0;
}

static int snd_stream_update_tstamp(struct snd_stream *stream,
        struct snd_stream_tstamp *tstamp)
{
    if (!stream->ops->pointer)
        return AIC_ENOTSUPP;
    stream->ops->pointer(stream, tstamp);
    aicx_debug(0x1995d8, "dd", "dsp consumed till %d total %d bytes\n",
        tstamp->byte_offset, tstamp->copied_total);
    if (stream->direction == SND_STREAM_PLAYBACK)
        stream->runtime->total_bytes_transferred = tstamp->copied_total;
    else
        stream->runtime->total_bytes_available = tstamp->copied_total;

    return 0;
}

static size_t snd_stream_calc_avail(struct snd_stream *stream,
        struct snd_stream_avail *avail)
{
    memset(avail, 0, sizeof(*avail));
    snd_stream_update_tstamp(stream, &avail->tstamp);
    /* Still need to return avail even if tstamp can't be filled in */

    if (stream->runtime->total_bytes_available == 0 &&
            stream->runtime->state == SNDRV_STREAM_STATE_SETUP &&
            stream->direction == SND_STREAM_PLAYBACK) {
        aicx_debug(0x07660e, "", "detected init and someone forgot to do a write\n");
        return stream->runtime->buffer_size;
    }
    aicx_debug(0x055735, "lldlld", "app wrote %lld, DSP consumed %lld\n",
            stream->runtime->total_bytes_available,
            stream->runtime->total_bytes_transferred);
    if (stream->runtime->total_bytes_available ==
                stream->runtime->total_bytes_transferred) {
        if (stream->direction == SND_STREAM_PLAYBACK) {
            aicx_debug(0x15e402, "", "both pointers are same, returning full avail\n");
            return stream->runtime->buffer_size;
        } else {
            aicx_debug(0x196de5, "", "both pointers are same, returning no avail\n");
            return 0;
        }
    }

    avail->avail = stream->runtime->total_bytes_available -
            stream->runtime->total_bytes_transferred;
    if (stream->direction == SND_STREAM_PLAYBACK)
        avail->avail = stream->runtime->buffer_size - avail->avail;

    aicx_debug(0x0bae8f, "lld", "ret avail as %lld\n", avail->avail);

    return avail->avail;
}

size_t snd_stream_get_avail(u32 file)
{
    struct snd_stream *stream = (struct snd_stream *)file;
    struct snd_stream_avail avail;

    return snd_stream_calc_avail(stream, &avail);
}

int snd_stream_ioctl_avail(u32 file, unsigned long arg)
{
    struct snd_stream *stream = (struct snd_stream *)file;
    struct snd_stream_avail ioctl_avail;
    size_t avail;

    avail = snd_stream_calc_avail(stream, &ioctl_avail);
    ioctl_avail.avail = avail;

    switch (stream->runtime->state) {
    case SNDRV_STREAM_STATE_OPEN:
        return AIC_EBADF;
    case SNDRV_STREAM_STATE_XRUN:
        return AIC_EPIPE;
    default:
        break;
    }

    if (copy_to_user((__u64 *)arg, &ioctl_avail, sizeof(ioctl_avail)))
        return AIC_EFAULT;

    return 0;
}

static int snd_stream_write_data(struct snd_stream *stream,
           const char *buf, size_t count)
{
    void *dstn;
    size_t copy;
    struct snd_stream_runtime *runtime = stream->runtime;

    /* 64-bit Modulus */
    /*
    u64 app_pointer = div64_u64(runtime->total_bytes_available,
                    runtime->buffer_size);
    */
    u64 app_pointer = runtime->total_bytes_available /
                    runtime->buffer_size;
    app_pointer = runtime->total_bytes_available -
              (app_pointer * runtime->buffer_size);

    dstn = (char *)runtime->buffer + app_pointer;
    aicx_debug(0x12d52a, "ldlld", "copying %ld at %lld\n",
            (unsigned long)count, app_pointer);
    if (count < runtime->buffer_size - app_pointer) {
        if (!copy_from_user(dstn, buf, count))
            return AIC_EFAULT;
    } else {
        copy = runtime->buffer_size - app_pointer;
        if (!copy_from_user(dstn, buf, copy))
            return AIC_EFAULT;
        if (!copy_from_user(runtime->buffer, buf + copy, count - copy))
            return AIC_EFAULT;
    }
    /* if DSP cares, let it know data has been written */
    if (stream->ops->ack)
        stream->ops->ack(stream, count);

    return count;
}

int snd_stream_write(u32 file, const char *buf, size_t count, void *offset)
{
    struct snd_stream *stream = (struct snd_stream *)file;
    size_t avail;
    int retval;

    snd_BUG_ON(!stream);
    if (!stream)
        return AIC_EFAULT;

    mutex_lock(stream->device->lock);
    /* write is allowed when stream is running or has been steup */
    switch (stream->runtime->state) {
    case SNDRV_STREAM_STATE_SETUP:
    case SNDRV_STREAM_STATE_PREPARED:
    case SNDRV_STREAM_STATE_RUNNING:
    case SNDRV_STREAM_STATE_PAUSED:
        break;
    default:
        mutex_unlock(stream->device->lock);
        return AIC_EBADF;
    }

    avail = snd_stream_get_avail(file);
    aicx_debug(0x07ef70, "ld", "avail returned %ld\n", (unsigned long)avail);
    /* calculate how much we can write to buffer */
    if (avail > count)
        avail = count;

    if (stream->ops->copy) {
        char *cbuf = (char *)buf;
        retval = stream->ops->copy(stream, cbuf, avail);
    } else {
        retval = snd_stream_write_data(stream, buf, avail);
    }
    if (retval > 0)
        stream->runtime->total_bytes_available += retval;

    /* while initiating the stream, write should be called before START
     * call, so in setup move state */
    if (stream->runtime->state == SNDRV_STREAM_STATE_SETUP) {
        stream->runtime->state = SNDRV_STREAM_STATE_PREPARED;
        aicx_debug(0x12e89e, "", "stream prepared, Houston we are good to go\n");
    }

    mutex_unlock(stream->device->lock);

    return retval;
}

static int snd_stream_read_data(struct snd_stream *stream,
                                const char *buf, size_t count)
{
   void *src;
   size_t copy;
   struct snd_stream_runtime *runtime = stream->runtime;

   u32 app_pointer = runtime->total_bytes_transferred %
                        runtime->buffer_size;

   src = (char *)runtime->buffer + app_pointer;

   aicx_debug(0x16e330, "dd", "transferred=%d, avail=%d\n",
              runtime->total_bytes_transferred, runtime->total_bytes_available);
   aicx_debug(0x09a362, "dd", "read %d at %d\n", count, app_pointer);
   if (count <= runtime->buffer_size - app_pointer) {
       if (!copy_to_user((char *)buf, src, count))
           return AIC_EFAULT;
   } else {
       copy = runtime->buffer_size - app_pointer;
       if (!copy_to_user((char *)buf, src, copy))
           return AIC_EFAULT;
       if (!copy_to_user((char *)buf + copy, runtime->buffer,  count - copy))
           return AIC_EFAULT;
   }

   return count;
}

int snd_stream_read(u32 file, char *buf, size_t count, void *offset)
{
    struct snd_stream *stream = (struct snd_stream *)file;
    size_t avail;
    int retval = 0;

    snd_BUG_ON(!stream);
    if (!stream)
        return AIC_EFAULT;

    mutex_lock(stream->device->lock);

    /* read is allowed when stream is running, paused, draining and setup
     * (yes setup is state which we transition to after stop, so if user
     * wants to read data after stop we allow that)
     */
    switch (stream->runtime->state) {
    case SNDRV_STREAM_STATE_OPEN:
    case SNDRV_STREAM_STATE_PREPARED:
    case SNDRV_STREAM_STATE_SUSPENDED:
    case SNDRV_STREAM_STATE_DISCONNECTED:
        retval = AIC_EBADF;
        goto out;
    case SNDRV_STREAM_STATE_XRUN:
        retval = AIC_EPIPE;
        goto out;
    }

    avail = snd_stream_get_avail(file);
    aicx_debug(0x07ef70, "ld", "avail returned %ld\n", (unsigned long)avail);
    /* calculate how much we can read from buffer */
    if (avail > count)
        avail = count;

    if (stream->ops->copy) {
        retval = stream->ops->copy(stream, buf, avail);
    } else {
        retval = snd_stream_read_data(stream, buf, avail);
    }
    if (retval > 0)
        stream->runtime->total_bytes_transferred += retval;

out:
    mutex_unlock(stream->device->lock);

    return retval;
}

unsigned int snd_stream_poll(u32 file, u32 wait)
{
    struct snd_stream *stream = (struct snd_stream *)file;
    size_t avail;
    int retval = 0;

    snd_BUG_ON(!stream);
    if (!stream)
        return 0;

    mutex_lock(stream->device->lock);

    retval = poll_wait(stream->runtime->sleep, wait);

    /* check if we have at least one fragment to fill */
    avail = snd_stream_get_avail(file);
    aicx_debug(0x09510f, "ld", "avail is %ld\n", (unsigned long)avail);

    switch (stream->runtime->state) {
    case SNDRV_STREAM_STATE_DRAINING:
        stream->runtime->state = SNDRV_STREAM_STATE_SETUP;
        break;
    case SNDRV_STREAM_STATE_RUNNING:
    case SNDRV_STREAM_STATE_PREPARED:
    case SNDRV_STREAM_STATE_PAUSED:
    default:
        break;
    }

    mutex_unlock(stream->device->lock);

    return retval;
}

int snd_stream_get_caps(struct snd_stream *stream, unsigned long arg)
{
    int retval;
    struct snd_stream_caps caps;

    if (!stream->ops->get_caps)
        return AIC_ENXIO;

    memset(&caps, 0, sizeof(caps));
    retval = stream->ops->get_caps(stream, &caps);
    if (retval)
        goto out;
    if (copy_to_user((void *)arg, &caps, sizeof(caps)))
        retval = AIC_EFAULT;
out:
    return retval;
}

int snd_stream_get_codec_caps(struct snd_stream *stream, unsigned long arg)
{
    int retval;
    struct snd_stream_codec_caps *caps;

    if (!stream->ops->get_codec_caps)
        return AIC_ENXIO;

    caps = snd_zalloc(sizeof(*caps));
    if (!caps)
        return AIC_ENOMEM;

    retval = stream->ops->get_codec_caps(stream, caps);
    if (retval)
        goto out;
    if (copy_to_user((void *)arg, caps, sizeof(*caps)))
        retval = AIC_EFAULT;

out:
    snd_free(caps);
    return retval;
}

/* revisit this with snd_pcm_preallocate_xxx */
static int snd_stream_allocate_buffer(struct snd_stream *stream,
        struct snd_stream_params *stream_params)
{
    unsigned int buffer_size;
    void *buffer;

    buffer_size = stream_params->period_size * stream_params->period_cnt;
    if (stream->ops->copy) {
        buffer = NULL;
        /* if copy is defined the driver will be required to copy
         * the data from core
         */
    } else {
        buffer = aos_dma_malloc(buffer_size);
        if (!buffer)
            return AIC_ENOMEM;
    }
    stream->runtime->fragment_size = stream_params->period_size;
    stream->runtime->fragments = stream_params->period_cnt;

    stream->runtime->buffer = buffer;
    stream->runtime->buffer_size = buffer_size;

    return 0;
}

static int snd_stream_check_input(struct snd_stream_params *stream_params)
{
    /* first let's check the buffer parameter's */
    if (stream_params->period_size == 0 ||
        stream_params->period_cnt > INT_MAX / stream_params->period_size ||
        stream_params->period_cnt == 0)
        return AIC_EINVAL;

    /* check compress codec param */
    if (!IS_PCM_PARAM(stream_params)) {
            /* TODO */
    }

    return 0;
}

unsigned int snd_stream_get_pcm_bits(int format)
{
    switch (format) {
    case PCM_FMT_S32_LE:
    case PCM_FMT_S24_LE:
        return 32;
    case PCM_FMT_S24_3LE:
        return 24;
    case PCM_FMT_S8:
        return 8;
    case PCM_FMT_S16_LE:
    default:
        return 16;
    };
}

int snd_stream_set_params(u32 file, u32 arg)
{
    struct snd_stream *stream = (struct snd_stream *)file;
    struct snd_stream_params *params;
    struct snd_dai *dai;
    int retval;

    if (!stream)
        return AIC_EINVAL;

    if (stream->runtime->state == SNDRV_STREAM_STATE_OPEN) {
        /*
         * we should allow parameter change only when stream has been
         * opened not in other cases
         */
        /*
        params = memdup_user((void __user *)arg, sizeof(*params));
        if (IS_ERR(params))
            return PTR_ERR(params);
        */
        params = (struct snd_stream_params *)arg;
        if (!params) {
            aicx_error(0x113edb, "s", "%s, arg is 0\n", __func__);
            return AIC_EINVAL;
        }

        retval = snd_stream_check_input(params);
        if (retval) {
            aicx_error(0x00a759, "sd", "%s, check input retval=%d\n", __func__, retval);
            goto out;
        }
        retval = snd_stream_allocate_buffer(stream, params);
        if (retval) {
            retval = AIC_ENOMEM;
            aicx_error(0x0a190c, "s", "%s, no memory\n", __func__);
            goto out;
        }

        retval = stream->ops->set_params(stream, params);
        if (retval) {
            aicx_error(0x0a6942, "sd", "%s, set params failed, retval=%d\n", __func__, retval);
            goto out;
        }
        stream->metadata_set = FALSE;
        stream->next_track = FALSE;

        if (stream->direction == SND_STREAM_PLAYBACK)
            stream->runtime->state = SNDRV_STREAM_STATE_SETUP;
        else
            stream->runtime->state = SNDRV_STREAM_STATE_PREPARED;

        dai = stream->runtime->dai;

        if (dai && dai->ops && dai->ops->set_params) {
            u32 bit_width = SND_STREAM_WIDTH_DEFAULT;

            if (IS_PCM_PARAM(params))
                bit_width = snd_stream_get_pcm_bits(params->format);

            dai->ops->set_params(dai, bit_width,
                                 params->channels, params->srate);
        }

    } else {
        aicx_error(0x030e7a, "sd", "%s, state=%d, wrong!\n", __func__, stream->runtime->state);
        return AIC_EPERM;
    }

out:
    return retval;
}

int snd_stream_get_params(struct snd_stream *stream, unsigned long arg)
{
    struct snd_codec_param *params;
    int retval;

    if (!stream->ops->get_params)
        return AIC_EBADF;

    params = snd_zalloc(sizeof(*params));
    if (!params)
        return AIC_ENOMEM;
    retval = stream->ops->get_params(stream, params);
    if (retval)
        goto out;
    if (copy_to_user((char *)arg, params, sizeof(*params)))
        retval = AIC_EFAULT;

out:
    snd_free(params);
    return retval;
}

int snd_stream_get_metadata(struct snd_stream *stream, unsigned long arg)
{
    struct snd_stream_metadata metadata;
    int retval;

    if (!stream->ops->get_metadata)
        return AIC_ENXIO;

    if (copy_from_user(&metadata, (void *)arg, sizeof(metadata)))
        return AIC_EFAULT;

    retval = stream->ops->get_metadata(stream, &metadata);
    if (retval != 0)
        return retval;

    if (copy_to_user((void *)arg, &metadata, sizeof(metadata)))
        return AIC_EFAULT;

    return 0;
}

int snd_stream_set_metadata(struct snd_stream *stream, unsigned long arg)
{
    struct snd_stream_metadata metadata;
    int retval;

    if (!stream->ops->set_metadata)
        return AIC_ENXIO;
    /*
    * we should allow parameter change only when stream has been
    * opened not in other cases
    */
    if (copy_from_user(&metadata, (void *)arg, sizeof(metadata)))
        return AIC_EFAULT;

    retval = stream->ops->set_metadata(stream, &metadata);
    stream->metadata_set = TRUE;

    return retval;
}

int snd_stream_tstamp(u32 file, unsigned long arg)
{
    struct snd_stream *stream = (struct snd_stream *)file;
    struct snd_stream_tstamp tstamp = {0};
    int ret;

    ret = snd_stream_update_tstamp(stream, &tstamp);
    if (ret == 0)
        copy_to_user((struct snd_stream_tstamp *)arg,
                     &tstamp, sizeof(tstamp)) ? AIC_EFAULT : 0;

    return ret;
}

int snd_stream_pause(u32 file)
{
    struct snd_stream *stream = (struct snd_stream *)file;
    int retval;

    if (!stream)
        return AIC_EINVAL;

    if (stream->runtime->state != SNDRV_STREAM_STATE_RUNNING &&
            stream->runtime->state != SNDRV_STREAM_STATE_DRAINING)
        return AIC_EPERM;
    retval = stream->ops->trigger(stream, SNDRV_STREAM_TRIGGER_PAUSE_PUSH);
    if (!retval && stream->runtime->state == SNDRV_STREAM_STATE_RUNNING) {
        stream->runtime->state = SNDRV_STREAM_STATE_PAUSED;
        SND_DAI_SAFE_RUN(stream->runtime->dai, stop)
    }
    return retval;
}

int snd_stream_resume(u32 file)
{
    struct snd_stream *stream = (struct snd_stream *)file;
    int retval;

    if (!stream)
        return AIC_EINVAL;
    if (stream->runtime->state != SNDRV_STREAM_STATE_PAUSED &&
            stream->runtime->state != SNDRV_STREAM_STATE_DRAINING)
        return AIC_EPERM;
    retval = stream->ops->trigger(stream, SNDRV_STREAM_TRIGGER_PAUSE_RELEASE);
    if (!retval && stream->runtime->state == SNDRV_STREAM_STATE_PAUSED) {
        stream->runtime->state = SNDRV_STREAM_STATE_RUNNING;
        SND_DAI_SAFE_RUN(stream->runtime->dai, start)
    }
    return retval;
}

int snd_stream_start(u32 file)
{
    struct snd_stream *stream = (struct snd_stream *)file;
    int retval;

    if (!stream)
        return AIC_EINVAL;
    if (stream->runtime->state != SNDRV_STREAM_STATE_PREPARED &&
        stream->runtime->state != SNDRV_STREAM_STATE_SETUP &&
        stream->runtime->state != SNDRV_STREAM_STATE_DRAINING)
        return AIC_EPERM;
    retval = stream->ops->trigger(stream, SNDRV_STREAM_TRIGGER_START);
    if (!retval) {
        if (stream->runtime->state != SNDRV_STREAM_STATE_DRAINING)
            stream->runtime->state = SNDRV_STREAM_STATE_RUNNING;
        SND_DAI_SAFE_RUN(stream->runtime->dai, start)
    }
    return retval;
}

int snd_stream_stop(u32 file)
{
    struct snd_stream *stream = (struct snd_stream *)file;
    int retval;

    if (!stream)
        return AIC_EINVAL;
    if (stream->runtime->state == SNDRV_STREAM_STATE_PREPARED ||
            stream->runtime->state == SNDRV_STREAM_STATE_SETUP)
        return AIC_EPERM;
    SND_DAI_SAFE_RUN(stream->runtime->dai, stop)
    retval = stream->ops->trigger(stream, SNDRV_STREAM_TRIGGER_STOP);
    if (!retval) {
        snd_stream_drain_notify(stream);
        stream->runtime->total_bytes_available = 0;
        stream->runtime->total_bytes_transferred = 0;
    }

    return retval;
}

static int snd_stream_wait_for_drain(struct snd_stream *stream)
{
    int ret;

    /*
     * We are called with lock held. So drop the lock while we wait for
     * drain complete notification from the driver
     *
     * It is expected that driver will notify the drain completion and then
     * stream will be moved to SETUP state, even if draining resulted in an
     * error. We can trigger next track after this.
     */
    stream->runtime->state = SNDRV_STREAM_STATE_DRAINING;
    //mutex_unlock(stream->device->lock);

    /* we wait for drain to complete here, drain can return when
     * interruption occurred, wait returned error or success.
     * For the first two cases we don't do anything different here and
     * return after waking up
     */

    ret = snd_stream_wait_event_drain(stream->runtime->sleep);
    if (ret == AIC_ERESTARTSYS)
        aicx_debug(0x1a0bd0, "", "wait aborted by a signal\n");
    else if (ret)
        aicx_debug(0x1d02e8, "d", "wait for drain failed with %d\n", ret);


    wake_up(stream->runtime->sleep);
    /* stream runtime state change to setup, dai cannot be stopped by stream
       stop API, we must stop it here */
    SND_DAI_SAFE_RUN(stream->runtime->dai, stop)

    //mutex_lock(stream->device->lock);

    return ret;
}

int snd_stream_drain(u32 file)
{
    struct snd_stream *stream = (struct snd_stream *)file;
    int retval;

    if (!stream)
        return AIC_EINVAL;
    //if (stream->runtime->state == SNDRV_STREAM_STATE_PREPARED ||
    //        stream->runtime->state == SNDRV_STREAM_STATE_SETUP)
    //    return AIC_EPERM;

    retval = stream->ops->trigger(stream, SNDRV_STREAM_TRIGGER_DRAIN);
    if (retval) {
        aicx_debug(0x005f57, "d", "SNDRV_STREAM_TRIGGER_DRAIN failed %d\n", retval);
        wake_up(stream->runtime->sleep);
        return retval;
    }

    return snd_stream_wait_for_drain(stream);
}

int snd_stream_next_track(struct snd_stream *stream)
{
    int retval;

    /* only a running stream can transition to next track */
    if (stream->runtime->state != SNDRV_STREAM_STATE_RUNNING)
        return AIC_EPERM;

    /* you can signal next track if this is intended to be a gapless stream
     * and current track metadata is set
     */
    if (stream->metadata_set == FALSE)
        return AIC_EPERM;

    retval = stream->ops->trigger(stream, SNDRV_STREAM_TRIGGER_NEXT_TRACK);
    if (retval != 0)
        return retval;
    stream->metadata_set = FALSE;
    stream->next_track = TRUE;

    return 0;
}

int snd_stream_partial_drain(struct snd_stream *stream)
{
    int retval;
    if (stream->runtime->state == SNDRV_STREAM_STATE_PREPARED ||
            stream->runtime->state == SNDRV_STREAM_STATE_SETUP)
        return AIC_EPERM;
    /* stream can be drained only when next track has been signalled */
    if (stream->next_track == FALSE)
        return AIC_EPERM;

    retval = stream->ops->trigger(stream, SNDRV_STREAM_TRIGGER_PARTIAL_DRAIN);
    if (retval) {
        aicx_debug(0x11c46f, "", "Partial drain returned failure\n");
        wake_up(stream->runtime->sleep);
        return retval;
    }

    stream->next_track = FALSE;

    return snd_stream_wait_for_drain(stream);
}

static struct snd_stream_vdev *get_stream_by_name(char *stream_name)
{
    struct snd_stream_platfrom *platform = g_snd_stream_plaform;
    int i, ret;

    for (i = 0; i < platform->cnt; i++) {
        ret = strcmp(platform->vdev[i].name, stream_name);
        if (ret == 0)
            return &platform->vdev[i];
    }

    return 0;
}

const char *snd_stream_get_stream_name(u32 file)
{
    struct snd_stream *stream = (struct snd_stream *)file;

    if (stream)
        return stream->name;

    return 0;
}

const char *snd_stream_get_dpm_stream_name(u32 file)
{
    struct snd_stream *stream = (struct snd_stream *)file;

    if (stream)
        return stream->device->dpm_stream;

    return 0;
}

int snd_stream_register(const struct snd_stream_vdev *vdev)
{
    struct snd_stream_platfrom *platform = g_snd_stream_plaform;

    if (!vdev)
        return AIC_EINVAL;
    if (!platform) {
        platform = snd_malloc(sizeof(struct snd_stream_platfrom));
        if (!platform) {
            aicx_error(0x0a190c, "s", "%s, no memory\n", __func__);
            return AIC_ENOMEM;
        }
        memset(platform, 0, sizeof(struct snd_stream_platfrom));
        g_snd_stream_plaform = platform;
    }

    if (platform->cnt >= STREAM_DRIVER_MAX) {
        aicx_error(0x117ade, "s", "%s, exceeded!\n", __func__);
        return AIC_ENOMEM;
    }
    platform->vdev[platform->cnt] = *vdev;
    platform->vdev[platform->cnt].active = FALSE;
    mutex_init(platform->vdev[platform->cnt].lock);
    platform->cnt++;

    return 0;
}

