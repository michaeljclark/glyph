package interp

import (
    "fmt"
    "unsafe"
    "math/bits"
)

type Opcode int
type Fun3Compare int
type Fun3Logic int

/*
 * opcodes
 */

const (
    Op_break        Opcode = 0b00000 << 2 // op0r_imm9
    Op_j            Opcode = 0b00001 << 2 // op0r_imm9 pcrel9*2
    Op_b            Opcode = 0b00010 << 2 // op0r_imm9 pcrel9*2
    Op_ibl          Opcode = 0b00011 << 2 // op1r_imm6 ibrel(imm6*8,i64)
    Op_jalib        Opcode = 0b00100 << 2 // op1r_imm6 ibrel(imm6*8,i32x2)
    Op_jtlib        Opcode = 0b00101 << 2 // op1r_imm6 ibrel(imm6*8,i32x2)
    Op_lib_i64      Opcode = 0b00110 << 2 // op1r_imm6 ibrel(imm6*8,i64)
    Op_li_i64       Opcode = 0b00111 << 2 // op1r_imm6
    Op_addi_i64     Opcode = 0b01000 << 2 // op1r_imm6
    Op_srli_i64     Opcode = 0b01001 << 2 // op2r_imm3
    Op_srai_i64     Opcode = 0b01010 << 2 // op2r_imm3
    Op_slli_i64     Opcode = 0b01011 << 2 // op2r_imm3
    Op_addib_i64    Opcode = 0b01100 << 2 // op2r_imm3
    Op_load_i64     Opcode = 0b01101 << 2 // op2r_imm3
    Op_loadib_i64   Opcode = 0b01110 << 2 // op2r_imm3
    Op_cmp_i64      Opcode = 0b01111 << 2 // op2r_fun3
    Op_subib_i64    Opcode = 0b10000 << 2 // op2r_imm3
    Op_store_i64    Opcode = 0b10001 << 2 // op2r_imm3
    Op_storeib_i64  Opcode = 0b10010 << 2 // op2r_imm3
    Op_logic_i64    Opcode = 0b10011 << 2 // op2r_fun3
    Op_pin_i64      Opcode = 0b10100 << 2 // op3r
    Op_and_i64      Opcode = 0b10101 << 2 // op3r
    Op_or_i64       Opcode = 0b10110 << 2 // op3r
    Op_xor_i64      Opcode = 0b10111 << 2 // op3r
    Op_sub_i64      Opcode = 0b11000 << 2 // op3r
    Op_srl_i64      Opcode = 0b11001 << 2 // op3r
    Op_sra_i64      Opcode = 0b11010 << 2 // op3r
    Op_sll_i64      Opcode = 0b11011 << 2 // op3r
    Op_add_i64      Opcode = 0b11100 << 2 // op3r
    Op_nop          Opcode = 0b11101 << 2 // op0r_imm9
    Op_ud1          Opcode = 0b11110 << 2 // op0r_imm9
    Op_ud2          Opcode = 0b11111 << 2 // op0r_imm9
)

/*
 * compare op fun3
 */

const (
    Compare_lt      Fun3Compare = 0b000
    Compare_ge      Fun3Compare = 0b001
    Compare_eq      Fun3Compare = 0b010
    Compare_ne      Fun3Compare = 0b011
    Compare_ltu     Fun3Compare = 0b100
    Compare_geu     Fun3Compare = 0b101
)

/*
 * logic op fun3
 */

const (
    Logic_mov       Fun3Logic = 0b000
    Logic_not       Fun3Logic = 0b001
    Logic_neg       Fun3Logic = 0b010
    Logic_bswap     Fun3Logic = 0b011
    Logic_ctz       Fun3Logic = 0b100
    Logic_clz       Fun3Logic = 0b101
    Logic_ctpop     Fun3Logic = 0b110
)

/*
 * cpu state
 */

const (
    CpuRegCount = 8
)

type CPUState struct {
    Flag     bool
    R        [CpuRegCount]uint64
    PC       uint64
    IB       uint64
    Mem      []byte
    MemSize  uintptr
    IsTrace  bool
    IsDump   bool
}

/*
 * instruction decode helpers
 */

