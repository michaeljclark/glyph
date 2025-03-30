#include "glyph.h"

void t()
{
    u64 c[] = {
    };
    u16 i[] = {
        cpu_encode_op_li_i64(0,1),
        cpu_encode_op_li_i64(1,2),
        cpu_encode_op_and_i64(2,0,1),
        cpu_encode_op_break(0),
    };
    cpu_test("and_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
