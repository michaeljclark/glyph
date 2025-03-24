#!/usr/bin/env python3

from interp import *

def t():
    c = [
    ]
    i = [
        cpu_encode_op_li_i64(0,2),
        cpu_encode_op_addi_i64(0,1),
        cpu_encode_op_break(0)
    ]
    cpu_test("addi_1", c, i)

t()
