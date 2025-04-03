#include "glyph.h"

void t()
{
    u64 c[] = {
        0,
        0xa0000000aull
    };
    u16 i[] = {
        cpu_encode_op_li_i64(0,1),
        cpu_encode_op_addib_i64(1,0,1),
        cpu_encode_op_break(0),
    };
    cpu_test("addib_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
