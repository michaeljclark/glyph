#include <stdio.h>
#include "glyph.h"
#include "test.h"

void t()
{
    u64 c[] = {
        0x1400,
        0x1802,
    };
    u16 i[] = {
        enc_lib_i64(0,0),
        enc_lib_i64(1,1),
        enc_pin(2,0,1),
        enc_break(0)
    };
    run_test("pin_1", c, sizeof(c), i, sizeof(i));
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
