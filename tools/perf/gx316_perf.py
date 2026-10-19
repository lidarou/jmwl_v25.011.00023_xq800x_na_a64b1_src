#!/usr/bin/env python
#-*- coding = utf-8 -*-
import os, sys, shutil, xlrd, xlwt
import subprocess, fileinput

root_path = os.path.abspath(sys.path[0])
perf_data_temp = os.path.abspath(root_path + "/perf_temp.txt")
global perf_data_file
global perf_data_output
global perf_data
global perf_temp
global perf_book
global perf_sheet
global bw_val
global bw_row


class perf_chn_info:
    def __init__(self):
        self.count = 6
        self.list = ['CP Read', 'CP Write', 'AP Read', 'AP Write', 'Modem Read', 'Modem Write']
        self.values = [0,0,0,0,0,0]

def perf_cfg_output_file(name):
    global perf_data_file
    global perf_data_output

    file_name = os.path.splitext(name)[0]
    perf_data_file   = os.path.abspath(root_path + r"/" + name)
    perf_data_output = os.path.abspath(root_path + r"/" + file_name + r".xls")

def perf_output_chn_info():
    chn_info = perf_chn_info()
    col_cnt = 0

    for info_name in chn_info.list:
        perf_sheet.write(0, col_cnt, "{}".format(info_name))
        col_cnt += 1
    perf_sheet.write(0, col_cnt, "Sum")

def perf_gen_temp_file():
    perf_data = open(perf_data_file, "r", encoding='utf-8')
    perf_temp = open(perf_data_temp, "w+")

    for line in perf_data.readlines():
        if line.find(r"perf monitor: ") >= 0:
            perf_temp.write(line)

    perf_data.close()
    perf_temp.close()

def perf_output_bandwidth_val():
    global bw_val
    global bw_row

    bw_col = 0
    sum_val = 0

    for val in bw_val.values:
        rev_val = int(val)
        perf_sheet.write(bw_row, bw_col, "{}".format(rev_val))
        bw_col += 1
        sum_val += rev_val
    perf_sheet.write(bw_row, bw_col, "{}".format(sum_val))
    bw_row += 1

def perf_output_bandwidth():
    global bw_val
    global bw_row
    global bw_col

    bw_row = 1
    bw_val = perf_chn_info()
    perf_temp = open(perf_data_temp, "r")
    line_cnt = 0

    for line in perf_temp.readlines():
        perf_split = line.split()
        # find the first perf channel
        if line.find(bw_val.list[0]) >= 0:
            line_cnt = 0x55acce55
            bw_val.values[0] = perf_split[-1]
            bw_val.values[1] = 0
            bw_val.values[2] = 0
            bw_val.values[3] = 0
            bw_val.values[4] = 0
            bw_val.values[5] = 0
            continue

        if line_cnt >= 0x55acce55:
            line_cnt += 1
            for n in range(1, bw_val.count):
                if line.find(bw_val.list[n]) >= 0:
                    bw_val.values[n] = perf_split[-1]

            # find the first perf channel, means that the log is missed
            if line.find(bw_val.list[0]) >= 0:
                line_cnt = 0x55acce55
                bw_val.values[0] = perf_split[-1]
                bw_val.values[1] = 0
                bw_val.values[2] = 0
                bw_val.values[3] = 0
                bw_val.values[4] = 0
                bw_val.values[5] = 0
                continue

            # find the last perf channel
            if line.find(bw_val.list[bw_val.count - 1]) >= 0:
                line_cnt -= 0x55acce55
                # check the perf channel integrality
                if not (line_cnt % (bw_val.count - 1)):
                    line_cnt = 0
                    perf_output_bandwidth_val()

    perf_temp.close()


if __name__ =='__main__':
    # check input param
    if not len(sys.argv) == 2:
        print("perf input param error!")
        print("Please input perf data file name!")
        exit(1)

    perf_cfg_output_file(sys.argv[1])

    perf_book = xlwt.Workbook(encoding='utf-8')
    perf_sheet = perf_book.add_sheet('Performance')

    perf_output_chn_info()

    perf_gen_temp_file()

    perf_output_bandwidth()

    perf_book.save(perf_data_output)
