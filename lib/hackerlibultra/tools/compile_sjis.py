#!/usr/bin/env python3

import sys, os
from shiftjis_conv import sjis_process

WORKING_DIR = os.getcwd()

fb = []
input_c_file = [i for i in sys.argv if ".c" in i][0]
CC = [i for i in sys.argv if "-D__CC=" in i][0][7:]
build_dir = [i for i in sys.argv if "-D__BUILD_DIR" in i][0][14:]


output_c_file = f"{build_dir}/{input_c_file}"

# Edit compile command to point to the converted file
sys.argv[sys.argv.index(input_c_file)] = output_c_file

with open(input_c_file) as f:
	fb = f.read()

with open(output_c_file, "w+") as outf:
	sjis_process(fb, outf)

os.system("%s %s" % (CC, " ".join(sys.argv[1:])))
