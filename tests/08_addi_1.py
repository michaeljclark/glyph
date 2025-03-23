#!/usr/bin/env python3

from interp import *

def t():
    c = []
    i = [
        enc_li_i64(0,2),
        enc_addi_i64(0,1),
        enc_break(0)
    ]
    run_test("addi_1", c, i)

t()
