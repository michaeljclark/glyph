#include "glyph.h"

void t()
{
    u64 c[] = {
        0x1111111100000000ull,
        0x3333333322222222ull,
        0xd5555555c4444444ull,
        0xf7777777e6666666ull,
    };
    u16 i[] = {
        cpu_encode_op_movd_i64(0,0),
        cpu_encode_op_movd_i64(1,1),
        cpu_encode_op_movd_i64(2,2),
        cpu_encode_op_movd_i64(3,3),
        cpu_encode_op_movd_i64(4,4),
        cpu_encode_op_movd_i64(5,5),
        cpu_encode_op_movd_i64(6,6),
        cpu_encode_op_movd_i64(7,7),
        cpu_encode_op_break(0),
    };
    cpu_test("movd_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
