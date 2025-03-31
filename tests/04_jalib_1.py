#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0x0000000800000002,
        0xffffffffffffffff,
    ]
    i = [
        cpu_encode_op_jalib(7,0),
        cpu_encode_op_nop(0),
        cpu_encode_op_lib_i64(0,0),
        cpu_encode_op_break(0),
    ]
    cpu_test("jalib_1", c, i)

t()
