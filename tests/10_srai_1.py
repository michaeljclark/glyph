#!/usr/bin/env python3

from interp import *

def t():
    c = []
    i = [
        enc_li_i64(0,-8),
        enc_srai_i64(0,2),
        enc_break(0)
    ]
    run_test("srai_1", c, i)

t()
