/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x07ea16
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
#include "sensor_bf20a6.h"

#ifdef __cplusplus
    extern "C"
    {
#endif

int bf20a6_power_on(void)
{
#ifdef USING_PMIC_GPIO
    aic_gpio_pmic_request(camera_sensor_bf20a6_cfg.gpio_pwdn);
    if (camera_sensor_bf20a6_cfg.gpio_rst != camera_sensor_bf20a6_cfg.gpio_pwdn)
        aic_gpio_pmic_request(camera_sensor_bf20a6_cfg.gpio_rst);
#else
    aic_gpio_request(camera_sensor_bf20a6_cfg.gpio_pwdn);
    if (camera_sensor_bf20a6_cfg.gpio_rst != camera_sensor_bf20a6_cfg.gpio_pwdn)
        aic_gpio_request(camera_sensor_bf20a6_cfg.gpio_rst);
#endif

#if defined(CONFIG_PMIC)
    struct pmu_user *puser;

    /* open IOVDD */
    puser = camera_sensor_bf20a6_cfg.vddcamio1v8;
    pmu_voltage_set(puser, 1880000, 10000);
    pmu_enable(puser);
    aic_udelay(10);

    /* open XCLK */
#ifdef CLK_CSI_24M_SRC_PLL_EN
    clk_pll_enable(CLK_CSI_24M_SRC_PLL_EN);
#endif
#ifdef CLK_CSI_24M_EN
    clk_pll_enable(CLK_CSI_24M_EN);
#endif
    /*AIC_WRITE32(0x4017002c, 0);*/
    aic_mdelay(100);

    /* open VDD3A 2.8v */
    puser = camera_sensor_bf20a6_cfg.vddcama2v8;
    pmu_voltage_set(puser, 2800000, 10000);
    pmu_enable(puser);
    aic_udelay(10);
#endif

    /* pull down the XPD */
#ifdef USING_PMIC_GPIO
    aic_gpio_pmic_output(camera_sensor_bf20a6_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_LOW);
#else
    aic_gpio_output(camera_sensor_bf20a6_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_LOW);
#endif
    aic_mdelay(100);

    return AIC_SUCCESS;
}

void bf20a6_power_down(void)
{
    /* stop XCLK */
#ifdef CLK_CSI_24M_EN
    clk_pll_disable(CLK_CSI_24M_EN);
#endif
#ifdef CLK_CSI_24M_SRC_PLL_EN
    clk_pll_disable(CLK_CSI_24M_SRC_PLL_EN);
#endif
    /*writel_relaxed(0x4017002c, 1);*/
    aic_udelay(100);

    /* Lower XPD */
#ifdef USING_PMIC_GPIO
    aic_gpio_pmic_output(camera_sensor_bf20a6_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_HIGH);
#else
    aic_gpio_output(camera_sensor_bf20a6_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_HIGH);
#endif
    aic_udelay(100);

#if defined(CONFIG_PMIC)
    /* close VDD3A 2.8V */
    pmu_disable(camera_sensor_bf20a6_cfg.vddcama2v8);
    aic_udelay(100);

    /* close VDDIO */
    pmu_disable(camera_sensor_bf20a6_cfg.vddcamio1v8);
#endif

#ifdef USING_PMIC_GPIO
    aic_gpio_pmic_free(camera_sensor_bf20a6_cfg.gpio_pwdn);
    if (camera_sensor_bf20a6_cfg.gpio_rst != camera_sensor_bf20a6_cfg.gpio_pwdn)
        aic_gpio_pmic_free(camera_sensor_bf20a6_cfg.gpio_rst);
#else
    aic_gpio_free(camera_sensor_bf20a6_cfg.gpio_pwdn);
    if (camera_sensor_bf20a6_cfg.gpio_rst != camera_sensor_bf20a6_cfg.gpio_pwdn)
        aic_gpio_free(camera_sensor_bf20a6_cfg.gpio_rst);
#endif
}

#define PRODUCT_ID_MSB             0xfc
#define PRODUCT_ID_MSB_VAL         0x20
#define PRODUCT_ID_LSB             0xfd
#define PRODUCT_ID_LSB_VAL         0xa6

int bf20a6_check_id(void)
{
    u8 val;
    int cnt;

    for (cnt = 0; cnt < 3; cnt++) {
        camera_sensor_reg_read(&bf20a6_client, PRODUCT_ID_MSB, &val);
        if (val == PRODUCT_ID_MSB_VAL) {
            camera_sensor_reg_read(&bf20a6_client, PRODUCT_ID_LSB, &val);
            if (val == PRODUCT_ID_LSB_VAL) {
                aicx_info(0x084ed1, "", "sensor bf20a6 id read success!\n");
                return AIC_SUCCESS;
            }
        }
    }

    aicx_error(0x04a5c8, "", "sensor bf20a6 id read failed!\n");
    return AIC_ENODEV;
}

int bf20a6_set_output_size(u32 width, u32 height)
{
    int ret;
    struct sensor_reg w_reg, h_reg;

    w_reg.reg = 0x18;
    h_reg.reg = 0x1a;

    w_reg.val = width >> 2;
    h_reg.val = height >> 2;

    ret = camera_sensor_reg_write(&bf20a6_client, (u8 *)&w_reg);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x088173, "", "bf20a6 set width size failed!\n");
        return ret;
    }
    ret = camera_sensor_reg_write(&bf20a6_client, (u8 *)&h_reg);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x0c599a, "", "bf20a6 set height size failed!\n");
        return ret;
    }

    return AIC_SUCCESS;
}

