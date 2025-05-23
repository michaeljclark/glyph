#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0x1111111100000000,
        0x3333333322222222,
        0xd5555555c4444444,
        0xf7777777e6666666,
    ]
    i = [
        cpu_encode_op_movh_i64(0,0),
        cpu_encode_op_movh_i64(1,1),
        cpu_encode_op_movh_i64(2,2),
        cpu_encode_op_movh_i64(3,3),
        cpu_encode_op_movh_i64(4,4),
        cpu_encode_op_movh_i64(5,5),
        cpu_encode_op_movh_i64(6,6),
        cpu_encode_op_movh_i64(7,7),
        cpu_encode_op_break(0),
    ]
    cpu_test("movhib_1", c, i)

t()
