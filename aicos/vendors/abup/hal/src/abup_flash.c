
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0fff40
#define LOG_MODULE    AMOD_VENDOR
#include "abup_typedef.h"
#include "abup_define.h"
#include "abup_debug.h"
#include "abup_stdlib.h"


#include "abup_flash.h"
#include <fmd.h>
#include <aic_errno.h>


static ABUP_FLASH_PARTATIONT_T s_abup_flash_info={0};

#ifdef CONFIG_FOTA_SUPPORT_ABUP
#ifndef WIN32
abup_extern abup_int32 abup_write_buffer(abup_uint8* data_ptr, abup_uint32 len);
abup_extern abup_uint32 abup_erase_disk(void);
abup_extern void abup_delete_header(void);
abup_extern abup_int32  abup_read_buffer(abup_uint32 addr, abup_uint8* data_ptr, abup_uint32 len);
#else
abup_int32 abup_write_buffer(abup_uint8* data_ptr, abup_uint32 len){return 0;}
abup_uint32 abup_erase_disk(void){return     0;}
void abup_delete_header(void){}
abup_int32  abup_read_buffer(abup_uint32 addr, abup_uint8* data_ptr, abup_uint32 len){return 0;}
#endif
#endif

void abup_get_flash_info(void)
{
    ABUP_FLASH_PARTATIONT_T *flash_info = &s_abup_flash_info;
    abup_uint32 tmp_handle = 0;
    abup_uint32 part_size = 0;
    abup_int32 ret = 0;

    if(flash_info)
    {
        ret = fmd_open(ABUP_DELTA_PART_NAME, &tmp_handle);
        if(ret != AIC_SUCCESS)
        {
            abup_DebugPrint("abup_get_flash_info: fmd_open error!ret:%d", ret);
            return;
        }
        ret = fmd_get_partition_size(tmp_handle, &part_size);
        if(ret != AIC_SUCCESS)
        {
            abup_DebugPrint("abup_get_flash_info: fmd_get_partition_size error!ret:%d", ret);
            fmd_close(tmp_handle);
            return;
        }
        abup_DebugPrint("abup_get_flash_info part_size:%d", part_size);
        flash_info->sect_size = ABUP_BLOCK_SIZE;
        flash_info->Spload_Addr = 0;
        flash_info->SploadSize = part_size;
        fmd_close(tmp_handle);
    }
}

abup_uint32 abup_flash_block_size(void)
{
    return s_abup_flash_info.sect_size; //ABUP_BLOCK_SIZE;
}

abup_uint32 abup_flash_update_info_size(void)
{
    return s_abup_flash_info.sect_size;
}

abup_uint32 abup_flash_update_info_base(void)
{
    return s_abup_flash_info.Spload_Addr + s_abup_flash_info.SploadSize - abup_flash_update_info_size();
}

abup_uint32 abup_flash_backup_size(void)
{
    return s_abup_flash_info.sect_size;
}

abup_uint32 abup_flash_backup_base(void)
{
    return s_abup_flash_info.Spload_Addr + s_abup_flash_info.SploadSize - s_abup_flash_info.sect_size*2;
}

abup_uint32 abup_flash_delta_base(void)
{
    return s_abup_flash_info.Spload_Addr;
}

abup_uint32 abup_flash_delta_size(void)
{
    return s_abup_flash_info.SploadSize - abup_flash_update_info_size() * 2;
}

abup_uint32 abupGetFlashDiskSize(void)
{
    return abup_flash_delta_size();
}

abup_uint32 abup_get_rom_len(void)
{
    return 0;//(abup_uint32)s_abup_flash_info.PSSize; ////NOR_BOOTING_NOR_FS_BASE_ADDRESS;
}

void abup_flash_get_partation(void)
{
    //abup_bl_get_flash_info(&s_abup_flash_info);
    abup_DebugPrint("abup: \r\n-----flash partation info:----->");
    abup_DebugPrint("abup Spload_Addr:0x%x, SploadSize:0x%x", s_abup_flash_info.Spload_Addr, abup_flash_delta_size());
    abup_DebugPrint("abup Delta Base:0x%x, Size:0x%x", abup_flash_delta_base(), abup_flash_delta_size());
    abup_DebugPrint("abup Update info Base:0x%x, Size:0x%x", abup_flash_update_info_base(), abup_flash_update_info_size());

    abup_DebugPrint("\r\n-----abup flash partation info <-----\r\n");
}


