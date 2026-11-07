#!/usr/bin/env python3
#coding=utf8

"""

stdin : the original LaTeX log file
stdout : the output filtered log file

"""

from __future__ import print_function
import optparse
import os.path
import re
import sys
import io

def main():
    parser = optparse.OptionParser()
    (options, args) = parser.parse_args()

    display = 0
    in_display = 0
    start_line = ''
    warnerror_re = re.compile(r"^(LaTeX|Package|Class)( (.*))? (Warning:|Error:)")
    fullbox_re = re.compile(r"^(Underfull|Overfull) \\[hv]box")
    accu = ''
    # PDFLaTeX log file is not really in latin-1 (in T1 more exactly)
    # but all bytes are corrects in latin-1, so python won't stop
    # while parsing log.
    # Without specifying this encoding (ie using default utf-8), we
    # can get decode errors (UnicodeDecodeError: 'utf-8' codec can't decode byte...)
    with io.open(sys.stdin.fileno(),'r',encoding='latin-1') as sin:
        for line in sin:
            if display > 0:
                display -= 1
            if line[0:4].lower() in ('info', 'warn') or line[0:5].lower() == 'error':
                display = 0
            line_groups = warnerror_re.match(line)
            if line_groups:
                start_line = line_groups.group(3)
                if not start_line:
                    start_line = ''
                if line_groups.group(2):
                    start_line = "(" + start_line + ")"
                display = 1
                in_display = 1
            elif (start_line != '') and (line[0:len(start_line)] == start_line):
                display = 1
            elif line == "\n":
                in_display = 0
            elif line[0:4] == 'Chap':
                display = 1
            elif fullbox_re.match(line):
                display = 2
            if display:
                print(accu, end="")
                accu = line
            elif in_display:
                print(accu[0:-1], end="")
                accu = line

if __name__ == "__main__":
    main()

