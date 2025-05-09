#include "glyph.h"

void t()
{
    u64 c[] = {
        0xffffffffffffffffull,
        1,
    };
    u16 i[] = {
        cpu_encode_op_movib_i64(0,0),
        cpu_encode_op_movib_i64(1,1),
        cpu_encode_op_cmp_lt_i64(0,1),
        cpu_encode_op_cmov_i64(2,0),
        cpu_encode_op_cmp_geu_i64(0,1),
        cpu_encode_op_cmov_i64(3,0),
        cpu_encode_op_cmp_ge_i64(1,0),
        cpu_encode_op_cmov_i64(4,0),
        cpu_encode_op_cmp_ltu_i64(1,0),
        cpu_encode_op_cmov_i64(5,0),
        cpu_encode_op_cmp_ne_i64(1,0),
        cpu_encode_op_cmov_i64(6,0),
        cpu_encode_op_cmp_eq_i64(1,0),
        cpu_encode_op_ncmov_i64(7,0),
        cpu_encode_op_break(0),
    };
    cpu_test("cmp_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
