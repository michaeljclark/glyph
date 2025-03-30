package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
        0x408,
        0xfedcba9876543210,
    }
    i := []uint16{
        CPU_encode_op_lib_i64(0,0),
        CPU_encode_op_load_i64(1,0,0),
        CPU_encode_op_break(0),
    }
    CPU_test("load_1", c, i);
}

func main() {
    t()
}
