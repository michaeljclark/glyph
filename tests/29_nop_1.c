#include "glyph.h"

void t()
{
    u64 c[] = {
    };
    u16 i[] = {
        cpu_encode_op_nop(0),
        cpu_encode_op_break(0)
    };
    cpu_test("nop_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
