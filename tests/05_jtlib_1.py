#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0x0000000800000002,
        0x00000000fffffffc,
        -1,
    ]
    i = [
        enc_jalib(7,0),
        enc_break(0),
        enc_lib_i64(0,1),
        enc_nop(0),
        enc_jtlib(7,0)
    ]
    run_test("jtlib_1", c, i)

t()
