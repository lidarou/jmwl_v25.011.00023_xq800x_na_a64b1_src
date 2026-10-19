#!/usr/bin/env python
#/****************************************************************************/
#/*                                                                          */
#/*      Copyright 2021 by AIC Corporation. All rights reserved.             */
#/*                                                                          */
#/****************************************************************************/

import os, sys
import shutil

def merge_dir(source_dir, destination_dir):
    if not os.path.exists(destination_dir):
        #print(destination_dir)
        os.makedirs(destination_dir)

    for file in os.listdir(source_dir):
        source_file = os.path.join(source_dir, file)
        destination_file = os.path.join(destination_dir, file)
       # print(source_file)
        if not os.path.isfile(source_file) :
            merge_dir(source_file, destination_file)
        else :
            if os.path.exists(destination_file):
                os.remove(destination_file)
            shutil.copy2(source_file, destination_dir)

def main():
    if len(sys.argv) < 2 :
        return

    merge_dir(sys.argv[1], sys.argv[2])

if __name__ == '__main__':
    main()