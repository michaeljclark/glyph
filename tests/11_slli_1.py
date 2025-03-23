#!/usr/bin/env python3

from interp import *

def t():
    c = []
    i = [
        enc_li_i64(0,1),
        enc_slli_i64(0,16),
        enc_break(0)
    ]
    run_test("slli_1", c, i)

t()