ABUP_BOOL AbupUseFlash(void)
{
#ifdef ABUP_FOTA_SUPPORT_TCARD
    return ABUP_FALSE;
#else
    return ABUP_TRUE;
#endif
}

abup_uint32 AbupGetBlockSize(void)
{
    return s_abup_flash_info.sect_size; //ABUP_BLOCK_SIZE;
}

abup_int32 abup_write_update_info_flash(abup_uint32 start,abup_uint8* data_ptr, abup_uint32 len)
{
    abup_int32 ret;
    abup_uint32 base=abup_flash_update_info_base();
    abup_uint32 addr = 0;
    abup_uint32 tmp_handle= 0;

    if((!data_ptr) || (len<=0))
        return -1;

#ifdef CONFIG_FOTA_SUPPORT_ABUP
    addr = base + start;
    abup_DebugPrint("abup_write_update_info_flash: addr:0x%x, , len:%d ********",addr, len);

    ret = fmd_open(ABUP_DELTA_PART_NAME, &tmp_handle);
    if(ret != AIC_SUCCESS)
    {
        abup_DebugPrint("abup: fmd_open error!ret:%d", ret);
        return -1;
    }
    ret = fmd_write(tmp_handle, addr, len,(abup_uint8 *)data_ptr);
    fmd_close(tmp_handle);

    if(ret == AIC_SUCCESS)
    {
        return len; //write success
    }
    else
    {
        abup_DebugPrint("abup_write_update_info_flash: write error!");
        return -1;
    }
#else
     return 0;
#endif
}

abup_int32 abup_read_update_info_flash(abup_uint32 addr,abup_uint8* data_ptr, abup_uint32 len)
{
     abup_int32 ret = 0;

     abup_uint32 base = abup_flash_update_info_base();
     abup_uint32 tmp_handle= 0;

    if((!data_ptr) || (len<=0) || (base<=0))
    {
        abup_DebugPrint("abup_read_update_info_flash: error!base:0x%x,len:%d,buf:0x%x",base, len, data_ptr);
        return 1;
    }

    if((addr+len) > abup_flash_update_info_size())
    {
        abup_DebugPrint("abup_read_update_info_flash: error!overlap offset+len:0x%x,delta maxsize:%d", (addr+len), abup_flash_update_info_size());
        return 2;
    }

#ifdef CONFIG_FOTA_SUPPORT_ABUP

    ret = fmd_open(ABUP_DELTA_PART_NAME, &tmp_handle);
    if(ret != AIC_SUCCESS)
    {
        abup_DebugPrint("abup: fmd_open error!ret:%d", ret);
        return 0;
    }
    ret = fmd_read(tmp_handle, base+addr, len, data_ptr);
    fmd_close(tmp_handle);

    if(ret >= 0)
        return 0; //read success
    else
    {
        abup_DebugPrint("abup_read_update_info_flash: error!base:0x%x,offset:0x%x,len:%d,ret:%d",base, addr, len,ret);
        return 3;
    }
#else
    return 0;
#endif
}

abup_int32 abup_write_backup_flash(abup_uint32 start,abup_uint8* data_ptr, abup_uint32 len)
{
    abup_int32 ret=-1;
    abup_uint32 base=abup_flash_backup_base();
    //abup_uint32 maxsize=abup_flash_backup_size() ;
    abup_uint32 addr=0;
    abup_uint32 tmp_handle= 0;

    if((!data_ptr) || (len<=0))
        return 0;

#ifdef CONFIG_FOTA_SUPPORT_ABUP
    addr = base + start;

    abup_DebugPrint("abup_write_backup_flash: addr:0x%x, len:%d ********",addr, len);

    ret = fmd_open(ABUP_DELTA_PART_NAME, &tmp_handle);
    if(ret != AIC_SUCCESS)
    {
        abup_DebugPrint("abup: fmd_open error!ret:%d", ret);
        return 0;
    }
    ret = fmd_write(tmp_handle, addr, len,(abup_uint8 *)data_ptr);
    fmd_close(tmp_handle);

    if(ret == AIC_SUCCESS)
    {
        return 0; //write success
    }
    else
    {
        abup_DebugPrint("abup_write_backup_flash: write error!");
        return 1;
    }
#else
     return 0;
#endif
}

