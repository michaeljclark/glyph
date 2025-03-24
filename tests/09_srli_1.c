#include "glyph.h"

void t()
{
    u64 c[] = {
    };
    u16 i[] = {
        enc_li_i64(0,-8),
        enc_srli_i64(0,16),
        enc_break(0)
    };
    run_test("srli_1", c, sizeof(c), i, sizeof(i));
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
