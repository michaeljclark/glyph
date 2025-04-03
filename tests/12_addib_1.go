package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
        0,
        0xa0000000a,
    }
    i := []uint16{
        CPU_encode_op_li_i64(0,1),
        CPU_encode_op_addib_i64(1,0,1),
        CPU_encode_op_break(0),
    }
    CPU_test("addib_1", c, i);
}

func main() {
    t()
}
