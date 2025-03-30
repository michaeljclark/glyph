package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
        1,
        2,
    }
    i := []uint16{
        CPU_encode_op_lib_i64(0,0),
        CPU_encode_op_lib_i64(1,1),
        CPU_encode_op_cmp_i64(0,1, Compare_lt),
        CPU_encode_op_break(0),
    }
    CPU_test("cmp_1", c, i);
}

func main() {
    t()
}