abup_int32 abup_read_backup_flash(abup_uint32 addr,abup_uint8* data_ptr, abup_uint32 len)
{
     abup_int32 ret = -1;

     abup_uint32 base = abup_flash_backup_base();
     abup_uint32 tmp_handle= 0;

    if((!data_ptr) || (len<=0) || (base<=0))
    {
        abup_DebugPrint("abup_read_backup_flash: error!base:0x%x,len:%d,buf:0x%x",base, len, data_ptr);
        return 1;
    }

    if((addr+len) > abup_flash_backup_size())
    {
        abup_DebugPrint("abup_read_backup_flash: error!overlap offset+len:0x%x,delta maxsize:%d", (addr+len), abup_flash_backup_size());
        return 2;
    }

#ifdef CONFIG_FOTA_SUPPORT_ABUP
    ret = fmd_open(ABUP_DELTA_PART_NAME, &tmp_handle);
    if(ret != AIC_SUCCESS)
    {
        abup_DebugPrint("abup: fmd_open error!ret:%d", ret);
        return 0;
    }
    ret = fmd_read(tmp_handle, base+addr, len, data_ptr);
    fmd_close(tmp_handle);

    if(ret >= 0)
        return 0; //read success
    else
    {
        abup_DebugPrint("abup_read_update_info_flash: error!base:0x%x,offset:0x%x,len:%d,ret:%d",base, addr, len,ret);
        return 3;
    }
#else
    return 0;
#endif
}


abup_int32 abup_write_flash(abup_uint32 start,abup_uint8* data_ptr, abup_uint32 len)
{
    abup_int32 ret= -1;

    abup_uint32 base=abup_flash_delta_base();
    abup_uint32 maxsize=abup_flash_delta_size() ;
    abup_uint32 addr=0, tolen=len;
    abup_uint32 tmp_handle= 0;

    if((!data_ptr) || (len<=0))
        return 0;

#ifdef CONFIG_FOTA_SUPPORT_ABUP
    addr = base + start;
    if((start+len) > maxsize)
        tolen = maxsize - start;

    abup_DebugPrint("abup_write_flash: addr:0x%x, tolen:%d, len:%d",addr, tolen, len);
    if(tolen > 0)
    {
        ret = fmd_open(ABUP_DELTA_PART_NAME, &tmp_handle);
        if(ret != AIC_SUCCESS)
        {
            abup_DebugPrint("abup: fmd_open error!ret:%d", ret);
            return 0;
        }
        ret = fmd_write(tmp_handle, addr, len,(abup_uint8 *)data_ptr);
        fmd_close(tmp_handle);

        if(ret == AIC_SUCCESS)
        {
            //return len;
            return 0; //write success
        }
        else
        {
            abup_DebugPrint("abup_write_flash: write error!");
            return 1;
        }
    }
    else
    {
        abup_DebugPrint("abup_write_flash: no data write error!");
        return 2;
    }
#else
     return 0;
#endif
}

abup_int32 abup_read_flash(abup_uint32 addr,abup_uint8* data_ptr, abup_uint32 len)
{
    ABUP_BOOL ret=ABUP_FALSE;
    abup_uint32 base = abup_flash_delta_base();
     abup_uint32 tmp_handle= 0;

    abup_DebugPrint("abup_read_flash:------->base:%d",base);
    if((!data_ptr) || (len<=0))
    {
        abup_DebugPrint("abup_read_flash: error!base:0x%x,len:%d,buf:0x%x",base, len, data_ptr);
        return 1;
    }
    if((addr+len) > abup_flash_delta_size())
    {
        abup_DebugPrint("abup_read_flash: error!overlap offset+len:0x%x,delta maxsize:%d", (addr+len), abup_flash_delta_size());
        return 2;
    }
#ifdef CONFIG_FOTA_SUPPORT_ABUP
    ret = fmd_open(ABUP_DELTA_PART_NAME, &tmp_handle);
    if(ret != AIC_SUCCESS)
    {
        abup_DebugPrint("abup_read_flash: fmd_open error!ret:%d", ret);
        return 0;
    }
    ret = fmd_read(tmp_handle, base+addr, len, data_ptr);
    fmd_close(tmp_handle);

    //abup_DebugPrint("abup_read_flash:222------->ret:%d",ret);

    if(ret >= 0){
        return 0; //read success
    }
    else{
        abup_DebugPrint("abup_read_flash: error!base:0x%x,offset:0x%x,len:%d,ret:%d",base, addr, len,ret);
        return 3;
    }
#else
    return 0;
#endif
}

