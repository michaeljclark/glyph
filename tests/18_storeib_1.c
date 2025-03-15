#include <stdio.h>
#include "glyph.h"
#include "test.h"

void t()
{
    u64 c[] = {
        0x410,
        0xfedcba9876543210ull,
    };
    u16 i[] = {
        enc_li_i64(0,0),
        enc_lib_i64(1,1),
        enc_storeib_i64(1,0,0),
        enc_break(0)
    };
    run_test("storeib_1", c, sizeof(c), i, sizeof(i));
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
