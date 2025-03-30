#include "glyph.h"

void t()
{
    u64 c[] = {
        0x408,
        0xfedcba9876543210ull,
    };
    u16 i[] = {
        cpu_encode_op_lib_i64(0,0),
        cpu_encode_op_load_i64(1,0,0),
        cpu_encode_op_break(0),
    };
    cpu_test("load_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
