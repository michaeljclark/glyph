#!/usr/bin/env python3

from interp import *

def t():
    c = [
    ]
    i = [
        cpu_encode_op_j(2),
        cpu_encode_op_nop(0),
        cpu_encode_op_break(0),
    ]
    cpu_test("j_1", c, i)

t()
