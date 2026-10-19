/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x16c5be
#define LOG_MODULE    AMOD_VENDOR

#include <host_apis.h>
#include <log.h>
#include <drv/ssv_drv.h>

#include "cli_cmd.h"
#include "cli_cmd_soc.h"
#if (CLI_ENABLE==1)
static void _hex_dump (ssv_type_u32 read_addr, ssv_type_u32 read_size)
{
    ssv_type_u32     data[64];
    ssv_type_u32     i, j, addr;
    
    // Maximum read size is 64.    
    if (read_size > 64)
        read_size = 64;
    // Read from SoC
    for (i = 0, addr = read_addr; i < read_size; i++, addr+=4)
         data[i] = ssv6xxx_drv_read_reg(addr);
    // Dump data
    LOG_PRINTF("        ");
    for (i = 0; i < 8; i++)
        LOG_PRINTF("       %02x", i*sizeof(ssv_type_u32));

    LOG_PRINTF("\r\n--------");
    for (i = 0; i < 8; i++)
        LOG_PRINTF("+--------");

    for (i = 0, addr = 0; i < read_size; i += 8)
    {
        LOG_PRINTF("\r\n%08X:%08X", read_addr+addr*4, data[addr]);
        
		addr++;
        for (j = 1; j < 8; j++)
        {
            LOG_PRINTF(" %08X", data[addr++]);
        }
    }
    LOG_PRINTF("\r\n");
}

void cmd_read(ssv_type_s32 argc, char *argv[])
{
#if 1
	{
    char if_name[32];
                
    if ((ssv6xxx_drv_get_name(if_name) == FALSE) || (ssv6xxx_strcmp(if_name, "sim") == 0))
    {
        log_printf("Not connected to SSV6200.\n");
        return;
    }
    }
#endif

    {
    ssv_type_u32 read_size = 8;
    ssv_type_u32 read_addr = 0xCD010000;
    
    if (argc < 2)
    {
        //LOG_PRINTF("Usage: r addr [size]\n");
        _hex_dump(read_addr, read_size);
        return;
    }
        
    read_addr = (ssv_type_u32)ssv6xxx_atoi_base(argv[1], 16);
    if (argc > 2)
        read_size = (ssv_type_u32)ssv6xxx_atoi(argv[2]);
    else
        read_size = 1;

    read_addr &= 0xFFFFFFE0;
    read_size = (read_size+7)&0xFFFFFFF8;
    _hex_dump(read_addr, read_size);
    }

} // end of - cmd_read - 

void cmd_regr(ssv_type_s32 argc, char *argv[])
{
    char if_name[32];
    char *p=NULL;                
    ssv_type_u32 read_size = 1;
    ssv_type_u32 read_addr = 0;

    if ((ssv6xxx_drv_get_name(if_name) == FALSE) || (ssv6xxx_strcmp(if_name, "sim") == 0))
    {
        log_printf("Not connected to SSV6200.\n");
        return;
    }

    
    if (argc < 2)
    {
        LOG_PRINTF("Usage: regr [addr],[size]\n");
        return;
    }
       
    p=ssv6xxx_strchr(argv[1],'x');
    if(p==NULL)
    {  
        read_addr = (ssv_type_u32)ssv6xxx_atoi_base(argv[1],16);
    }
    else
    {
        read_addr = (ssv_type_u32)ssv6xxx_atoi(argv[1]);
    }

    if (argc > 2)
    {
        read_size = (ssv_type_u32)ssv6xxx_atoi(argv[2]);
    }

    read_addr &= 0xFFFFFFFC;
    while(read_size)
    {
        LOG_PRINTF("0x%x = 0x%x\r\n",read_addr,ssv6xxx_drv_read_reg(read_addr));
        read_addr+=4;
        read_size--;
    }
    return;
} // end of - cmd_read - 

