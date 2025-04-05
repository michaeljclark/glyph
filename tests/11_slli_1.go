package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
    }
    i := []uint16{
        CPU_encode_op_movi_i64(0,1),
        CPU_encode_op_slli_i64(0,16),
        CPU_encode_op_break(0),
    }
    CPU_test("slli_1", c, i);
}

func main() {
    t()
}
