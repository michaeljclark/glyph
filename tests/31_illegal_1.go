package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
    }
    i := []uint16{
        CPU_encode_op_illegal(0),
        CPU_encode_op_break(0),
    }
    CPU_test("illegal_1", c, i);
}

func main() {
    t()
}
