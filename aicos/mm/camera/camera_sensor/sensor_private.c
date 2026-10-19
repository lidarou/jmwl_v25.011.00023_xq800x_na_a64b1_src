/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x083fb2
#define LOG_MODULE    AMOD_CAMERA

#include <aic_errno.h>
#include <csi.h>
#include <sensor_private.h>

int camera_sensor_reg_read(struct i2c_client *sensor_i2c_client, u8 addr, u8 *data)
{
    int ret;

    ret = i2c_master_send_then_recv(sensor_i2c_client, (u8 *)&addr, 1, data, 1);
    if (ret != 1)
        return ret;

    return AIC_SUCCESS;
}

int camera_sensor_reg_write(struct i2c_client *sensor_i2c_client, u8 *buf)
{
    int ret;

    ret = i2c_master_send(sensor_i2c_client, buf, 2);
    if (ret != 2)
        return ret;

    return AIC_SUCCESS;
}

int camera_sensor_regs_write(struct i2c_client *sensor_i2c_client, u8 *buf, int num)
{
    int ret;

    ret = i2c_master_send(sensor_i2c_client, buf, num);
    if (ret != num)
        return ret;

    return AIC_SUCCESS;
}

void camera_sensor_output_mode_config(camera_sensor_output_mode_e cs_output_mode)
{
    if (cs_output_mode == CAMERA_SENSOR_OUTPUT_MODE_Z) {
        csi_set_sync_word_head(0xff0000);
        csi_set_sync_word_id(0xab, 0x80, 0x9d, 0xb6);
    } else {
        csi_set_sync_word_id(0x01, 0x02, 0x40, 0x00);
        csi_set_sync_word_head(0xffffff);
        csi_mformat_enable(1);
        /*csi_crc_enable(1);*/
        csi_lsb_mode(1);
    }
}

void camera_sensor_common_config(camera_sensor_sample_mode_e cs_sample_mode,
                                 camera_sensor_data_line_num_e line_num,
                                 yuv_plane_num_e plane_num,
                                 int src_img_size)
{
    /* set sensor sample mode */
    csi_ddr_mode_config(cs_sample_mode);

    /* set sensor data line number */
    csi_wire_num_config(line_num);

    /* lsb first */
    csi_wire_swap(1);

#ifdef CAM_CSI_DEBUG
    /* when csi_enable = 1, it will capture one frame */
    csi_cap_mode_enable(1);

    /* 0 only dump valid image to memory, 1 will dump all data execept frame start header*/
    csi_debug_enable(1);
#endif

    csi_output_plane_config(plane_num);

    csi_set_addr_limit(src_img_size);

    csi_int_mask_eol();
    csi_int_mask_sol();
    csi_int_mask_sof();

    csi_rxfifo_thrd_config(0x10);

#ifdef CLK_MCU_AHB_CSI_EN
    clk_enable(CLK_MCU_AHB_CSI_EN);
#endif

#ifdef CLK_MCU_APB_CSI_EN
    clk_enable(CLK_MCU_APB_CSI_EN);
#endif
}

void camera_sensor_common_disable(void)
{
    csi_stop();
#ifdef CLK_MCU_AHB_CSI_EN
    clk_disable(CLK_MCU_AHB_CSI_EN);
#endif

#ifdef CLK_MCU_APB_CSI_EN
    clk_disable(CLK_MCU_APB_CSI_EN);
#endif
}
