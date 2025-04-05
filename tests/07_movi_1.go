package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
    }
    i := []uint16{
        CPU_encode_op_movi_i64(0,0),
        CPU_encode_op_movi_i64(1,-1),
        CPU_encode_op_movi_i64(2,2),
        CPU_encode_op_movi_i64(3,-3),
        CPU_encode_op_movi_i64(4,4),
        CPU_encode_op_movi_i64(5,-5),
        CPU_encode_op_movi_i64(6,6),
        CPU_encode_op_movi_i64(7,-7),
        CPU_encode_op_break(0),
    }
    CPU_test("li_1", c, i);
}

func main() {
    t()
}