abup_uint32 abup_init_flash(void)
{
    abup_uint32 rom_offset = abup_flash_delta_base();
    abup_uint32 idx = 0,blocknum = 0,blocksize = 0,tmp_handle = 0;
    abup_uint32 ret = 0;

    abup_DebugPrint("abup: rom_offset:0x%x", rom_offset);

#ifdef CONFIG_FOTA_SUPPORT_ABUP
    blocksize = abup_flash_block_size();
    if(blocksize <= 0)
    {
        abup_DebugPrint("abup: blocksize error!blocksize:%d", blocksize);
        return 0;
    }
    ret = fmd_open(ABUP_DELTA_PART_NAME, &tmp_handle);
    if(ret != AIC_SUCCESS)
    {
        abup_DebugPrint("abup: fmd_open error!ret:%d", ret);
        return 0;
    }

    blocknum = abup_flash_delta_size() / blocksize;

    abup_DebugPrint("abup_init_flash: blocknum:%d, blocksize:%d", blocknum, blocksize);
    if(blocksize !=0)
    rom_offset = (rom_offset/blocksize) * blocksize;

    for(idx=0;idx<blocknum;idx++)
    {
        ret = fmd_erase(tmp_handle, rom_offset+blocksize*idx, blocksize);
        if(ret != AIC_SUCCESS)
        {
            abup_DebugPrint("abup_init_flash: erase error!idx/num:%d/%d", idx, blocknum);
            break;
        }
    }
    fmd_close(tmp_handle);

    if((ret==AIC_SUCCESS) && (idx>=blocknum))
    {
        abup_DebugPrint("abup_init_flash: Erase OK]");
        return 0; ////erase success
    }
    else
    {
        abup_DebugPrint("abup_init_flash: Erase fail!");
        return 1;
    }
#else
   return 0;
#endif
}

void abup_erase_delata(void)
{
    abup_uint32 base = abup_flash_delta_base();
    abup_int32 ret = 0;
    abup_uint32 tmp_handle = 0;
    abup_uint32 blocksize = abup_flash_block_size();

#ifdef CONFIG_FOTA_SUPPORT_ABUP
    ret = fmd_open(ABUP_DELTA_PART_NAME, &tmp_handle);
    if(ret != AIC_SUCCESS)
    {
        abup_DebugPrint("abup_erase_delata: fmd_open error!ret:%d", ret);
        return;
    }

    if(blocksize !=0)
    base = (base/blocksize) * blocksize;

    ret = fmd_erase(tmp_handle, base, blocksize);
    fmd_close(tmp_handle);

    if(ret != AIC_SUCCESS)
        abup_DebugPrint("abup_erase_delata: erase error!offset:0x%x", base);
    else
        abup_DebugPrint("abup_erase_delata: ERASE HDR PASS[0x%x]", base);
#else
#endif
}

void abup_erase_update_info(void)
{
    abup_uint32 base = abup_flash_update_info_base();
    abup_int32 ret = 0;
    abup_uint32 tmp_handle = 0;
    abup_uint32 blocksize = abup_flash_block_size();
#ifdef CONFIG_FOTA_SUPPORT_ABUP
    ret = fmd_open(ABUP_DELTA_PART_NAME, &tmp_handle);
    if(ret != AIC_SUCCESS)
    {
        abup_DebugPrint("abup_erase_update_info: fmd_open error!ret:%d", ret);
        return;
    }

    if(blocksize !=0)
    base = (base/blocksize) * blocksize;

    ret = fmd_erase(tmp_handle, base, blocksize);
    fmd_close(tmp_handle);

    if(ret != AIC_SUCCESS)
    abup_DebugPrint("abup_erase_update_info: erase error!offset:0x%x,ret;%d", base, ret);
    else
    abup_DebugPrint("abup_erase_update_info: ERASE HDR PASS[0x%x]", base);
#endif
}

void abup_erase_backup(void)
{
    abup_uint32 base = abup_flash_backup_base();
    abup_int32 ret = 0;
    abup_uint32 tmp_handle = 0;
    abup_uint32 blocksize = abup_flash_block_size();

#ifdef CONFIG_FOTA_SUPPORT_ABUP
    ret = fmd_open(ABUP_DELTA_PART_NAME, &tmp_handle);
    if(ret != AIC_SUCCESS)
    {
        abup_DebugPrint("abup_erase_backup: fmd_open error!ret:%d", ret);
        return;
    }
    if(blocksize !=0)
    base = (base/blocksize) * blocksize;

    ret = fmd_erase(tmp_handle, base, blocksize);
    fmd_close(tmp_handle);

    if(ret != AIC_SUCCESS)
    abup_DebugPrint("abup_erase_backup: erase error!offset:0x%x", base);
    else
    abup_DebugPrint("abup_erase_backup: ERASE HDR PASS[0x%x]", base);
#endif
}

