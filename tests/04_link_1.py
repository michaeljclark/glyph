#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0x0000004000000004,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0x00000000fffffffa,
        0xffffffffffffffff,
    ]
    i = [
        cpu_encode_op_jalib_i64(7,0),
        cpu_encode_op_break(0),
        cpu_encode_op_movw_i64(0,1),
        cpu_encode_op_or_i64(0,0,0),
        cpu_encode_op_jtlib_i64(7,0),
    ]
    cpu_test("jlr_1", c, i)

t()
