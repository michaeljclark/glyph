package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
        0x0000004000000002,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0xffffffffffffffff,
    }
    i := []uint16{
        CPU_encode_op_jalib(7,0),
        CPU_encode_op_or_i64(0,0,0),
        CPU_encode_op_movib_i64(0,0),
        CPU_encode_op_break(0),
    }
    CPU_test("jalib_1", c, i);
}

func main() {
    t()
}
