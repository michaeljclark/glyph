#!/usr/bin/env python3

from interp import *

def t():
    c = [
        1,
        2,
    ]
    i = [
        cpu_encode_op_lib_i64(0,0),
        cpu_encode_op_lib_i64(1,1),
        cpu_encode_op_cmp_i64(0,1, Fun3Compare.compare_lt.value),
        cpu_encode_op_break(0)
    ]
    cpu_test("cmp_1", c, i)

t()
