#include <stdio.h>
#include "glyph.h"
#include "test.h"

void t()
{
    u64 c[] = {
    };
    u16 i[] = {
        enc_dump(0),
        enc_break(0)
    };
    run_test("dump_1", c, sizeof(c), i, sizeof(i));
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
