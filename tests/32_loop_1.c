#include "glyph.h"

void t()
{
    u64 c[] = {
    };
    u16 i[] = {
        cpu_encode_op_li_i64(0,0),
        cpu_encode_op_li_i64(1,5),
        cpu_encode_op_addi_i64(1,-1),
        cpu_encode_op_cmp_i64(0,1, cpu_compare_lt),
        cpu_encode_op_b(-3),
        cpu_encode_op_break(0)
    };
    cpu_test("loop_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
