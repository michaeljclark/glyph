#include "glyph.h"

void t()
{
    u64 c[] = {
    };
    u16 i[] = {
        cpu_encode_op_movi_i64(0,31),
        cpu_encode_op_mov_i64(1,0),
        cpu_encode_op_neg_i64(2,0),
        cpu_encode_op_not_i64(3,0),
        cpu_encode_op_clz_i64(4,0),
        cpu_encode_op_ctz_i64(5,3),
        cpu_encode_op_bswap_i64(6,4),
        cpu_encode_op_cpop_i64(7,4),
        cpu_encode_op_break(0),
    };
    cpu_test("logic_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