abup_uint32 abup_addr_to_block(abup_uint32 addr)
{
    abup_uint32 blocksize=0;
    blocksize = abup_flash_block_size();
#ifdef CONFIG_FOTA_SUPPORT_ABUP
    return     (addr / blocksize);
#endif
}

void abup_init_block(abup_uint32 block_idx)
{
    abup_uint32 rom_offset = abup_flash_delta_base();
    abup_uint32 blocksize=0;
    blocksize = abup_flash_block_size();
    abup_uint32 tmp_handle = 0;
    abup_uint32 ret = 0;
#ifdef CONFIG_FOTA_SUPPORT_ABUP
ret = fmd_open(ABUP_DELTA_PART_NAME, &tmp_handle);
if(ret != AIC_SUCCESS)
{
    abup_DebugPrint("abup_init_block: fmd_open error!ret:%d", ret);
    return;
}
if(blocksize !=0)
rom_offset = (rom_offset/blocksize) * blocksize;

ret = fmd_erase(tmp_handle, rom_offset+blocksize*block_idx, blocksize);
fmd_close(tmp_handle);

#endif
}

ABUP_ALIGN(8) abup_uint8 abup_hal_flash_tmp[ABUP_BLOCK_SIZE];
abup_update_struct* abup_hal_update = NULL;

abup_update_struct* abup_hal_get_update(void)
{
    return (abup_update_struct*)abup_hal_update;
}
abup_update_struct* abup_hal_init_update(void)
{
    if(abup_hal_update == NULL)
    {
        abup_uint32 abup_hal_update_size = sizeof(abup_hal_flash_tmp);
#if defined(ABUP_UB)
        abup_uint8* p = NULL;
        abup_uint8* p1 = NULL;
        abup_uint32 crc;
#endif
        abup_hal_update = (abup_update_struct*)abup_hal_flash_tmp;

        abup_memset(abup_hal_update,0,(abup_uint32)abup_hal_update_size);
#if defined(ABUP_UB)
        p = (abup_uint8*)&abup_hal_update->crc;
        p1 = (abup_uint8*)abup_hal_update;
#endif
        abup_read_update_info_flash(0,(abup_uint8*)abup_hal_update,abup_hal_update_size);

#if defined(ABUP_UB)
        crc = calc_crc((abup_uint8*)abup_hal_update,p - p1);
        if(AbupIsBigEndian())
        {
            crc = AbupSW32(crc);
        }
#endif
        if((abup_hal_update->inited != 0x8)
#if defined(ABUP_UB)
                ||(abup_hal_update->crc != crc)
#endif
          )
        {
            abup_erase_update_info();
#if defined(ABUP_UB)
            abup_memset(abup_hal_update,0,abup_sizeof(abup_update_struct));
#else
            abup_memset(abup_hal_update,0,(abup_uint32)abup_hal_update_size);
#endif
            abup_hal_update->inited = 0x8;
//            abup_hal_update->serverNO = 0x2;//测试服务器
#if defined(ABUP_UB)
            crc = calc_crc((abup_uint8*)abup_hal_update,p - p1);
            if(AbupIsBigEndian())
            {
                abup_hal_update->crc = AbupSW32(crc);
            }
            else
            {
                abup_hal_update->crc = crc;
            }
#endif
            abup_write_update_info_flash(0,(abup_uint8*)abup_hal_update,abup_hal_update_size);
            abup_memset(abup_hal_update,0,(abup_uint32)abup_hal_update_size);
            abup_read_update_info_flash(0,(abup_uint8*)abup_hal_update,abup_hal_update_size);
        }
    }

#if defined(ABUP_BOOTLOADER)&&!defined(__CCRL__)
#if defined(ABUP_UB)||defined(ABUP_UA)
    if((abup_hal_update->ShareFunc[0] != (void*)abup_get_uuid_ext)
            ||(abup_hal_update->ShareFunc[1] != (void*)abup_get_bid)
            ||(abup_hal_update->ShareFunc[2] != (void*)abup_check_key)
      )
    {
        abup_hal_update->ShareFunc[0] = (void*)abup_get_uuid_ext;
        abup_hal_update->ShareFunc[1] = (void*)abup_get_bid;
        abup_hal_update->ShareFunc[2] = (void*)abup_check_key;
        abup_hal_set_update(abup_hal_update);
    }
#endif
#endif
#if defined(ABUP_BOOT0_ENABLE)&&(ABUP_BOOT0_ENABLE == 1)
#else
#if defined(ABUP_BOOTLOADER)&&!defined(__CCRL__)
#if defined(ABUP_UB)||defined(ABUP_UA)
    if(abup_hal_update->bootupdate != 0)
    {
        abup_hal_update->bootupdate = 0;
        abup_hal_set_update(abup_hal_update);
    }
#endif
#endif
#endif
    return (abup_update_struct*)abup_hal_update;
}

