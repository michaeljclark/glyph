#!/usr/bin/env python3

from interp import *

def t():
    c = [
    ]
    i = [
        cpu_encode_op_li_i64(0,1),
        cpu_encode_op_logic_i64(1,0, Fun3Logic.logic_not.value),
        cpu_encode_op_break(0),
    ]
    cpu_test("logic_1", c, i)

t()
