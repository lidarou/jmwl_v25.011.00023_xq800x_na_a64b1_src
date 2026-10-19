
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x146807
#define LOG_MODULE    AMOD_VENDOR
#include "abup_typedef.h"
#include "abup_debug.h"
#include "abup_flash.h"

#include <stdio.h>
#include <stdarg.h>
#include <fs_api.h>
#include <aic_errno.h>

#ifdef ABUP_FOTA_SUPPORT_TCARD
const abup_uint16      s_abup_disk_name[2] = {'E', 0};

#define ABUP_ROOT_FOLDER		L"E:\\@fota_abup"

#define ABUP_DELTAFILE_PATH  	L"E:\\@fota_abup\\abupfota.delta"
#define ABUP_DELTAFILE_PATH12 	L"E:\\@fota_abup\\abupfota12.delta"
#define ABUP_DELTAFILE_PATH21 	L"E:\\@fota_abup\\abupfota21.delta"

#define ABUPVERSION 			L"E:\\@fota_abup\\abupUpState.txt"
#define ABUPDETAID 			    L"E:\\@fota_abup\\abupDetaId.txt"
#define ABUP_BACKUP_PATH  		L"E:\\@fota_abup\\backup.txt"
#define ABUP_LOGIN_PATH 		L"E:\\@fota_abup\\login.txt"

#else
const abup_uint16      s_abup_disk_name[2] = {'C', 0};

#define ABUP_ROOT_FOLDER		"C:/fota_abup"

#define ABUP_DELTAFILE_PATH  	"C:/fota_abup/abupfota.delta"
#define ABUP_DELTAFILE_PATH12 	"C:/fota_abup/abupfota12.delta"
#define ABUP_DELTAFILE_PATH21 	"C:/fota_abup/abupfota21.delta"

#define ABUPVERSION 			"C:/fota_abup/abupUpState.txt"
#define ABUPDETAID 				"C:/fota_abup/abupDetaId.txt"
#define ABUP_BACKUP_PATH  		"C:/fota_abup/backup.txt"
#define ABUP_LOGIN_PATH 		"C:/fota_abup/login.txt"

#endif

ABUP_BOOL abup_get_fota_drv_size(abup_uint64* total_size, abup_uint64* free_size)
{
    //abup_uint32 high_word = 0;
    //abup_uint32 low_word = 0;
    //abup_int32 rv = 0;

	*total_size = abup_flash_delta_size();
	*free_size = abup_flash_delta_size();
	return 0;

#if 0//file sys
    rv = SFS_GetDeviceFreeSpace((uint16*)s_abup_disk_name, &high_word, &low_word);

    if (SFS_NO_ERROR != rv)
        return -1;
    *free_size = low_word;

    rv = SFS_GetDeviceUsedSpace((uint16*)s_abup_disk_name, &high_word, &low_word);

    if (SFS_NO_ERROR != rv)
        return -1;

    *total_size = low_word + *free_size;
    return 0;
#endif
}


abup_wchar* abup_get_delta_file_path(void)
{
	return (abup_wchar*)ABUP_DELTAFILE_PATH;
}
abup_wchar* abup_get_delta_file_path12(void)
{
	return (abup_wchar*)ABUP_DELTAFILE_PATH12;
}
abup_wchar* abup_get_delta_file_path21(void)
{
	return (abup_wchar*)ABUP_DELTAFILE_PATH21;
}
abup_wchar* abup_get_backup_file_path(void)
{
	return (abup_wchar*)ABUP_BACKUP_PATH;
}
abup_wchar* abup_get_version_file_path(void)
{
	return (abup_wchar*)ABUPVERSION;
}

abup_wchar* abup_get_login_file_path(void)
{
	return (abup_wchar*)ABUP_LOGIN_PATH;
}

abup_wchar* abup_get_delta_id_path(void)
{
	return (abup_wchar*)ABUPDETAID;
}

abup_wchar* abup_get_abup_root_path(void)
{
	return (abup_wchar*)ABUP_ROOT_FOLDER;
}

