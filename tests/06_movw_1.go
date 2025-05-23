package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
        0x0000000000000000,
        0x1111111111111111,
        0x2222222222222222,
        0x3333333333333333,
        0x4444444444444444,
        0x5555555555555555,
        0x6666666666666666,
        0x7777777777777777,
   }
    i := []uint16{
        CPU_encode_op_movw_i64(0,0),
        CPU_encode_op_movw_i64(1,1),
        CPU_encode_op_movw_i64(2,2),
        CPU_encode_op_movw_i64(3,3),
        CPU_encode_op_movw_i64(4,4),
        CPU_encode_op_movw_i64(5,5),
        CPU_encode_op_movw_i64(6,6),
        CPU_encode_op_movw_i64(7,7),
        CPU_encode_op_break(0),
    }
    CPU_test("movwib_1", c, i);
}

func main() {
    t()
}
