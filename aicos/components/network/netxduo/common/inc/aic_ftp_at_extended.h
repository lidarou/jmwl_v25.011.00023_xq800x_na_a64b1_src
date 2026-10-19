/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_FTP_AT_EXTENDED_H__
#define __AIC_FTP_AT_EXTENDED_H__

#include "nxd_ftp_client.h"

#define MAX_PACKET_DATA_LEN 1500

/* Define global functions. */

u32 ftp_upload_by_com(u32 handle, u8 *file_name, u8 *file_path, u8 put_option,
                      ulong *bytes_upload, ulong wait_option);

u32 ftp_download_to_com(u32 handle, u8 *file_name, u8 *file_path, u8 urc_port,
                        ulong offset, ulong * total_len, ulong wait_option);

#endif