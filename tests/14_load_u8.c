#include <stdio.h>
#include "glyph.h"
#include "test.h"

void t()
{
    u64 c[] = {
        0x408,
        0xff,
    };
    u16 i[] = {
        enc_lib_i64(0,0),
        enc_load_u8(1,0,0),
        enc_break(0)
    };
    run_test("load_u8_1", c, sizeof(c), i, sizeof(i));
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
