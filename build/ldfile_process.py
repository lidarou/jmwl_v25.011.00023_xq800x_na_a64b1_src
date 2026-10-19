#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys

if __name__ == '__main__':
    input_path = sys.argv[1]
    output_path =sys.argv[2]

    if not os.path.exists(input_path):
        print(input_path + " file is not exit!\n")
        exit(1)

    with open(output_path, 'w') as out_f:
        start_write = 0

        with open(input_path, 'r') as input_f:
            for line in input_f.readlines():
                if line.startswith("MEMORY"):
                    start_write = 1
                elif line.startswith("LOAD"):
                    start_write = 1

                if start_write:
                    if not line.startswith("#"):
                        out_f.write(line)
