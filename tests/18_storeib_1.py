#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0x410,
        0xfedcba9876543210,
    ]
    i = [
        enc_li_i64(0,0),
        enc_lib_i64(1,1),
        enc_storeib_i64(1,0,0),
        enc_break(0)
    ]
    run_test("storeib_1", c, i)

t()
