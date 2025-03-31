#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0xfffffff5fffffff6,
    ]
    i = [
        cpu_encode_op_li_i64(0,1),
        cpu_encode_op_subib_i64(1,0,0),
        cpu_encode_op_break(0),
    ]
    cpu_test("subib_1", c, i)

t()
