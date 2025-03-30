#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0x410,
        0xfedcba9876543210,
    ]
    i = [
        cpu_encode_op_lib_i64(0,0),
        cpu_encode_op_lib_i64(1,1),
        cpu_encode_op_store_i64(1,0,0),
        cpu_encode_op_break(0),
    ]
    cpu_test("store_1", c, i)

t()
