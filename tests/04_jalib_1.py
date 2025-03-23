#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0x0000000800000002,
        -1,
    ]
    i = [
        enc_jalib(7,0),
        enc_nop(0),
        enc_lib_i64(0,0),
        enc_break(0),
    ]
    run_test("jalib_1", c, i)

t()
