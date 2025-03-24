#include "glyph.h"

void t()
{
    u64 c[] = {
        0x0000000800000002ull,
        -1ull,
    };
    u16 i[] = {
        enc_jalib(7,0),
        enc_nop(0),
        enc_lib_i64(0,0),
        enc_break(0),
    };
    run_test("jalib_1", c, sizeof(c), i, sizeof(i));
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
