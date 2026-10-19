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

#ifndef __DATA_MODEL_H__
#define __DATA_MODEL_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cpluscplus */

#include "iv_dm.h"

int dm_init(void);
int dm_exit(void);
int dm_sync(void);

#ifdef __cplusplus
}
#endif /* __cpluscplus */

#endif /* __DATA_MODEL_H__ */
