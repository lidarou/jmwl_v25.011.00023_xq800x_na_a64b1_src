#!/usr/bin/env python
#-*- coding = utf-8 -*-

import os, sys, re, shutil,json
from colorama import init
init(autoreset=True)

# set global path and value
root_path = os.path.abspath(sys.path[0] + r"/../")
aicos_path = os.path.abspath(root_path + r"/aicos")
board_info_path = os.path.abspath(root_path + r"/targets")
board_info_file = os.path.abspath(root_path + r"/targets/board_info.json")
board_index = 0

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

def copy_build_files():
    shutil.copy(os.path.abspath(root_path + "/build/make.py"),
                os.path.abspath(root_path + "/make.py"))
    shutil.copy(os.path.abspath(root_path + "/build/lunch.py"),
                os.path.abspath(root_path + "/lunch.py"))
    shutil.copy(os.path.abspath(root_path + "/build/cmake/CMakeLists.txt"),
                os.path.abspath(aicos_path + "/CMakeLists.txt"))
    shutil.copy(os.path.abspath(root_path + "/build/kconfig/Kconfig"),
                os.path.abspath(aicos_path + "/Kconfig"))

def analysis_board_info():
    global board_index

    data = read_json_file(board_info_file)
    info = data["Board info"]

    for chip_name, chip_value in info.items():
        board_cfg_path = os.path.abspath(aicos_path + "/projects/" + chip_name)
        for board_info in chip_value:
            for title_name, title_value in board_info.items():
                if title_name == "project board name":
                    board_name = title_value
                elif title_name == "ap board name":
                    ap_board = title_value
                elif title_name == "md board name":
                    md_board = title_value
                elif title_name == "cp board name":
                    cp_board = title_value
                elif title_name == "nv board name":
                    nv_board = title_value

                elif title_name == "Comments":
                    if (not os.path.exists(root_path + r"/targets/" + chip_name + r"/" + board_name)) or\
                        (not os.path.exists(board_cfg_path + r"/ap/" + ap_board)) or \
                        (os.path.exists(board_cfg_path + r"/cp") and (not os.path.exists(board_cfg_path + r"/cp/" + cp_board))):
                        break
                    print("\033[1;32m-- %2d %-36s\033[0m  \033[36m/* %s */\033[0m " %(board_index, board_name, title_value))
                    board_index += 1


if __name__ == '__main__':

    copy_build_files()

    print("\033[1;33m==========================  aic support boards  ==========================")

    for (dirpath, dirname, filenames) in os.walk(board_info_path):
        for searched_file in filenames:
            if searched_file == "board_info.json":
                board_info_file = os.path.abspath(dirpath + r"/" + searched_file)
                analysis_board_info()