int bf20a6_initial_setting(camera_data_fmt_e data_fmt, u32 width, u32 height)
{
    int i, ret, initial_array_size;
    struct sensor_reg const *initial_array;

    if ((data_fmt != YUV422) && (data_fmt != Y) && (data_fmt != RAW8)) {
        aicx_error(0x06c5ca, "", "bf20a6 does not support this format!\n");
        return AIC_EINVAL;
    }

    /* default init as YUV422 */
    initial_array = bf20a6_init_m_yuv_2line_nocrc;
    initial_array_size =
        sizeof(bf20a6_init_m_yuv_2line_nocrc) / sizeof(struct sensor_reg);
    for (i = 0; i < initial_array_size; i++) {
        ret = camera_sensor_reg_write(&bf20a6_client, (u8 *)&initial_array[i]);
        if (ret != AIC_SUCCESS) {
            aicx_error(0x189555, "", "bf20a6 initial setting failed!\n");
            return ret;
        }
    }

    initial_array = AIC_NULL;

    if (data_fmt == Y) {
        initial_array = bf20a6_m_y_2line_nocrc;
        initial_array_size = sizeof(bf20a6_m_y_2line_nocrc) / sizeof(struct sensor_reg);
    }

    if (data_fmt == RAW8) {
        initial_array = bf20a6_m_raw8_2line_nocrc;
        initial_array_size = sizeof(bf20a6_m_raw8_2line_nocrc) / sizeof(struct sensor_reg);
    }

    if (initial_array != AIC_NULL) {
        for (i = 0; i < initial_array_size; i++) {
            ret = camera_sensor_reg_write(&bf20a6_client, (u8 *)&initial_array[i]);
            if (ret != AIC_SUCCESS) {
                aicx_error(0x189555, "", "bf20a6 initial setting failed!\n");
                return ret;
            }
        }
    }

    ret = bf20a6_set_output_size(width, height);
    if (ret < 0)
        return ret;

    aicx_info(0x0d491d, "", "sensor bf20a6 init success!\n");

    return AIC_SUCCESS;
}

int bf20a6_open(void)
{
    camera_sensor_sample_mode_e cs_sample_mode;
    camera_sensor_data_line_num_e cs_line_num;
    struct stream_info *stream_info_ptr;
    int ret;

    i2c_open(&bf20a6_client);

    stream_info_ptr = camera_get_stream_info(CAM_STREAM_PREVIEW_INFO);
#ifdef CAMERA_SENSOR_OUTPUT_Y
    stream_info_ptr->fmt = Y;
    stream_info_ptr->bytes_per_pixel = 1;
#else
    stream_info_ptr->fmt = YUV422;
    stream_info_ptr->bytes_per_pixel = 2;
#endif
    stream_info_ptr->plane_num = YUV_ONE_PLANE;
    stream_info_ptr->width = CAMERA_SENSOR_OUTPUT_WIDTH;
    stream_info_ptr->height = CAMERA_SENSOR_OUTPUT_HEIGHT;
    stream_info_ptr->img_size = stream_info_ptr->width * stream_info_ptr->height * 2;

    ret = bf20a6_power_on();
    if (ret != AIC_SUCCESS) {
        aicx_error(0x174616, "", "sensor bf20a6 power on failed!\n");
        return ret;
    }

    ret = bf20a6_initial_setting(stream_info_ptr->fmt, stream_info_ptr->width,
                                 stream_info_ptr->height);
    if (ret != AIC_SUCCESS)
        goto power_down_bf20a6;

    stream_info_ptr->fb_num = camera_fb_alloc(stream_info_ptr->img_size);

    csi_enable();

    /* byd sensor only support SDR sample mode */
    cs_sample_mode = CAMERA_SENSOR_SAMPLE_SDR;

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

    aicx_info(0x16841a, "", "sensor bf20a6 open success!\n");

    return AIC_SUCCESS;

power_down_bf20a6:
    bf20a6_power_down();
    return ret;
}

void bf20a6_close(void)
{
    camera_sensor_common_disable();

    bf20a6_power_down();

    aicx_info(0x0f4eac, "", "sensor bf20a6 close success!\n");
}

