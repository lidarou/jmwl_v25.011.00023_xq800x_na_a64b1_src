import re
import sys
import os

def update_jenkins_and_version_num(jenkins_version, file_path, rel_num_add):
    new_file_content = ''
    with open(file_path, "r") as file:
        for line in file:
            if ("#define" in line) and ("AIC_RELEASE_NUM" in line) and rel_num_add:
                ops_temp = line
                pattern = r'\"([^\"]+)\"'
                matches = re.findall(pattern, ops_temp)
                for match in matches:
                    number = int(match) + 1
                    new = str("{:0>5}".format(number))
                    new_file_content += ops_temp.replace('"{}"'.format(match),'"{}"'.format(new))
            elif ("#define" in line) and ("AIC_BUILD_NUMBER" in line):
                ops_temp = line
                pattern = r'\"([^\"]+)\"'
                matches = re.findall(pattern, ops_temp)
                for match in matches:
                    new_file_content += ops_temp.replace('"{}"'.format(match),'"{}"'.format(jenkins_version))
            else:
                new_file_content += line

    with open(file_path, "w") as file:
        file.write(new_file_content)

def aic_version_ops(jenkins_version, project_list, rel_num_add):
    print("jenkins build version:" + jenkins_version)
    total_changed_list = []
    for item in project_list.split(','):
        chip = "genesis"
        path = "../aicos/projects/"+chip+"/ap/" + item
        if not os.path.exists(path):
            chip = "banyan"

        path = "../aicos/projects/"+chip+"/ap/" + item + "/config/aic_version.c"

        update_jenkins_and_version_num(jenkins_version, path, rel_num_add)
        cur_file = chip+"/ap/" + item + "/config/aic_version.c"
        total_changed_list.append(cur_file)
    print(total_changed_list)
    return total_changed_list

if __name__ == '__main__':
    aic_version_ops(sys.argv[1], sys.argv[2], bool(eval(sys.argv[3])))