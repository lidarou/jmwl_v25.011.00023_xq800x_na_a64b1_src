#!/usr/bin/env python
#-*- coding = utf-8 -*-

import argparse
import os, sys, time, re, shutil, platform
import subprocess, fileinput
from multiprocessing import cpu_count
from colorama import init
init(autoreset=True)

# set global path and value
root_path = os.path.abspath(sys.path[0])
aicos_path = os.path.abspath(root_path + "/aicos")
script_path = os.path.abspath(root_path + "/build")
script_path = script_path.replace('\\', '/')
targets_path = os.path.abspath(root_path + "/targets")
nvfile_script_path = os.path.abspath(root_path + "/build/tools/nv-tool/")
build_info_file = os.path.abspath(root_path + "/out/build_info.txt")
global chip_name, project_name, sdk_base_board
global ap_board_name, cp_board_name, adsp_board_name, bl_board_name, tfm_board_name
global md_board_name, nv_board_name, nvram_board_config
global subsys_config, board_config
global binary_path, board_path, common_path
global maincmd, subcmd, thirdcmd

#
# Get build command parameter
#
def get_build_cmd():
    global maincmd, subcmd, thirdcmd

    if len(sys.argv) == 4:
        maincmd = sys.argv[1]
        subcmd = sys.argv[2]
        thirdcmd = sys.argv[3]
    elif len(sys.argv) == 3:
        maincmd = sys.argv[1]
        subcmd = sys.argv[2]
        thirdcmd = ""
    elif len(sys.argv) == 2:
        maincmd = sys.argv[1]
        subcmd = ""
        thirdcmd = ""
    elif len(sys.argv) == 1:
        maincmd = get_legacy_cmd()
        subcmd = ""
        thirdcmd = ""
    else:
        print("\033[31m make command error!")
        print("\033[31m ./make.py -h to get the command help info")
        exit(1)

#
# Get board configurations
# Get board info from build_info.txt in out dir.
#
def get_board_config():
    global chip_name, project_name, sdk_base_board
    global ap_board_name, cp_board_name, adsp_board_name, bl_board_name, tfm_board_name
    global board_config, software_version, secure_board
    global md_board_name, nv_board_name

    if not os.path.exists(build_info_file):
        print("\033[31m-- build info file is not exit, PLS lunch first!")
        exit(1)

    cmake_file = open(build_info_file)

    secure_board = "NSEC"
    for line in cmake_file.readlines():
        if not line.find("CMAKE_CONFIG_CHIP"):
            chip_name = re.findall("CMAKE_CONFIG_CHIP:STRING=(.*?)\n", line)[0]
        if not line.find("CMAKE_CONFIG_PROJ_NAME"):
            project_name = re.findall("CMAKE_CONFIG_PROJ_NAME:STRING=(.*?)\n", line)[0]
        if not line.find("CMAKE_CONFIG_AP_BOARD_NAME"):
            ap_board_name = re.findall("CMAKE_CONFIG_AP_BOARD_NAME:STRING=(.*?)\n", line)[0]
        if not line.find("CMAKE_CONFIG_SDK_BASE_BOARD"):
            sdk_base_board = re.findall("CMAKE_CONFIG_SDK_BASE_BOARD:STRING=(.*?)\n", line)[0]
        if not line.find("CMAKE_CONFIG_MD_BOARD_NAME"):
            md_board_name = re.findall("CMAKE_CONFIG_MD_BOARD_NAME:STRING=(.*?)\n", line)[0]
        if not line.find("CMAKE_CONFIG_CP_BOARD_NAME"):
            cp_board_name = re.findall("CMAKE_CONFIG_CP_BOARD_NAME:STRING=(.*?)\n", line)[0]
        if not line.find("CMAKE_CONFIG_NV_BOARD_NAME"):
            nv_board_name = re.findall("CMAKE_CONFIG_NV_BOARD_NAME:STRING=(.*?)\n", line)[0]
        if not line.find("CMAKE_CONFIG_ADSP_BOARD_NAME"):
            adsp_board_name = re.findall("CMAKE_CONFIG_ADSP_BOARD_NAME:STRING=(.*?)\n", line)[0]
        if not line.find("CMAKE_CONFIG_BL_BOARD_NAME"):
            bl_board_name = re.findall("CMAKE_CONFIG_BL_BOARD_NAME:STRING=(.*?)\n", line)[0]
        if not line.find("CMAKE_CONFIG_TFM_BOARD_NAME"):
            tfm_board_name = re.findall("CMAKE_CONFIG_TFM_BOARD_NAME:STRING=(.*?)\n", line)[0]
        if not line.find("AIC_SW_VERISON"):
            software_version = re.findall("AIC_SW_VERISON:VALUE=(.*?)\n", line)[0]
        if not line.find("AIC_SECURITY"):
            secure_board = re.findall("AIC_SECURITY:VALUE=(.*?)\n", line)[0]
    cmake_file.close()
#
# set subsys configurations
#
def _subsys_config(subsys):
    global subsys_config, board_config, nvram_board_config
    global board_path
    global common_path

    subsys_config = subsys
    if subsys == "ap":
        board_config = ap_board_name
        nvram_board_config = md_board_name
    elif subsys == "cp":
        board_config = cp_board_name
        nvram_board_config = nv_board_name
    elif subsys == "adsp":
        board_config = adsp_board_name
    elif subsys == "bootloader":
        board_config = bl_board_name
    elif subsys == "tfm":
        board_config = tfm_board_name

    board_path = os.path.abspath(aicos_path + "/projects/" + chip_name + "/" + subsys + "/" + board_config)
    common_path = os.path.abspath(aicos_path + "/projects/" + chip_name + "/" + subsys + "/" + "common_inc")

