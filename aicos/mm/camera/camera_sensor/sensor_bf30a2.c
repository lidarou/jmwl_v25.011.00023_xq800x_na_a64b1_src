/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x04cf34
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
#include "sensor_bf30a2.h"

#ifdef         __cplusplus
    extern         "C"
    {
#endif

int bf30a2_power_on(void)
{
#ifdef USING_PMIC_GPIO
    aic_gpio_pmic_request(camera_sensor_bf30a2_cfg.gpio_pwdn);
    if (camera_sensor_bf30a2_cfg.gpio_rst != camera_sensor_bf30a2_cfg.gpio_pwdn)
        aic_gpio_pmic_request(camera_sensor_bf30a2_cfg.gpio_rst);
#else
    aic_gpio_request(camera_sensor_bf30a2_cfg.gpio_pwdn);
    if (camera_sensor_bf30a2_cfg.gpio_rst != camera_sensor_bf30a2_cfg.gpio_pwdn)
        aic_gpio_request(camera_sensor_bf30a2_cfg.gpio_rst);
#endif

    /* open XCLK */
#ifdef CLK_CSI_24M_SRC_PLL_EN
    clk_pll_enable(CLK_CSI_24M_SRC_PLL_EN);
#endif
#ifdef CLK_CSI_24M_EN
    clk_pll_enable(CLK_CSI_24M_EN);
#endif
    /*AIC_WRITE32(0x4017002c, 0);*/
    aic_mdelay(100);

#if defined(CONFIG_PMIC)
    struct pmu_user *puser;

    /* open IOVDD */
    puser = camera_sensor_bf30a2_cfg.vddcamio1v8;
    pmu_voltage_set(puser, 1880000, 10000);
    pmu_enable(puser);
    aic_udelay(10);

    /* open AVDD28 */
    puser = camera_sensor_bf30a2_cfg.vddcama2v8;
    pmu_voltage_set(puser, 2800000, 10000);
    pmu_enable(puser);
    aic_udelay(10);
#endif

#ifdef USING_PMIC_GPIO
    aic_gpio_pmic_output(camera_sensor_bf30a2_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_HIGH);
    aic_udelay(200);

    aic_gpio_pmic_output(camera_sensor_bf30a2_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_LOW);
    aic_udelay(50);
#else
    aic_gpio_output(camera_sensor_bf30a2_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_HIGH);
    aic_udelay(200);

    aic_gpio_output(camera_sensor_bf30a2_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_LOW);
    aic_udelay(50);
#endif

    return AIC_SUCCESS;
}

void bf30a2_power_down(void)
{
#ifdef USING_PMIC_GPIO
    /* Set PWDN as high */
    aic_gpio_pmic_output(camera_sensor_bf30a2_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_HIGH);
    aic_udelay(100);

    /* set PWDN at low */
    aic_gpio_pmic_output(camera_sensor_bf30a2_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_LOW);
#else
    /* Set PWDN as high */
    aic_gpio_output(camera_sensor_bf30a2_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_HIGH);
    aic_udelay(100);

    /* set PWDN at low */
    aic_gpio_output(camera_sensor_bf30a2_cfg.gpio_pwdn, GPIO_OUTPUT_LVL_LOW);
#endif

#if defined(CONFIG_PMIC)
    /* close VDD3A 2.8V */
    pmu_disable(camera_sensor_bf30a2_cfg.vddcama2v8);
    aic_udelay(100);

    /* close VDDIO */
    pmu_disable(camera_sensor_bf30a2_cfg.vddcamio1v8);
#endif

    /* Stop MCLK */
#ifdef CLK_CSI_24M_EN
    clk_pll_disable(CLK_CSI_24M_EN);
#endif
#ifdef CLK_CSI_24M_SRC_PLL_EN
    clk_pll_disable(CLK_CSI_24M_SRC_PLL_EN);
#endif

#ifdef USING_PMIC_GPIO
    aic_gpio_pmic_free(camera_sensor_bf30a2_cfg.gpio_pwdn);
    if (camera_sensor_bf30a2_cfg.gpio_rst != camera_sensor_bf30a2_cfg.gpio_pwdn)
        aic_gpio_pmic_free(camera_sensor_bf30a2_cfg.gpio_rst);
#else
    aic_gpio_free(camera_sensor_bf30a2_cfg.gpio_pwdn);
    if (camera_sensor_bf30a2_cfg.gpio_rst != camera_sensor_bf30a2_cfg.gpio_pwdn)
        aic_gpio_free(camera_sensor_bf30a2_cfg.gpio_rst);
#endif
}

#define PRODUCT_ID_MSB             0xfc
#define PRODUCT_ID_MSB_VAL         0x3b
#define PRODUCT_ID_LSB             0xfd
#define PRODUCT_ID_LSB_VAL         0x02

int bf30a2_check_id(void)
{
    u8 val;
    int cnt;

    for (cnt = 0; cnt < 3; cnt++) {
        camera_sensor_reg_read(&bf30a2_client, PRODUCT_ID_MSB, &val);
        if (val == PRODUCT_ID_MSB_VAL) {
            camera_sensor_reg_read(&bf30a2_client, PRODUCT_ID_LSB, &val);
            if (val == PRODUCT_ID_LSB_VAL) {
                aicx_error(0x18c70f, "", "sensor bf30a2 id read success!\n");
                return AIC_SUCCESS;
            }
        }
    }

    aicx_error(0x156479, "", "sensor bf30a2 id read failed!\n");
    return AIC_ENODEV;
}

int bf30a2_set_output_size(u32 width, u32 height)
{
    return AIC_SUCCESS;
}

int bf30a2_initial_setting(camera_data_fmt_e data_fmt, u32 width, u32 height)
{
    int i, ret, initial_array_size;
    struct sensor_reg const *initial_array;

    if ((data_fmt != YUV422) && (data_fmt != Y) && (data_fmt != RAW8)) {
        aicx_error(0x11f38d, "", "bf30a2 does not support this format!\n");
        return AIC_EINVAL;
    }

    /* default init as YUV422 */
    initial_array = bf30a2_init_z_sdr_1line_nocrc_default;
    initial_array_size =
        sizeof(bf30a2_init_z_sdr_1line_nocrc_default) / sizeof(struct sensor_reg);
    for (i = 0; i < initial_array_size; i++) {
        ret = camera_sensor_reg_write(&bf30a2_client, (u8 *)&initial_array[i]);
        if (ret != AIC_SUCCESS) {
            aicx_error(0x056787, "", "bf30a2 initial setting failed!\n");
            return ret;
        }
    }

    ret = bf30a2_set_output_size(width, height);
    if (ret < 0)
        return ret;

    aicx_info(0x1b3455, "", "sensor bf30a2 init success!\n");

    return AIC_SUCCESS;
}

int bf30a2_open(void)
{
    camera_sensor_sample_mode_e cs_sample_mode;
    camera_sensor_data_line_num_e cs_line_num;
    struct stream_info *stream_info_ptr;
    int ret;

    i2c_open(&bf30a2_client);

    stream_info_ptr = camera_get_stream_info(CAM_STREAM_PREVIEW_INFO);

#ifdef CAMERA_SENSOR_OUTPUT_Y
    stream_info_ptr->fmt = Y;
    stream_info_ptr->bytes_per_pixel = 1;
#else
    stream_info_ptr->fmt = YUV422;
    stream_info_ptr->bytes_per_pixel = 2;
#endif
    stream_info_ptr->plane_num = YUV_ONE_PLANE;

    stream_info_ptr->src_width = CAMERA_SENSOR_OUTPUT_WIDTH;
    stream_info_ptr->src_height = CAMERA_SENSOR_OUTPUT_HEIGHT;
    stream_info_ptr->need_binning = AIC_FALSE;
    stream_info_ptr->rot_mode = ROT_MODE_NONE;
    stream_info_ptr->src_img_size = stream_info_ptr->src_width * stream_info_ptr->src_height *
                                    stream_info_ptr->bytes_per_pixel;

    ret = bf30a2_power_on();
    if (ret != AIC_SUCCESS) {
        aicx_error(0x13644e, "", "sensor bf30a2 power on failed!\n");
        return ret;
    }

    ret = bf30a2_initial_setting(stream_info_ptr->fmt, stream_info_ptr->src_width,
                                 stream_info_ptr->src_height);
    if (ret != AIC_SUCCESS)
        goto power_down_bf30a2;

    stream_info_ptr->fb_num = camera_fb_alloc(stream_info_ptr->src_img_size);

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

    csi_endian_mode_config(3);

    aicx_info(0x0b495e, "", "sensor bf30a2 open success!\n");
    return AIC_SUCCESS;

power_down_bf30a2:
    bf30a2_power_down();
    return ret;
}

void bf30a2_close(void)
{
   camera_sensor_common_disable();

   bf30a2_power_down();

   aicx_info(0x0d1dbc, "", "sensor bf30a2 close success!\n");
}

int bf30a2_set_clock(u32 rate)
{
    return AIC_SUCCESS;
}

#if 0
int bf30a2_set_mirror(u32 level)
{
    /* ToDo */
}

int bf30a2_set_filp(u32 level)
{
    /* ToDo */
}

void bf30a2_writeshutter(u32 Shutter)
{
}


void bf30a2_set_flash_enable(uint32 enable)
{
    /* ToDo */
}

void bf30a2_delayms (u32 ms)
{
    /* ToDo */
}

int bf30a2_set_contrast(u32 level)
{
    /* ToDo */
}

int bf30a2_set_image_effect(int effect_type)
{
    /* ToDo */
}

int bf30a2_set_fps(u32 fps)
{
    /* ToDo */
}
#endif

int bf30a2_set_output_format(u8 fmt)
{
    return AIC_SUCCESS;
}

camera_data_fmt_e bf30a2_get_output_format(void)
{
#ifdef CAMERA_SENSOR_OUTPUT_Y
    return Y;
#else
    return YUV422;
#endif
}

struct camera_spi_sensor_ops cs_bf30a2_ops = {
    .cs_open = &bf30a2_open,
    .cs_close = &bf30a2_close,
    .cs_start_xfer = &csi_start,
    .cs_stop_xfer = &csi_stop,
    .cs_set_clock = &bf30a2_set_clock,
    .cs_set_output_size = &bf30a2_set_output_size,
    .cs_set_output_format = &bf30a2_set_output_format,
    .cs_get_output_format = &bf30a2_get_output_format,
};

int sensor_bf30a2_init(void)
{
    int ret;

#if defined(CONFIG_PMIC)
    camera_sensor_bf30a2_cfg.vddcamio1v8 = pmu_get(CAMERA_SENSOR_IO_USER_NAME, CAMERA_SENSOR_IO_UNIT_NAME);
    if (!camera_sensor_bf30a2_cfg.vddcamio1v8) {
        aicx_warn(0x11bcdc, "", "get vddcamio1v8 failed!\n");
        ret = AIC_ENXIO;
        goto ERR_RETURN;
    }

    camera_sensor_bf30a2_cfg.vddcama2v8 = pmu_get(CAMERA_SENSOR_AVDD_USER_NAME, CAMERA_SENSOR_AVDD_UNIT_NAME);
    if (!camera_sensor_bf30a2_cfg.vddcama2v8) {
        aicx_warn(0x16aeb5, "", "get vddcama2v8 failed!\n");
        pmu_put(camera_sensor_bf30a2_cfg.vddcamio1v8);
        ret = AIC_ENXIO;
        goto ERR_RETURN;
    }
#endif

    i2c_open(&bf30a2_client);

    ret = bf30a2_power_on();
    if (ret != AIC_SUCCESS) {
        aicx_warn(0x13644e, "", "sensor bf30a2 power on failed!\n");
        goto ERR_PMU_PUT;
    }

    ret = bf30a2_check_id();
    if (ret != AIC_SUCCESS) {
        bf30a2_power_down();
        i2c_close(&bf30a2_client);
        goto ERR_PMU_PUT;
    }

    bf30a2_power_down();
    i2c_close(&bf30a2_client);

    camera_sensor_bf30a2_cfg.cs_sensor_ops = &cs_bf30a2_ops;
    camera_sensor_register(CAMERA_FRONT, &camera_sensor_bf30a2_cfg);
    aicx_info(0x1bba0d, "", "camera sensor bf30a2 register success!\n");

    register_irq(camera_sensor_bf30a2_cfg.irq, csi_irq_handler, NULL, NULL, NULL);

    return AIC_SUCCESS;

ERR_PMU_PUT:
#if defined(CONFIG_PMIC)
    pmu_put(camera_sensor_bf30a2_cfg.vddcamio1v8);
    pmu_put(camera_sensor_bf30a2_cfg.vddcama2v8);

ERR_RETURN:
    aicx_error(0x1aa803, "", "camera sensor bf30a2 register failed!\n");
#endif

    return ret;
}

AIC_NORMAL_COMP_L1_INIT_EXPORT(sensor_bf30a2_init);

