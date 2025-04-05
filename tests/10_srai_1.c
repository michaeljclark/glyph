#include "glyph.h"

void t()
{
    u64 c[] = {
    };
    u16 i[] = {
        cpu_encode_op_movi_i64(0,-8),
        cpu_encode_op_srai_i64(0,2),
        cpu_encode_op_break(0),
    };
    cpu_test("srai_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
