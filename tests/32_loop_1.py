#!/usr/bin/env python3

from interp import *

def t():
    c = []
    i = [
        enc_li_i64(0,0),
        enc_li_i64(1,5),
        enc_addi_i64(1,-1),
        enc_cmp_i64(0,1, Fun3Compare.compare_lt.value),
        enc_b(-3),
        enc_break(0)
    ]
    run_test("loop_1", c, i)

t()