#
# Get subsys configurations
# Get subsys info from build_info.txt in out dir.
#
def get_subsys_config():
    f = open(build_info_file)

    while True:
        line = f.readline()
        # read all the file, and do not find subsys info.
        if line == '':
            print("\033[31m=============build error!============")
            print("\033[33m--- can not find subsys info, pls use cmd:---")
            print("\033[36m--- ./make.py -h to get help info         ---")
            f.close()
            exit(1)
        if not line.find("CMAKE_BUILD_SUBSYS_CMD"):
            break

    subsys_config = re.findall("CMAKE_BUILD_SUBSYS_CMD:STRING=(.*?)\n", line)[0]
    f.close()

    _subsys_config(subsys_config)

    return subsys_config

#
# set_subsys_config
# Update make command to build info file.
# The command include ap/ap/bootloader/tfm and others, these
# commands were used to decide which os will be compiled.
#
def set_subsys_config(cmd):
    cmdfind = False

    f = open(build_info_file, "r+")
    for line in f.readlines():
        if not line.find("CMAKE_BUILD_SUBSYS_CMD"):
            # Fixed me if there has better way to replace irregular strings
            f.seek(0, 0)
            str_data = f.read()
            str_data = re.sub("CMAKE_BUILD_SUBSYS_CMD:STRING=(.*?)\n",
                            "CMAKE_BUILD_SUBSYS_CMD:STRING={}\n".format(cmd), str_data)
            f.seek(0, 0)
            f.write(str_data)
            cmdfind = True
            break

    if not cmdfind:
        f.write("CMAKE_BUILD_SUBSYS_CMD:STRING=")
        f.write(cmd + "\n")
    f.close()

    _subsys_config(cmd)

#
# Get legacy command
# Get legacy command info from build_info.txt in out dir.
#
def get_legacy_cmd():

    f = open(build_info_file)

    while True:
        line = f.readline()
        # read all the file, and do not find subsys info.
        if line == '':
            print("\033[31m================build error!==============")
            print("\033[33m--- can not find cmd info, pls use cmd:---")
            print("\033[36m--- ./make.py -h to get help info      ---")
            f.close()
            exit(1)
        if not line.find("CMAKE_LEGACY_CMD"):
            break

    legacy_cmd = re.findall("CMAKE_LEGACY_CMD:STRING=(.*?)\n", line)[0]
    f.close()
    return legacy_cmd

#
# set legacy command
# Update make command to build info file.
# The command include ap/ap/bootloader/tfm and others, these
# commands were used to decide which os will be compiled.
#
def set_legacy_cmd(cmd):
    cmdfind = False

    if cmd == "clean":
        return
    f = open(build_info_file, "r+")
    for line in f.readlines():
        if not line.find("CMAKE_LEGACY_CMD"):
            # Fixed me if there has better way to replace irregular strings
            f.seek(0, 0)
            str_data = f.read()
            str_data = re.sub("CMAKE_LEGACY_CMD:STRING=(.*?)\n",
                            "CMAKE_LEGACY_CMD:STRING={}\n".format(cmd), str_data)
            f.seek(0, 0)
            f.write(str_data)
            cmdfind = True
            break

    if not cmdfind:
        f.write("CMAKE_LEGACY_CMD:STRING=")
        f.write(cmd + "\n")
    f.close()

#
# check the sdk configurations.
# return value:
# 0: do not define SDK macro
# 1: define but not open SDK macro
# 2: SDK macro is open
#
def sdk_check_config(subsys):
    mode = 0
    #get board info
    get_board_config()

    if subsys == "ap":
        board_name = ap_board_name
    else:
        board_name = cp_board_name
    board_cfg_file = aicos_path + r"/projects/" + chip_name + "/" + subsys + "/" + board_name + r"/default_config"
    if not os.path.exists(board_cfg_file):
        return 0

    f = open(board_cfg_file, "r+")
    while True:
        line = f.readline()
        # read all the file, and do not find sdk info.
        if line == '':
            mode = 0
            break;
        if not line.find("# CONFIG_SDK_ENABLE is not set"):
            mode = 1
            break;
        if not line.find("CONFIG_SDK_ENABLE=y"):
            # fix multi projects use same ap or cp board bug
            # if the first board make all and make sdk, the default config file will
            # be modified, then the next board make all, it will build failed because
            # of last board modified configuration file.
            subsys_libs = root_path + r"/libs/" + sdk_base_board + r"/" + subsys
            subsys_out = root_path + r"/out/" + project_name + r"/" + subsys + r"/build.ninja"
            if not os.path.exists(subsys_libs) and not os.path.exists(subsys_out):
                f.close()
                subsys_cfg_path = root_path + r"/aicos/projects/" + chip_name + r"/" + subsys + r"/" + board_name
                os.chdir(subsys_cfg_path)
                res = subprocess.call('git checkout default_config')
                if res != 0:
                    print("\033[31m-- checkout default config failed {}".format(res))
                    exit(1)
                os.chdir(aicos_path)
                return 1
            else:
                mode = 2
            break;
    f.close()
    return mode

#
# set sdk configurations.
#
def sdk_set_config(subsys):
    get_board_config()

    # set aicos default config
    if subsys == "ap":
        board_name = ap_board_name
    else:
        board_name = cp_board_name
    aicos_cfg_file = aicos_path + r"/projects/" + chip_name + "/" + subsys + "/" + board_name + r"/default_config"
    if not os.path.exists(aicos_cfg_file):
        return 0

    for line in fileinput.input(aicos_cfg_file, inplace=1):
        line=line.replace("# CONFIG_SDK_ENABLE is not set", "CONFIG_SDK_ENABLE=y")
        print(line,end="")

    # set bootloader config
    if subsys == "ap":
        bl_cfg_file = root_path + r"/bootloader/projects/" + chip_name + r"/" + bl_board_name + r"/common/common.cmake"
        for line in fileinput.input(bl_cfg_file, inplace=1):
            line=line.replace("# set(CONFIG_BL_SDK ON", "set(CONFIG_BL_SDK ON")
            print(line,end="")

#
# copy target config files
#
def copy_target_config_file(file):
    config_file_src = os.path.abspath(targets_path + "/" + chip_name + "/" + project_name + "/" + file)
    config_file_dest = os.path.abspath(binary_path + "/config" + "/" + file)
    shutil.copy(config_file_src, config_file_dest)

