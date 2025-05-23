package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
        0xffffffffffffffff,
        1,
    }
    i := []uint16{
        CPU_encode_op_movw_i64(0,0),
        CPU_encode_op_movw_i64(1,1),
        CPU_encode_op_cmp_lt_i64(0,1),
        CPU_encode_op_cmov_i64(2,0),
        CPU_encode_op_cmp_geu_i64(0,1),
        CPU_encode_op_cmov_i64(3,0),
        CPU_encode_op_cmp_ge_i64(1,0),
        CPU_encode_op_cmov_i64(4,0),
        CPU_encode_op_cmp_ltu_i64(1,0),
        CPU_encode_op_cmov_i64(5,0),
        CPU_encode_op_cmp_ne_i64(1,0),
        CPU_encode_op_cmov_i64(6,0),
        CPU_encode_op_cmp_eq_i64(1,0),
        CPU_encode_op_ncmov_i64(7,0),
        CPU_encode_op_break(0),
    }
    CPU_test("cmp_1", c, i);
}

func main() {
    t()
}
