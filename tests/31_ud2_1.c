#include "glyph.h"

void t()
{
    u64 c[] = {
    };
    u16 i[] = {
        cpu_encode_op_ud2(0),
        cpu_encode_op_break(0),
    };
    cpu_test("ud2_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