#if defined(FOTA_SUPPORT_ABUP_FLASH_DATA)
#define ABUP_FLASH_DATA_ID_VERSION  1
#define ABUP_FLASH_DATA_ID_LOGIN    2
#define ABUP_FLASH_DATA_ID_BLOCK    3
#define ABUP_FLASH_DATA_UNIT 128
static abup_uint8 abup_flash_data[ABUP_FLASH_DATA_UNIT * 3] = {0};
abup_int32 abup_is_need_flash_save(abup_wchar *file_path)
{
	abup_DebugPrint("abup: %s ,line = %d file_path = %s",__FUNCTION__,__LINE__,file_path);

	if(abup_wstrcmp(abup_get_version_file_path(), file_path) == 0)
	{
		abup_DebugPrint("abup: %s ,ABUP_FLASH_DATA_ID_VERSION",__FUNCTION__);
		return ABUP_FLASH_DATA_ID_VERSION;
	}
	else if(abup_wstrcmp(abup_get_login_file_path(), file_path) == 0)
	{
		abup_DebugPrint("abup: %s ,ABUP_FLASH_DATA_ID_LOGIN",__FUNCTION__);
		return ABUP_FLASH_DATA_ID_LOGIN;
	}
	else if(abup_wstrcmp(abup_get_delta_id_path(), file_path) == 0)
	{
		abup_DebugPrint("abup: %s ,ABUP_FLASH_DATA_ID_BLOCK",__FUNCTION__);
		return ABUP_FLASH_DATA_ID_BLOCK;
	}

	return E_ABUP_FAILURE;
}
#endif

ABUP_BOOL abup_fs_pathorfile_exist(const abup_wchar *path, ABUP_BOOL pathorfile)
{
	abup_uint32 nFlag=0;
    struct stat buf;
    int ret;

	abup_DebugPrint("abup_fs_pathorfile_exist: flag:%s",(pathorfile==ABUP_TRUE)?"folder":"file");
	if(pathorfile)
	{
		abup_DebugPrint("abup_fs_pathorfile_exist: a");

        ret = stat((const char *)path, &buf);
        if (ret < 0)
            return ABUP_FALSE;

        if (S_ISDIR(buf.st_mode))
            return ABUP_TRUE;

        return ABUP_FALSE;
	}
	else
	{
		nFlag = FS_O_RDONLY;

		abup_DebugPrint("abup_fs_pathorfile_exist: x");
	#if defined(FOTA_SUPPORT_ABUP_FLASH_DATA)
		handle = abup_is_need_flash_save(path);
		if (handle > 0)
		{
			abup_read_update_info_flash(ABUP_FLASH_DATA_UNIT, abup_flash_data, ABUP_FLASH_DATA_UNIT * 3);
			if (handle == ABUP_FLASH_DATA_ID_VERSION && abup_flash_data[0] == 'a')
			{
				abup_DebugPrint("abup: %s exist flash.%02X%02X%02X%02X%02X%02X%02X%02X", __FUNCTION__,
					abup_flash_data[0], abup_flash_data[1], abup_flash_data[2], abup_flash_data[3],
					abup_flash_data[4], abup_flash_data[5], abup_flash_data[6], abup_flash_data[7]);
				return ABUP_TRUE;
			}
			if (handle == ABUP_FLASH_DATA_ID_LOGIN && abup_flash_data[ABUP_FLASH_DATA_UNIT] != 0xFF && abup_flash_data[ABUP_FLASH_DATA_UNIT] != 0)
			{
				abup_DebugPrint("abup: %s exist flash.", __FUNCTION__);
				return ABUP_TRUE;
			}
			if (handle == ABUP_FLASH_DATA_ID_BLOCK && abup_flash_data[ABUP_FLASH_DATA_UNIT * 2] == 'c')
			{
				abup_DebugPrint("abup: %s exist flash.", __FUNCTION__);
				return ABUP_TRUE;
			}
		}
		abup_DebugPrint("abup: %s no exist flash!", __FUNCTION__);
		return ABUP_FALSE;
	#endif

		abup_DebugPrint("abup_fs_pathorfile_exist: a");

        ret = stat((const char *)path, &buf);
        if (ret < 0)
            return ABUP_FALSE;

        if (S_ISREG(buf.st_mode)) {
            return ABUP_TRUE;
        }

		abup_DebugPrint("abup_fs_pathorfile_exist: z");

        return ABUP_FALSE;
	}
}

