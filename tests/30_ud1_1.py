#!/usr/bin/env python3

from interp import *

def t():
    c = []
    i = [
        enc_ud1(0),
        enc_break(0)
    ]
    run_test("ud1_1", c, i)

t()