#
# sdk copy config and binary files.
#
def sdk_copy_files(subsys):
    get_board_config()

    if subsys == "ap":
        board_name = ap_board_name
    else:
        board_name = cp_board_name
    # copy aicos default config
    aicos_src_config = root_path + r"/out/" + project_name + "/" + subsys + r"/.config"
    aicos_cfg_path = aicos_path + r"/projects/" + chip_name + "/" + subsys + "/" + board_name + r"/default_config"
    if os.path.exists(aicos_src_config) and os.path.exists(aicos_cfg_path):
        shutil.copy(aicos_src_config, aicos_cfg_path)
    # only ap needs do these
    if subsys == "ap":
        if sdk_check_config("cp") == 0:
            # copy cp binaries
            cp_src = root_path + r"/out/" + project_name + r"/cp/bin"
            cp_dst = root_path + r"/libs/" + project_name + r"/cp/bin"
            if not os.path.exists(cp_dst):
                shutil.copytree(cp_src, cp_dst)
            # remove cp elf/map
            if os.path.exists(cp_dst + "/aicos_cp.elf"):
                os.remove(cp_dst + "/aicos_cp.elf")
            if os.path.exists(cp_dst + "/aicos_cp.map"):
                os.remove(cp_dst + "/aicos_cp.map")
        elif sdk_check_config("cp") != 0:
            cp_dst = root_path + r"/libs/" + project_name + r"/cp/bin"
            if not os.path.exists(cp_dst):
                os.makedirs(cp_dst, exist_ok=True)

        # copy cp nvram
        nvram_src_dir = root_path + r"/out/" + project_name + r"/cp"
        files = os.listdir(nvram_src_dir)
        for file in files:
            nvram_src_dir_path = os.path.join(nvram_src_dir, file)
            if os.path.isdir(nvram_src_dir_path) and file.startswith(r"nvram_"):
                cp_src = root_path + r"/out/" + project_name + r"/cp/" + file + r"/custom"
                cp_dst = root_path + r"/libs/" + project_name + r"/cp/" + file + r"/custom"
                if not os.path.exists(cp_dst):
                    shutil.copytree(cp_src, cp_dst)
                cp_src = root_path + r"/out/" + project_name + r"/cp/" + file + r"/itembin/custom"
                cp_dst = root_path + r"/libs/" + project_name + r"/cp/" + file + r"/itembin/custom"
                if not os.path.exists(cp_dst):
                    shutil.copytree(cp_src, cp_dst)
                cp_src = root_path + r"/out/" + project_name + r"/cp/" + file + r"/hwdiff"
                cp_dst = root_path + r"/libs/" + project_name + r"/cp/" + file + r"/hwdiff"
                if not os.path.exists(cp_dst):
                    shutil.copytree(cp_src, cp_dst)
                cp_src = root_path + r"/out/" + project_name + r"/cp/" + file + r"/nvram_custom.prj"
                cp_dst = root_path + r"/libs/" + project_name + r"/cp/" + file + r"/nvram_custom.prj"
                if os.path.exists(cp_src):
                    shutil.copy2(cp_src, cp_dst)
                cp_src = root_path + r"/out/" + project_name + r"/cp/" + file + r"/nvram_custom_native.bin"
                cp_dst = root_path + r"/libs/" + project_name + r"/cp/" + file + r"/nvram_custom_native.bin"
                if os.path.exists(cp_src):
                    shutil.copy2(cp_src, cp_dst)

        # copy gnss binaries
        gnss_bl_bin = root_path + r"/out/" + project_name + r"/ap/bin/gnss_bl.bin"
        gnss_fw_bin = root_path + r"/out/" + project_name + r"/ap/bin/gnss_fw.bin"
        gnss_dst_path = root_path + r"/libs/" + project_name + r"/ap/bin"
        gnss_bl_dst_bin = root_path + r"/libs/" + project_name + r"/ap/bin/gnss_bl.bin"
        gnss_fw_dst_bin = root_path + r"/libs/" + project_name + r"/ap/bin/gnss_fw.bin"
        if os.path.exists(gnss_bl_bin):
            os.makedirs(gnss_dst_path, exist_ok=True)
            shutil.copy2(gnss_bl_bin, gnss_bl_dst_bin)
            shutil.copy2(gnss_fw_bin, gnss_fw_dst_bin)

        # copy adsp binaries
        dsp_src_bin = root_path + r"/out/" + project_name + r"/adsp"
        dsp_dst_bin = root_path + r"/libs/" + project_name + r"/adsp"
        if os.path.exists(dsp_src_bin) and not os.path.exists(dsp_dst_bin):
            shutil.copytree(dsp_src_bin, dsp_dst_bin)

#
# Output start build timestamp
#
def build_timestamp_start():
    global time_start

    if maincmd == "-h" or maincmd == "--help":
        return
    time_start = time.time()
    print("\033[32m==============================================")
    print("\033[32m-- build %s %s start" % (project_name, maincmd))
    print("\033[32m==============================================")

#
# Output start build timestamp
#
def build_timestamp_end():
    global time_end

    if maincmd == "-h" or maincmd == "--help":
        return
    time_end = time.time()
    print("\033[32m==============================================")
    print("\033[32mbuild %s %s end, time cost:%.2fs" % (project_name, maincmd, time_end - time_start))
    print("\033[32m==============================================")

