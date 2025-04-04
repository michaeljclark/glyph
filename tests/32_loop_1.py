#!/usr/bin/env python3

from interp import *

def t():
    c = [
    ]
    i = [
        cpu_encode_op_li_i64(0,0),
        cpu_encode_op_li_i64(1,5),
        cpu_encode_op_addi_i64(1,-1),
        cpu_encode_op_cmp_lt_i64(0,1),
        cpu_encode_op_b(-6),
        cpu_encode_op_break(0),
    ]
    cpu_test("loop_1", c, i)

t()
