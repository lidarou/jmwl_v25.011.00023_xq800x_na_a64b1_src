/*****************************************************************************
 * Copyright (C) 2022 THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the MIT License (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://opensource.org/licenses/MIT
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#ifndef QCLOUD_IOT_EXPORT_DEVINFO_H_
#define QCLOUD_IOT_EXPORT_DEVINFO_H_

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Set device info to NVS(flash/files)
 *
 * @param pdevInfo reference to device info
 * @return         QCLOUD_RET_SUCCESS for success, or err code for failure
 */
int IOT_Set_DevInfo(void *pdevInfo);

/**
 * @brief Get device info from NVS(flash/files)
 *
 * @param pdevInfo reference to device info
 * @return         QCLOUD_RET_SUCCESS for success, or err code for failure
 */
int IOT_Get_DevInfo(void *pdevInfo);

/**
 * @brief Get device info from a JSON file
 *
 * @param file_name JSON file path
 * @param pdevInfo reference to device info
 * @return         QCLOUD_RET_SUCCESS for success, or err code for failure
 */
int IOT_Get_DevInfo_From_File(const char *file_name, void *dev_info);

/**
 * @brief Set the name of file which contain device info
 *
 * @param file_name the name of file which contain device info
 * @return         QCLOUD_RET_SUCCESS for success, or err code for failure
 */
int IOT_Set_DevInfo_To_File(const char *file_name);

#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_DEVINFO_H_ */
