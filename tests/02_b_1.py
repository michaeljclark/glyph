#!/usr/bin/env python3

from interp import *

def t():
    c = [
    ]
    i = [
        cpu_encode_op_b(1),
        cpu_encode_op_nop(0),
        cpu_encode_op_break(0),
    ]
    cpu_test("b_1", c, i)

t()
