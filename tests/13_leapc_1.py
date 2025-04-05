#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0,
        0xfffffc00,
    ]
    i = [
        cpu_encode_op_leapc_i64(0,2),
        cpu_encode_op_break(0),
    ]
    cpu_test("leapc_1", c, i)

t()
