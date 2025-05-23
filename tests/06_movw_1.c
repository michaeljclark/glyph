#include <stdio.h>
#include "glyph.h"

void t()
{
    u64 c[] = {
        0x0000000000000000ull,
        0x1111111111111111ull,
        0x2222222222222222ull,
        0x3333333333333333ull,
        0x4444444444444444ull,
        0x5555555555555555ull,
        0x6666666666666666ull,
        0x7777777777777777ull,
    };
    u16 i[] = {
        cpu_encode_op_movw_i64(0,0),
        cpu_encode_op_movw_i64(1,1),
        cpu_encode_op_movw_i64(2,2),
        cpu_encode_op_movw_i64(3,3),
        cpu_encode_op_movw_i64(4,4),
        cpu_encode_op_movw_i64(5,5),
        cpu_encode_op_movw_i64(6,6),
        cpu_encode_op_movw_i64(7,7),
        cpu_encode_op_break(0),
    };
    cpu_test("movwib_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
