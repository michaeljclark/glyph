#include <stdio.h>
#include "glyph.h"
#include "test.h"

void t()
{
    u64 c[] = {
        8,
        -1,
    };
    u16 i[] = {
        enc_ibl(0,0),
        enc_lib_i64(0,0),
        enc_break(0)
    };
    run_test("ibl_1", c, sizeof(c), i, sizeof(i));
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
