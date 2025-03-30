#!/usr/bin/env python3

from interp import *

def t():
    c = [
    ]
    i = [
        cpu_encode_op_li_i64(0,1),
        cpu_encode_op_slli_i64(0,16),
        cpu_encode_op_break(0),
    ]
    cpu_test("slli_1", c, i)

t()
