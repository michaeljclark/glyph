package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
        0x440,
        0x80a,
        0xfffffffe,
        0,
        0,
        0,
        0,
        0,
        0xffffffffffffffff,
    }
    i := []uint16{
        CPU_encode_op_movw_i64(0,0),
        CPU_encode_op_movw_i64(1,1),
        CPU_encode_op_pin_i64(7,0,1),
        CPU_encode_op_jtlib_i64(7,2),
        CPU_encode_op_or_i64(0,0,0),
        CPU_encode_op_movw_i64(3,0),
        CPU_encode_op_break(0),
    }
    CPU_test("pin_1", c, i);
}

func main() {
    t()
}