int bf20a6_set_clock(u32 rate)
{
    return AIC_SUCCESS;
}

int bf20a6_set_output_format(camera_data_fmt_e data_fmt)
{
    int i, ret, initial_array_size;
    struct sensor_reg const *initial_array;

    if (data_fmt == YUV422) {
        initial_array = bf20a6_init_m_yuv_2line_nocrc;
        initial_array_size = sizeof(bf20a6_m_yuv_2line_nocrc) / sizeof(struct sensor_reg);
    } else if (data_fmt == Y) {
        initial_array = bf20a6_m_y_2line_nocrc;
        initial_array_size = sizeof(bf20a6_m_y_2line_nocrc) / sizeof(struct sensor_reg);
    } else if (data_fmt == RAW8) {
        initial_array = bf20a6_m_raw8_2line_nocrc;
        initial_array_size = sizeof(bf20a6_m_raw8_2line_nocrc) / sizeof(struct sensor_reg);
    } else {
        aicx_info(0x1a8824, "", "the data format not support, set failed!\n");
        return AIC_EINVAL;
    }

    for (i = 0; i < initial_array_size; i++) {
        ret = camera_sensor_reg_write(&bf20a6_client, (u8 *)&initial_array[i]);
        if (ret != AIC_SUCCESS) {
            aicx_error(0x195f72, "", "bf20a6 set format failed!\n");
            return ret;
        }
    }

    return AIC_SUCCESS;
}

camera_data_fmt_e bf20a6_get_output_format(void)
{
#ifdef CAMERA_SENSOR_OUTPUT_Y
    return Y;
#else
    return YUV422;
#endif
}

struct camera_spi_sensor_ops cs_bf20a6_ops = {
    .initial_setting = &bf20a6_initial_setting,
    .cs_open = &bf20a6_open,
    .cs_close = &bf20a6_close,
    .cs_start_xfer = &csi_start,
    .cs_stop_xfer = &csi_stop,
    .cs_set_clock = &bf20a6_set_clock,
    .cs_set_output_size = &bf20a6_set_output_size,
    .cs_set_output_format = &bf20a6_set_output_format,
    .cs_get_output_format = &bf20a6_get_output_format,
};

int sensor_bf20a6_init(void)
{
    int ret;

#if defined(CONFIG_PMIC)
    camera_sensor_bf20a6_cfg.vddcamio1v8 = pmu_get(CAMERA_SENSOR_IO_USER_NAME, CAMERA_SENSOR_IO_UNIT_NAME);
    if (!camera_sensor_bf20a6_cfg.vddcamio1v8) {
        aicx_warn(0x11bcdc, "", "get vddcamio1v8 failed!\n");
        ret = AIC_ENXIO;
        goto ERR_RETURN;
    }

    camera_sensor_bf20a6_cfg.vddcama2v8 = pmu_get(CAMERA_SENSOR_AVDD_USER_NAME, CAMERA_SENSOR_AVDD_UNIT_NAME);
    if (!camera_sensor_bf20a6_cfg.vddcama2v8) {
        aicx_warn(0x16aeb5, "", "get vddcama2v8 failed!\n");
        pmu_put(camera_sensor_bf20a6_cfg.vddcamio1v8);
        ret = AIC_ENXIO;
        goto ERR_RETURN;
    }
#endif

    i2c_open(&bf20a6_client);

    ret = bf20a6_power_on();
    if (ret != AIC_SUCCESS) {
        aicx_warn(0x174616, "", "sensor bf20a6 power on failed!\n");
        goto ERR_PMU_PUT;
    }

    ret = bf20a6_check_id();
    if (ret != AIC_SUCCESS) {
        bf20a6_power_down();
        i2c_close(&bf20a6_client);
        goto ERR_PMU_PUT;
    }

    bf20a6_power_down();
    i2c_close(&bf20a6_client);

    register_irq(camera_sensor_bf20a6_cfg.irq, csi_irq_handler, NULL, NULL, NULL);
    camera_sensor_bf20a6_cfg.cs_sensor_ops = &cs_bf20a6_ops;
    camera_sensor_register(CAMERA_FRONT, &camera_sensor_bf20a6_cfg);
    aicx_info(0x0da97e, "", "camera sensor bf20a6 register success!\n");


    return AIC_SUCCESS;
ERR_PMU_PUT:
#if defined(CONFIG_PMIC)
    pmu_put(camera_sensor_bf20a6_cfg.vddcamio1v8);
    pmu_put(camera_sensor_bf20a6_cfg.vddcama2v8);

ERR_RETURN:
    aicx_error(0x13f119, "", "camera sensor bf20a6 register failed!\n");
#endif

    return ret;
}

AIC_NORMAL_COMP_L1_INIT_EXPORT(sensor_bf20a6_init);