abup_update_struct* abup_hal_rst_fota(void)
{
 #if defined(ABUP_FILE)
    abup_uint32 abup_hal_update_size = sizeof(abup_hal_flash_tmp);

    abup_memset((void *)abup_hal_update,0,abup_hal_update_size);
    abup_hal_flash_write(ABUP_FILETYPE_PATCH_INFO,0,(abup_uint8*)abup_hal_update, (abup_uint32)abup_hal_update_size);
#else
    abup_erase_update_info();
#endif
    abup_hal_update = NULL;
    return abup_hal_init_update();
}

void abup_rst_key(void)
{
    abup_update_struct* update = abup_hal_get_update();
    if(update)
    {
        abup_DebugPrint("abup_rst_key update->inited:%d",update->inited);
    }
    else
    {
        update = abup_hal_init_update();
    }
    abup_memset(update->abup_code, 0, sizeof(update->abup_code));
    update->update_result = 100;
    abup_hal_set_update(update);

    return;
}

ABUP_BOOL abup_hal_set_update(abup_update_struct *abup_update)
{
    abup_uint32 abup_hal_update_size = sizeof(abup_hal_flash_tmp);
    abup_erase_update_info();
    if(1)
    {
#if defined(ABUP_UB)
        abup_uint8* p = (abup_uint8*)&abup_update->crc;
        abup_uint8* p1 = (abup_uint8*)abup_update;
        abup_update->crc = calc_crc((abup_uint8*)abup_update,p - p1);
        if(AbupIsBigEndian())
        {
            abup_update->crc = AbupSW32(abup_update->crc);
        }
#endif
        if(abup_write_update_info_flash(0,(abup_uint8*)abup_update, (abup_uint32)abup_hal_update_size) == abup_hal_update_size)
        {
            return ABUP_TRUE;
        }
    }
    return ABUP_FALSE;
}
void abup_hal_set_download_result(abup_uint16 result)
{
    //100:suc ,else fail
    abup_update_struct* update = abup_hal_get_update();
    update->update_result = result;
    abup_hal_set_update(update);
    abup_DebugPrint("abup_hal_set_download_result,result:%d",update->update_result);
    /*abup_DebugPrint("update->abup_code:");
    for (abup_uint8 cnt=0; cnt < 16;cnt++)
    {
        abup_DebugPrint("%x", update->abup_code[cnt]);
    }
    abup_DebugPrint("\r\n");*/
}

abup_uint8* abup_hal_get_mid(void)
{
    abup_update_struct* update = abup_hal_get_update();
    if(update)
        return update->mid;
    else
        return NULL;
}

void abup_hal_set_mid(abup_char *value)
{
    abup_uint8 len = 0;
    abup_update_struct* update = abup_hal_get_update();
    if(update)
    {
        if (sizeof(update->mid) < abup_strlen(value))
            len = sizeof(update->mid);
        else
            len = abup_strlen(value);
        abup_DebugPrint("abup_hal_set_mid len:%d", len);

        if (abup_memcmp(update->mid, value, len) != 0)
        {
            abup_memset(update->mid, 0, sizeof(update->mid));
            abup_memcpy(update->mid, value, len);
            abup_hal_set_update(update);
        }
    }
}
abup_uint16 abup_hal_get_update_result(void)
{
    abup_update_struct* update = abup_hal_get_update();
    if(update)
    {
        abup_DebugPrint("update->inited:%d",update->inited);
        return update->update_result;
    }
    else
        return 0;
}


