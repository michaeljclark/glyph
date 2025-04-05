#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0xfedcba9876543210,
        0xfffffc0e,
    ]
    i = [
        cpu_encode_op_movib_i64(0,0),
        cpu_encode_op_storepc_i64(0,2),
        cpu_encode_op_movib_i64(1,2),
        cpu_encode_op_break(0),
    ]
    cpu_test("storepc_1", c, i)

t()
