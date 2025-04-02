#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0xffffffffffffffff,
    ]
    i = [
        cpu_encode_op_ibj(64),
        cpu_encode_op_lib_i64(0,0),
        cpu_encode_op_break(0),
    ]
    cpu_test("ibl_1", c, i)

t()
