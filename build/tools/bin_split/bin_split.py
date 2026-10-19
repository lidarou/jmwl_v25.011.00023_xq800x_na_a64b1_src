#!/usr/bin/env python
#-*- coding = utf-8 -*-

import argparse
import os, sys, re, shutil,json
import binascii

# set global path and value
global bin0_start_addr, bin0_end_addr
global bin1_start_addr, bin1_end_addr
global bin2_start_addr, bin2_end_addr


def find_bin_addr():
    global bin1_start_addr, bin1_end_addr
    global bin2_start_addr, bin2_end_addr
    global bin3_start_addr, bin3_end_addr

    f=open(map_file, "r", encoding='utf-8')

    for line in f.readlines():
        if line.find("__bin1_start__") >= 0:
            if bin1_start_addr == 0:
                data_split = line.split()
                value = data_split[0]
                bin1_start_addr = int(value, 16)

        if line.find("__bin1_end__") >= 0:
            if bin1_end_addr == 0:
                data_split = line.split()
                value = data_split[0]
                bin1_end_addr = int(value, 16)

        if line.find("__bin2_start__") >= 0:
            if bin2_start_addr == 0:
                data_split = line.split()
                value = data_split[0]
                bin2_start_addr = int(value, 16)

        if line.find("__bin2_end__") >= 0:
            if bin2_end_addr == 0:
                data_split = line.split()
                value = data_split[0]
                bin2_end_addr = int(value, 16)

        if line.find("__bin3_start__") >= 0:
            if bin3_start_addr == 0:
                data_split = line.split()
                value = data_split[0]
                bin3_start_addr = int(value, 16)

        if line.find("__bin3_end__") >= 0:
            if bin3_end_addr == 0:
                data_split = line.split()
                value = data_split[0]
                bin3_end_addr = int(value, 16)
                break

    f.close()

def split_binary():
    bin1_size = bin1_end_addr - bin1_start_addr
    bin2_size = bin2_end_addr - bin2_start_addr
    bin3_size = bin3_end_addr - bin3_start_addr

    f_origin = open(origin_bin, "rb")
    f_bin1 = open(bin1, "wb")
    f_bin2 = open(bin2, "wb")
    f_bin3 = open(bin3, "wb")

    stats = os.stat(origin_bin)

    temp = f_origin.read(bin1_size)
    f_bin1.write(temp)

    temp = f_origin.read(bin2_size)
    f_bin2.write(temp)

    temp = f_origin.read(bin3_size)
    f_bin3.write(temp)

    f_bin1.close()
    f_bin2.close()
    f_bin3.close()
    f_origin.close()


if __name__ == '__main__':
    root_path = sys.argv[1]
    map_file = os.path.abspath(root_path + r"\bin\aicos_cp.map")
    origin_bin = os.path.abspath(root_path + r"\bin\aicos_cp.bin")
    bin1 = os.path.abspath(root_path + r"\ATCM")
    bin2 = os.path.abspath(root_path + r"\CODE_2")
    bin3 = os.path.abspath(root_path + r"\MCX_CODE")

    bin1_start_addr = 0
    bin1_end_addr = 0
    bin2_start_addr = 0
    bin2_end_addr = 0
    bin3_start_addr = 0
    bin3_end_addr = 0

    find_bin_addr()

    split_binary()
