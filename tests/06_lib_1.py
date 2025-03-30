#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0x0000000000000000,
        0x1111111111111111,
        0x2222222222222222,
        0x3333333333333333,
        0x4444444444444444,
        0x5555555555555555,
        0x6666666666666666,
        0x7777777777777777,
    ]
    i = [
        cpu_encode_op_lib_i64(0,0),
        cpu_encode_op_lib_i64(1,1),
        cpu_encode_op_lib_i64(2,2),
        cpu_encode_op_lib_i64(3,3),
        cpu_encode_op_lib_i64(4,4),
        cpu_encode_op_lib_i64(5,5),
        cpu_encode_op_lib_i64(6,6),
        cpu_encode_op_lib_i64(7,7),
        cpu_encode_op_break(0),
    ]
    cpu_test("lib_1", c, i)

t()
