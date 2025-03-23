#!/usr/bin/env python3

from interp import *

def t():
    c = []
    i = [
        enc_j(1),
        enc_nop(0),
        enc_break(0)
    ]
    run_test("j_1", c, i)

t()
