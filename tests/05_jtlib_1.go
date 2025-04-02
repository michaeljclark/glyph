package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
        0x0000000800000002,
        0x00000000fffffffc,
        0xffffffffffffffff,
    }
    i := []uint16{
        CPU_encode_op_jalib(7,0),
        CPU_encode_op_break(0),
        CPU_encode_op_lib_i64(0,1),
        CPU_encode_op_or_i64(0,0,0),
        CPU_encode_op_jtlib(7,0),
    }
    CPU_test("jtlib_1", c, i);
}

func main() {
    t()
}
