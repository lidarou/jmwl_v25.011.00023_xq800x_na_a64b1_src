#!/usr/bin/env python
#-*- coding = utf-8 -*-

import argparse
import os, sys, re, shutil,json
import openpyxl
import subprocess
from colorama import init
init(autoreset=True)

# set global path and value
root_path = os.path.abspath(sys.path[0])
aicos_path = os.path.abspath(root_path + "/aicos")
build_info_file = os.path.abspath(root_path + "/out/build_info.txt")
board_info_path = os.path.abspath(root_path + r"/targets")
global board_info_file
global chip_name, project_name, sdk_base_board
global ap_board_name, cp_board_name, bl_board_name, tfm_board_name, software_version, comments, security
global md_board_name, nv_board_name

# read json file data
def read_json_file(input_file):
    try:
        with open(input_file, 'r') as input_f:
            json_data = json.load(input_f)
            input_f.close()
            return json_data
    except:
            print(input_file + " not find or load failed!")
            sys.exit(-1)

#
# Create build info file
# It is used to store board build info, e.g board config, board name.
#
def create_build_info():
    if not os.path.exists("out/" + project_name):
        os.makedirs("out/" + project_name)

    f=open(build_info_file, "w+")
    f.write("/* store aic board build info */\n")
    f.write("CMAKE_CONFIG_CHIP:STRING=")
    f.write(chip_name + "\n")
    f.write("CMAKE_CONFIG_PROJ_NAME:STRING=")
    f.write(project_name + "\n")
    f.write("CMAKE_CONFIG_AP_BOARD_NAME:STRING=")
    f.write(ap_board_name + "\n")
    f.write("CMAKE_CONFIG_SDK_BASE_BOARD:STRING=")
    f.write(sdk_base_board + "\n")
    f.write("CMAKE_CONFIG_MD_BOARD_NAME:STRING=")
    f.write(md_board_name + "\n")
    f.write("CMAKE_CONFIG_CP_BOARD_NAME:STRING=")
    f.write(cp_board_name + "\n")
    f.write("CMAKE_CONFIG_NV_BOARD_NAME:STRING=")
    f.write(nv_board_name + "\n")
    f.write("CMAKE_CONFIG_ADSP_BOARD_NAME:STRING=")
    f.write(dsp_board_name + "\n")
    f.write("CMAKE_CONFIG_BL_BOARD_NAME:STRING=")
    f.write(bl_board_name + "\n")
    f.write("CMAKE_CONFIG_TFM_BOARD_NAME:STRING=")
    f.write(tfm_board_name + "\n")
    f.write("AIC_SW_VERISON:VALUE=")
    f.write(software_version + "\n")
    f.write("AIC_SECURITY:VALUE=")
    f.write(security + "\n")
    f.close()

def board_name_check(name):
    data = read_json_file(board_info_file)
    info = data["Board info"]

    for chip_name, chip_value in info.items():
        for board_info in chip_value:
            for title_name, title_value in board_info.items():
                if title_name == "project board name" and title_value == name:
                    return True

    return False

def board_check():
    global board_info_file
    board_index = 0

    for (dirpath, dirname, filenames) in os.walk(board_info_path):
        for searched_file in filenames:
            if searched_file == "board_info.json":
                board_info_file = os.path.abspath(dirpath + r"/" + searched_file)
                if sys.argv[1].isnumeric():
                    if eval(sys.argv[1]) == board_index:
                        return True
                else:
                    if board_name_check(sys.argv[1]) == True:
                        return True
                board_index += 1

    return False

if __name__ == '__main__':
    #
    # lunch command.
    # It supports only one paramater.
    #
    if not len(sys.argv) == 2 or board_check() == False:
        print("\033[31mlunch command error!")
        print("\033[31mPlease select correct board!")
        exit(1)

    data = read_json_file(board_info_file)
    info = data["Board info"]

    for chip_key, chip_value in info.items():
        chip_name = chip_key
        board_cfg_path = os.path.abspath(aicos_path + "/projects/" + chip_name)
        for board_info in chip_value:
            project_name    = board_info["project board name"]
            ap_board_name   = board_info["ap board name"]
            if "sdk base board name" in board_info:
                sdk_base_board  = board_info["sdk base board name"]
            else:
                sdk_base_board = project_name
            md_board_name   = board_info["md board name"]
            cp_board_name   = board_info["cp board name"]
            nv_board_name   = board_info["nv board name"]
            if "dsp board name" in board_info:
                dsp_board_name  = board_info["dsp board name"]
            else:
                dsp_board_name = ""
            bl_board_name   = board_info["bootloader board name"]
            tfm_board_name  = board_info["tfm board name"]
            software_version= board_info["SW version"]
            comments        = board_info["Comments"]
            if "security" in board_info:
                security        = board_info["security"]
            else:
                security = "NSEC"
            if re.findall("sec", ap_board_name):
                security = "SEC"

    # board invalid check
    if (not os.path.exists(root_path + r"/targets/" + chip_name + r"/" + project_name)) or\
        (not os.path.exists(board_cfg_path + r"/ap/" + ap_board_name)) or\
        (os.path.exists(board_cfg_path + r"/cp") and (not os.path.exists(board_cfg_path + r"/cp/" + cp_board_name))):
        print("\033[31mBoard file deficiency, please check!")
        exit(1)

    create_build_info()

    print("\033[1;33m===========================  aic lunch boards  ===========================\033[0m")
    print("\033[1;32m-- lunched: {}".format(project_name))
    print("\033[36m--  |")
    print("\033[36m-- board info:")
    print("\033[36m--            chip        -> {}".format(chip_name))
    print("\033[36m--            ap board    -> {}".format(ap_board_name))
    print("\033[36m--            md board    -> {}".format(md_board_name))
    print("\033[36m--            cp board    -> {}".format(cp_board_name))
    print("\033[36m--            nv board    -> {}".format(nv_board_name))
    if not dsp_board_name == "":
        print("\033[36m--            dsp board   -> {}".format(dsp_board_name))
    print("\033[36m--            bl board    -> {}".format(bl_board_name))
    print("\033[36m--            tfm board   -> {}".format(tfm_board_name))
    print("\033[36m--            sw version  -> {}".format(software_version))
    print("\033[36m--            information :  {}".format(comments))
    print("\033[36m--            security :  {}".format(security))
