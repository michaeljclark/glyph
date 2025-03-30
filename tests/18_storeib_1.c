#include "glyph.h"

void t()
{
    u64 c[] = {
        0x410,
        0xfedcba9876543210ull,
    };
    u16 i[] = {
        cpu_encode_op_li_i64(0,0),
        cpu_encode_op_lib_i64(1,1),
        cpu_encode_op_storeib_i64(1,0,0),
        cpu_encode_op_break(0),
    };
    cpu_test("storeib_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
