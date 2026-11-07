#!/usr/bin/env python3
#coding=utf8

from optparse import OptionParser
import shutil
import os
import subprocess

svg2eps = 'inkscape %s -C --export-filename=%s.eps --export-type=eps --export-latex'
svg2pdf = 'inkscape %s -C --export-filename=%s.pdf --export-type=pdf --export-latex'

def create_image(input_filename, output_filename, mode, ext):
    subprocess.Popen(mode % (input_filename, output_filename),
        stdout=subprocess.PIPE, shell=True).communicate()[0]

    o_ext = output_filename + '.' + ext
    o = output_filename
    o_ext_tex = output_filename + '.' + ext + '_tex'
    o_t = output_filename + '_t'

    shutil.move(o_ext, o)

    fin = open(o_ext_tex, 'r')
    fout = open(o_t, 'w')

    #\includegraphics[width=\unitlength,page=1]{logo.pdftex}
    for line in fin:
        # FIXME: be more conservative in the replacement
        # (in case '{'+o_ext+'}' appeares somewhere else)
        out = line.replace('{'+os.path.basename(o_ext)+'}', '{'+os.path.basename(o)+'}')
        fout.write(out)

    fin.close()
    fout.close()
    os.remove(o_ext_tex)

def main():
    parser = OptionParser()
    parser.add_option("-L", "--format", dest="outputFormat",
        metavar="FORMAT", help="output format", default="spstex")
    parser.add_option("-p", "--portrait", dest="portrait", help="dummy arg")
    (options, args) = parser.parse_args()
    if len(args) != 2: return
    (input_filename, output_filename) = args
    fmt = options.outputFormat
    portrait = options.portrait

    if fmt == 'eps':
        create_image(input_filename, output_filename, svg2eps, 'eps')
    elif fmt == 'spstex' or fmt == 'pstex':
        create_image(input_filename, output_filename, svg2eps, 'eps')
    elif fmt == 'spstex_t' or fmt == 'pstex_t':
        pass
    elif fmt == 'spdftex' or fmt == 'pdftex':
        create_image(input_filename, output_filename, svg2pdf, 'pdf')
    elif fmt == 'spdftex_t' or fmt == 'pdftex_t':
        pass

if __name__ == "__main__":
    main()

