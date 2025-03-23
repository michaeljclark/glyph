#!/usr/bin/env python3

from interp import *

def t():
    c = []
    i = [
        enc_nop(0),
        enc_break(0)
    ]
    run_test("nop_1", c, i)

t()
