package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
        0,
        0x400,
        0xfedcba9876543210,
    }
    i := []uint16{
        CPU_encode_op_movw_i64(0,1),
        CPU_encode_op_movw_i64(1,2),
        CPU_encode_op_store_i64(1,0,0x18),
        CPU_encode_op_movw_i64(2,3),
        CPU_encode_op_break(0),
    }
    CPU_test("store_1", c, i);
}

func main() {
    t()
}
