#include "glyph.h"

void t()
{
    u64 c[] = {
        0,
        0x400,
        0xfedcba9876543210ull,
    };
    u16 i[] = {
        cpu_encode_op_movib_i64(0,1),
        cpu_encode_op_load_i64(1,0,0x10),
        cpu_encode_op_break(0),
    };
    cpu_test("load_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