#
# aicos cp sign function.
#
def aicos_sign_cp(cmd):
    if cmd == "cp" and secure_board == "SEC":
        cp_bin_path = root_path + r"/libs/" + sdk_base_board + r"/cp/bin"
        target_cfg_path = os.path.abspath(targets_path + "/" + chip_name + "/" + project_name)
        imgsign = script_path + r"/sign_script/img_resign.py"

        if os.path.exists(cp_bin_path + r"/aicos_cp_flash.bin"):
            #sign aicos_cp_flash.bin
            print("sign aicos_cp_flash.bin...")
            res = subprocess.call(["python",
                                  "-B",
                                  imgsign,
                                  "-m", "0x55aa7e7e",
                                  "-s", software_version,
                                  "-k", target_cfg_path + "/key/bl_priv_key.pem",
                                  "-pub", target_cfg_path + "/key/bl_pub_key.pem",
                                  cp_bin_path + r"/aicos_cp_flash.bin",
                                  cp_bin_path + r"/aicos_cp_flash_signed.bin"
                                ])
            if res != 0:
                print("\033[31m-- sign aicos_cp_flash.bin failed {}".format(res))
                exit(1)

        if os.path.exists(cp_bin_path + r"/aicos_cp_ram1.bin"):
            print("sign and compress aicos_cp_ram1.bin...")
            #sign aicos_cp_ram1.bin
            res = subprocess.call(["python",
                                  "-B",
                                  imgsign,
                                  "-m", "0x55aa7e7e",
                                  "-s", software_version,
                                  "-k", target_cfg_path + "/key/bl_priv_key.pem",
                                  "-pub", target_cfg_path + "/key/bl_pub_key.pem",
                                  cp_bin_path + r"/aicos_cp_ram1.bin",
                                  cp_bin_path + r"/aicos_cp_ram1_signed.bin"
                                 ])
            if res != 0:
                print("\033[31m-- sign aicos_cp_ram1.bin failed {}".format(res))
                exit(1)
            #compress aicos_cp_ram1_signed.bin
            res = subprocess.call(["python",
                                  "-B",
                                  script_path + "/compression_proc.py",
                                  cp_bin_path + r"/aicos_cp_ram1_signed.bin",
                                  cp_bin_path + r"/aicos_cp_ram1_signed_compressed.bin"
                                 ])
            if res != 0:
                print("\033[31m-- compress aicos_cp_ram1_signed.bin failed {}".format(res))
                exit(1)

        if os.path.exists(cp_bin_path + r"/aicos_cp_ram2.bin"):
            print("sign and compress aicos_cp_ram2.bin...")
            #sign aicos_cp_ram2.bin
            res = subprocess.call(["python",
                                  "-B",
                                  imgsign,
                                  "-m", "0x55aa7e7e",
                                  "-s", software_version,
                                  "-k", target_cfg_path + "/key/bl_priv_key.pem",
                                  "-pub", target_cfg_path + "/key/bl_pub_key.pem",
                                  cp_bin_path + r"/aicos_cp_ram2.bin",
                                  cp_bin_path + r"/aicos_cp_ram2_signed.bin"
                                 ])
            if res != 0:
                print("\033[31m-- sign aicos_cp_ram2.bin failed {}".format(res))
                exit(1)
            #compress aicos_cp_ram2_signed.bin
            res = subprocess.call(["python",
                                  "-B",
                                  script_path + "/compression_proc.py",
                                  cp_bin_path + r"/aicos_cp_ram2_signed.bin",
                                  cp_bin_path + r"/aicos_cp_ram2_signed_compressed.bin"
                                 ])
            if res != 0:
                print("\033[31m-- compress aicos_cp_ram2_signed.bin failed {}".format(res))
                exit(1)

        if os.path.exists(cp_bin_path + r"/aicos_cp_ram3.bin"):
            print("sign and compress aicos_cp_ram3.bin...")
            #sign aicos_cp_ram3.bin
            res = subprocess.call(["python",
                                  "-B",
                                  imgsign,
                                  "-m", "0x55aa7e7e",
                                  "-s", software_version,
                                  "-k", target_cfg_path + "/key/bl_priv_key.pem",
                                  "-pub", target_cfg_path + "/key/bl_pub_key.pem",
                                  cp_bin_path + r"/aicos_cp_ram3.bin",
                                  cp_bin_path + r"/aicos_cp_ram3_signed.bin"
                                 ])
            if res != 0:
                print("\033[31m-- sign aicos_cp_ram3.bin failed {}".format(res))
                exit(1)
            #compress aicos_cp_ram3_signed.bin
            res = subprocess.call(["python",
                                  "-B",
                                  script_path + "/compression_proc.py",
                                  cp_bin_path + r"/aicos_cp_ram3_signed.bin",
                                  cp_bin_path + r"/aicos_cp_ram3_signed_compressed.bin"
                                 ])
            if res != 0:
                print("\033[31m-- compress aicos_cp_ram3_signed.bin failed {}".format(res))
                exit(1)

        if os.path.exists(cp_bin_path + r"/aicos_cp_ram4.bin"):
            print("sign and compress aicos_cp_ram4.bin...")
            #sign aicos_cp_ram4.bin
            res = subprocess.call(["python",
                                  "-B",
                                  imgsign,
                                  "-m", "0x55aa7e7e",
                                  "-s", software_version,
                                  "-k", target_cfg_path + "/key/bl_priv_key.pem",
                                  "-pub", target_cfg_path + "/key/bl_pub_key.pem",
                                  cp_bin_path + r"/aicos_cp_ram4.bin",
                                  cp_bin_path + r"/aicos_cp_ram4_signed.bin"
                                 ])
            if res != 0:
                print("\033[31m-- sign aicos_cp_ram4.bin failed {}".format(res))
                exit(1)
            #compress aicos_cp_ram4_signed.bin
            res = subprocess.call(["python",
                                  "-B",
                                  script_path + "/compression_proc.py",
                                  cp_bin_path + r"/aicos_cp_ram4_signed.bin",
                                  cp_bin_path + r"/aicos_cp_ram4_signed_compressed.bin"
                                 ])
            if res != 0:
                print("\033[31m-- compress aicos_cp_ram4_signed.bin failed {}".format(res))
                exit(1)


