#!/usr/bin/env python3

from interp import *

def t():
    c = [
        1,
        2,
    ]
    i = [
        enc_lib_i64(0,0),
        enc_lib_i64(1,1),
        enc_cmp_i64(0,1, Fun3Compare.compare_lt.value),
        enc_break(0)
    ]
    run_test("cmp_1", c, i)

t()