abup_int32 Abup_Open_File(abup_wchar *file_path,abup_uint32 flag)
{
	abup_int32 result=-1;
	abup_uint32 nFlag=0;

#if defined(FOTA_SUPPORT_ABUP_FLASH_DATA)
	return abup_is_need_flash_save(file_path);
#endif
	if((ABUP_READONLY & flag) == ABUP_READONLY)
	{
		nFlag = FS_O_RDONLY | FS_O_EXCL;
	}
	else if(((ABUP_READWRITE & flag) == ABUP_READWRITE) || ((ABUP_APPEND&flag) == ABUP_APPEND) ||
			((ABUP_TRUNCATE & flag) == ABUP_TRUNCATE))
	{
		nFlag = FS_O_RDWR;

		if((ABUP_TRUNCATE & flag) == ABUP_TRUNCATE)
			nFlag |= FS_O_TRUNC;
		if((ABUP_CREATE & flag) == ABUP_CREATE)
			nFlag |= FS_O_CREATE;
		if((ABUP_APPEND & flag) == ABUP_APPEND)
			nFlag |= FS_O_APPEND;
	}
	else
	{
		abup_DebugPrint("Abup_Open_File: bad flag, flag = %d", flag);

		return -1;
	}

	if(!abup_fs_pathorfile_exist(abup_get_abup_root_path(), 1))
	{
		abup_DebugPrint("Abup_Open_File: create root folder");
		//AIC_SUCCESS: Success
		result = fs_createdir((const char *)abup_get_abup_root_path());
		if(result != AIC_SUCCESS)
		{
			abup_DebugPrint("Abup_Open_File: create fail!!!");
			return -1;
		}
	}

	abup_DebugPrint("Abup_Open_File: nFlag = %d", nFlag);

	//success: none-zero, fail: zero
	result = fs_open((const char *)file_path,nFlag);

	abup_DebugPrint("Abup_Open_File: result:%d", result);

	return result; //HANDLE

}

abup_int32 Abup_Seek_File(abup_int32 file_handle,abup_uint32 offset,abup_int32 origin)
{
	abup_int32 result=0;
	abup_int32 whence=0;

#if defined(FOTA_SUPPORT_ABUP_FLASH_DATA)
	if (file_handle == ABUP_FLASH_DATA_ID_VERSION || file_handle == ABUP_FLASH_DATA_ID_LOGIN || file_handle == ABUP_FLASH_DATA_ID_BLOCK)
	{
		return E_ABUP_SUCCESS;
	}
	else
	{
		return E_ABUP_FAILURE;
	}
#endif

	switch(origin)
	{
		case ABUP_SEEK_START:
			whence = FS_SEEK_BEGIN;
			break;
		case ABUP_SEEK_END:
			whence = FS_SEEK_END;
			break;
		case ABUP_SEEK_CURRENT:
			whence = FS_SEEK_CUR;
			break;
		default:
			return E_ABUP_FAILURE;
	}
	//AIC_SUCCESS: Success
	result = fs_seek(file_handle,offset,whence);

	abup_DebugPrint("Abup_Seek_File: file_handle=%d,result = %d\r\n",file_handle,result);

	if(result != AIC_SUCCESS)
		return E_ABUP_FAILURE;
	else
		return E_ABUP_SUCCESS;

}

