#include <stdio.h>
#include "glyph.h"
#include "test.h"

void t()
{
    u64 c[] = {
    };
    u16 i[] = {
        enc_li_i64(0,1),
        enc_li_i64(1,2),
        enc_sub_i64(2,0,1),
        enc_break(0)
    };
    run_test("sub_1", c, sizeof(c), i, sizeof(i));
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
