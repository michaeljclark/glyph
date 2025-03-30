package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{ }
    i := []uint16{
        CPU_encode_op_li_i64(0,0),
        CPU_encode_op_li_i64(1,5),
        CPU_encode_op_addi_i64(1,-1),
        CPU_encode_op_cmp_i64(0,1, Compare_lt),
        CPU_encode_op_b(-3),
        CPU_encode_op_break(0),
    }
    CPU_test("loop_1", c, i);
}

func main() {
    t()
}
