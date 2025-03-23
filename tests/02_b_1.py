#!/usr/bin/env python3

from interp import *

def t():
    c = []
    i = [
        enc_b(1),
        enc_nop(0),
        enc_break(0)
    ]
    run_test("b_1", c, i)

t()