#
# aicos software version get.
#
def aicos_sw_version_get():
    global aicos_path

    try:
        verion_path = aicos_path + "/components/include/aic_version.h"
        with open(verion_path, 'r') as input_f:
            version = input_f.read()
            index = version.index("AIC_SW_VERSION")
            index_s = version.index('"', index, -1)
            index_e = version.index('"', index_s + 1, -1)
            if (version[index_e - 1] == 'n' and version[index_e - 2] == '/'):
                sw_version = version[index_s + 1 : index_e - 2]
            else:
                sw_version = version[index_s + 1 : index_e]
            return sw_version
    except:
        print("\033[31m-- " + verion_path + " not found \"AIC_SW_VERSION\"!")
        exit(1)

#
# aicos nvfile build function.
#
def aicos_nvfile_build(cmd):
    global binary_path
    global common_path
    global nvram_board_config

    os_name = platform.system()
    if os_name.lower() == "windows":
        ntcmd_path = nvfile_script_path + "/NTCmd/NTCmd.exe"
    elif os_name.lower() == "linux":
        ntcmd_path = nvfile_script_path + "/NTCmd_Linux/NTCmd"
        res = subprocess.call(["chmod", "a+x", ntcmd_path])
        if res != 0:
            print("\033[31m-- chmod a+x {} failed.".format(ntcmd_path))
            exit(1)
    else:
        print("\033[31m-- Unknown operating system environment!")
        exit(1)

    set_subsys_config(cmd)

    if sdk_check_config("ap") == 2 and cmd == "cp":
        sdk_lib_path = os.path.abspath(root_path + "/libs/" + sdk_base_board + "/" + cmd)
        nvram_dir = sdk_lib_path + "/nvram_native"
        if subcmd:
            if subcmd == "nvfile" or subcmd == "user":
                if thirdcmd:
                    nvram_dir = sdk_lib_path + "/" + thirdcmd
            else:
                nvram_dir = sdk_lib_path + "/" + subcmd

        nvram_dir = nvram_dir.replace("\\", "/")
        if not os.path.exists(nvram_dir):
            print("\033[31m-- nvram dir {} not exists".format(nvram_dir))
            exit(1)
        else:
            print(nvram_dir)

        res = subprocess.call([ntcmd_path,
                               nvram_dir,
                               cmd
                              ])
        #shutil.copy(nvram_dir + "/aic_nvram_data_native.bin", sdk_lib_path + "/bin/")
        #if os.path.exists(nvram_dir + "/aic_nvram_diff_native.bin"):
            #shutil.copy(nvram_dir + "/aic_nvram_diff_native.bin", sdk_lib_path + "/bin/")
    else:
        nv_version = nvram_board_config + "-" + aicos_sw_version_get()

        nv_board_path = os.path.abspath(aicos_path + "/projects/" + chip_name + "/nvram/" + cmd + "/" + nvram_board_config)
        nv_common_path = os.path.abspath(aicos_path + "/projects/" + chip_name + "/nvram/" + cmd + "/" + "common")
        binary_path = os.path.abspath(root_path + "/out/" + project_name + "/" + cmd)

        if not os.path.exists(binary_path):
            os.makedirs(binary_path)
        if not os.path.exists(binary_path + "/bin"):
            os.makedirs(binary_path + "/bin")

        os.chdir(aicos_path)

        res = subprocess.call([ntcmd_path,
                              root_path,
                              nv_common_path,
                              nv_board_path,
                              binary_path,
                              cmd,
                              nv_version
                              ])
    if res != 0:
        print("\033[31m-- make nvfile failed {}".format(res))
        exit(1)

#
# aicos aidx build function.
#
def aicos_aidx_build(build_type, scan_path):

    if scan_path == "":
        res = subprocess.call(["python",
                              "-B",
                              "./tools/alog/aidx_str2id.py",
                              "./aicos",
                              ".\\tools\\alog",
                              build_type
                              ])
    else:
        res = subprocess.call(["python",
                              "-B",
                              "./tools/alog/aidx_str2id.py",
                              "./aicos",
                              ".\\tools\\alog",
                              build_type,
                              scan_path
                              ])

    if res != 0:
        print("\033[31m-- build aidx failed {}".format(res))
        exit(1)

