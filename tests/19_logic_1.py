#!/usr/bin/env python3

from interp import *

def t():
    c = []
    i = [
        enc_li_i64(0,1),
        enc_logic_i64(1,0, Fun3Logic.logic_not.value),
        enc_break(0)
    ]
    run_test("log_1", c, i)

t()
