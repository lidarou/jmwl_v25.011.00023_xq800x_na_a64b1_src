/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of example module.example module is
 *    a virtual hw module to demostration driver development and usage.
 *    The public function implementation here will be used as interface to
 *    software which need driver service.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x083f45
#define LOG_MODULE    AMOD_DRV

#include <example_inc.h>
#include "example_v1.h"

/* driver version */
#define AIC_EXAMPLE_DRV_VERSION    AIC_DRIVER_VERSION(1, 0)
/* hw version, it should be read from register */
#define AIC_EXAMPLE_HW_VERSION     AIC_DRIVER_VERSION(1, 0)

/* example version */
static const struct aic_driver_version example_version = {
    AIC_EXAMPLE_HW_VERSION,
    AIC_EXAMPLE_DRV_VERSION
};

/* driver capabilities */
static const struct aic_example_capabilities driver_cap = {
    AIC_EXAMPLE_CAP_A|
    AIC_EXAMPLE_CAP_B|
    AIC_EXAMPLE_CAP_C1
};

/* aic_example_get_version - get hardware and driver version */
static struct aic_driver_version aic_example_get_version(void)
{
    /* FIRST read hw version and fill example_version */
    aicx_info(0x103cc2, "dddd", "example driver version:%d.%d, hw version: "
                 "%d.%d\n",
                 (example_version.drv_ver & 0xFF00) >> 8,
                 example_version.drv_ver & 0xFF,
                 (example_version.hw_ver & 0xFF00) >> 8,
                 example_version.hw_ver & 0xFF);

    return example_version;
}

/* aic_example_get_capabilities - get driver capabilities */
static struct aic_example_capabilities
aic_example_get_capabilities(void)
{
    aicx_info(0x02694c, "x", "example driver capabilities: 0x%x\n",
                 driver_cap.cap);

    return driver_cap;
}

/* aic_example_initialize - initialize driver */
static int32_t aic_example_initialize(void)
{
    aicx_info(0x14f817, "", "Initialize: Success!\n");

    return AIC_SUCCESS;
}

/* aic_example_uninitialize - uninitialize driver */
static int32_t aic_example_uninitialize(void)
{
    aicx_info(0x113abf, "", "Uninitialize: Success!\n");

    return AIC_SUCCESS;
}

/**
 * aic_example_io_control - io control function
 * @control: io control command id
 * @arg: argument for control command
 */
static int32_t aic_example_io_control(uint32_t control, uint32_t arg)
{
    switch(control){
    case AIC_EXAMPLE_IO_A:
        aicx_info(0x12fd0e, "", "IO control A!\n");
        break;
    case AIC_EXAMPLE_IO_B:
        aicx_info(0x170fbc, "", "IO control B!\n");
        break;
    case AIC_EXAMPLE_IO_C1:
        aicx_info(0x04c697, "", "IO control C1!\n");
        break;
    default:
        aicx_info(0x013841, "x", "Control word 0x%x\n", control);
}

    return AIC_SUCCESS;
}

/* aic_example_function_a - function a implementation */
static int32_t aic_example_function_a(void)
{
    aicx_info(0x083fef, "", "function a success!\n");

    return AIC_SUCCESS;
}

/* aic_example_function_b - function b implementation */
static int32_t aic_example_function_b(void)
{
    aicx_info(0x19e012, "", "function b success!\n");

    return AIC_SUCCESS;
}

struct aic_example_driver driver_example = {
    aic_example_get_version,
    aic_example_get_capabilities,
    aic_example_initialize,
    aic_example_uninitialize,
    aic_example_io_control,
    aic_example_function_a,
    aic_example_function_b
};
