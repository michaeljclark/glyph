#include <stdio.h>
#include "glyph.h"
#include "test.h"

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
        enc_lib_i64(0,0),
        enc_lib_i64(1,1),
        enc_lib_i64(2,2),
        enc_lib_i64(3,3),
        enc_lib_i64(4,4),
        enc_lib_i64(5,5),
        enc_lib_i64(6,6),
        enc_lib_i64(7,7),
        enc_break(0)
    };
    run_test("lib_1", c, sizeof(c), i, sizeof(i));
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