func opc(insn uint64) Opcode { return Opcode((insn >> 2) & 0b11111) }
func uimm9(insn uint64) uint64 { return insn << 48 >> 55 }
func uimm6(insn uint64) uint64 { return insn << 51 >> 58 }
func uimm3(insn uint64) uint64 { return insn << 54 >> 61 }
func simm9(insn uint64) int64 { return int64(insn) << 48 >> 55 }
func simm6(insn uint64) int64 { return int64(insn) << 51 >> 58 }
func simm3(insn uint64) int64 { return int64(insn) << 54 >> 61 }
func ra(insn uint64) uint64 { return (insn >> 7) & 7 }
func rb(insn uint64) uint64 { return (insn >> 10) & 7 }
func rc(insn uint64) uint64 { return (insn >> 13) & 7 }

/*
 * cpu load, store and constants
 */

func CPU_load_i64(cpu *CPUState, offset uint64) uint64 {
    return uint64(*(*int64)(unsafe.Pointer(&cpu.Mem[offset])))
}
func CPU_load_i32(cpu *CPUState, offset uint64) uint64 {
    return uint64(*(*int32)(unsafe.Pointer(&cpu.Mem[offset])))
}
func CPU_load_i16(cpu *CPUState, offset uint64) uint64 {
    return uint64(*(*int16)(unsafe.Pointer(&cpu.Mem[offset])))
}
func CPU_load_i8(cpu *CPUState, offset uint64) uint64 {
    return uint64(int8(cpu.Mem[offset]))
}
func CPU_store_i64(cpu *CPUState, offset uint64, val uint64) {
    *(*uint64)(unsafe.Pointer(&cpu.Mem[offset])) = val
}
func CPU_store_i32(cpu *CPUState, offset uint64, val uint64) {
    *(*uint32)(unsafe.Pointer(&cpu.Mem[offset])) = uint32(val)
}
func CPU_store_i16(cpu *CPUState, offset uint64, val uint64) {
    *(*uint16)(unsafe.Pointer(&cpu.Mem[offset])) = uint16(val)
}
func CPU_store_i8(cpu *CPUState, offset uint64, val uint64) {
    cpu.Mem[offset] = byte(val)
}
func CPU_const_i64(cpu *CPUState, offset uint64) uint64 {
    return *(*uint64)(unsafe.Pointer(&cpu.Mem[cpu.IB + offset * 8]))
}
func CPU_fetch(cpu *CPUState) uint64 {
    return uint64(*(*uint16)(unsafe.Pointer(&cpu.Mem[cpu.PC])))
}

/*
 * cpu emulation
 */

