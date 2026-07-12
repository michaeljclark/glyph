package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
    }
    i := []uint16{
        CPU_encode_op_movi_i64(0,31),
        CPU_encode_op_mov_i64(1,0),
        CPU_encode_op_neg_i64(2,0),
        CPU_encode_op_not_i64(3,0),
        CPU_encode_op_lzcnt_i64(4,0),
        CPU_encode_op_tzcnt_i64(5,3),
        CPU_encode_op_bswap_i64(6,4),
        CPU_encode_op_popcnt_i64(7,4),
        CPU_encode_op_break(0),
    }
    CPU_test("logic_1", c, i);
}

func main() {
    t()
}
