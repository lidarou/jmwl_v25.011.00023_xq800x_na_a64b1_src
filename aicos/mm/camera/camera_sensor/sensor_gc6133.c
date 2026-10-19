/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0fc4e4
#define LOG_MODULE    AMOD_CAMERA

#include <aic_type.h>
#include <aic_io.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_clk_map.h>
#include <aic_iot_clk.h>

#include <gpio_inc.h>
#include <timer_inc.h>
#include <i2c.h>
#include <display_cfg.h>
#include <camera_hal.h>
#include <csi_hal.h>

#include "sensor_private.h"
#include "sensor_gc6133.h"

#ifdef         __cplusplus
    extern         "C"
    {
#endif

int gc6133_power_on(void)
{
#ifdef USING_PMIC_GPIO
    aic_gpio_pmic_request(camera_sensor_gc6133_cfg.gpio_pwdn);
    if (camera_sensor_gc6133_cfg.gpio_rst != camera_sensor_gc6133_cfg.gpio_pwdn)
        aic_gpio_pmic_request(camera_sensor_gc6133_cfg.gpio_rst);
#else
    aic_gpio_request(camera_sensor_gc6133_cfg.gpio_pwdn);
    if (camera_sensor_gc6133_cfg.gpio_rst != camera_sensor_gc6133_cfg.gpio_pwdn)
        aic_gpio_request(camera_sensor_gc6133_cfg.gpio_rst);
#endif

#if defined(CONFIG_PMIC)
    struct pmu_user *puser;

    /* open IOVDD */
    puser = camera_sensor_gc6133_cfg.vddcamio1v8;
    pmu_voltage_set(puser, 1880000, 10000);
    pmu_enable(puser);

    /* Delay some time */
    aic_mdelay(2);

    /* open AVDD28 */
    puser = camera_sensor_gc6133_cfg.vddcama2v8;
    pmu_voltage_set(puser, 2800000, 10000);
    pmu_enable(puser);
#endif
    /* Delay some time */
    aic_mdelay(2);

    /* open mclk */
#ifdef CLK_CSI_24M_SRC_PLL_EN
    clk_pll_enable(CLK_CSI_24M_SRC_PLL_EN);
#endif
#ifdef CLK_CSI_24M_EN
    clk_pll_enable(CLK_CSI_24M_EN);
#endif
    //writel_relaxed(0x4017002c, 0);
    aic_mdelay(100);

#ifdef USING_PMIC_GPIO
    aic_gpio_pmic_output(camera_sensor_gc6133_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_HIGH);
    aic_mdelay(100);

    aic_gpio_pmic_output(camera_sensor_gc6133_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_LOW);
#else
    aic_gpio_output(camera_sensor_gc6133_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_HIGH);
    aic_mdelay(100);

    aic_gpio_output(camera_sensor_gc6133_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_LOW);
#endif

    return AIC_SUCCESS;
}

void gc6133_power_down(void)
{
    /* Set PWDN as high */
#ifdef USING_PMIC_GPIO
    aic_gpio_pmic_output(camera_sensor_gc6133_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_HIGH);
#else
    aic_gpio_output(camera_sensor_gc6133_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_HIGH);
#endif
    aic_mdelay(100);

    /* Stop MCLK */
#ifdef CLK_CSI_24M_EN
    clk_pll_disable(CLK_CSI_24M_EN);
#endif
#ifdef CLK_CSI_24M_SRC_PLL_EN
    clk_pll_disable(CLK_CSI_24M_SRC_PLL_EN);
#endif
    /* Delay some time */
#if defined(CONFIG_PMIC)
    /* Stop AVDD */
    pmu_disable(camera_sensor_gc6133_cfg.vddcama2v8);
    /* Delay some time */
    aic_mdelay(3);

    /* Stop Iovdd */
    pmu_disable(camera_sensor_gc6133_cfg.vddcamio1v8);
    /* Delay some time*/
    aic_mdelay(4);
#endif
    /* set PWDN at low */
#ifdef USING_PMIC_GPIO
    aic_gpio_pmic_output(camera_sensor_gc6133_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_LOW);

    aic_gpio_pmic_free(camera_sensor_gc6133_cfg.gpio_pwdn);
    if (camera_sensor_gc6133_cfg.gpio_rst != camera_sensor_gc6133_cfg.gpio_pwdn)
        aic_gpio_pmic_free(camera_sensor_gc6133_cfg.gpio_rst);
#else
    aic_gpio_output(camera_sensor_gc6133_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_LOW);

    aic_gpio_free(camera_sensor_gc6133_cfg.gpio_pwdn);
    if (camera_sensor_gc6133_cfg.gpio_rst != camera_sensor_gc6133_cfg.gpio_pwdn)
        aic_gpio_free(camera_sensor_gc6133_cfg.gpio_rst);
#endif
}

#define SENSOR_ID_ADDR_LOW         0xf0
#define SENSOR_ID_VALUE_LOW        0xba

int gc6133_check_id(void)
{
    u8 val;
    int cnt;

    for (cnt = 0; cnt < 3; cnt++) {
        camera_sensor_reg_read(&gc6133_client, SENSOR_ID_ADDR_LOW, &val);
        if (val == SENSOR_ID_VALUE_LOW) {
            aicx_info(0x01275a, "", "sensor gc6133 id read success!\n");
            return AIC_SUCCESS;
        }
    }

    aicx_error(0x12d194, "", "sensor gc6133 id read failed!\n");
    return AIC_ENODEV;
}

int gc6133_set_output_size(u32 width, u32 height)
{
    return AIC_SUCCESS;
}

int gc6133_initial_setting(camera_data_fmt_e data_fmt, u32 width, u32 height)
{
    int i, ret, initial_array_size;
    struct sensor_reg const *initial_array;

    if ((data_fmt != YUV422) && (data_fmt != Y) && (data_fmt != RAW8)) {
        aicx_error(0x1c9619, "", "gc6133 does not support this format!\n");
        return AIC_EINVAL;
    }

    /* default init as YUV */
    initial_array = gc6133_init_z_yuv_sdr_1line_nocrc_default;
    initial_array_size =
        sizeof(gc6133_init_z_yuv_sdr_1line_nocrc_default) / sizeof(struct sensor_reg);
    for (i = 0; i < initial_array_size; i++) {
        ret = camera_sensor_reg_write(&gc6133_client, (u8 *)&initial_array[i]);
        if (ret < 0) {
            aicx_error(0x0aae1d, "", "gc6133 initial setting failed!\n");
            return ret;
        }
    }

    ret = gc6133_set_output_size(width, height);
    if (ret < 0)
        return ret;

    aicx_info(0x02dbad, "", "sensor gc6133 init success!\n");

    return AIC_SUCCESS;
}

int gc6133_open(void)
{
    camera_sensor_sample_mode_e cs_sample_mode;
    camera_sensor_data_line_num_e cs_line_num;
    struct stream_info *stream_info_ptr;
    int ret;

    ret = gc6133_power_on();
    if (ret != AIC_SUCCESS) {
        aicx_error(0x01e747, "", "sensor gc6133 power on failed!\n");
        return ret;
    }

    i2c_open(&gc6133_client);

    stream_info_ptr = camera_get_stream_info(CAM_STREAM_PREVIEW_INFO);

#ifdef CAMERA_SENSOR_OUTPUT_Y
    stream_info_ptr->fmt = Y;
    stream_info_ptr->bytes_per_pixel = 1;
#else
    stream_info_ptr->fmt = YUV422;
    stream_info_ptr->bytes_per_pixel = 2;
#endif
    stream_info_ptr->plane_num = YUV_ONE_PLANE;

#if defined(CAMERA_SENSOR_OUTPUT_640x480)
    stream_info_ptr->src_width = 640;
    stream_info_ptr->src_height = 480;
    if ((stream_info_ptr->src_width / CAMERA_SENSOR_OUTPUT_WIDTH == 2) &&
        (stream_info_ptr->src_height / CAMERA_SENSOR_OUTPUT_HEIGHT == 2)) {
        stream_info_ptr->need_binning = AIC_TRUE;
        stream_info_ptr->rot_mode = ROT_MODE_ANTI_CW90;
    }
#else
    stream_info_ptr->src_width = CAMERA_SENSOR_OUTPUT_WIDTH;
    stream_info_ptr->src_height = CAMERA_SENSOR_OUTPUT_HEIGHT;
    stream_info_ptr->need_binning = AIC_FALSE;
    stream_info_ptr->rot_mode = ROT_MODE_NONE;
#endif
    stream_info_ptr->src_img_size = stream_info_ptr->src_width * stream_info_ptr->src_height *
                                    stream_info_ptr->bytes_per_pixel;

    ret = gc6133_initial_setting(stream_info_ptr->fmt, stream_info_ptr->src_width,
                                 stream_info_ptr->src_height);
    if (ret != AIC_SUCCESS)
        goto power_down_gc6133;

    stream_info_ptr->fb_num = camera_fb_alloc(stream_info_ptr->src_img_size);

    csi_enable();

#ifdef CAMERA_DDR_MODE
    cs_sample_mode = CAMERA_SENSOR_SAMPLE_DDR;
#else
    cs_sample_mode = CAMERA_SENSOR_SAMPLE_SDR;
#endif

#ifdef CAMERA_OUTPUT_2LINE
    cs_line_num = CAMERA_SENSOR_DATA_2_LINE;
#elif defined(CAMERA_OUTPUT_4LINE)
    cs_line_num = CAMERA_SENSOR_DATA_4_LINE;;
#else
    cs_line_num = CAMERA_SENSOR_DATA_1_LINE;
#endif

    camera_sensor_common_config(cs_sample_mode, cs_line_num,
                                stream_info_ptr->plane_num,
                                stream_info_ptr->src_img_size);
#ifdef CAMERA_OUTPUT_Z
    camera_sensor_output_mode_config(CAMERA_SENSOR_OUTPUT_MODE_Z);
#else
    camera_sensor_output_mode_config(CAMERA_SENSOR_OUTPUT_MODE_M);
#endif

    csi_endian_mode_config(3);

    aicx_info(0x09ffac, "", "sensor gc6133 open success!\n");

    return AIC_SUCCESS;

power_down_gc6133:
    gc6133_power_down();
    return ret;
}

void gc6133_close(void)
{
   camera_sensor_common_disable();

   gc6133_power_down();

   i2c_close(&gc6133_client);

   aicx_info(0x14ca69, "", "sensor gc6133 close success!\n");
}

int gc6133_set_clock(u32 rate)
{
    return AIC_SUCCESS;
}

int gc6133_set_output_format(u8 fmt)
{
    return AIC_SUCCESS;
}

camera_data_fmt_e gc6133_get_output_format(void)
{
#ifdef CAMERA_SENSOR_OUTPUT_Y
    return Y;
#else
    return YUV422;
#endif
}

struct camera_spi_sensor_ops cs_gc6133_ops = {
    .cs_open = &gc6133_open,
    .cs_close = &gc6133_close,
    .cs_start_xfer = &csi_start,
    .cs_stop_xfer = &csi_stop,
    .cs_set_clock = &gc6133_set_clock,
    .cs_set_output_size = &gc6133_set_output_size,
    .cs_set_output_format = &gc6133_set_output_format,
    .cs_get_output_format = &gc6133_get_output_format,
};

int sensor_gc6133_init(void)
{
    int ret;

#if defined(CONFIG_PMIC)
    camera_sensor_gc6133_cfg.vddcamio1v8 = pmu_get(CAMERA_SENSOR_IO_USER_NAME, CAMERA_SENSOR_IO_UNIT_NAME);
    if (!camera_sensor_gc6133_cfg.vddcamio1v8) {
        aicx_warn(0x11bcdc, "", "get vddcamio1v8 failed!\n");
        ret = AIC_ENXIO;
        goto ERR_RETURN;
    }

    camera_sensor_gc6133_cfg.vddcama2v8 = pmu_get(CAMERA_SENSOR_AVDD_USER_NAME, CAMERA_SENSOR_AVDD_UNIT_NAME);
    if (!camera_sensor_gc6133_cfg.vddcama2v8) {
        aicx_warn(0x16aeb5, "", "get vddcama2v8 failed!\n");
        pmu_put(camera_sensor_gc6133_cfg.vddcamio1v8);
        ret = AIC_ENXIO;
        goto ERR_RETURN;
    }
#endif

    i2c_open(&gc6133_client);

    ret = gc6133_power_on();
    if (ret != AIC_SUCCESS) {
        aicx_warn(0x01e747, "", "sensor gc6133 power on failed!\n");
        goto ERR_PMU_PUT;
    }

    ret = gc6133_check_id();
    if (ret != AIC_SUCCESS) {
        gc6133_power_down();
        i2c_close(&gc6133_client);
        goto ERR_PMU_PUT;
    }

    gc6133_power_down();
    i2c_close(&gc6133_client);

    register_irq(camera_sensor_gc6133_cfg.irq, csi_irq_handler, NULL, NULL, NULL);
    camera_sensor_gc6133_cfg.cs_sensor_ops = &cs_gc6133_ops;
    camera_sensor_register(CAMERA_FRONT, &camera_sensor_gc6133_cfg);
    aicx_info(0x06f317, "", "camera sensor gc6133 register success!\n");

    return AIC_SUCCESS;

ERR_PMU_PUT:
#if defined(CONFIG_PMIC)
    pmu_put(camera_sensor_gc6133_cfg.vddcamio1v8);
    pmu_put(camera_sensor_gc6133_cfg.vddcama2v8);

ERR_RETURN:
    aicx_error(0x156d4d, "", "camera sensor gc6133 register failed!\n");
#endif

    return ret;
}

AIC_NORMAL_COMP_L1_INIT_EXPORT(sensor_gc6133_init);

