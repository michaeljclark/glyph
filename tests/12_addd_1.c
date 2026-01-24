#include "glyph.h"

void t()
{
    u64 c[] = {
        0,
        0xfffffc00,
    };
    u16 i[] = {
        cpu_encode_op_addd_i64(0,2),
        cpu_encode_op_break(0),
    };
    cpu_test("addd_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
