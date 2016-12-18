#!/usr/bin/env python3

import re

def get_defines(ver=''):
    regexes = [r'#define PCRE%s_(\w+)\s*0x'%ver,
               r'#define PCRE%s_(INFO_\w+)\s*\d'%ver,
               r'#define PCRE%s_(CONFIG_\w+)\s*\d'%ver]
    defines = dict()
    for i in range(len(regexes)):
        defines[i] = set()

    with open('/usr/include/pcre%s.h'%ver,'r') as fp:
        for line in fp:
            for (i, regex) in enumerate(regexes):
                m = re.match(regex, line)
                if m:
                    break
            if m:
                defines[i].add(m.group(1))
    return defines


pcre_defines = get_defines()
pcre2_defines = get_defines('2')

for i in range(len(pcre_defines)):
    for define in sorted(pcre_defines[i] & pcre2_defines[i]):
        print('#define AG_PCRE_%-20s AG_PCRE_PREFIX(%s)'%(define, define))
