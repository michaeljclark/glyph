#include <stdio.h>
#include "glyph.h"
#include "test.h"

void t()
{
    u64 c[] = {
        -0xa0000000aull
    };
    u16 i[] = {
        enc_li_i64(0,1),
        enc_subib_i64(1,0,0),
        enc_break(0)
    };
    run_test("subib_1", c, sizeof(c), i, sizeof(i));
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
