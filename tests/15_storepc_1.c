#include "glyph.h"

void t()
{
    u64 c[] = {
        0xfedcba9876543210ull,
        0xfffffc0e,
    };
    u16 i[] = {
        cpu_encode_op_movib_i64(0,0),
        cpu_encode_op_storepc_i64(0,2),
        cpu_encode_op_movib_i64(1,2),
        cpu_encode_op_break(0),
    };
    cpu_test("storepc_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
