#!/usr/bin/env python3

from interp import *

def t():
    c = [
        -0xa0000000a
    ]
    i = [
        enc_li_i64(0,1),
        enc_subib_i64(1,0,0),
        enc_break(0)
    ]
    run_test("subib_1", c, i)

t()
