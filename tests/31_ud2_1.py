#!/usr/bin/env python3

from interp import *

def t():
    c = []
    i = [
        enc_ud2(0),
        enc_break(0)
    ]
    run_test("ud2_1", c, i)

t()