# aicos_build
# aicos build function, threadx source code used this function
# to build. For example make.py ap/cp/adsp.
#
def aicos_build(cmd):
    global binary_path
    global common_path
    global script_path
    global board_path

    if cmd == "mkpkg":
        set_subsys_config("ap")
    else:
        set_subsys_config(cmd)

    if cmd == "mkpkg":
        binary_path = os.path.abspath(root_path + "/out/" + project_name + "/")
        sdk_bin_path = os.path.abspath(root_path + "/libs/" + sdk_base_board + "/")
    elif cmd == "cp" and sdk_check_config("ap") == 2 and sdk_check_config("cp") == 0:
        aicos_sign_cp(cmd)
        aicos_nvfile_build(cmd)
        return
    else:
        binary_path = os.path.abspath(root_path + "/out/" + project_name + "/" + cmd)

        if not os.path.exists(binary_path):
            os.makedirs(binary_path)
        if not os.path.exists(binary_path + "/bin"):
            os.makedirs(binary_path + "/bin")

    # process adsp bin
    if cmd == "adsp" :
        ##if need merge the ptcm and dtcm, can use the below code.
        #res = subprocess.call(["python",
        #                      "-B",
        #                      script_path + "/mkpkg/makeadsp.py",
        #                      dst_path + "adspptcm_compressed.bin",
        #                      dst_path + "adspdtcm_compressed.bin",
        #                      dst_path + "adsptcm_compressed.bin"
        #                      ])
        #if res != 0:
        #    print("\033[31m-- makeadsp failed {}".format(res))
        #    exit(1)

        # Generate dtcm compressed binary image
        src     = board_path + "/adsp_dtcm.bin"
        dtcmdst = binary_path + "/bin/adspdtcm_compressed.bin"
        res = subprocess.call(["python",
                              "-B",
                              script_path + "/compression_proc.py",
                              src,
                              dtcmdst
                             ])
        if res != 0:
            print("\033[31m-- make adsp failed {}".format(res))
            exit(1)

        # Generate ptcm compressed binary image
        src     = board_path + "/adsp_ptcm.bin"
        ptcmdst = binary_path + "/bin/adspptcm_compressed.bin"
        res = subprocess.call(["python",
                              "-B",
                              script_path + "/compression_proc.py",
                              src,
                              ptcmdst
                             ])
        if res != 0:
            print("\033[31m-- make adsp failed {}".format(res))
            exit(1)

        # Generate psram dsp compressed binary image
        src = board_path + "/adsp_psram.bin"
        dst = binary_path + "/bin/adspram_compressed.bin"
        res = subprocess.call(["python",
                              "-B",
                              script_path + "/compression_proc.py",
                              src,
                              dst
                             ])
        if res != 0:
            print("\033[31m-- make adsp failed {}".format(res))
            exit(1)
        return

    elif cmd == "mkpkg":
        if subcmd == "user":
            suffix = "release"
        else:
            suffix = "debug"

        pkg_cfg_dir = targets_path  + "/" + chip_name + "/" + project_name + "/pkg"
        for root, dirs, files in os.walk(pkg_cfg_dir):
            for pkg_cfg_name in files:
                pkg_cfg_suffix_name = ""
                rex = re.compile('json$')
                if re.findall(rex, pkg_cfg_name):
                    pos = pkg_cfg_name.find("pkg_cfg")
                    if pos != 0:
                        continue

                    if pkg_cfg_name == "pkg_cfg_ext.json":
                        continue
                    elif pkg_cfg_name != "pkg_cfg.json":
                        pos = pkg_cfg_name.find("pkg_cfg_")
                        if pos != 0:
                            continue
                        pkg_cfg_suffix_name = pkg_cfg_name[len("pkg_cfg_"):-len(".json")] + "_"

                    pkg_cfg_path = os.path.join(root) + "/" + pkg_cfg_name
                    res = subprocess.call(["python",
                                          "-B",
                                          script_path + "/mkpkg/makepkg.py",
                                          project_name,
                                          pkg_cfg_path,
                                          binary_path,
                                          sdk_bin_path,
                                          pkg_cfg_suffix_name + suffix
                                          ])
                    if res != 0:
                        print("\033[31m-- makepkg failed {}".format(res))
                        exit(1)
        return

    if not os.path.exists(binary_path + "/config"):
        os.makedirs(binary_path + "/config")

    os.chdir(aicos_path)

    if not os.path.exists("CMakeLists.txt"):
        print("please run make at CMakelist path!")
        exit(1)

    cfg_file_path = os.path.abspath(targets_path + "/" + chip_name + "/" + project_name)
    out_file_path = os.path.abspath(binary_path + "/config")
    res = subprocess.call(["python",
                          "-B",
                          script_path + "/parse_fmd_partitions.py",
                          cmd,
                          cfg_file_path,
                          out_file_path
                          ])
    if res != 0:
        print("\033[31m-- parse fmd partitions failed {}".format(res))
        exit(1)

    # copy iram/psram layout file to config directory
    copy_target_config_file("psram_mem_config.h")
    copy_target_config_file("aonram_mem_config.h")

    aicos_nvfile_build(cmd)

    if res != 0:
        print("\033[31m-- build nvfile failed {}".format(res))
        exit(1)

    if secure_board == "SEC":
        res = subprocess.call(["cmake",
                        "-B{}".format(binary_path),
                        "-D", "CMAKE_CONFIG_CHIP={}".format(chip_name),
                        "-D", "CMAKE_CONFIG_PROJECT={}".format(project_name),
                        "-D", "CMAKE_CONFIG_BOARD={}".format(board_config),
                        "-D", "CMAKE_SDK_BASE_BOARD={}".format(sdk_base_board),
                        "-D", "CMAKE_SCRIPT_PATH={}".format(script_path),
                        "-D", "CMAKE_SUBSYS_CONFIG={}".format(cmd),
                        "-D", "CMAKE_SW_VERISON={}".format(software_version),
                        "-D", "CMAKE_SECURE_BOARD={}".format(secure_board),
                        "-D", "CMAKE_NVRAM_BOARD={}".format(nvram_board_config),
                        "-GNinja", "."])
    else:
        res = subprocess.call(["cmake",
                    "-B{}".format(binary_path),
                    "-D", "CMAKE_CONFIG_CHIP={}".format(chip_name),
                    "-D", "CMAKE_CONFIG_PROJECT={}".format(project_name),
                    "-D", "CMAKE_CONFIG_BOARD={}".format(board_config),
                    "-D", "CMAKE_SDK_BASE_BOARD={}".format(sdk_base_board),
                    "-D", "CMAKE_SCRIPT_PATH={}".format(script_path),
                    "-D", "CMAKE_SUBSYS_CONFIG={}".format(cmd),
                    "-D", "CMAKE_SECURE_BOARD={}".format(secure_board),
                    "-D", "CMAKE_NVRAM_BOARD={}".format(nvram_board_config),
                    "-GNinja", "."])
    if res != 0:
        print("\033[31m-- make menuconfig cmake failed {}".format(res))
        exit(1)

    res = subprocess.call(["cmake", "--build", "{}".format(binary_path)])
    if res != 0:
        exit(1)

