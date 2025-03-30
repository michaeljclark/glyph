package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
    }
    i := []uint16{
        CPU_encode_op_nop(0),
        CPU_encode_op_break(0),
    }
    CPU_test("nop_1", c, i);
}

func main() {
    t()
}
