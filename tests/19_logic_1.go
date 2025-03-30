package main

import . "glyph/tests/interp"

func t() {
    c := []uint64{
    }
    i := []uint16{
        CPU_encode_op_li_i64(0,1),
        CPU_encode_op_logic_i64(1,0, Logic_not),
        CPU_encode_op_break(0),
    }
    CPU_test("logic_1", c, i);
}

func main() {
    t()
}
