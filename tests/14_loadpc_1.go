package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
        0xfedcba9876543210,
        0xfffffc00,
    }
    i := []uint16{
        CPU_encode_op_loadpc_i64(0,2),
        CPU_encode_op_break(0),
    }
    CPU_test("loadpc_1", c, i);
}

func main() {
    t()
}
