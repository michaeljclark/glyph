#!/usr/bin/env python3

from interp import *

def t():
    c = [
        8,
        -1,
    ]
    i = [
        cpu_encode_op_ibl(0,0),
        cpu_encode_op_lib_i64(0,0),
        cpu_encode_op_break(0)
    ]
    cpu_test("ibl_1", c, i)

t()
