#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0x408,
        0xfedcba9876543210,
    ]
    i = [
        cpu_encode_op_lib_i64(0,0),
        cpu_encode_op_load_i64(1,0,0),
        cpu_encode_op_break(0)
    ]
    cpu_test("load_1", c, i)

t()
