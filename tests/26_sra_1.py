#!/usr/bin/env python3

from interp import *

def t():
    c = [
    ]
    i = [
        cpu_encode_op_movi_i64(0,-8),
        cpu_encode_op_movi_i64(1,2),
        cpu_encode_op_sra_i64(2,0,1),
        cpu_encode_op_break(0),
    ]
    cpu_test("sra_1", c, i)

t()
