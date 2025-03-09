#include <stdio.h>
#include "glyph.h"
#include "test.h"

void t()
{
    u64 c[] = {
        1,
        2,
    };
    u16 i[] = {
        enc_lib_i64(0,0),
        enc_lib_i64(1,1),
        enc_cmp_i64(0,1, cpu_cmp_lt),
        enc_break(0)
    };
    run_test("cmp_1", c, sizeof(c), i, sizeof(i));
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
