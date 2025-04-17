#include "glyph.h"

void t()
{
    u64 c[] = {
        0x0000004000000002ull,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0xffffffffffffffffull,
    };
    u16 i[] = {
        cpu_encode_op_jalib_i64(7,0),
        cpu_encode_op_or_i64(0,0,0),
        cpu_encode_op_movib_i64(0,0),
        cpu_encode_op_break(0),
    };
    cpu_test("jalib_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
