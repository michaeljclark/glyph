#!/usr/bin/env python3

from interp import *

def t():
    c = []
    i = [
        enc_li_i64(0,1),
        enc_li_i64(1,2),
        enc_and_i64(2,0,1),
        enc_break(0)
    ]
    run_test("and_1", c, i)

t()
