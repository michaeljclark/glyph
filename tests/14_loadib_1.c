#include "glyph.h"

void t()
{
    u64 c[] = {
        0x408,
        0xfedcba9876543210ull,
    };
    u16 i[] = {
        cpu_encode_op_li_i64(0,0),
        cpu_encode_op_loadib_i64(1,0,0),
        cpu_encode_op_break(0)
    };
    cpu_test("loadib_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
