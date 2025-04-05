package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
    }
    i := []uint16{
        CPU_encode_op_movi_i64(0,4),
        CPU_encode_op_movi_i64(1,2),
        CPU_encode_op_div_i64(2,0,1),
        CPU_encode_op_break(0),
    }
    CPU_test("div_1", c, i);
}

func main() {
    t()
}
