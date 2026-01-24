#include "glyph.h"

void t()
{
    u64 c[] = {
        0,
        0x400,
        0xfedcba9876543210ull,
    };
    u16 i[] = {
        cpu_encode_op_movq_i64(0,1),
        cpu_encode_op_movq_i64(1,2),
        cpu_encode_op_store_i64(1,0,0x18),
        cpu_encode_op_movq_i64(2,3),
        cpu_encode_op_break(0),
    };
    cpu_test("store_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
