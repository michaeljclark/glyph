#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0x0000004000000002,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0x00000000fffffffc,
        0xffffffffffffffff,
    ]
    i = [
        cpu_encode_op_jalib(7,0),
        cpu_encode_op_break(0),
        cpu_encode_op_lib_i64(0,1),
        cpu_encode_op_or_i64(0,0,0),
        cpu_encode_op_jtlib(7,0),
    ]
    cpu_test("jtlib_1", c, i)

t()
