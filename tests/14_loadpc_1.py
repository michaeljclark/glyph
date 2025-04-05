#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0xfedcba9876543210,
        0xfffffc00,
    ]
    i = [
        cpu_encode_op_loadpc_i64(0,2),
        cpu_encode_op_break(0),
    ]
    cpu_test("loadpc_1", c, i)

t()
