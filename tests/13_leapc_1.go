package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
        0,
        0xfffffc00,
    }
    i := []uint16{
        CPU_encode_op_leapc_i64(0,2),
        CPU_encode_op_break(0),
    }
    CPU_test("leapc_1", c, i);
}

func main() {
    t()
}
