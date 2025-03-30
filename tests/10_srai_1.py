#!/usr/bin/env python3

from interp import *

def t():
    c = [
    ]
    i = [
        cpu_encode_op_li_i64(0,-8),
        cpu_encode_op_srai_i64(0,2),
        cpu_encode_op_break(0),
    ]
    cpu_test("srai_1", c, i)

t()