func CPU_exec_op_break(cpu *CPUState, inst uint64) int {
    return -1
}
func CPU_exec_op_j(cpu *CPUState, inst uint64) int {
    cpu.PC = cpu.PC + uint64((simm9(inst) << 1) + 2)
    return 0
}
func CPU_exec_op_b(cpu *CPUState, inst uint64) int {
    if !cpu.Flag {
        return 2
    }
    cpu.PC = cpu.PC + uint64((simm9(inst) << 1) + 2)
    return 0
}
func CPU_exec_op_ibl(cpu *CPUState, inst uint64) int {
    var tmp uint64
    cpu.R[rc(inst)] = cpu.IB
    tmp = CPU_const_i64(cpu, uimm6(inst)) &^ 7
    cpu.IB =  cpu.IB + tmp
    return 2
}
func CPU_exec_op_jalib(cpu *CPUState, inst uint64) int {
    var upc, uib int32
    var tmp uint64
    tmp = CPU_const_i64(cpu, uimm6(inst))
    upc = int32((tmp      ) &^ 1)
    uib = int32((tmp >> 32) &^ 7)
    cpu.PC = cpu.PC + uint64(upc) + 2
    cpu.IB = cpu.IB + uint64(uib)
    cpu.R[rc(inst)] = tmp
    return 0
}
func CPU_exec_op_jtlib(cpu *CPUState, inst uint64) int {
    var upc, uib int32
    var npc, nib int32
    var tmp uint64
    tmp = cpu.R[rc(inst)]
    upc = int32((tmp      ) &^ 1)
    uib = int32((tmp >> 32) &^ 7)
    tmp = CPU_const_i64(cpu, uimm6(inst))
    npc = int32((tmp      ) &^ 1)
    nib = int32((tmp >> 32) &^ 7)
    cpu.PC = cpu.PC + uint64(npc - upc)
    cpu.IB = cpu.IB + uint64(nib - uib)
    return 0
}
func CPU_exec_op_lib_i64(cpu *CPUState, inst uint64) int {
    var tmp uint64
    tmp = CPU_const_i64(cpu, uimm6(inst))
    cpu.R[rc(inst)] = tmp
    return 2
}
func CPU_exec_op_li_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = uint64(simm6(inst))
    return 2
}
func CPU_exec_op_addi_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = cpu.R[rc(inst)] + uint64(simm6(inst))
    return 2
}
func CPU_exec_op_srli_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = cpu.R[rc(inst)] >> uimm6(inst)
    return 2
}
func CPU_exec_op_srai_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = uint64(int64(cpu.R[rc(inst)]) >> uimm6(inst))
    return 2
}
func CPU_exec_op_slli_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = cpu.R[rc(inst)] << uimm6(inst)
    return 2
}
func CPU_exec_op_addib_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = cpu.R[rb(inst)] + CPU_const_i64(cpu, uint64(uimm3(inst)))
    return 2
}
func CPU_exec_op_load_i64(cpu *CPUState, inst uint64) int {
    var tmp uint64
    tmp = cpu.R[rb(inst)] + (uimm3(inst) << 3)
    cpu.R[rc(inst)] = CPU_load_i64(cpu, tmp)
    return 2
}
func CPU_exec_op_loadib_i64(cpu *CPUState, inst uint64) int {
    var tmp uint64
    tmp = cpu.R[rb(inst)] + CPU_const_i64(cpu, uimm3(inst))
    cpu.R[rc(inst)] = CPU_load_i64(cpu, tmp)
    return 2
}
func CPU_exec_op_cmp_i64(cpu *CPUState, inst uint64) int {
    switch(Fun3Compare(uimm3(inst))) {
    case Compare_lt:
        cpu.Flag = int64(cpu.R[rc(inst)]) < int64(cpu.R[rb(inst)])
        break
    case Compare_ge:
        cpu.Flag = int64(cpu.R[rc(inst)]) >= int64(cpu.R[rb(inst)])
        break
    case Compare_eq:
        cpu.Flag = cpu.R[rc(inst)] == cpu.R[rb(inst)]
        break
    case Compare_ne:
        cpu.Flag = cpu.R[rc(inst)] != cpu.R[rb(inst)]
        break
    case Compare_ltu:
        cpu.Flag = cpu.R[rc(inst)] < cpu.R[rb(inst)]
        break
    case Compare_geu:
        cpu.Flag = cpu.R[rc(inst)] >= cpu.R[rb(inst)]
        break
    default:
        return -1
    }
    return 2
}
func CPU_exec_op_subib_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = cpu.R[rb(inst)] - CPU_const_i64(cpu, uimm3(inst))
    return 2
}
func CPU_exec_op_store_i64(cpu *CPUState, inst uint64) int {
    var tmp uint64
    tmp = cpu.R[rb(inst)] + (uimm3(inst) << 3)
    CPU_store_i64(cpu, tmp, cpu.R[rc(inst)])
    return 2
}
func CPU_exec_op_storeib_i64(cpu *CPUState, inst uint64) int {
    var tmp uint64
    tmp = cpu.R[rb(inst)] + CPU_const_i64(cpu, uimm3(inst))
    CPU_store_i64(cpu, tmp, cpu.R[rc(inst)])
    return 2
}
func CPU_exec_op_logic_i64(cpu *CPUState, inst uint64) int {
    switch(Fun3Logic(uimm3(inst))) {
    case Logic_mov:
        cpu.R[rc(inst)] = cpu.R[rb(inst)]
        break
    case Logic_not:
        cpu.R[rc(inst)] = ^cpu.R[rb(inst)]
        break
    case Logic_neg:
        cpu.R[rc(inst)] = -cpu.R[rb(inst)]
        break
    case Logic_bswap:
        cpu.R[rc(inst)] = bits.ReverseBytes64(cpu.R[rb(inst)])
        break
    case Logic_ctz:
        cpu.R[rc(inst)] = uint64(bits.TrailingZeros64(cpu.R[rb(inst)]))
        break
    case Logic_clz:
        cpu.R[rc(inst)] = uint64(bits.LeadingZeros64(cpu.R[rb(inst)]))
        break
    case Logic_ctpop:
        cpu.R[rc(inst)] = uint64(bits.OnesCount64(cpu.R[rb(inst)]))
        break
    default:
        return -1
    }
    return 2
}
func CPU_exec_op_pin_i64(cpu *CPUState, inst uint64) int {
    var upc, uib int32
    var tmp uint64
    upc = int32(cpu.PC - uint64(cpu.R[ra(inst)]) + 2)
    uib = int32(cpu.IB - uint64(cpu.R[rb(inst)]))
    tmp =  uint64(uint32(upc)) | (uint64(uib) << 32)
    cpu.R[rc(inst)] = tmp
    return 2
}
func CPU_exec_op_and_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = cpu.R[rb(inst)] & cpu.R[ra(inst)]
    return 2
}
func CPU_exec_op_or_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = cpu.R[rb(inst)] | cpu.R[ra(inst)]
    return 2
}
func CPU_exec_op_xor_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = cpu.R[rb(inst)] ^ cpu.R[ra(inst)]
    return 2
}
func CPU_exec_op_sub_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = cpu.R[rb(inst)] - cpu.R[ra(inst)]
    return 2
}
func CPU_exec_op_srl_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = cpu.R[rb(inst)] >> cpu.R[ra(inst)]
    return 2
}
func CPU_exec_op_sra_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = uint64(int64(cpu.R[rb(inst)]) >> cpu.R[ra(inst)])
    return 2
}
func CPU_exec_op_sll_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = cpu.R[rb(inst)] << cpu.R[ra(inst)]
    return 2
}
func CPU_exec_op_add_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = cpu.R[rb(inst)] + cpu.R[ra(inst)]
    return 2
}
func CPU_exec_op_nop(cpu *CPUState, inst uint64) int {
    return 2
}
func CPU_exec_op_ud1(cpu *CPUState, inst uint64) int {
    return -1
}
func CPU_exec_op_ud2(cpu *CPUState, inst uint64) int {
    return -1
}

