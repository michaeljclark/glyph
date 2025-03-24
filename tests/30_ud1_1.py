#!/usr/bin/env python3

from interp import *

def t():
    c = [
    ]
    i = [
        cpu_encode_op_ud1(0),
        cpu_encode_op_break(0)
    ]
    cpu_test("ud1_1", c, i)

t()
