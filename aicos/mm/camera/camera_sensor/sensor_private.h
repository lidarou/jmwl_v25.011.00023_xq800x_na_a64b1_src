/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __SENSOR_PRIVATE_H__
#define __SENSOR_PRIVATE_H__

#include <aic_type.h>
#include <aic_osa.h>
#include <i2c.h>

#include <camera_sensor.h>

int camera_sensor_reg_read(struct i2c_client *sensor_i2c_client, u8 addr, u8 *data);
int camera_sensor_reg_write(struct i2c_client *sensor_i2c_client, u8 *buf);
int camera_sensor_regs_write(struct i2c_client *sensor_i2c_client, u8 *buf, int num);

void camera_sensor_output_mode_config(camera_sensor_output_mode_e cs_output_mode);
void camera_sensor_common_config(camera_sensor_sample_mode_e cs_sample_mode,
                                 camera_sensor_data_line_num_e line_num,
                                 yuv_plane_num_e plane_num,
                                 int src_img_size);
void camera_sensor_common_disable(void);

#endif

