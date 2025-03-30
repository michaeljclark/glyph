package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
    }
    i := []uint16{
        CPU_encode_op_li_i64(0,-8),
        CPU_encode_op_srli_i64(0,16),
        CPU_encode_op_break(0),
    }
    CPU_test("srli_1", c, i);
}

func main() {
    t()
}
