#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0x408,
        0xfedcba9876543210,
    ]
    i = [
        enc_lib_i64(0,0),
        enc_load_i64(1,0,0),
        enc_break(0)
    ]
    run_test("load_1", c, i)

t()
