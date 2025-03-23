#!/usr/bin/env python3

from interp import *

def t():
    c = []
    i = [
        enc_li_i64(0,-8),
        enc_srli_i64(0,16),
        enc_break(0)
    ]
    run_test("srli_1", c, i)

t()
