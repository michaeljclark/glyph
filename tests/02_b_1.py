#!/usr/bin/env python3

from interp import *

def t():
    c = [
    ]
    i = [
        cpu_encode_op_b(2),
        cpu_encode_op_or_i64(0,0,0),
        cpu_encode_op_break(0),
    ]
    cpu_test("b_1", c, i)

t()
