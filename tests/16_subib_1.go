package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
        0xfffffff5fffffff6,
    }
    i := []uint16{
        CPU_encode_op_li_i64(0,1),
        CPU_encode_op_subib_i64(1,0,0),
        CPU_encode_op_break(0),
    }
    CPU_test("subib_1", c, i);
}

func main() {
    t()
}
