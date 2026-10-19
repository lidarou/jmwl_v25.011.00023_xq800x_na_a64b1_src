#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,shutil,json,traceback

# read json file data
def read_json_file(input_file):
    try:
        with open(input_file, 'r') as input_f:
            json_data = json.load(input_f)
            return json_data
    except:
            print(input_file + " load failed!")
            traceback.print_exc()
            sys.exit(-1)

if __name__ == '__main__':
    cmd = sys.argv[1]
    input_path = sys.argv[2]
    out_path = sys.argv[3]

    input_file = os.path.abspath(input_path + "/fmd_partition_table.json")
    data = read_json_file(input_file)

    block_size = data['fmd_head']['block_size']
    device_num = data['fmd_head']['device_num']
    
    # check if the number of patition tables is consistent with the number of devices
    ptable_count = 0
    for listkey, listvalue in data.items():
        if listkey == "qspi_nor" or listkey == "ssp_nor":
            ptable_array = data[listkey]['partition_table']
            ptable_count = ptable_count + len(ptable_array)
    if device_num != ptable_count:
        print("fatal error: the number of partition tables(%d) is not match the number of devices(%d)!!!" % (ptable_count, device_num))
        sys.exit(1)

    # check fmd partition config size
    for listkey, listvalue in data.items():
        if listkey == "qspi_nor" or listkey == "ssp_nor":
            device_info = data[listkey]['device_info']
            ptable_array = data[listkey]['partition_table']
            count = 0
            for ptable in ptable_array:
                dev_index = "dev" + str(count) + "_size"
                dev_size = int(device_info[dev_index], 16)
                size = 0x0
                for key, value in ptable.items():
                    if "block_num" not in value:
                        for itemkey, itemvalue in value.items():
                            size += itemvalue['block_num'] * block_size
                    else:
                        size += value['block_num'] * block_size
                if size != dev_size:
                    print("fatal error: the configured flash%d partition table size(%d) not equal to the flash device size(%d)!!!" % (count, size, dev_size))
                    sys.exit(1)
                count += 1

            # check fmd partition attribute config
            rw_find_flag = 0
            for ptable in ptable_array:
                for key, value in ptable.items():
                    if "block_num" not in value:
                        for itemkey, itemvalue in value.items():
                            if itemvalue['attribute'] == "RW":
                                rw_find_flag = 1
                            elif itemvalue['attribute'] == "RO" and rw_find_flag == 1:
                                print("fatal error: RO and RW attribute should not be configured alternately!!!")
                                sys.exit(1)
                    else:
                        if value['attribute'] == "RW":
                            rw_find_flag = 1
                        elif value['attribute'] == "RO" and rw_find_flag == 1:
                            print("fatal error: RO and RW attribute should not be configured alternately!!!")
                            sys.exit(1)
                rw_find_flag = 0

    # generate fmd_partition_config.h and flash_config.h file
    if cmd == 'ap' or cmd == 'bootloader':
        shutil.copy(input_file, out_path)
        out_file = os.path.abspath(out_path + "/fmd_partition_config.h")
        with open(out_file, 'w') as wfile:
            wfile.write("/*****************************************************************************/" + "\n")
            wfile.write("/*                                                                           */" + "\n")
            wfile.write("/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */" + "\n")
            wfile.write("/*                                                                           */" + "\n")
            wfile.write("/*****************************************************************************/" + "\n\n")

            wfile.write("/**" + "\n")
            wfile.write(" *  DESCRIPTION" + "\n")
            wfile.write(" *" + "\n")
            wfile.write(" *    This file defines FMD partition table configuration." + "\n")
            wfile.write(" */" + "\n\n")

            wfile.write("#ifndef __FMD_PARTITION_CFG_H__" + "\n")
            wfile.write("#define __FMD_PARTITION_CFG_H__" + "\n\n")

            max_name_size = 20
            max_attr_size = 8

            wfile.write("struct fmd_part_cfg {" + "\n")
            wfile.write("{:<4}{:<20}{}".format("", "char", "name[%d];" % (max_name_size)) + "\n")
            wfile.write("{:<4}{:<20}{}".format("", "unsigned int", "block_num;") + "\n")
            wfile.write("{:<4}{:<20}{}".format("", "char", "attribute[%d];" % (max_attr_size)) + "\n")
            wfile.write("};" + "\n\n")

            wfile.write("struct fmd_group_cfg {" + "\n")
            wfile.write("{:<4}{:<20}{}".format("", "char", "name[%d];" % (max_name_size)) + "\n")
            wfile.write("{:<4}{:<20}{}".format("", "unsigned int", "offset;") + "\n")
            wfile.write("{:<4}{:<20}{}".format("", "unsigned int", "block_num;") + "\n")
            wfile.write("{:<4}{:<20}{}".format("", "char", "attribute[%d];" % (max_attr_size)) + "\n")
            wfile.write("};" + "\n\n")

            wfile.write("struct fmd_part_table {" + "\n")
            wfile.write("{:<4}{:<30}{}".format("", "char", "*name;") + "\n")
            wfile.write("{:<4}{:<30}{}".format("", "const struct fmd_part_cfg", "*part_cfg;") + "\n")
            wfile.write("{:<4}{:<30}{}".format("", "unsigned int", "num;") + "\n")
            wfile.write("{:<4}{:<30}{}".format("", "const struct fmd_group_cfg", "*group_cfg;") + "\n")
            wfile.write("{:<4}{:<30}{}".format("", "unsigned int", "group_num;") + "\n")
            wfile.write("};" + "\n\n")

            count = 0
            for listkey, listvalue in data.items():
                if listkey == "qspi_nor" or listkey == "ssp_nor":
                    ptable_array = data[listkey]['partition_table']
                    for ptable in ptable_array:
                        wfile.write("static const struct fmd_part_cfg s_%s_part%d[] = {" % (listkey, count) + "\n")
                        for key, value in ptable.items():
                            if "block_num" not in value:
                                for itemkey, itemvalue in value.items():
                                    if len(itemkey) > (max_name_size - 1):
                                        print("fatal error: The name of partition \"%s\" exceeds the maximum size of partition name. The maximum size is %d!!!" % (itemkey, max_name_size))
                                        sys.exit(1)
                                    if len(itemvalue['attribute']) > (max_attr_size - 1):
                                        print("fatal error: The attribute of partition \"%s\" exceeds the maximum size of attribute name. The maximum size is %s!!!" % (itemkey, max_attr_size))
                                        sys.exit(1)
                                    wfile.write("{:<4}{:<20}".format("", "{\"%s\", " % (itemkey)) + "{:<10}".format("%d, " % (itemvalue['block_num'])) + "\"%s\"}," % (itemvalue['attribute']) + "\n")
                            else:
                                if len(key) > (max_name_size - 1):
                                    print("fatal error: The name of partition \"%s\" exceeds the maximum size of partition name. The maximum size is %d!!!" % (key, max_name_size))
                                    sys.exit(1)
                                if len(value['attribute']) > (max_attr_size - 1):
                                    print("fatal error: The attribute of partition \"%s\" exceeds the maximum size of attribute name. The maximum size is %s!!!" % (key, max_attr_size))
                                    sys.exit(1)
                                wfile.write("{:<4}{:<20}".format("", "{\"%s\", " % (key)) + "{:<10}".format("%d, " % (value['block_num'])) + "\"%s\"}," % (value['attribute']) + "\n")
                        wfile.write("};" + "\n\n")
                        count += 1

            count = 0
            for listkey, listvalue in data.items():
                if listkey == "qspi_nor" or listkey == "ssp_nor":
                    ptable_array = data[listkey]['partition_table']
                    for ptable in ptable_array:
                        offset = 0
                        group_num = 0
                        for key, value in ptable.items():
                            if "block_num" not in value:
                                group_num += 1
                        if group_num != 0:
                            wfile.write("static const struct fmd_group_cfg s_%s_group%d[] = {" % (listkey, count) + "\n")
                            for key, value in ptable.items():
                                if "block_num" not in value:
                                    group_block_num = 0
                                    group_attribute = ""
                                    for itemkey, itemvalue in value.items():
                                        group_block_num += itemvalue['block_num']
                                        if group_attribute == "":
                                            group_attribute = itemvalue['attribute']
                                        if group_attribute != "":
                                            if itemvalue['attribute'] != group_attribute:
                                                print("fatal error: The attributes of the members in the group \"%s\" are inconsistent!!!" % key)
                                                sys.exit(1)
                                    wfile.write("{:<4}{:<20}".format("", "{\"%s\", " % (key)) + "{:<10}{:<10}".format("%d," % (offset), "%d, " % (group_block_num)) + "\"%s\"}," % (group_attribute) + "\n")
                                    offset += group_block_num
                                else:
                                    offset += value['block_num']
                            wfile.write("};" + "\n\n")
                        count += 1

            count = 0
            wfile.write("static const struct fmd_part_table s_fmd_ptable_array[] = {" + "\n")
            for listkey, listvalue in data.items():
                if listkey == "qspi_nor" or listkey == "ssp_nor":
                    ptable_array = data[listkey]['partition_table']
                    for ptable in ptable_array:
                        wfile.write("{:<4}{}".format("", "{") + "\n")
                        wfile.write("{:<8}{}".format("", ".name = \"%s_ptable%d\"," % (listkey, count)) + "\n")
                        wfile.write("{:<8}{}".format("", ".part_cfg = s_%s_part%d," % (listkey, count)) + "\n")
                        wfile.write("{:<8}{}".format("", ".num = sizeof(s_%s_part%d)/sizeof(struct fmd_part_cfg)," % (listkey, count)) + "\n")
                        group_num = 0
                        for key, value in ptable.items():
                            if "block_num" not in value:
                                group_num += 1
                        if group_num == 0:
                            wfile.write("{:<8}{}".format("", ".group_cfg = NULL," + "\n"))
                            wfile.write("{:<8}{}".format("", ".group_num = 0," + "\n"))
                        else:
                            wfile.write("{:<8}{}".format("", ".group_cfg = s_%s_group%d," % (listkey, count)) + "\n")
                            wfile.write("{:<8}{}".format("", ".group_num = sizeof(s_%s_group%d)/sizeof(struct fmd_group_cfg)," % (listkey, count)) + "\n")
                        wfile.write("{:<4}{}".format("", "},") + "\n")
                        count += 1
            wfile.write("};" + "\n\n")

            wfile.write("#endif /* __FMD_PARTITION_CFG_H__ */" + "\n")

        out_file = os.path.abspath(out_path + "/flash_config.h")
        with open(out_file, 'w') as wfile:
            wfile.write("/*****************************************************************************/" + "\n")
            wfile.write("/*                                                                           */" + "\n")
            wfile.write("/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */" + "\n")
            wfile.write("/*                                                                           */" + "\n")
            wfile.write("/*****************************************************************************/" + "\n\n")

            wfile.write("/**" + "\n")
            wfile.write(" *  DESCRIPTION" + "\n")
            wfile.write(" *" + "\n")
            wfile.write(" *    This file defines flash device configuration." + "\n")
            wfile.write(" */" + "\n\n")

            wfile.write("#ifndef __FLASH_CFG_H__" + "\n")
            wfile.write("#define __FLASH_CFG_H__" + "\n\n")

            wfile.write("{:<40}{}".format('#define FLASH_DEVICE_NUM', "%d" % device_num) + "\n")
            wfile.write("{:<40}{}".format('#define FLASH_BLOCK_SIZE', "0x%x" % (block_size)) + "\n")
            wfile.write("{:<40}{}".format('#define LX_SECTOR_CONFIG_SIZE', "0x%x" % (data['fmd_head']['lx_sector_size'])) + "\n")

            for listkey, listvalue in data.items():
                if listkey == "qspi_nor" or listkey == "ssp_nor":
                    ptable_array = data[listkey]['partition_table']
                    wfile.write("{:<40}{}".format('#define %s_DEVICE_NUM' % (listkey.upper()), "%d" % len(ptable_array)) + "\n")

            wfile.write("\n")

            wfile.write("#endif /* __FLASH_CFG_H__ */" + "\n")

    # generate flash_mem_config.h file
    if cmd == 'ap' or cmd == 'cp' or cmd == 'bootloader':
        shutil.copy(input_file, out_path)
        out_file = os.path.abspath(out_path + "/flash_mem_config.h")
        with open(out_file, 'w') as wfile:
            wfile.write("/*****************************************************************************/" + "\n")
            wfile.write("/*                                                                           */" + "\n")
            wfile.write("/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */" + "\n")
            wfile.write("/*                                                                           */" + "\n")
            wfile.write("/*****************************************************************************/" + "\n\n")

            wfile.write("/**" + "\n")
            wfile.write(" *  DESCRIPTION" + "\n")
            wfile.write(" *" + "\n")
            wfile.write(" *    This file defines flash memory configuration." + "\n")
            wfile.write(" */" + "\n\n")

            wfile.write("#ifndef __FLASH_MEM_CFG_H__" + "\n")
            wfile.write("#define __FLASH_MEM_CFG_H__" + "\n\n")

            wfile.write("{:<50}{}".format('#define FLASH_BASE', '0x10000000') + "\n\n")

            dev_index = 0
            if "qspi_nor" in data:
                ro_size = 0;
                rw_size = 0;
                device_info = data['qspi_nor']['device_info']
                ptable_array = data['qspi_nor']['partition_table']
                count = 0
                for ptable in ptable_array:
                    offset = 0x0
                    dev_no = "dev" + str(count) + "_size"
                    dev_size = int(device_info[dev_no], 16)
                    wfile.write("/* flash%d memory config */" % dev_index + "\n")
                    if count == 0:
                        wfile.write("{:<50}{}".format("#define FLASH%d_BASE" % dev_index, "(FLASH_BASE + 0x1000000)") + "\n")
                    else:
                        wfile.write("{:<50}{}".format("#define FLASH%d_BASE" % dev_index, "(FLASH_BASE + %d * 0x4000000)" % count) + "\n")

                    for key, value in ptable.items():
                        if "block_num" not in value:
                            group_block_num = 0
                            group_offset = offset
                            for itemkey, itemvalue in value.items():
                                wfile.write("{:<50}{}".format("#define FLASH%d_%s_BASE" % (dev_index, itemkey.upper()), "(FLASH%d_BASE + 0x%08x)" % (dev_index, offset)) + "\n")
                                wfile.write("{:<50}{}".format("#define FLASH%d_%s_SIZE" % (dev_index, itemkey.upper()), "(0x%08x)" % (itemvalue['block_num'] * block_size)) + "\n")
                                offset = offset + itemvalue['block_num'] * block_size
                                group_block_num += itemvalue['block_num']
                                if itemvalue['attribute'] == "RO":
                                    ro_size = ro_size + itemvalue['block_num'] * block_size
                                elif itemvalue['attribute'] == "RW":
                                    rw_size = rw_size + itemvalue['block_num'] * block_size
                            wfile.write("{:<50}{}".format("#define FLASH%d_%s_BASE" % (dev_index, key.upper()), "(FLASH%d_BASE + 0x%08x)" % (dev_index, group_offset)) + "\n")
                            wfile.write("{:<50}{}".format("#define FLASH%d_%s_SIZE" % (dev_index, key.upper()), "(0x%08x)" % (group_block_num * block_size)) + "\n")
                        else:
                            wfile.write("{:<50}{}".format("#define FLASH%d_%s_BASE" % (dev_index, key.upper()), "(FLASH%d_BASE + 0x%08x)" % (dev_index, offset)) + "\n")
                            wfile.write("{:<50}{}".format("#define FLASH%d_%s_SIZE" % (dev_index, key.upper()), "(0x%08x)" % (value['block_num'] * block_size)) + "\n")
                            offset = offset + value['block_num'] * block_size
                            if value['attribute'] == "RO":
                                ro_size = ro_size + value['block_num'] * block_size
                            elif value['attribute'] == "RW":
                                rw_size = rw_size + value['block_num'] * block_size

                    wfile.write("{:<50}{}".format("#define FLASH%d_TOTAL_SIZE" % dev_index, "(0x%08x)" % dev_size) + "\n")
                    wfile.write("{:<50}{}".format("#define FLASH%d_DAC_RO_BASE" % dev_index, "(FLASH%d_BASE)" % dev_index) + "\n")
                    wfile.write("{:<50}{}".format("#define FLASH%d_DAC_RO_SIZE" % dev_index, "(FLASH%d_TOTAL_SIZE)" % dev_index) + "\n")
                    wfile.write("{:<50}{}".format("#define FLASH%d_INDAC_RW_BASE" %dev_index, "(FLASH_BASE + 0x%08x)" % ro_size) + "\n")
                    wfile.write("{:<50}{}".format("#define FLASH%d_INDAC_RW_SIZE" %dev_index, "(0x%08x)" % rw_size) + "\n")

                    count += 1
                    dev_index += 1
                    ro_size = 0
                    rw_size = 0
                    wfile.write("\n")

            if "ssp_nor" in data:
                device_info = data['ssp_nor']['device_info']
                ptable_array = data['ssp_nor']['partition_table']
                count = 0
                for ptable in ptable_array:
                    offset = 0x0
                    dev_no = "dev" + str(count) + "_size"
                    dev_size = int(device_info[dev_no], 16)
                    wfile.write("/* flash%d memory config */" % dev_index + "\n")

                    for key, value in ptable.items():
                        if "block_num" not in value:
                            group_block_num = 0
                            group_offset = offset
                            for itemkey, itemvalue in value.items():
                                wfile.write("{:<50}{}".format("#define FLASH%d_%s_BASE" % (dev_index, itemkey.upper()), "(0x%08x)" % offset) + "\n")
                                wfile.write("{:<50}{}".format("#define FLASH%d_%s_SIZE" % (dev_index, itemkey.upper()), "(0x%08x)" % (itemvalue['block_num'] * block_size)) + "\n")
                                offset += itemvalue['block_num'] * block_size
                                group_block_num += itemvalue['block_num']
                            wfile.write("{:<50}{}".format("#define FLASH%d_%s_BASE" % (dev_index, key.upper()), "(0x%08x)" % group_offset) + "\n")
                            wfile.write("{:<50}{}".format("#define FLASH%d_%s_SIZE" % (dev_index, key.upper()), "(0x%08x)" % (group_block_num * block_size)) + "\n")
                        else:
                            wfile.write("{:<50}{}".format("#define FLASH%d_%s_BASE" % (dev_index, key.upper()), "(0x%08x)" % offset) + "\n")
                            wfile.write("{:<50}{}".format("#define FLASH%d_%s_SIZE" % (dev_index, key.upper()), "(0x%08x)" % (value['block_num'] * block_size)) + "\n")
                            offset = offset + value['block_num'] * block_size

                    wfile.write("{:<50}{}".format("#define FLASH%d_TOTAL_SIZE" % dev_index, "(0x%08x)" % dev_size) + "\n")
                    wfile.write("\n")

            wfile.write("#endif /* __FLASH_MEM_CFG_H__ */" + "\n")

    # generate ftl_partition_config.h file
    if cmd == 'ap' or cmd == 'cp' or cmd == 'bootloader':
        # check ftl partition config size
        lx_sector_size = data['fmd_head']['lx_sector_size']
        for listkey, listvalue in data.items():
            if "fmd_head" == listkey:
                continue
            ptable_array = data[listkey]['partition_table']
            for ptable in ptable_array:
                for key, value in ptable.items():
                    if "ftl_table" in value:
                        block_num = value['block_num']
                        ftl_ptable = value['ftl_table']
                        sector_num = 0
                        for itemkey, itemvalue in ftl_ptable.items():
                            if (int)(itemvalue['sector_num'] / 16) * 16 != itemvalue['sector_num'] and itemkey != "C":
                                print("fatal error: ftl_table(" + itemkey + ") sector number(" + str(itemvalue['sector_num']) + ") is not multiple of 16!!!")
                                sys.exit(1)
                            sector_num = sector_num + itemvalue['sector_num']
                        if lx_sector_size == 512:
                            available_sector = 7 * (block_num - 1)
                        elif lx_sector_size == 256:
                            available_sector = ((15 * (block_num - 1)) & ~0x1) / 2
                        else :
                            print("fatal error: lx sector size not support!!!")
                            sys.exit(1)

                        if sector_num > available_sector:
                            print("fatal error: ftl_table sector num(%d) exceeds %s available sector num(%d)!!!" % (sector_num, key, available_sector))
                            sys.exit(1)

        out_file = os.path.abspath(out_path + "/ftl_partition_config.h")
        with open(out_file, 'w') as wfile:
            wfile.write("/*****************************************************************************/" + "\n")
            wfile.write("/*                                                                           */" + "\n")
            wfile.write("/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */" + "\n")
            wfile.write("/*                                                                           */" + "\n")
            wfile.write("/*****************************************************************************/" + "\n\n")

            wfile.write("/**" + "\n")
            wfile.write(" *  DESCRIPTION" + "\n")
            wfile.write(" *" + "\n")
            wfile.write(" *    This file defines FTL partition table configuration." + "\n")
            wfile.write(" */" + "\n\n")

            wfile.write("#ifndef __FTL_PARTITION_CFG_H__" + "\n")
            wfile.write("#define __FTL_PARTITION_CFG_H__" + "\n\n")

            wfile.write("struct ftl_part_cfg {" + "\n")
            wfile.write("{:<4}{:<20}{}".format("", "char", "*name;") + "\n")
            wfile.write("{:<4}{:<20}{}".format("", "unsigned int", "sector_num;") + "\n")
            wfile.write("};" + "\n\n")

            wfile.write("struct ftl_part_table {" + "\n")
            wfile.write("{:<4}{:<30}{}".format("", "char", "*name;") + "\n")
            wfile.write("{:<4}{:<30}{}".format("", "const struct ftl_part_cfg", "*part_cfg;") + "\n")
            wfile.write("{:<4}{:<30}{}".format("", "unsigned int", "num;") + "\n")
            wfile.write("};" + "\n\n")

            count = 0
            for listkey, listvalue in data.items():
                if "fmd_head" == listkey:
                    continue
                ptable_array = data[listkey]['partition_table']
                for ptable in ptable_array:
                    for key, value in ptable.items():
                        if "ftl_table" in value:
                            wfile.write("static const struct ftl_part_cfg s_ftl_part" + str(count) + "[] = {" + "\n")
                            for itemkey, itemvalue in value['ftl_table'].items():
                                wfile.write("{:<4}{}{:<25}{}".format("", "{", "\"%s\"," % itemkey, "%d}," % itemvalue['sector_num']) + "\n")
                            wfile.write("};" + "\n\n")
                            count += 1

            count = 0
            wfile.write("static const struct ftl_part_table s_ftl_table[] = {" + "\n")
            for listkey, listvalue in data.items():
                if "fmd_head" == listkey:
                    continue
                ptable_array = data[listkey]['partition_table']
                for ptable in ptable_array:
                    for key, value in ptable.items():
                        if "ftl_table" in value:
                            wfile.write("{:<4}{}".format("", "{") + "\n")
                            wfile.write("{:<8}{}".format("", ".name = \"%s\"," % key) + "\n")
                            wfile.write("{:<8}{}".format("", ".part_cfg = s_ftl_part%d," % count) + "\n")
                            wfile.write("{:<8}{}".format("", ".num = sizeof(s_ftl_part%d)/sizeof(struct ftl_part_cfg)," % count) + "\n")
                            wfile.write("{:<4}{}".format("", "},") + "\n")
                            count += 1
            wfile.write("};" + "\n\n")

            wfile.write("#endif /* __FTL_PARTITION_CFG_H__ */" + "\n")

        out_file = os.path.abspath(out_path + "/ftl_partition_size.h")
        with open(out_file, 'w') as wfile:
            wfile.write("/*****************************************************************************/" + "\n")
            wfile.write("/*                                                                           */" + "\n")
            wfile.write("/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */" + "\n")
            wfile.write("/*                                                                           */" + "\n")
            wfile.write("/*****************************************************************************/" + "\n\n")

            wfile.write("/**" + "\n")
            wfile.write(" *  DESCRIPTION" + "\n")
            wfile.write(" *" + "\n")
            wfile.write(" *    This file defines FTL partition size configuration." + "\n")
            wfile.write(" */" + "\n\n")

            wfile.write("#ifndef __FTL_PARTITION_SIZE_H__" + "\n")
            wfile.write("#define __FTL_PARTITION_SIZE_H__" + "\n\n")

            ftl_sector_size = data['fmd_head']['ftl_sector_size']
            wfile.write("{:<50}{}".format("#define FTL_SECTOR_SIZE", "(0x%08x)" % ftl_sector_size) + "\n\n")

            for listkey, listvalue in data.items():
                if "fmd_head" == listkey:
                    continue
                ptable_array = data[listkey]['partition_table']
                for ptable in ptable_array:
                    for key, value in ptable.items():
                        if "ftl_table" in value:
                            for itemkey, itemvalue in value['ftl_table'].items():
                                wfile.write("{:<50}{}".format("#define FTL_%s_%s_SIZE" % (key.upper(), itemkey.upper()), "(0x%08x)" % (itemvalue['sector_num'] * ftl_sector_size)) + "\n")

            wfile.write("\n")

            wfile.write("#endif /* __FTL_PARTITION_SIZE_H__ */" + "\n")
