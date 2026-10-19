import re
import sys
from prettytable import PrettyTable

class Node(object):
    def __init__(self, name, text, bss, data, rodata, pnext=None):
        self.name = name
        self.text = text
        self.bss = bss
        self.data = data
        self.rodata = rodata
        self._next = pnext

    def __repr__(self):
        return str(self.data)

class lib_link_list_operate(object):
    def __init__(self):
        self.head = None
        self.length = 0
        self.table = PrettyTable(["libname", "text", "bss", "data", "rodata","RO(text+rodata)","RW(bss+data)","lib_total"])
        self.text_total = 0
        self.bss_total = 0
        self.data_total = 0
        self.rodata_total = 0
        self.read_only_total = 0
        self.rw_total = 0
        self.lib_total = 0
        self.table.align["libname"] = "l"
        self.table.align["text"] = "r"
        self.table.align["bss"] = "r"
        self.table.align["data"] = "r"
        self.table.align["rodata"] = "r"
        self.table.align["RO(text+rodata)"] = "r"
        self.table.align["RW(bss+data)"] = "r"
        self.table.align["lib_total"] = "r"

    def isEmpty(self):
        return (self.length == 0)

    def append(self, Node):
        item = None

        if self.isEmpty():
            self.head = Node
            self.length += 1
            return True
        else:
            node = self.head
            while node._next:
                node = node._next
            node._next = Node
            self.length += 1
            return True

    def update(self, text, bss, data, rodata, index):
        if self.isEmpty():
            return False
        if index > self.length or index < 0:
            return False

        node = self.head
        i = 0
        while node:
            if i == index:
                node.text += text
                node.bss += bss
                node.data += data
                node.rodata += rodata
                return True
            i += 1
            node = node._next

    def find(self, name):
        index = 0
        if self.isEmpty():
            return -1
        node = self.head

        while node:
            if name == node.name:
                return index
            node = node._next
            index += 1
        else:
            return -1

    def show(self):
        if self.isEmpty():
            print('map lib list is empty')
        else:
            node = self.head
            while node:
                self.table.add_row([node.name, node.text, node.bss, node.data, node.rodata, node.text+node.rodata, node.bss+node.data, node.text+node.bss+node.data+node.rodata])
                self.text_total += node.text
                self.bss_total += node.bss
                self.data_total += node.data
                self.rodata_total += node.rodata
                node = node._next
            self.table.add_row(["Total Size:", self.text_total, self.bss_total, self.data_total, self.rodata_total,
                                self.text_total+self.rodata_total, self.bss_total+self.data_total,
                                self.text_total+self.bss_total+self.data_total+self.rodata_total])
            print(self.table)

    def clear(self):
        self.head = None
        self.length = 0

    def save_table(self, map_name):
        table_string = self.table.get_string()
        textfile = open(map_name, 'a')
        textfile.write(table_string)
        textfile.close()

def analyse_map(map_file):
    with open(map_file, "r") as mf:
        bss_flag = ".bss"
        data_flag = ".data"
        text_flag = ".text"
        rodata_flag = ".rodata"
        start_line_flag = "Memory Configuration"
        start_flag = None
        bss_total = 0x0
        data_total = 0x0
        text_total = 0x0
        rodata_total = 0x0
        prev_lib = None
        prev_line = None
        line_counter = 0
        lib_list = lib_link_list_operate()

        for l in mf.readlines():
            line_counter += 1
            if ( (start_flag == None) and (start_line_flag not in l)):
                continue
            else:
                start_flag = "start"

            if ("SORT_BY_ALIGNMENT" in l):
                continue

            if prev_line == None:
                prev_line = l

            cur_lib = re.findall(r'\blib[A-Za-z0-9_]*.a', l, re.I)

            if (cur_lib):
                if((bss_flag in l) or (data_flag in l) or (text_flag in l) or (rodata_flag in l)):
                    op_line = l
                elif( (bss_flag in prev_line) or (data_flag in prev_line) or (text_flag in prev_line) or (rodata_flag in prev_line)):
                    op_line = prev_line
                else:
                    prev_line = l
                    continue
            else:
                prev_line = l
                continue

            cur_num_res = re.findall(r'0x[0-9A-F]+', l, re.I)
            if (cur_num_res):
                cur_num = cur_num_res[1]
            else:
                prev_line = l
                continue

            #add for remove none node
            if prev_lib == None:
                prev_lib = cur_lib

            if cur_lib == prev_lib:
                if text_flag in l or ((op_line == prev_line) and (text_flag in prev_line)):
                    text_total += int(cur_num, 16)
                if data_flag in l or ((op_line == prev_line) and (data_flag in prev_line)):
                    data_total += int(cur_num, 16)
                if bss_flag in l or ((op_line == prev_line) and (bss_flag in prev_line)):
                    bss_total += int(cur_num, 16)
                if rodata_flag in l or ((op_line == prev_line) and (rodata_flag in prev_line)):
                    rodata_total += int(cur_num, 16)
            else:
                index = lib_list.find(prev_lib)
                if index >= 0:
                    lib_list.update(text_total,bss_total,data_total,rodata_total,index)
                else:
                    item = Node(prev_lib,text_total,bss_total,data_total,rodata_total)
                    lib_list.append(item)

                #clear prev lib total counter
                prev_lib = cur_lib
                text_total = 0
                data_total = 0
                bss_total = 0
                rodata_total = 0

                #after clear prev lib,new counter for current lib
                if text_flag in l or ((op_line == prev_line) and (text_flag in prev_line)):
                    text_total += int(cur_num, 16)
                if data_flag in l or ((op_line == prev_line) and (data_flag in prev_line)):
                    data_total += int(cur_num, 16)
                if bss_flag in l or ((op_line == prev_line) and (bss_flag in prev_line)):
                    bss_total += int(cur_num, 16)
                if rodata_flag in l or ((op_line == prev_line) and (rodata_flag in prev_line)):
                    rodata_total += int(cur_num, 16)
            prev_line = l

        index = lib_list.find(prev_lib)
        if index >= 0:
            lib_list.update(text_total, bss_total, data_total, rodata_total, index)
        else:
            item = Node(prev_lib, text_total, bss_total, data_total, rodata_total)
            lib_list.append(item)

        lib_list.show()
        lib_list.save_table(map_file)

if __name__ == '__main__':
    analyse_map(sys.argv[1])