#
# cp_rom_build
# aicos build function, threadx source code used this function
# to build. For example make ap/cp.
#
def cp_rom_build(cmd):
    global binary_path
    sw_version = 0

    cmd = "romcode_cp"

    binary_path = os.path.abspath(root_path + "/out/" + project_name + "/romcode_cp")

    if not os.path.exists(binary_path):
        os.makedirs(binary_path)

    cp_rom_path = os.path.abspath(root_path + "/" + cmd)
    os.chdir(cp_rom_path)

    if not os.path.exists("CMakeLists.txt"):
        print("please run make at CMakelist path!")
        exit(1)

    if not os.path.exists(binary_path + "/config"):
        os.makedirs(binary_path + "/config")

    cfg_file_path = os.path.abspath(targets_path + "/" + chip_name + "/" + project_name)
    out_file_path = os.path.abspath(binary_path + "/config")
    res = subprocess.call(["python",
                    "-B",
                    script_path + "/parse_fmd_partitions.py",
                    cmd,
                    cfg_file_path,
                    out_file_path
                    ])
    if res != 0:
        print("\033[31m-- parse fmd partitions failed {}".format(res))
        exit(1)

    if subcmd == "user":
        sw_version = 1

    res = subprocess.call(["cmake",
                        "-G", "Unix Makefiles",
                        "-S", cp_rom_path,
                        "-B", binary_path,
                        "-D", "TARGET_PROJECT={}".format(cp_board_name),
                        "-D", "SUB_TARGET={}".format(cmd),
                        "-D", "CMAKE_CONFIG_CHIP={}".format(chip_name),
                        "-D", "CMAKE_CONFIG_PROJ={}".format(project_name),
                        "-D", "CMAKE_SUBSYS_CONFIG={}".format(cmd),
                        "-D", "CONFIG_USER_VERSION={}".format(sw_version),
                        "-DCMAKE_BUILD_TYPE=Debug",
                        "-GNinja"])
    if res != 0:
        print("\033[31m-- cp_rom cmake failed {}".format(res))
        exit(1)

    res = subprocess.call(["cmake", "--build", "{}".format(binary_path)])
    if res != 0:
        exit(1)

#
# bl_build
# aicos build function, threadx source code used this function
# to build. For example make ap/cp.
#
def bl_build(cmd):
    global binary_path
    sub_targets = {"bl", "dl", "mbl", "mdl"}
    sw_version = 0

    cmd = "bootloader"
    set_subsys_config(cmd)

    for sub_target in sub_targets :
        binary_path = os.path.abspath(root_path + "/out/" + project_name + "/" + subsys_config + "/" + sub_target)
        if not os.path.exists(binary_path):
            os.makedirs(binary_path)

        bl_path = os.path.abspath(root_path + "/" + subsys_config)
        os.chdir(bl_path)

        if not os.path.exists("CMakeLists.txt"):
            print("please run make at CMakelist path!")
            exit(1)

        if not os.path.exists(binary_path + "/config"):
            os.makedirs(binary_path + "/config")

        cfg_file_path = os.path.abspath(targets_path + "/" + chip_name + "/" + project_name)
        out_file_path = os.path.abspath(binary_path + "/config")
        res = subprocess.call(["python",
                        "-B",
                        script_path + "/parse_fmd_partitions.py",
                        cmd,
                        cfg_file_path,
                        out_file_path
                        ])
        if res != 0:
            print("\033[31m-- parse fmd partitions failed {}".format(res))
            exit(1)

        if subcmd == "user":
            sw_version = 1

        res = subprocess.call(["cmake",
                            "-G", "Unix Makefiles",
                            "-S", bl_path,
                            "-B", binary_path,
                            "-D", "TARGET_PROJECT={}".format(bl_board_name),
                            "-D", "SUB_TARGET={}".format(sub_target),
                            "-D", "CMAKE_CONFIG_CHIP={}".format(chip_name),
                            "-D", "CMAKE_CONFIG_PROJ={}".format(project_name),
                            "-D", "CMAKE_SUBSYS_CONFIG={}".format(cmd),
                            "-D", "CONFIG_USER_VERSION={}".format(sw_version),
                            "-D", "CMAKE_SECURE_BOARD={}".format(secure_board),
                            "-DCMAKE_BUILD_TYPE=Debug",
                            "-GNinja"])
        if res != 0:
            print("\033[31m-- bootloader cmake failed {}".format(res))
            exit(1)

        res = subprocess.call(["cmake", "--build", "{}".format(binary_path)])
        if res != 0:
            exit(1)

#
# kconfig_build
# This function used by aicos, and only aicos support kconfig.
# The build command could be menuconfi/kuconfig/guiconfig or
# menuconfig/kuconfig/guiconfig -s ap/cp.
#
def kconfig_build(subsys, kcfg_cmd):
    global binary_path

    set_subsys_config(subsys)

    binary_path = os.path.abspath(root_path + "/out/" + project_name + "/" + subsys)

    if not os.path.exists(binary_path):
        os.makedirs(binary_path)

    os.chdir(aicos_path)

    if not os.path.exists("CMakeLists.txt"):
        print("please run make at CMakelist path!")
        exit(1)

    res = subprocess.call(["cmake",
                    "-B{}".format(binary_path),
                    "-D", "CMAKE_CONFIG_CHIP={}".format(chip_name),
                    "-D", "CMAKE_CONFIG_BOARD={}".format(board_config),
                    "-D", "CMAKE_SCRIPT_PATH={}".format(script_path),
                    "-D", "CMAKE_SUBSYS_CONFIG={}".format(subsys),
                    "-D", "CMAKE_KCONFIG_BUILD:BOOL=True",
                    "-GNinja", "."])
    if res != 0:
        print("\033[31m-- make menuconfig cmake failed {}".format(res))
        exit(1)

    os.chdir(binary_path)

    if kcfg_cmd == "kuconfig":
        res = subprocess.call(["ninja", "menuconfig"])
    else:
        res = subprocess.call(["ninja", "{}".format(kcfg_cmd)])
    if res != 0:
        print("\033[31m-- make {} failed".format(kcfg_cmd))
        exit(1)

    if kcfg_cmd == "kuconfig":
        # copy default_config file
        source_cfg = os.path.abspath(binary_path + "/.config")
        target_cfg = os.path.abspath(board_path + "/default_config")
        try:
            shutil.copy(source_cfg, target_cfg)
            print("New config copy to file %s" % target_cfg)
        except IOError as e:
            print("Unable to copy file. %s" % e)
        except:
            print("Unexpected error:", sys.exc_info())
            exit(1)

