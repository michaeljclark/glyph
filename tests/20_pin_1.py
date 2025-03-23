#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0x400,
        0x80a,
        0
    ]
    i = [
        enc_lib_i64(0,0),
        enc_lib_i64(1,1),
        enc_pin_i64(2,0,1),
        enc_jtlib(2,2),
        enc_nop(0),
        enc_break(0)
    ]
    run_test("pin_1", c, i)

t()