/*
 * cpu disassembly
 */

func CPU_disasm_op_break(i, c uint64) string {
    return fmt.Sprintf("break %d", uimm9(i))
}
func CPU_disasm_op_j(i, c uint64) string {
    return fmt.Sprintf("j %d", simm9(i))
}
func CPU_disasm_op_b(i, c uint64) string {
    return fmt.Sprintf("b %d", simm9(i))
}
func CPU_disasm_op_ibl(i, c uint64) string {
    return fmt.Sprintf("ibl ib(%d)", uimm6(i))
}
func CPU_disasm_op_jalib(i, c uint64) string {
    return fmt.Sprintf("jalib r%d, ib(%d)", rc(i), uimm6(i))
}
func CPU_disasm_op_jtlib(i, c uint64) string {
    return fmt.Sprintf("jtlib ib(%d), r%d", uimm6(i), rc(i))
}
func CPU_disasm_op_lib_i64(i, c uint64) string {
    return fmt.Sprintf("lib.i64 r%d, ib(%d)", rc(i), uimm6(i))
}
func CPU_disasm_op_li_i64(i, c uint64) string {
    return fmt.Sprintf("li.i64 r%d, %d", rc(i), simm6(i))
}
func CPU_disasm_op_addi_i64(i, c uint64) string {
    return fmt.Sprintf("addi.i64 r%d, %d", rc(i), simm6(i))
}
func CPU_disasm_op_srli_i64(i, c uint64) string {
    return fmt.Sprintf("srli.i64 r%d, %d", rc(i), uimm6(i))
}
func CPU_disasm_op_srai_i64(i, c uint64) string {
    return fmt.Sprintf("srai.i64 r%d, %d", rc(i), uimm6(i))
}
func CPU_disasm_op_slli_i64(i, c uint64) string {
    return fmt.Sprintf("slli.i64 r%d, %d", rc(i), uimm6(i))
}
func CPU_disasm_op_addib_i64(i, c uint64) string {
    return fmt.Sprintf("addib.i64 r%d, r%d, ib(%d)",
        rc(i), rb(i), uimm3(i))
}
func CPU_disasm_op_load_i64(i, c uint64) string {
    return fmt.Sprintf("load.i64 r%d, %d(r%d)",
        rc(i), uimm3(i) << 3, rb(i))
}
func CPU_disasm_op_loadib_i64(i, c uint64) string {
    return fmt.Sprintf("loadib.i64 r%d, ib(%d)(r%d)",
        rc(i), uimm3(i) << 3, rb(i))
}
func CPU_disasm_op_cmp_i64(i, c uint64) string {
    switch(Fun3Compare(uimm3(i))) {
    case Compare_lt:
        return fmt.Sprintf("cmp.lt.i64 r%d, r%d", rc(i), rb(i))
    case Compare_ge:
        return fmt.Sprintf("cmp.ge.i64 r%d, r%d", rc(i), rb(i))
    case Compare_eq:
        return fmt.Sprintf("cmp.eq.i64 r%d, r%d", rc(i), rb(i))
    case Compare_ne:
        return fmt.Sprintf("cmp.ne.i64 r%d, r%d", rc(i), rb(i))
    case Compare_ltu:
        return fmt.Sprintf("cmp.ltu.i64 r%d, r%d", rc(i), rb(i))
    case Compare_geu:
        return fmt.Sprintf("cmp.geu.i64 r%d, r%d", rc(i), rb(i))
    default:
        break
    }
    return fmt.Sprintf("invalid")
}
func CPU_disasm_op_subib_i64(i, c uint64) string {
    return fmt.Sprintf("subib.i64 r%d, r%d, ib(%d)",
        rc(i), rb(i), uimm3(i))
}
func CPU_disasm_op_store_i64(i, c uint64) string {
    return fmt.Sprintf("store.i64 r%d, %d(r%d)",
        rc(i), uimm3(i), rb(i))
}
func CPU_disasm_op_storeib_i64(i, c uint64) string {
    return fmt.Sprintf("storeib.i64 r%d, ib(%d)(r%d)",
        rc(i), uimm3(i), rb(i))
}
func CPU_disasm_op_logic_i64(i, c uint64) string {
    switch(Fun3Logic(uimm3(i))) {
    case Logic_mov:
        return fmt.Sprintf("mov.i64 r%d, r%d", rc(i), rb(i))
    case Logic_not:
        return fmt.Sprintf("not.i64 r%d, r%d", rc(i), rb(i))
    case Logic_neg:
        return fmt.Sprintf("neg.i64 r%d, r%d", rc(i), rb(i))
    case Logic_bswap:
        return fmt.Sprintf("bswap.i64 r%d, r%d", rc(i), rb(i))
    case Logic_ctz:
        return fmt.Sprintf("ctz.i64 r%d, r%d", rc(i), rb(i))
    case Logic_clz:
        return fmt.Sprintf("clz.i64 r%d, r%d", rc(i), rb(i))
    case Logic_ctpop:
        return fmt.Sprintf("ctpop.i64 r%d, r%d", rc(i), rb(i))
    default:
        break
    }
    return fmt.Sprintf("invalid")
}
func CPU_disasm_op_pin_i64(i, c uint64) string {
    return fmt.Sprintf("pin.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i))
}
func CPU_disasm_op_and_i64(i, c uint64) string {
    return fmt.Sprintf("and.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i))
}
func CPU_disasm_op_or_i64(i, c uint64) string {
    return fmt.Sprintf("or.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i))
}
func CPU_disasm_op_xor_i64(i, c uint64) string {
    return fmt.Sprintf("xor.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i))
}
func CPU_disasm_op_sub_i64(i, c uint64) string {
    return fmt.Sprintf("sub.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i))
}
func CPU_disasm_op_srl_i64(i, c uint64) string {
    return fmt.Sprintf("srl.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i))
}
func CPU_disasm_op_sra_i64(i, c uint64) string {
    return fmt.Sprintf("sra.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i))
}
func CPU_disasm_op_sll_i64(i, c uint64) string {
    return fmt.Sprintf("sll.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i))
}
func CPU_disasm_op_add_i64(i, c uint64) string {
    return fmt.Sprintf("add.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i))
}
func CPU_disasm_op_nop(i, c uint64) string {
    return fmt.Sprintf("nop %d", uimm9(i))
}
func CPU_disasm_op_ud1(i, c uint64) string {
    return fmt.Sprintf("ud1 %d", uimm9(i))
}
func CPU_disasm_op_ud2(i, c uint64) string {
    return fmt.Sprintf("ud2 %d", uimm9(i))
}


/*
 * cpu instruction encoding
 */

func CPU_encode_op_break(imm9 int) uint16 {
    return uint16(int(Op_break) | ((imm9 & 511)<<7))
}
func CPU_encode_op_j(pcrel9 int) uint16 {
    return uint16(int(Op_j) | ((pcrel9 & 511)<<7))
}
func CPU_encode_op_b(pcrel9 int) uint16 {
    return uint16(int(Op_b) | ((pcrel9 & 511)<<7))
}
func CPU_encode_op_ibl(rc, ibrel6 int) uint16 {
    return uint16(int(Op_ibl) | ((ibrel6 & 63)<<7) | ((rc & 7)<<13))
}
func CPU_encode_op_jalib(rc, ibrel6 int) uint16 {
    return uint16(int(Op_jalib) | ((ibrel6 & 63)<<7) | ((rc & 7)<<13))
}
func CPU_encode_op_jtlib(rc, ibrel6 int) uint16 {
    return uint16(int(Op_jtlib) | ((ibrel6 & 63)<<7) | ((rc & 7)<<13))
}
func CPU_encode_op_lib_i64(rc, ibrel6 int) uint16 {
    return uint16(int(Op_lib_i64) | ((ibrel6 & 63)<<7) | ((rc & 7)<<13))
}
func CPU_encode_op_li_i64(rc, imm6 int) uint16 {
    return uint16(int(Op_li_i64) | ((imm6 & 63)<<7) | ((rc & 7)<<13))
}
func CPU_encode_op_addi_i64(rc, imm6 int) uint16 {
    return uint16(int(Op_addi_i64) | ((imm6 & 63)<<7) | ((rc & 7)<<13))
}
func CPU_encode_op_srli_i64(rc, imm6 int) uint16 {
    return uint16(int(Op_srli_i64) | ((imm6 & 63)<<7) | ((rc & 7)<<13))
}
func CPU_encode_op_srai_i64(rc, imm6 int) uint16 {
    return uint16(int(Op_srai_i64) | ((imm6 & 63)<<7) | ((rc & 7)<<13))
}
func CPU_encode_op_slli_i64(rc, imm6 int) uint16 {
    return uint16(int(Op_slli_i64) | ((imm6 & 63)<<7) | ((rc & 7)<<13))
}
func CPU_encode_op_addib_i64(rc, rb, ibimm3 int) uint16 {
    return uint16(int(Op_addib_i64) | ((ibimm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func CPU_encode_op_load_i64(rc, rb, imm3 int) uint16 {
    return uint16(int(Op_load_i64) | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func CPU_encode_op_loadib_i64(rc, rb, ibimm3 int) uint16 {
    return uint16(int(Op_loadib_i64) | ((ibimm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func CPU_encode_op_cmp_i64(rc, rb int, fun3 Fun3Compare) uint16 {
    return uint16(int(Op_cmp_i64) | ((int(fun3) & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func CPU_encode_op_subib_i64(rc, rb, ibimm3 int) uint16 {
    return uint16(int(Op_subib_i64) | ((ibimm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func CPU_encode_op_store_i64(rc, rb, imm3 int) uint16 {
    return uint16(int(Op_store_i64) | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func CPU_encode_op_storeib_i64(rc, rb, ibimm3 int) uint16 {
    return uint16(int(Op_storeib_i64) | ((ibimm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func CPU_encode_op_logic_i64(rc, rb int, fun3 Fun3Logic) uint16 {
    return uint16(int(Op_logic_i64) | ((int(fun3) & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func CPU_encode_op_pin_i64(rc, rb, ra int) uint16 {
    return uint16(int(Op_pin_i64) | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func CPU_encode_op_and_i64(rc, rb, ra int) uint16 {
    return uint16(int(Op_and_i64) | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func CPU_encode_op_or_i64(rc, rb, ra int) uint16 {
    return uint16(int(Op_or_i64) | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func CPU_encode_op_xor_i64(rc, rb, ra int) uint16 {
    return uint16(int(Op_xor_i64) | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func CPU_encode_op_sub_i64(rc, rb, ra int) uint16 {
    return uint16(int(Op_sub_i64) | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func CPU_encode_op_srl_i64(rc, rb, ra int) uint16 {
    return uint16(int(Op_srl_i64) | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func CPU_encode_op_sra_i64(rc, rb, ra int) uint16 {
    return uint16(int(Op_sra_i64) | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func CPU_encode_op_sll_i64(rc, rb, ra int) uint16 {
    return uint16(int(Op_sll_i64) | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func CPU_encode_op_add_i64(rc, rb, ra int) uint16 {
    return uint16(int(Op_add_i64) | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func CPU_encode_op_nop(imm9 int) uint16 {
    return uint16(int(Op_nop) | ((imm9 & 511)<<7))
}
func CPU_encode_op_ud1(imm9 int) uint16 {
    return uint16(int(Op_ud1) | ((imm9 & 511)<<7))
}
func CPU_encode_op_ud2(imm9 int) uint16 {
    return uint16(int(Op_ud2) | ((imm9 & 511)<<7))
}

/*
 * cpu dispatch
 */

func CPU_exec(cpu *CPUState, inst uint64) int {
    switch opc(inst) {
    case Op_break >> 2: return CPU_exec_op_break(cpu, inst)
    case Op_j >> 2: return CPU_exec_op_j(cpu, inst)
    case Op_b >> 2: return CPU_exec_op_b(cpu, inst)
    case Op_ibl >> 2: return CPU_exec_op_ibl(cpu, inst)
    case Op_jalib >> 2: return CPU_exec_op_jalib(cpu, inst)
    case Op_jtlib >> 2: return CPU_exec_op_jtlib(cpu, inst)
    case Op_lib_i64 >> 2: return CPU_exec_op_lib_i64(cpu, inst)
    case Op_li_i64 >> 2: return CPU_exec_op_li_i64(cpu, inst)
    case Op_addi_i64 >> 2: return CPU_exec_op_addi_i64(cpu, inst)
    case Op_srli_i64 >> 2: return CPU_exec_op_srli_i64(cpu, inst)
    case Op_srai_i64 >> 2: return CPU_exec_op_srai_i64(cpu, inst)
    case Op_slli_i64 >> 2: return CPU_exec_op_slli_i64(cpu, inst)
    case Op_addib_i64 >> 2: return CPU_exec_op_addib_i64(cpu, inst)
    case Op_load_i64 >> 2: return CPU_exec_op_load_i64(cpu, inst)
    case Op_loadib_i64 >> 2: return CPU_exec_op_loadib_i64(cpu, inst)
    case Op_cmp_i64 >> 2: return CPU_exec_op_cmp_i64(cpu, inst)
    case Op_subib_i64 >> 2: return CPU_exec_op_subib_i64(cpu, inst)
    case Op_store_i64 >> 2: return CPU_exec_op_store_i64(cpu, inst)
    case Op_storeib_i64 >> 2: return CPU_exec_op_storeib_i64(cpu, inst)
    case Op_logic_i64 >> 2: return CPU_exec_op_logic_i64(cpu, inst)
    case Op_pin_i64 >> 2: return CPU_exec_op_pin_i64(cpu, inst)
    case Op_and_i64 >> 2: return CPU_exec_op_and_i64(cpu, inst)
    case Op_or_i64 >> 2: return CPU_exec_op_or_i64(cpu, inst)
    case Op_xor_i64 >> 2: return CPU_exec_op_xor_i64(cpu, inst)
    case Op_sub_i64 >> 2: return CPU_exec_op_sub_i64(cpu, inst)
    case Op_srl_i64 >> 2: return CPU_exec_op_srl_i64(cpu, inst)
    case Op_sra_i64 >> 2: return CPU_exec_op_sra_i64(cpu, inst)
    case Op_sll_i64 >> 2: return CPU_exec_op_sll_i64(cpu, inst)
    case Op_add_i64 >> 2: return CPU_exec_op_add_i64(cpu, inst)
    case Op_nop >> 2: return CPU_exec_op_nop(cpu, inst)
    case Op_ud1 >> 2: return CPU_exec_op_ud1(cpu, inst)
    case Op_ud2 >> 2: return CPU_exec_op_ud2(cpu, inst)
    }
    return -1
}


func CPU_disasm(i, c uint64) string {
    switch Opcode(opc(i)) {
    case Op_break >> 2: return CPU_disasm_op_break(i, c)
    case Op_j >> 2: return CPU_disasm_op_j(i, c)
    case Op_b >> 2: return CPU_disasm_op_b(i, c)
    case Op_ibl >> 2: return CPU_disasm_op_ibl(i, c)
    case Op_jalib >> 2: return CPU_disasm_op_jalib(i, c)
    case Op_jtlib >> 2: return CPU_disasm_op_jtlib(i, c)
    case Op_lib_i64 >> 2: return CPU_disasm_op_lib_i64(i, c)
    case Op_li_i64 >> 2: return CPU_disasm_op_li_i64(i, c)
    case Op_addi_i64 >> 2: return CPU_disasm_op_addi_i64(i, c)
    case Op_srli_i64 >> 2: return CPU_disasm_op_srli_i64(i, c)
    case Op_srai_i64 >> 2: return CPU_disasm_op_srai_i64(i, c)
    case Op_slli_i64 >> 2: return CPU_disasm_op_slli_i64(i, c)
    case Op_addib_i64 >> 2: return CPU_disasm_op_addib_i64(i, c)
    case Op_load_i64 >> 2: return CPU_disasm_op_load_i64(i, c)
    case Op_loadib_i64 >> 2: return CPU_disasm_op_loadib_i64(i, c)
    case Op_cmp_i64 >> 2: return CPU_disasm_op_cmp_i64(i, c)
    case Op_subib_i64 >> 2: return CPU_disasm_op_subib_i64(i, c)
    case Op_store_i64 >> 2: return CPU_disasm_op_store_i64(i, c)
    case Op_storeib_i64 >> 2: return CPU_disasm_op_storeib_i64(i, c)
    case Op_logic_i64 >> 2: return CPU_disasm_op_logic_i64(i, c)
    case Op_pin_i64 >> 2: return CPU_disasm_op_pin_i64(i, c)
    case Op_and_i64 >> 2: return CPU_disasm_op_and_i64(i, c)
    case Op_or_i64 >> 2: return CPU_disasm_op_or_i64(i, c)
    case Op_xor_i64 >> 2: return CPU_disasm_op_xor_i64(i, c)
    case Op_sub_i64 >> 2: return CPU_disasm_op_sub_i64(i, c)
    case Op_srl_i64 >> 2: return CPU_disasm_op_srl_i64(i, c)
    case Op_sra_i64 >> 2: return CPU_disasm_op_sra_i64(i, c)
    case Op_sll_i64 >> 2: return CPU_disasm_op_sll_i64(i, c)
    case Op_add_i64 >> 2: return CPU_disasm_op_add_i64(i, c)
    case Op_nop >> 2: return CPU_disasm_op_nop(i, c)
    case Op_ud1 >> 2: return CPU_disasm_op_ud1(i, c)
    case Op_ud2 >> 2: return CPU_disasm_op_ud2(i, c)
    }
    return "unknown"
}

/*
 * cpu implementation
 */

func CPU_init(memSize uintptr) *CPUState {
    cpu := &CPUState{
        Flag:    false,
        PC:      0x800,
        IB:      0x400,
        Mem:     make([]byte, memSize),
        MemSize: memSize,
        IsTrace: true,
        IsDump:  false,
    }
    return cpu
}

func CPU_debug(format string, args ...interface{}) {
    fmt.Printf(format + "\n", args...)
}


func CPU_dump(cpu *CPUState) {
    CPU_debug("pc:%016x ib:%016x flag:%d", cpu.PC, cpu.IB,
        map[bool]int{false: 0, true: 1}[cpu.Flag])

    for i := 0; i < CpuRegCount; i += 4 {
        CPU_debug("r%d:%016x r%d:%016x r%d:%016x r%d:%016x",
            i+0, uint64(cpu.R[i+0]), i+1, uint64(cpu.R[i+1]),
            i+2, uint64(cpu.R[i+2]), i+3, uint64(cpu.R[i+3]))
    }
}

func CPU_run(cpu *CPUState) {
    var inst uint64
    var ret int = 0
    for {
        inst = CPU_fetch(cpu)
        if cpu.IsTrace {
            var asm = CPU_disasm(inst, 0)
            CPU_debug("-- %08x %04x %s", cpu.PC, inst, asm)
        }
        ret = CPU_exec(cpu, inst)
        if ret < 0 {
            if cpu.IsTrace {
                CPU_debug("** %08x cpu exception", cpu.PC)
            }
            return
        }
        cpu.PC += uint64(ret)
        if cpu.IsDump {
            CPU_dump(cpu)
        }
    }
}

func CPU_setup(cpu *CPUState, c []uint64, i []uint16) {
    CPU_debug("")
    CPU_debug("# constants:")
    for x := 0; x < len(c); x++ {
        a := cpu.IB + uint64(x<<3)
        CPU_debug("# %08x ib(%d) <- %016x", a, x, c[x])
        CPU_store_i64(cpu, a, c[x])
    }
    CPU_debug("")
    CPU_debug("# instructions:")
    for x := 0; x < len(i); x++ {
        a := cpu.PC + uint64(x<<1)
        asm := CPU_disasm(uint64(i[x]), 0)
        CPU_debug("# %08x %04x %s", a, i[x], asm);
        CPU_store_i16(cpu, a, uint64(i[x]))
    }
}

func CPU_test(name string, c []uint64, i []uint16) {
    CPU_debug("# test: %s", name)
    cpu := CPU_init(8192)
    CPU_setup(cpu, c, i)
    CPU_debug("")
    CPU_debug("++ begin")
    CPU_run(cpu)
    CPU_debug("++ end\n")
    CPU_debug("# state")
    CPU_dump(cpu)
}