if __name__ == '__main__':
    # Init global board parameters.
    get_board_config()

    # Get build maincmd and subcmd
    get_build_cmd()

    # Set legacy command for next build
    set_legacy_cmd(maincmd)

    # Print start build time
    build_timestamp_start()

    # build ap / cp / dsp or kconfig
    if maincmd == "ap" or maincmd == "cp" or maincmd == "adsp" or maincmd == "mkpkg" :
        if subcmd:
            if subcmd == "nvfile":
                aicos_nvfile_build(maincmd)

            elif subcmd == "mkpkg":
                aicos_build(maincmd)

                aicos_build("mkpkg")
            else:
                print("\033[36m-- make {} aicos kconfig".format(project_name))

                kconfig_build(maincmd, subcmd)

                print("\033[36mkconfig done!")
        else:
            if maincmd == "ap" and ap_board_name.find("fpga") != -1 and adsp_board_name:
                aicos_build("adsp")
            aicos_build(maincmd)

    # build bootloader
    elif maincmd == "bl":

        bl_build(maincmd)
        if subcmd == "mkpkg":
            aicos_build("mkpkg")

    # build cp_rom
    elif maincmd == "cp_rom":

        cp_rom_build(maincmd)

    # build log idx
    elif maincmd == "idx":

        aicos_aidx_build(subcmd, thirdcmd);
    # build all, it will build all the project and generate package
    elif maincmd == "all":
        print("\033[32m-- build {} ap start".format(project_name))
        # fpga should build dsp before ap
        if ap_board_name.find("fpga") != -1 and sdk_check_config("ap") == 1 and adsp_board_name:
            aicos_build("adsp")
        aicos_build("ap")
        print("\033[32m-- build {} ap end".format(project_name))

        print("\033[32m-- build {} cp start".format(project_name))
        aicos_build("cp")
        print("\033[32m-- build {} cp end".format(project_name))

        if sdk_check_config("ap") == 1 and adsp_board_name:
            print("\033[32m-- build {} dsp start".format(project_name))
            aicos_build("adsp")
            print("\033[32m-- build {} dsp end".format(project_name))

        print("\033[32m-- build {} bl start".format(project_name))
        bl_build("bl")
        print("\033[32m-- build {} bl end".format(project_name))
        print("\033[36m-- make all the {} os binary done".format(project_name))

        aicos_build("mkpkg")
    # sdk
    elif maincmd == "sdk":
        sdk_set_config("ap")

        aicos_build("ap")

        sdk_copy_files("ap")

        # if CP support SDK
        if sdk_check_config("cp") == 1:
            sdk_set_config("cp")
            aicos_build("cp")
            sdk_copy_files("cp")

    # clean
    elif maincmd == "clean" or subcmd == "clean":
        print("\033[33m-- clean now")
        if os.path.exists(root_path + "/out"):
            if subcmd == "clean":
                tmp_path = os.path.abspath(root_path + "/out/" + maincmd)
                if os.path.exists(tmp_path):
                    shutil.rmtree(tmp_path, ignore_errors=True)
            else:
                get_board_config()
                subsys_config = get_subsys_config()
                if get_legacy_cmd() == "all":
                    tmp_path = os.path.abspath(root_path + "/out/" + project_name)
                    if os.path.exists(tmp_path):
                        shutil.rmtree(tmp_path, ignore_errors=True)
                else:
                    tmp_path = os.path.abspath(root_path + "/out/" + project_name + "/" + subsys_config)
                    tmp_path = tmp_path.replace('\\', '/')
                    if os.path.exists(tmp_path):
                        shutil.rmtree(tmp_path, ignore_errors=True)
        if sdk_check_config("ap") != 2:
            if os.path.exists(root_path + "/libs"):
                shutil.rmtree(root_path + "/libs/" + sdk_base_board + "/" + subsys_config, ignore_errors=True)

        print("\033[36m-- clean complete")
    # help
    elif maincmd == "-h" or maincmd == "--help":
        print("\033[32m=======================build command help=======================")
        print("\033[36mUsage:./make.py [cmd] [optition]                              --")
        print("\033[36mEnumerate all the used commands:                              --")
        print("\033[36m-- ap               build aicos ap subsys                     --")
        print("\033[36m-- cp               build aicos cp subsys                     --")
        print("\033[36m-- adsp             build aicos adsp subsys                   --")
        print("\033[36m-- bl               build bootloader                          --")
        print("\033[36m-- ap menuconfig    build aicos ap subsys kconfig             --")
        print("\033[36m-- cp menuconfig    build aicos cp subsys kconfig             --")
        print("\033[36m-- ap kuconfig      build aicos ap subsys kconfig             --")
        print("\033[36m--                  and copy generated config para to         --")
        print("\033[36m--                  board default_config file                 --")
        print("\033[36m-- cp kuconfig      build aicos cp subsys kconfig             --")
        print("\033[36m--                  and copy generated config para to         --")
        print("\033[36m--                  board default_config file                 --")
        print("\033[36m-- ap guiconfig     build aicos ap subsys guiconfig           --")
        print("\033[36m-- cp guiconfig     build aicos cp subsys guiconfig           --")
        print("\033[36m-- idx              update the new files and new traces.      --")
        print("\033[36m-- idx file         Only update the new files.                --")
        print("\033[36m-- idx update xxx   make idx only scan path xxx.              --")
        print("\033[36m-- idx file xxx     make idx file only scan path xxx.         --")
        print("\033[36m-- idx rebuild      Rebuild all the idx, be carefure!         --")
        print("\033[36m-- ap nvfile        build aicos ap misc                       --")
        print("\033[36m-- cp nvfile        build aicos cp nvram                      --")
        print("\033[36m-- mkpkg            build package                             --")
        print("\033[36m-- ap mkpkg         build ap and package                      --")
        print("\033[36m-- cp mkpkg         build cp and package                      --")
        print("\033[36m-- all              build all the target of debug version,    --")
        print("\033[36m-- all user         build all the target of user version,     --")
        print("\033[36m-- clean            clean all the output files of this build  --")
        print("\033[36m--                  target.                                   --")
    else:
        print("\033[31mError: Unknown command")
        print("\033[31m./make.py -h to get the command help info")
        exit(1)

    # Print build end time and build time cost
    build_timestamp_end()
