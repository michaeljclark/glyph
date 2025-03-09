#include <stdio.h>
#include "glyph.h"
#include "test.h"

void t()
{
    u64 c[] = {
        0x0000000800000002ull,
        0x00000000fffffffcull,
        -1,
    };
    u16 i[] = {
        enc_jalib(7,0),
        enc_break(0),
        enc_lib_i64(0,1),
        enc_dump(0),
        enc_jtlib(7,0)
    };
    run_test("jtlib_1", c, sizeof(c), i, sizeof(i));
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
