#include "glyph.h"

void t()
{
    u64 c[] = {
        1,
        2,
    };
    u16 i[] = {
        cpu_encode_op_lib_i64(0,0),
        cpu_encode_op_lib_i64(1,1),
        cpu_encode_op_cmp_i64(0,1, cpu_compare_lt),
        cpu_encode_op_break(0)
    };
    cpu_test("cmp_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
