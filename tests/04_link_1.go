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
        0x00000000fffffffa,
        0xffffffffffffffff,
    }
    i := []uint16{
        CPU_encode_op_jalib_i64(7,0),
        CPU_encode_op_break(0),
        CPU_encode_op_movw_i64(0,1),
        CPU_encode_op_or_i64(0,0,0),
        CPU_encode_op_jtlib_i64(7,0),
    }
    CPU_test("jlr_1", c, i);
}

func main() {
    t()
}
