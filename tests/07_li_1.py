#!/usr/bin/env python3

from interp import *

def t():
    c = []
    i = [
        enc_li_i64(0,0),
        enc_li_i64(1,-1),
        enc_li_i64(2,2),
        enc_li_i64(3,-3),
        enc_li_i64(4,4),
        enc_li_i64(5,-5),
        enc_li_i64(6,6),
        enc_li_i64(7,-7),
        enc_break(0)
    ]
    run_test("li_1", c, i)

t()
