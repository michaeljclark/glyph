#!/usr/bin/env python3

from interp import *

def t():
    c = [
    ]
    i = [
        cpu_encode_op_li_i64(0,1),
        cpu_encode_op_li_i64(1,2),
        cpu_encode_op_or_i64(2,0,1),
        cpu_encode_op_break(0)
    ]
    cpu_test("or_1", c, i)

t()