abup_int32 Abup_Remove_File(abup_wchar *file_path)
{
	abup_int32 ret=0;

#if defined(FOTA_SUPPORT_ABUP_FLASH_DATA)
	ret = abup_is_need_flash_save(file_path);
	if (ret == ABUP_FLASH_DATA_ID_VERSION)
	{
		memset(abup_flash_data, 0, ABUP_FLASH_DATA_UNIT);
		abup_erase_update_info();
		abup_write_update_info_flash(ABUP_FLASH_DATA_UNIT, abup_flash_data, ABUP_FLASH_DATA_UNIT * 3);
	}
	else if (ret == ABUP_FLASH_DATA_ID_LOGIN)
	{
		memset(abup_flash_data + ABUP_FLASH_DATA_UNIT, 0, ABUP_FLASH_DATA_UNIT);
		abup_erase_update_info();
		abup_write_update_info_flash(ABUP_FLASH_DATA_UNIT, abup_flash_data, ABUP_FLASH_DATA_UNIT * 3);
	}
	else if (ret == ABUP_FLASH_DATA_ID_BLOCK)
	{
		memset(abup_flash_data + ABUP_FLASH_DATA_UNIT * 2, 0, ABUP_FLASH_DATA_UNIT);
		abup_erase_update_info();
		abup_write_update_info_flash(ABUP_FLASH_DATA_UNIT, abup_flash_data, ABUP_FLASH_DATA_UNIT * 3);
	}
	return E_ABUP_SUCCESS;
#endif
	if(abup_fs_pathorfile_exist((const abup_wchar *)file_path,0) == ABUP_FALSE)
		return E_ABUP_FAILURE;

	//AIC_SUCCESS: Success
	ret = fs_delete((const char *)file_path);

	if(ret == AIC_SUCCESS)
		return E_ABUP_SUCCESS;
	else
		return E_ABUP_FAILURE;

}

abup_int32 Abup_Write_File(abup_int32 file_handle,abup_char *inBuff,abup_uint32 len,abup_uint32 *write_len)
{
	abup_int32 ret = 0;

	if((inBuff==NULL) || (write_len==NULL) || (len==0))
	{
		abup_DebugPrint("Abup_Write_File: ptr NULL or len 0 FAIL!");
		return 0;
	}
#if defined(FOTA_SUPPORT_ABUP_FLASH_DATA)
	abup_DebugPrint("abup: %s ,line = %d file_handle =%d len=%d",__FUNCTION__,__LINE__,file_handle,len);
	if (file_handle == ABUP_FLASH_DATA_ID_VERSION)
	{
		memset(abup_flash_data, 0, ABUP_FLASH_DATA_UNIT);
		memcpy(abup_flash_data, inBuff, len);
		abup_erase_update_info();
		abup_write_update_info_flash(ABUP_FLASH_DATA_UNIT, abup_flash_data, ABUP_FLASH_DATA_UNIT * 3);
		*write_len = len;
	}
	else if (file_handle == ABUP_FLASH_DATA_ID_LOGIN)
	{
		memset(abup_flash_data + ABUP_FLASH_DATA_UNIT, 0, ABUP_FLASH_DATA_UNIT);
		memcpy(abup_flash_data + ABUP_FLASH_DATA_UNIT, inBuff, len);
		abup_erase_update_info();
		abup_write_update_info_flash(ABUP_FLASH_DATA_UNIT, abup_flash_data, ABUP_FLASH_DATA_UNIT * 3);
		*write_len = len;
	}
	else if (file_handle == ABUP_FLASH_DATA_ID_BLOCK)
	{
		memset(abup_flash_data + ABUP_FLASH_DATA_UNIT * 2, 0, ABUP_FLASH_DATA_UNIT);
		memcpy(abup_flash_data + ABUP_FLASH_DATA_UNIT * 2, inBuff, len);
		abup_erase_update_info();
		abup_write_update_info_flash(ABUP_FLASH_DATA_UNIT, abup_flash_data, ABUP_FLASH_DATA_UNIT * 3);
		*write_len = len;
	}
	return E_ABUP_SUCCESS;
#endif

	//SFS_ERROR_NONE: Success
	ret = fs_write(file_handle,inBuff,len);

	abup_DebugPrint("Abup_Write_File ret=%d,len=%d" , ret,len);

	if(ret != len)
	{
		ret = -1;
	}
	else
	{
		*write_len = len;
		ret = fs_flush(file_handle);
		abup_DebugPrint("Abup_Write_File flush ret=%d" , ret);
	}
	return ret;
}

