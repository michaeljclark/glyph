#!/usr/bin/env python3

from interp import *

def t():
    c = [
        0x0000000000000000,
        0x1111111111111111,
        0x2222222222222222,
        0x3333333333333333,
        0x4444444444444444,
        0x5555555555555555,
        0x6666666666666666,
        0x7777777777777777,
    ]
    i = [
        enc_lib_i64(0,0),
        enc_lib_i64(1,1),
        enc_lib_i64(2,2),
        enc_lib_i64(3,3),
        enc_lib_i64(4,4),
        enc_lib_i64(5,5),
        enc_lib_i64(6,6),
        enc_lib_i64(7,7),
        enc_break(0)
    ]
    run_test("lib_1", c, i)

t()
