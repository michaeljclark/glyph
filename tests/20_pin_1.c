#include "glyph.h"

void t()
{
    u64 c[] = {
        0x400,
        0x80a,
        0
    };
    u16 i[] = {
        cpu_encode_op_lib_i64(0,0),
        cpu_encode_op_lib_i64(1,1),
        cpu_encode_op_pin_i64(2,0,1),
        cpu_encode_op_jtlib(2,2),
        cpu_encode_op_nop(0),
        cpu_encode_op_break(0)
    };
    cpu_test("pin_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
