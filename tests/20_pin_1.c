#include "glyph.h"

void t()
{
    u64 c[] = {
        0x440,
        0x80a,
        0xfffffffe,
        0,
        0,
        0,
        0,
        0,
        0xffffffffffffffffull,
    };
    u16 i[] = {
        cpu_encode_op_movq_i64(0,0),
        cpu_encode_op_movq_i64(1,1),
        cpu_encode_op_pin_i64(7,0,1),
        cpu_encode_op_jtlib_i64(7,2),
        cpu_encode_op_or_i64(0,0,0),
        cpu_encode_op_movq_i64(3,0),
        cpu_encode_op_break(0),
    };
    cpu_test("pin_1", c, i);
}

int main(int argc, char **argv)
{
    t();
    return 0;
}
