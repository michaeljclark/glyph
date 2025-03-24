#include "glyph.h"

void t()
{
    u64 c[] = {
    };
    u16 i[] = {
        enc_nop(0),
        enc_break(0)
    };
    run_test("nop_1", c, sizeof(c), i, sizeof(i));
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
