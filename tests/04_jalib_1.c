#include "glyph.h"

void t()
{
    u64 c[] = {
        0x0000000800000002ull,
        0xffffffffffffffffull,
    };
    u16 i[] = {
        cpu_encode_op_jalib(7,0),
        cpu_encode_op_nop(0),
        cpu_encode_op_lib_i64(0,0),
        cpu_encode_op_break(0),
    };
    cpu_test("jalib_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
