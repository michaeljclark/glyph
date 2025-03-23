#!/usr/bin/env python3

from interp import *

def t():
    c = [
        8,
        -1,
    ]
    i = [
        enc_ibl(0,0),
        enc_lib_i64(0,0),
        enc_break(0)
    ]
    run_test("ibl_1", c, i)

t()
