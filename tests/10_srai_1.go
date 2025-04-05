package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
    }
    i := []uint16{
        CPU_encode_op_movi_i64(0,-8),
        CPU_encode_op_srai_i64(0,2),
        CPU_encode_op_break(0),
    }
    CPU_test("srai_1", c, i);
}

func main() {
    t()
}
