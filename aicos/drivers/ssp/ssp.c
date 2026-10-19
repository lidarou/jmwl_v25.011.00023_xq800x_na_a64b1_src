/*****************************************************************************/
/*                                                                           */
/*         Copyright 2021 by AIC Corporation. All rights reserved.           */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x153f2e
#define LOG_MODULE    AMOD_SSP

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_gpio_pin.h>
#include <aic_io.h>
#include <core_op.h>
#include <slp_inc.h>
#include <dvfs_inc.h>

#include "sspcore.h"

static void ssp_lock(struct ssp_device *dev)
{
    aos_status status;

    if (aos_is_user_thread_context()) {
        status = aos_get_mutex(dev->bus->mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(status == AOS_SUCCESS);

        aic_get_wakelock();
    }
}

static void ssp_unlock(struct ssp_device *dev)
{
    if (aos_is_user_thread_context()) {
        aic_put_wakelock();
        aos_put_mutex(dev->bus->mutex);
    }
}

static boolean ssp_is_device_registered(struct aic_ssp *bus, char *dev_name,
                                        struct ssp_cfg *cfg)
{
    struct ssp_device *dev;
    boolean ret = FALSE;
    AIC_INT_SAVE_AREA;

    AIC_INT_DISABLE;

    list_for_each_entry(dev, &bus->head, node, __typeof(*dev)) {
        if (dev) {
            if (strcmp(dev->name, dev_name) == 0 ||
                (dev->cfg.cs != SSP_NO_CS && dev->cfg.cs == cfg->cs)) {
                ret = TRUE;
                break;
            }
        }
    }

    AIC_INT_RESTORE;

    return ret;
}

int ssp_device_register(u32 *handle, char *name, char *bus_name,
                        struct ssp_cfg *cfg)
{
    struct aic_ssp *bus;
    struct ssp_device *dev;
    AIC_INT_SAVE_AREA;

    if (!handle || !name || !bus_name|| !cfg)
        return AIC_EINVAL;

    bus = ssp_get_handle(bus_name);
    if (!bus)
        return AIC_EOPNOTSUPP;

    /* check if device has exsited */
    if (ssp_is_device_registered(bus, name, cfg))
        return AIC_EOPNOTSUPP;

    dev = aos_malloc(sizeof(*dev));
    AIC_ASSERT(dev != NULL);

    dev->name = name;
    dev->bus = bus;
    memcpy(&(dev->cfg), cfg, sizeof(*cfg));

    AIC_INT_DISABLE;

    bus->dev_cnt++;

    list_head_init(&(dev->node));
    /* add ssp device to ssp bus */
    list_add_tail(&dev->node, &(bus->head));

    *handle = (u32)dev;

    AIC_INT_RESTORE;

    return AIC_SUCCESS;
}

int ssp_configure(u32 handle, struct ssp_cfg *cfg)
{
    struct ssp_device *dev = (struct ssp_device *)handle;

    if (!dev || !cfg)
        return AIC_EINVAL;

    if (!(dev->bus))
        return AIC_EOPNOTSUPP;

    memcpy(&(dev->cfg), cfg, sizeof(*cfg));

    ssp_lock(dev);

    if (dev->bus->owner == dev)
        dev->bus->ops->ctrl_configure(dev, cfg);

    ssp_unlock(dev);

    return AIC_SUCCESS;
}

int ssp_read(u32 handle, u8 *buf, u32 size)
{
    struct ssp_device *dev = (struct ssp_device *)handle;
    struct ssp_transfer xfer;
    struct ssp_message msg;
    int ret;

    if (!dev || !buf || !size)
        return AIC_EINVAL;

    if (!(dev->bus))
        return AIC_EOPNOTSUPP;

    ssp_lock(dev);

    if (dev->bus->owner != dev) {
        dev->bus->ops->ctrl_configure(dev, &(dev->cfg));
        dev->bus->owner = dev;
    }

    ssp_message_init(&msg);

    xfer.tx_buf = NULL;
    xfer.rx_buf = buf;
    xfer.length = size;
    list_head_init(&(xfer.node));
    ssp_message_add_tail(&xfer, &msg);

    ret = dev->bus->ops->transfer(dev, &msg);

    ssp_unlock(dev);

    return ret;
}

int ssp_write(u32 handle, u8 *buf, u32 size)
{
    struct ssp_device *dev = (struct ssp_device *)handle;
    struct ssp_transfer xfer;
    struct ssp_message msg;
    int ret;

    if (!dev || !buf || !size)
        return AIC_EINVAL;

    if (!(dev->bus))
        return AIC_EOPNOTSUPP;

    ssp_lock(dev);

    if (dev->bus->owner != dev) {
        dev->bus->ops->ctrl_configure(dev, &(dev->cfg));
        dev->bus->owner = dev;
    }

    ssp_message_init(&msg);

    xfer.tx_buf = buf;
    xfer.rx_buf = NULL;
    xfer.length = size;
    list_head_init(&(xfer.node));
    ssp_message_add_tail(&xfer, &msg);

    ret = dev->bus->ops->transfer(dev, &msg);

    ssp_unlock(dev);

    return ret;
}

int ssp_write_read(u32 handle, u8 *wbuf, u8 *rbuf, u32 size)
{
    struct ssp_device *dev = (struct ssp_device *)handle;
    struct ssp_transfer xfer;
    struct ssp_message msg;
    int ret;

    if (!dev || !wbuf || !rbuf || !size)
        return AIC_EINVAL;

    if (!(dev->bus))
        return AIC_EOPNOTSUPP;

    ssp_lock(dev);

    if (dev->bus->owner != dev) {
        dev->bus->ops->ctrl_configure(dev, &(dev->cfg));
        dev->bus->owner = dev;
    }

    ssp_message_init(&msg);

    xfer.tx_buf = wbuf;
    xfer.rx_buf = rbuf;
    xfer.length = size;
    list_head_init(&(xfer.node));
    ssp_message_add_tail(&xfer, &msg);

    ret = dev->bus->ops->transfer(dev, &msg);

    ssp_unlock(dev);

    return ret;
}

int ssp_write_then_read(u32 handle, u8 *wbuf, u32 wsize, u8 *rbuf, u32 rsize)
{
    struct ssp_device *dev = (struct ssp_device *)handle;
    struct ssp_transfer xfers[2];
    struct ssp_message msg;
    int ret;

    if (!dev || !wbuf || !wsize || !rbuf || !rsize)
        return AIC_EINVAL;

    if (!(dev->bus))
        return AIC_EOPNOTSUPP;

    ssp_lock(dev);

    if (dev->bus->owner != dev) {
        dev->bus->ops->ctrl_configure(dev, &(dev->cfg));
        dev->bus->owner = dev;
    }

    ssp_message_init(&msg);

    xfers[0].tx_buf = wbuf;
    xfers[0].rx_buf = NULL;
    xfers[0].length = wsize;
    list_head_init(&(xfers[0].node));
    ssp_message_add_tail(&xfers[0], &msg);

    xfers[1].tx_buf = NULL;
    xfers[1].rx_buf = rbuf;
    xfers[1].length = rsize;
    list_head_init(&(xfers[1].node));
    ssp_message_add_tail(&xfers[1], &msg);

    ret = dev->bus->ops->transfer(dev, &msg);

    ssp_unlock(dev);

    return ret;
}

int ssp_write_then_write(u32 handle, u8 *wbuf, u32 wsize, u8 *wbuf1, u32 wsize1)
{
    struct ssp_device *dev = (struct ssp_device *)handle;
    struct ssp_transfer xfers[2];
    struct ssp_message msg;
    int ret;

    if (!dev || !wbuf || !wsize || !wbuf1 || !wsize1)
        return AIC_EINVAL;

    if (!(dev->bus))
        return AIC_EOPNOTSUPP;

    ssp_lock(dev);

    if (dev->bus->owner != dev) {
        dev->bus->ops->ctrl_configure(dev, &(dev->cfg));
        dev->bus->owner = dev;
    }

    ssp_message_init(&msg);

    xfers[0].tx_buf = wbuf;
    xfers[0].rx_buf = NULL;
    xfers[0].length = wsize;
    list_head_init(&(xfers[0].node));
    ssp_message_add_tail(&xfers[0], &msg);

    xfers[1].tx_buf = wbuf1;
    xfers[1].rx_buf = NULL;
    xfers[1].length = wsize1;
    list_head_init(&(xfers[1].node));
    ssp_message_add_tail(&xfers[1], &msg);

    ret = dev->bus->ops->transfer(dev, &msg);

    ssp_unlock(dev);

    return ret;
}

int ssp_sync(u32 handle, struct ssp_message *msg)
{
    struct ssp_device *dev = (struct ssp_device *)handle;
    int ret;

    if (!dev || !msg)
        return AIC_EINVAL;

    if (!(dev->bus))
        return AIC_EOPNOTSUPP;

    ssp_lock(dev);

    if (dev->bus->owner != dev) {
        dev->bus->ops->ctrl_configure(dev, &(dev->cfg));
        dev->bus->owner = dev;
    }

    ret = dev->bus->ops->transfer(dev, msg);

    ssp_unlock(dev);

    return ret;
}
