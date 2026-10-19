#/****************************************************************************/
#/*                                                                          */
#/*      Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.             */
#/*                                                                          */
#/****************************************************************************/

import sys

def main():
    binListData = []
    # 读取二进制文件存放到list列表中
    file = open("D:/workspace/test/pub_key.pem", 'rb')
    file.seek(0, 0)
    while True:
        binByte = file.read(1)
        if len(binByte) == 0:
            break
        else:
            binListData.append("0x%.2x" % ord(binByte))
    file.close()

    # 将列表中的数据写入到 .c 源文件中
    fileOutput = open("D:/workspace/test/pubkey.h", 'w')
    fileOutput.write("unsigned long hexLength = {};\n".format(len(binListData)))
    fileOutput.write("unsigned char hexData[] = \n")
    fileOutput.write("{\n")
    for i in range(len(binListData)):
        if (i != 0) and (i % 16 == 0):
            fileOutput.write("\n")
            fileOutput.write(binListData[i] + ",")
        elif (i + 1) == len(binListData):
            fileOutput.write(binListData[i])
        else:
            fileOutput.write(binListData[i] + ",")
    fileOutput.write("\n};")
    fileOutput.close()
    print("bin file to C array file success!!!")

if __name__ == '__main__':
    main()