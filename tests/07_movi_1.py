#!/usr/bin/env python3

from interp import *

def t():
    c = [
    ]
    i = [
        cpu_encode_op_movi_i64(0,0),
        cpu_encode_op_movi_i64(1,-1),
        cpu_encode_op_movi_i64(2,2),
        cpu_encode_op_movi_i64(3,-3),
        cpu_encode_op_movi_i64(4,4),
        cpu_encode_op_movi_i64(5,-5),
        cpu_encode_op_movi_i64(6,6),
        cpu_encode_op_movi_i64(7,-7),
        cpu_encode_op_break(0),
    ]
    cpu_test("li_1", c, i)

t()
