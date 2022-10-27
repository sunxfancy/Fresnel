import sys
import os
import re

pattern = "sptr<([A-Za-z][A-Za-z0-9_]*)>"
prog = re.compile(pattern)

def gen_code(class_name, record):
    begin = "\nvoid %s::DepsOn(DI* di) {\n" % class_name
    end = "}\n"
    ret = begin
    for type, name in record:
        ret += "\tDEPS_REG(%s, %s);\n" % (type, name)
    ret += end
    return ret

headers = []
functions = []

work_dir = sys.argv[1]
for parent, dirnames, filenames in os.walk(work_dir, followlinks=False):
        for filename in filenames:
            file_path = os.path.join(parent, filename)
            if filename == 'di.hpp': continue
            # print('文件名：%s' % filename)
            print('扫描 %s' % file_path)
            with open(file_path, 'r') as file:
                start_record = False
                class_name = ""
                record_list = []
                for line in file:
                    l = line.strip()
                    if start_record:
                        if l == "":
                            start_record = False
                            relpath = os.path.relpath(file_path, work_dir)
                            headers.append(relpath.replace("\\", "/"))
                            functions.append(gen_code(class_name, record_list))
                            continue
                        words = line.split()
                        m = prog.match(words[0])
                        if m:
                            type = m.group(1)
                            name = words[1].rstrip(';')
                            record_list.append((type, name))
                    if l == "DEPS_ON":
                        start_record = True
                        record_list = []
                    else:
                        if l != "":
                            words = line.split()
                            if words[0] == "class":
                                class_name = words[1]

with open(sys.argv[2], 'w') as out:
    for h in headers:
        out.write("#include \"%s\"\n" % h)
    out.write("\nnamespace fl {\n")
    for f in functions:
        out.write(f)
    out.write("\n}\n")