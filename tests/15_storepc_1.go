package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
        0xfedcba9876543210,
        0xfffffc0e,
    }
    i := []uint16{
        CPU_encode_op_movq_i64(0,0),
        CPU_encode_op_storepc_i64(0,2),
        CPU_encode_op_movq_i64(1,2),
        CPU_encode_op_break(0),
    }
    CPU_test("storepc_1", c, i);
}

func main() {
    t()
}