abup_int32 Abup_Read_File(abup_int32 file_handle,abup_char *outBuff,abup_uint32 len,abup_uint32 *write_len)
{
	abup_int32 ret = -1;

	if((outBuff==NULL) || (write_len==NULL))
	{
		abup_DebugPrint("Abup_Read_File: ptr FAIL!");
		return -1;
	}
#if defined(FOTA_SUPPORT_ABUP_FLASH_DATA)
	abup_DebugPrint("abup: %s ,line = %d file_handle =%d len=%d",__FUNCTION__,__LINE__,file_handle,len);
	if (file_handle == ABUP_FLASH_DATA_ID_VERSION)
	{
		abup_read_update_info_flash(ABUP_FLASH_DATA_UNIT, outBuff, len);
		*write_len = len;
	}
	else if (file_handle == ABUP_FLASH_DATA_ID_LOGIN)
	{
		abup_read_update_info_flash(ABUP_FLASH_DATA_UNIT * 2, outBuff, len);
		*write_len = len;
	}
	else if (file_handle == ABUP_FLASH_DATA_ID_BLOCK)
	{
		abup_read_update_info_flash(ABUP_FLASH_DATA_UNIT * 3, outBuff, len);
		*write_len = len;
	}
	return E_ABUP_SUCCESS;
#endif

	//SFS_ERROR_NONE: Success
	ret = fs_read(file_handle,outBuff,len);

	abup_DebugPrint("Abup_Read_File ret=%d" , ret);

	if(ret < 0)
		ret = -1;

	*write_len = len;
	return ret;
}

abup_int32 abup_Get_File_Size(abup_int32 file_handle,abup_uint32 *file_size)
{
	abup_int32 ret = -1;
	struct stat stat_buf;

	if(file_size==NULL)
	{
		abup_DebugPrint("abup_Get_File_Size: size ptr FAIL!");
		return 0;
	}
#if defined(FOTA_SUPPORT_ABUP_FLASH_DATA)
	abup_read_update_info_flash(ABUP_FLASH_DATA_UNIT, abup_flash_data, ABUP_FLASH_DATA_UNIT * 3);
	if (file_handle == ABUP_FLASH_DATA_ID_BLOCK)
	{
		if (abup_flash_data[ABUP_FLASH_DATA_UNIT * 2] != 'c')
		{
			*file_size = 0;
			return -1;
		}
		else
		{
			*file_size = 18;
		}
	}
	else if (file_handle == ABUP_FLASH_DATA_ID_LOGIN)
	{
		if (abup_flash_data[ABUP_FLASH_DATA_UNIT] == 0xFF)
		{
			*file_size = 0;
			return -1;
		}
		else
		{
			*file_size = strlen(abup_flash_data + ABUP_FLASH_DATA_UNIT);
		}
	}
	else
	{
		if (abup_flash_data[0] != 'a')
		{
			*file_size = 0;
			return -1;
		}
		else
		{
			*file_size = strlen(abup_flash_data);
		}
	}
	if (*file_size == 0) return -1;
	return 0;
#endif

	//AIC_SUCCESS: Success
	ret = fstat(file_handle, &stat_buf);
	if(ret != AIC_SUCCESS)
	{
		*file_size = 0;
		ret = -1;
	}
	else{
		*file_size = stat_buf.st_size;
	}

	abup_DebugPrint("abup_Get_File_Size: ret:%d", ret);
	return ret;
}

void Abup_Close_File(abup_int32 file_handle)
{
	abup_int32 ret = -1;
#if defined(FOTA_SUPPORT_ABUP_FLASH_DATA)
	return;
#endif
	ret = fs_close(file_handle);
	abup_DebugPrint("Abup_Close_File: handle:%d, ret:%d", file_handle, ret);

}

ABUP_BOOL abup_init_fs(void)
{
	//add deal if need
	abup_DebugPrint("abup: abup_init_fs");

	return ABUP_TRUE;
}



