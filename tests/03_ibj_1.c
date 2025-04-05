#include "glyph.h"

void t()
{
    u64 c[] = {
        0,
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
        cpu_encode_op_ibj(64),
        cpu_encode_op_movib_i64(0,0),
        cpu_encode_op_break(0),
    };
    cpu_test("ibl_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
