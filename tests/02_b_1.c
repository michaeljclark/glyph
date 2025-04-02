#include "glyph.h"

void t()
{
    u64 c[] = {
    };
    u16 i[] = {
        cpu_encode_op_b(2),
        cpu_encode_op_or_i64(0,0,0),
        cpu_encode_op_break(0),
    };
    cpu_test("b_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
