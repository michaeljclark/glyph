#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0,
        0x400,
        0xfedcba9876543210,
    ]
    i = [
        cpu_encode_op_lib_i64(0,1),
        cpu_encode_op_lib_i64(1,2),
        cpu_encode_op_store_i64(1,0,0x18),
        cpu_encode_op_lib_i64(2,3),
        cpu_encode_op_break(0),
    ]
    cpu_test("store_1", c, i)

t()
