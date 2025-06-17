package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
        0x1111111100000000,
        0x3333333322222222,
        0xd5555555c4444444,
        0xf7777777e6666666,
    }
    i := []uint16{
        CPU_encode_op_movh_i64(0,0),
        CPU_encode_op_movh_i64(1,1),
        CPU_encode_op_movh_i64(2,2),
        CPU_encode_op_movh_i64(3,3),
        CPU_encode_op_movh_i64(4,4),
        CPU_encode_op_movh_i64(5,5),
        CPU_encode_op_movh_i64(6,6),
        CPU_encode_op_movh_i64(7,7),
        CPU_encode_op_break(0),
    }
    CPU_test("movh_1", c, i);
}

func main() {
    t()
}
