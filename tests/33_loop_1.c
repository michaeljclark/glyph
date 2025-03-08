#include <stdio.h>
#include "glyph.h"
#include "test.h"

void t()
{
    u64 c[] = {
    };
    u16 i[] = {
        enc_li_i64(0,0),
        enc_li_i64(1,5),
        enc_addi_i64(1,-1),
        enc_cmp_i64(0,1, cpu_cmp_lt),
        enc_b(-3),
        enc_break(0)
    };
    run_test("loop_1", c, sizeof(c), i, sizeof(i));
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