void cmd_write(ssv_type_s32 argc, char *argv[])
{
    char if_name[32];
    ssv_type_u32  write_addr;
    ssv_type_u32  value, rb_value;
                
    if ((ssv6xxx_drv_get_name(if_name) == FALSE) || (ssv6xxx_strcmp(if_name, "sim") == 0))
    {
        log_printf("Not connected to SSV6200.\r\n");
        return;
    }

    if (argc != 3)
    {
        LOG_PRINTF("Usage: w addr value\r\n");
        return;
    }

    write_addr = (ssv_type_u32)ssv6xxx_atoi_base(argv[1], 16);
    value = (ssv_type_u32)ssv6xxx_atoi_base(argv[2], 16);
    
    if (ssv6xxx_drv_write_reg(write_addr, value) == FALSE)
    {
        LOG_PRINTF("Failed to write to SSV6200.\r\n");
        return;
    }
    rb_value = ssv6xxx_drv_read_reg(write_addr);
    LOG_PRINTF("\r\n    %08X => %08X : %08X\r\n", value, write_addr, rb_value);
} // end of - cmd_write - 

void cmd_regw(ssv_type_s32 argc, char *argv[])
{
    char if_name[32];
    ssv_type_u32  write_addr;
    ssv_type_u32  value, rb_value;
    char *p=NULL;                

    if ((ssv6xxx_drv_get_name(if_name) == FALSE) || (ssv6xxx_strcmp(if_name, "sim") == 0))
    {
        log_printf("Not connected to SSV6200.\r\n");
        return;
    }

    if (argc != 3)
    {
        LOG_PRINTF("Usage: regw [addr],[value]\r\n");
        return;
    }

    
    p=ssv6xxx_strchr(argv[1],'x');
    if(p==NULL)
    {  
        write_addr = (ssv_type_u32)ssv6xxx_atoi_base(argv[1],16);
    }
    else
    {
        write_addr = (ssv_type_u32)ssv6xxx_atoi(argv[1]);
    }
   
    p=ssv6xxx_strchr(argv[2],'x');
    if(p==NULL)
    {  
        value = (ssv_type_u32)ssv6xxx_atoi_base(argv[2],16);
    }
    else
    {
        value = (ssv_type_u32)ssv6xxx_atoi(argv[2]);
    }
    
    if (ssv6xxx_drv_write_reg(write_addr, value) == FALSE)
    {
        LOG_PRINTF("Failed to write to SSV6200.\r\n");
        return;
    }
    rb_value = ssv6xxx_drv_read_reg(write_addr);
    LOG_PRINTF("\r\nreg : write %08X %08X\r\n", write_addr, rb_value);
    return;
}

void cmd_read_52(ssv_type_s32 argc, char *argv[])
{
    ssv_type_u32  write_addr;
    ssv_type_u32  rb_value;
               

    write_addr = (ssv_type_u32)ssv6xxx_atoi_base(argv[1], 16);

    rb_value = ssv6xxx_drv_read_byte(write_addr);
    LOG_PRINTF("\n %08X : %08X\n", write_addr, rb_value);
} // end of - cmd_write_52 - 

void cmd_write_52(ssv_type_s32 argc, char *argv[])
{
    char if_name[32];
    ssv_type_u32  write_addr;
    ssv_type_u32  value, rb_value;
                
    if ((ssv6xxx_drv_get_name(if_name) == FALSE) || (ssv6xxx_strcmp(if_name, "sim") == 0))
    {
        log_printf("Not connected to SSV6200.\n");
        return;
    }

    if (argc != 3)
    {
        LOG_PRINTF("Usage: w addr value\n");
        return;
    }

    write_addr = (ssv_type_u32)ssv6xxx_atoi_base(argv[1], 16);
    value = (ssv_type_u32)ssv6xxx_atoi_base(argv[2], 16);
    
    if (ssv6xxx_drv_write_byte(write_addr, value) == FALSE)
    {
        LOG_PRINTF("Failed to write to SSV6200.\n");
        return;
    }
    rb_value = ssv6xxx_drv_read_byte(write_addr);
    LOG_PRINTF("\n    %08X => %08X : %08X\n", value, write_addr, rb_value);
} // end of - cmd_write_52 - 
#endif
