package interp

import (
    "fmt"
    "unsafe"
    "strings"
    "math/bits"
)

type Opcode int
type Fun3Compare int
type Fun3Logic int

/*
 * opcodes
 */

const (
    CPU_op_break        Opcode = 0b00000 // op0r_imm9
    CPU_op_j            Opcode = 0b00001 // op0r_imm9 pcrel9*2
    CPU_op_b            Opcode = 0b00010 // op0r_imm9 pcrel9*2
    CPU_op_ibj          Opcode = 0b00011 // op0r_imm9 pcrel9*64
    CPU_op_jalib        Opcode = 0b00100 // op1r_imm6 ib64(imm6*8)
    CPU_op_jtlib        Opcode = 0b00101 // op1r_imm6 ib64(imm6*8)
    CPU_op_movib_i64    Opcode = 0b00110 // op1r_imm6 ib64(imm6*8)
    CPU_op_movi_i64     Opcode = 0b00111 // op1r_imm6
    CPU_op_addi_i64     Opcode = 0b01000 // op1r_imm6
    CPU_op_srli_i64     Opcode = 0b01001 // op1r_imm6
    CPU_op_srai_i64     Opcode = 0b01010 // op1r_imm6
    CPU_op_slli_i64     Opcode = 0b01011 // op1r_imm6
    CPU_op_addib_i64    Opcode = 0b01100 // op1r_imm6 ib32(imm6*4)
    CPU_op_leapc_i64    Opcode = 0b01101 // op1r_imm6 ib32(imm6*4)(pc)
    CPU_op_loadpc_i64   Opcode = 0b01110 // op1r_imm6 ib32(imm6*4)(pc)
    CPU_op_storepc_i64  Opcode = 0b01111 // op1r_imm6 ib32(imm6*4)(pc)
    CPU_op_load_i64     Opcode = 0b10000 // op2r_imm3
    CPU_op_store_i64    Opcode = 0b10001 // op2r_imm3
    CPU_op_compare_i64  Opcode = 0b10010 // op2r_fun3
    CPU_op_logic_i64    Opcode = 0b10011 // op2r_fun3
    CPU_op_pin_i64      Opcode = 0b10100 // op3r
    CPU_op_and_i64      Opcode = 0b10101 // op3r
    CPU_op_or_i64       Opcode = 0b10110 // op3r
    CPU_op_xor_i64      Opcode = 0b10111 // op3r
    CPU_op_add_i64      Opcode = 0b11000 // op3r
    CPU_op_srl_i64      Opcode = 0b11001 // op3r
    CPU_op_sra_i64      Opcode = 0b11010 // op3r
    CPU_op_sll_i64      Opcode = 0b11011 // op3r
    CPU_op_sub_i64      Opcode = 0b11100 // op3r
    CPU_op_mul_i64      Opcode = 0b11101 // op3r
    CPU_op_div_i64      Opcode = 0b11110 // op3r
    CPU_op_illegal      Opcode = 0b11111 // op0r_imm9
)

/*
 * compare op fun3
 */

const (
    CPU_compare_lt      Fun3Compare = 0b000
    CPU_compare_ge      Fun3Compare = 0b001
    CPU_compare_eq      Fun3Compare = 0b010
    CPU_compare_ne      Fun3Compare = 0b011
    CPU_compare_ltu     Fun3Compare = 0b100
    CPU_compare_geu     Fun3Compare = 0b101
    CPU_compare_mov     Fun3Compare = 0b110
)

/*
 * logic op fun3
 */

const (
    CPU_logic_mov       Fun3Logic = 0b000
    CPU_logic_not       Fun3Logic = 0b001
    CPU_logic_neg       Fun3Logic = 0b010
    CPU_logic_bswap     Fun3Logic = 0b011
    CPU_logic_ctz       Fun3Logic = 0b100
    CPU_logic_clz       Fun3Logic = 0b101
    CPU_logic_ctpop     Fun3Logic = 0b110
)

/*
 * op arg
 */

const (
    op_none  OpArg = iota
    op_nm
    op_compare
    op_logic
    op_ib3
    op_ib6
    op_pcib6
    op_ui3x8
    op_ui6
    op_ui9
    op_si6
    op_si9x2
    op_si9x64
    op_rc
    op_rb
    op_ra
    op_sp
    op_sc
    op_op
    op_cp
)

/*
 * op form
 */

const (
    op0r_uimm9 OpForm = iota
    op0r_simm9x2
    op0r_simm9x64
    op1r_ib32x2_uimm6_src
    op1r_ib32x2_uimm6_dst
    op1r_ib64_uimm6
    op1r_simm6
    op1r_uimm6
    op1r_mib64_uimm6
    op2r_mem64_uimm3x8
    op2r_fun3_compare
    op2r_fun3_logic
    op3r
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

func ops(insn uint64) uint64 { return insn & 0b11 }
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
func CPU_const_i32(cpu *CPUState, offset uint64) uint64 {
    return uint64(*(*int32)(unsafe.Pointer(&cpu.Mem[cpu.IB + offset * 4])))
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
func CPU_exec_op_ibj(cpu *CPUState, inst uint64) int {
    cpu.IB =  cpu.IB + uint64(simm9(inst) << 6)
    return 2
}
func CPU_exec_op_jalib(cpu *CPUState, inst uint64) int {
    rav := CPU_const_i64(cpu, uimm6(inst))
    rpc := int32(rav      )
    rib := int32(rav >> 32)
    cpu.PC = cpu.PC + uint64(rpc) + 2
    cpu.IB = cpu.IB + uint64(rib)
    cpu.R[rc(inst)] = rav
    return 0
}
func CPU_exec_op_jtlib(cpu *CPUState, inst uint64) int {
    rav := cpu.R[rc(inst)]
    rpc := int32(rav      )
    rib := int32(rav >> 32)
    dav := CPU_const_i64(cpu, uimm6(inst))
    dpc := int32(dav      )
    dib := int32(dav >> 32)
    cpu.PC = cpu.PC + uint64(dpc - rpc)
    cpu.IB = cpu.IB + uint64(dib - rib)
    return 0
}
func CPU_exec_op_movib_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = CPU_const_i64(cpu, uimm6(inst))
    return 2
}
func CPU_exec_op_movi_i64(cpu *CPUState, inst uint64) int {
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
    cpu.R[rc(inst)] = cpu.R[rc(inst)] + CPU_const_i32(cpu, uimm6(inst))
    return 2
}
func CPU_exec_op_leapc_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = cpu.PC + CPU_const_i32(cpu, uimm6(inst))
    return 2
}
func CPU_exec_op_loadpc_i64(cpu *CPUState, inst uint64) int {
    var tmp uint64
    tmp = cpu.PC + CPU_const_i32(cpu, uimm6(inst))
    cpu.R[rc(inst)] = CPU_load_i64(cpu, tmp)
    return 2
}
func CPU_exec_op_storepc_i64(cpu *CPUState, inst uint64) int {
    var tmp uint64
    tmp = cpu.PC + CPU_const_i32(cpu, uimm6(inst))
    CPU_store_i64(cpu, tmp, cpu.R[rc(inst)])
    return 2
}
func CPU_exec_op_load_i64(cpu *CPUState, inst uint64) int {
    var tmp uint64
    tmp = cpu.R[rb(inst)] + (uimm3(inst) << 3)
    cpu.R[rc(inst)] = CPU_load_i64(cpu, tmp)
    return 2
}
func CPU_exec_op_store_i64(cpu *CPUState, inst uint64) int {
    var tmp uint64
    tmp = cpu.R[rb(inst)] + (uimm3(inst) << 3)
    CPU_store_i64(cpu, tmp, cpu.R[rc(inst)])
    return 2
}
func CPU_exec_op_compare_i64(cpu *CPUState, inst uint64) int {
    switch(Fun3Compare(uimm3(inst))) {
    case CPU_compare_lt:
        cpu.Flag = int64(cpu.R[rc(inst)]) < int64(cpu.R[rb(inst)])
        break
    case CPU_compare_ge:
        cpu.Flag = int64(cpu.R[rc(inst)]) >= int64(cpu.R[rb(inst)])
        break
    case CPU_compare_eq:
        cpu.Flag = cpu.R[rc(inst)] == cpu.R[rb(inst)]
        break
    case CPU_compare_ne:
        cpu.Flag = cpu.R[rc(inst)] != cpu.R[rb(inst)]
        break
    case CPU_compare_ltu:
        cpu.Flag = cpu.R[rc(inst)] < cpu.R[rb(inst)]
        break
    case CPU_compare_geu:
        cpu.Flag = cpu.R[rc(inst)] >= cpu.R[rb(inst)]
        break
    case CPU_compare_mov:
        if cpu.Flag {
            cpu.R[rc(inst)] = cpu.R[rb(inst)]
        }
        break
    default:
        return -1
    }
    return 2
}
func CPU_exec_op_logic_i64(cpu *CPUState, inst uint64) int {
    switch(Fun3Logic(uimm3(inst))) {
    case CPU_logic_mov:
        cpu.R[rc(inst)] = cpu.R[rb(inst)]
        break
    case CPU_logic_not:
        cpu.R[rc(inst)] = ^cpu.R[rb(inst)]
        break
    case CPU_logic_neg:
        cpu.R[rc(inst)] = -cpu.R[rb(inst)]
        break
    case CPU_logic_bswap:
        cpu.R[rc(inst)] = bits.ReverseBytes64(cpu.R[rb(inst)])
        break
    case CPU_logic_ctz:
        cpu.R[rc(inst)] = uint64(bits.TrailingZeros64(cpu.R[rb(inst)]))
        break
    case CPU_logic_clz:
        cpu.R[rc(inst)] = uint64(bits.LeadingZeros64(cpu.R[rb(inst)]))
        break
    case CPU_logic_ctpop:
        cpu.R[rc(inst)] = uint64(bits.OnesCount64(cpu.R[rb(inst)]))
        break
    default:
        return -1
    }
    return 2
}
func CPU_exec_op_pin_i64(cpu *CPUState, inst uint64) int {
    rpc := int32(cpu.PC - cpu.R[ra(inst)] + 2)
    rib := int32(cpu.IB - cpu.R[rb(inst)])
    cpu.R[rc(inst)] = uint64(uint32(rpc)) | (uint64(rib) << 32)
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
func CPU_exec_op_add_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = cpu.R[rb(inst)] + cpu.R[ra(inst)]
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
func CPU_exec_op_sub_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = cpu.R[rb(inst)] - cpu.R[ra(inst)]
    return 2
}
func CPU_exec_op_mul_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = uint64(int64(cpu.R[rb(inst)]) * int64(cpu.R[ra(inst)]))
    return 2
}
func CPU_exec_op_div_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = uint64(int64(cpu.R[rb(inst)]) / int64(cpu.R[ra(inst)]))
    return 2
}
func CPU_exec_op_illegal(cpu *CPUState, inst uint64) int {
    return -1
}

/*
 * cpu disassembly
 */

type OpOutFn func(inst uint64) string
type OpArg byte
type OpForm byte

func op_out_nm(inst uint64) string {
    return fmt.Sprintf("%s", cpu_opcode_str[opc(inst)])
}
func op_out_compare(inst uint64) string {
    return fmt.Sprintf("%s", cpu_fun3_compare_str[uimm3(inst)]);
}
func op_out_logic(inst uint64) string {
    return fmt.Sprintf("%s", cpu_fun3_logic_str[uimm3(inst)]);
}
func op_out_ib3(inst uint64) string {
    return fmt.Sprintf("ib(%d)", uimm3(inst));
}
func op_out_ib6(inst uint64) string {
    return fmt.Sprintf("ib(%d)", uimm6(inst));
}
func op_out_pcib6(inst uint64) string {
    return fmt.Sprintf("ib(%d)(pc)", uimm6(inst));
}
func op_out_ui3x8(inst uint64) string {
    return fmt.Sprintf("%d", uimm3(inst) << 3);
}
func op_out_ui6(inst uint64) string {
    return fmt.Sprintf("%d", uimm6(inst));
}
func op_out_ui9(inst uint64) string {
    return fmt.Sprintf("%d", uimm9(inst));
}
func op_out_si6(inst uint64) string {
    return fmt.Sprintf("%d", simm6(inst));
}
func op_out_si9x2(inst uint64) string {
    return fmt.Sprintf("%d", simm9(inst) << 1);
}
func op_out_si9x64(inst uint64) string {
    return fmt.Sprintf("%d", simm9(inst) << 6);
}
func op_out_rc(inst uint64) string {
    return fmt.Sprintf("r%d", rc(inst));
}
func op_out_rb(inst uint64) string {
    return fmt.Sprintf("r%d", rb(inst));
}
func op_out_ra(inst uint64) string {
    return fmt.Sprintf("r%d", ra(inst));
}
func op_out_sp(inst uint64) string {
    return fmt.Sprintf(" ");
}
func op_out_sc(inst uint64) string {
    return fmt.Sprintf(", ");
}
func op_out_op(inst uint64) string {
    return fmt.Sprintf("(");
}
func op_out_cp(inst uint64) string {
    return fmt.Sprintf(")");
}

var cpu_opcode_str = [32]string{
    CPU_op_break:          "break",
    CPU_op_j:              "j",
    CPU_op_b:              "b",
    CPU_op_ibj:            "ibj",
    CPU_op_jalib:          "jalib",
    CPU_op_jtlib:          "jtlib",
    CPU_op_movib_i64:      "movib.i64",
    CPU_op_movi_i64:       "movi.i64",
    CPU_op_addi_i64:       "addi.i64",
    CPU_op_srli_i64:       "srli.i64",
    CPU_op_srai_i64:       "srai.i64",
    CPU_op_slli_i64:       "slli.i64",
    CPU_op_addib_i64:      "addib.i64",
    CPU_op_leapc_i64:      "leapc.i64",
    CPU_op_loadpc_i64:     "loadpc.i64",
    CPU_op_storepc_i64:    "storepc.i64",
    CPU_op_load_i64:       "load.i64",
    CPU_op_store_i64:      "store.i64",
    CPU_op_compare_i64:    "compare.i64",
    CPU_op_logic_i64:      "logic.i64",
    CPU_op_pin_i64:        "pin.i64",
    CPU_op_and_i64:        "and.i64",
    CPU_op_or_i64:         "or.i64",
    CPU_op_xor_i64:        "xor.i64",
    CPU_op_add_i64:        "add.i64",
    CPU_op_srl_i64:        "srl.i64",
    CPU_op_sra_i64:        "sra.i64",
    CPU_op_sll_i64:        "sll.i64",
    CPU_op_sub_i64:        "sub.i64",
    CPU_op_mul_i64:        "mul.i64",
    CPU_op_div_i64:        "div.i64",
    CPU_op_illegal:        "illegal",
}

var cpu_fun3_compare_str = [8]string{
    CPU_compare_lt:        "cmp.lt.i64",
    CPU_compare_ge:        "cmp.ge.i64",
    CPU_compare_eq:        "cmp.eq.i64",
    CPU_compare_ne:        "cmp.ne.i64",
    CPU_compare_ltu:       "cmp.ltu.i64",
    CPU_compare_geu:       "cmp.geu.i64",
    CPU_compare_mov:       "cmov.i64",
}

var cpu_fun3_logic_str = [8]string{
    CPU_logic_mov:         "mov.i64",
    CPU_logic_not:         "not.i64",
    CPU_logic_neg:         "neg.i64",
    CPU_logic_bswap:       "bswap.i64",
    CPU_logic_ctz:         "ctz.i64",
    CPU_logic_clz:         "clz.i64",
    CPU_logic_ctpop:       "ctpop.i64",
}

var cpu_op_out = []OpOutFn{
    op_nm:                 op_out_nm,
    op_compare:            op_out_compare,
    op_logic:              op_out_logic,
    op_ib3:                op_out_ib3,
    op_ib6:                op_out_ib6,
    op_pcib6:              op_out_pcib6,
    op_ui3x8:              op_out_ui3x8,
    op_ui6:                op_out_ui6,
    op_ui9:                op_out_ui9,
    op_si6:                op_out_si6,
    op_si9x2:              op_out_si9x2,
    op_si9x64:             op_out_si9x64,
    op_rc:                 op_out_rc,
    op_rb:                 op_out_rb,
    op_ra:                 op_out_ra,
    op_sp:                 op_out_sp,
    op_sc:                 op_out_sc,
    op_op:                 op_out_op,
    op_cp:                 op_out_cp,
}

var cpu_op_args = [][10]OpArg{
    op0r_uimm9:            { op_nm, op_sp, op_ui9 },
    op0r_simm9x2:          { op_nm, op_sp, op_si9x2 },
    op0r_simm9x64:         { op_nm, op_sp, op_si9x64 },
    op1r_ib32x2_uimm6_src: { op_nm, op_sp, op_rc, op_sc, op_ib6 },
    op1r_ib32x2_uimm6_dst: { op_nm, op_sp, op_ib6, op_sc, op_rc },
    op1r_ib64_uimm6:       { op_nm, op_sp, op_rc, op_sc, op_ib6 },
    op1r_simm6:            { op_nm, op_sp, op_rc, op_sc, op_si6 },
    op1r_uimm6:            { op_nm, op_sp, op_rc, op_sc, op_ui6 },
    op1r_mib64_uimm6:      { op_nm, op_sp, op_rc, op_sc, op_pcib6 },
    op2r_mem64_uimm3x8:    { op_nm, op_sp, op_rc, op_sc, op_ui3x8,
                             op_op, op_rb, op_cp },
    op2r_fun3_compare:     { op_compare, op_sp, op_rc, op_sc, op_rb },
    op2r_fun3_logic:       { op_logic, op_sp, op_rc, op_sc, op_rb },
    op3r:                  { op_nm, op_sp, op_rc, op_sc, op_rb,
                             op_sc, op_ra },
}

var cpu_op_type = [32]OpForm{
    CPU_op_break:          op0r_uimm9,
    CPU_op_j:              op0r_simm9x2,
    CPU_op_b:              op0r_simm9x2,
    CPU_op_ibj:            op0r_simm9x64,
    CPU_op_jalib:          op1r_ib32x2_uimm6_src,
    CPU_op_jtlib:          op1r_ib32x2_uimm6_dst,
    CPU_op_movib_i64:      op1r_ib64_uimm6,
    CPU_op_movi_i64:       op1r_simm6,
    CPU_op_addi_i64:       op1r_simm6,
    CPU_op_srli_i64:       op1r_uimm6,
    CPU_op_srai_i64:       op1r_uimm6,
    CPU_op_slli_i64:       op1r_uimm6,
    CPU_op_addib_i64:      op1r_mib64_uimm6,
    CPU_op_leapc_i64:      op1r_mib64_uimm6,
    CPU_op_loadpc_i64:     op1r_mib64_uimm6,
    CPU_op_storepc_i64:    op1r_mib64_uimm6,
    CPU_op_load_i64:       op2r_mem64_uimm3x8,
    CPU_op_store_i64:      op2r_mem64_uimm3x8,
    CPU_op_compare_i64:    op2r_fun3_compare,
    CPU_op_logic_i64:      op2r_fun3_logic,
    CPU_op_pin_i64:        op3r,
    CPU_op_and_i64:        op3r,
    CPU_op_or_i64:         op3r,
    CPU_op_xor_i64:        op3r,
    CPU_op_add_i64:        op3r,
    CPU_op_srl_i64:        op3r,
    CPU_op_sra_i64:        op3r,
    CPU_op_sll_i64:        op3r,
    CPU_op_sub_i64:        op3r,
    CPU_op_mul_i64:        op3r,
    CPU_op_div_i64:        op3r,
    CPU_op_illegal:        op0r_uimm9,
}

func CPU_disasm(inst, c uint64) string {
    if ops(inst) != 0 {
        return "unknown"
    }
    var tab [10]OpArg = cpu_op_args[cpu_op_type[Opcode(opc(inst))]]
    var sb strings.Builder
    for i := 0; i < len(tab) && tab[i] != op_none; i++ {
        sb.WriteString(cpu_op_out[tab[i]](inst))
    }
    return sb.String()
}

/*
 * cpu instruction encoding
 */

func op0ri9_enc(opcode Opcode, imm9 int) uint16 {
    return uint16((int(opcode) << 2) | ((imm9 & 511)<<7))
}
func op1ri6_enc(opcode Opcode, rc, imm6 int) uint16 {
    return uint16((int(opcode) << 2) | ((imm6 & 63)<<7) | ((rc & 7)<<13))
}
func op2ri3_enc(opcode Opcode, rc, rb, imm3  int) uint16 {
    return uint16((int(opcode) << 2) | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}
func op3ri0_enc(opcode Opcode, rc, rb, ra int) uint16 {
    return uint16((int(opcode) << 2) | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13))
}

func CPU_encode_op_break(imm9 int) uint16 {
    return op0ri9_enc(CPU_op_break, imm9)
}
func CPU_encode_op_j(pcrel9 int) uint16 {
    return op0ri9_enc(CPU_op_j, pcrel9 >> 1)
}
func CPU_encode_op_b(pcrel9 int) uint16 {
    return op0ri9_enc(CPU_op_b, pcrel9 >> 1)
}
func CPU_encode_op_ibj(pcrel9 int) uint16 {
    return op0ri9_enc(CPU_op_ibj, pcrel9 >> 6)
}
func CPU_encode_op_jalib(rc, ibrel6 int) uint16 {
    return op1ri6_enc(CPU_op_jalib, rc, ibrel6)
}
func CPU_encode_op_jtlib(rc, ibrel6 int) uint16 {
    return op1ri6_enc(CPU_op_jtlib, rc, ibrel6)
}
func CPU_encode_op_movib_i64(rc, ibrel6 int) uint16 {
    return op1ri6_enc(CPU_op_movib_i64, rc, ibrel6)
}
func CPU_encode_op_movi_i64(rc, imm6 int) uint16 {
    return op1ri6_enc(CPU_op_movi_i64, rc, imm6)
}
func CPU_encode_op_addi_i64(rc, imm6 int) uint16 {
    return op1ri6_enc(CPU_op_addi_i64, rc, imm6)
}
func CPU_encode_op_srli_i64(rc, imm6 int) uint16 {
    return op1ri6_enc(CPU_op_srli_i64, rc, imm6)
}
func CPU_encode_op_srai_i64(rc, imm6 int) uint16 {
    return op1ri6_enc(CPU_op_srai_i64, rc, imm6)
}
func CPU_encode_op_slli_i64(rc, imm6 int) uint16 {
    return op1ri6_enc(CPU_op_slli_i64, rc, imm6)
}
func CPU_encode_op_addib_i64(rc, ibimm6 int) uint16 {
    return op1ri6_enc(CPU_op_addib_i64, rc, ibimm6)
}
func CPU_encode_op_leapc_i64(rc, ibimm6 int) uint16 {
    return op1ri6_enc(CPU_op_leapc_i64, rc, ibimm6)
}
func CPU_encode_op_loadpc_i64(rc, ibimm6 int) uint16 {
    return op1ri6_enc(CPU_op_loadpc_i64, rc, ibimm6)
}
func CPU_encode_op_storepc_i64(rc, ibimm6 int) uint16 {
    return op1ri6_enc(CPU_op_storepc_i64, rc, ibimm6)
}
func CPU_encode_op_load_i64(rc, rb, imm3 int) uint16 {
    return op2ri3_enc(CPU_op_load_i64, rc, rb, imm3 >> 3)
}
func CPU_encode_op_store_i64(rc, rb, imm3 int) uint16 {
    return op2ri3_enc(CPU_op_store_i64, rc, rb, imm3 >> 3)
}
func CPU_encode_op_cmp_lt_i64(rc, rb int) uint16 {
    return op2ri3_enc(CPU_op_compare_i64, rc, rb, int(CPU_compare_lt))
}
func CPU_encode_op_cmp_ge_i64(rc, rb int) uint16 {
    return op2ri3_enc(CPU_op_compare_i64, rc, rb, int(CPU_compare_ge))
}
func CPU_encode_op_cmp_eq_i64(rc, rb int) uint16 {
    return op2ri3_enc(CPU_op_compare_i64, rc, rb, int(CPU_compare_eq))
}
func CPU_encode_op_cmp_ne_i64(rc, rb int) uint16 {
    return op2ri3_enc(CPU_op_compare_i64, rc, rb, int(CPU_compare_ne))
}
func CPU_encode_op_cmp_ltu_i64(rc, rb int) uint16 {
    return op2ri3_enc(CPU_op_compare_i64, rc, rb, int(CPU_compare_ltu))
}
func CPU_encode_op_cmp_geu_i64(rc, rb int) uint16 {
    return op2ri3_enc(CPU_op_compare_i64, rc, rb, int(CPU_compare_geu))
}
func CPU_encode_op_cmov_i64(rc, rb int) uint16 {
    return op2ri3_enc(CPU_op_compare_i64, rc, rb, int(CPU_compare_mov))
}
func CPU_encode_op_mov_i64(rc, rb int) uint16 {
    return op2ri3_enc(CPU_op_logic_i64, rc, rb, int(CPU_logic_mov))
}
func CPU_encode_op_not_i64(rc, rb int) uint16 {
    return op2ri3_enc(CPU_op_logic_i64, rc, rb, int(CPU_logic_not))
}
func CPU_encode_op_neg_i64(rc, rb int) uint16 {
    return op2ri3_enc(CPU_op_logic_i64, rc, rb, int(CPU_logic_neg))
}
func CPU_encode_op_bswap_i64(rc, rb int) uint16 {
    return op2ri3_enc(CPU_op_logic_i64, rc, rb, int(CPU_logic_bswap))
}
func CPU_encode_op_ctz_i64(rc, rb int) uint16 {
    return op2ri3_enc(CPU_op_logic_i64, rc, rb, int(CPU_logic_ctz))
}
func CPU_encode_op_clz_i64(rc, rb int) uint16 {
    return op2ri3_enc(CPU_op_logic_i64, rc, rb, int(CPU_logic_clz))
}
func CPU_encode_op_ctpop_i64(rc, rb int) uint16 {
    return op2ri3_enc(CPU_op_logic_i64, rc, rb, int(CPU_logic_ctpop))
}
func CPU_encode_op_pin_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(CPU_op_pin_i64, rc, rb, ra)
}
func CPU_encode_op_and_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(CPU_op_and_i64, rc, rb, ra)
}
func CPU_encode_op_or_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(CPU_op_or_i64, rc, rb, ra)
}
func CPU_encode_op_xor_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(CPU_op_xor_i64, rc, rb, ra)
}
func CPU_encode_op_sub_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(CPU_op_sub_i64, rc, rb, ra)
}
func CPU_encode_op_srl_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(CPU_op_srl_i64, rc, rb, ra)
}
func CPU_encode_op_sra_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(CPU_op_sra_i64, rc, rb, ra)
}
func CPU_encode_op_sll_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(CPU_op_sll_i64, rc, rb, ra)
}
func CPU_encode_op_add_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(CPU_op_add_i64, rc, rb, ra)
}
func CPU_encode_op_mul_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(CPU_op_mul_i64, rc, rb, ra)
}
func CPU_encode_op_div_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(CPU_op_div_i64, rc, rb, ra)
}
func CPU_encode_op_illegal(imm9 int) uint16 {
    return op0ri9_enc(CPU_op_illegal, imm9)
}

/*
 * cpu dispatch
 */

func CPU_exec(cpu *CPUState, inst uint64) int {
    if ops(inst) != 0 {
        return -1
    }
    switch opc(inst) {
    case CPU_op_break: return CPU_exec_op_break(cpu, inst)
    case CPU_op_j: return CPU_exec_op_j(cpu, inst)
    case CPU_op_b: return CPU_exec_op_b(cpu, inst)
    case CPU_op_ibj: return CPU_exec_op_ibj(cpu, inst)
    case CPU_op_jalib: return CPU_exec_op_jalib(cpu, inst)
    case CPU_op_jtlib: return CPU_exec_op_jtlib(cpu, inst)
    case CPU_op_movib_i64: return CPU_exec_op_movib_i64(cpu, inst)
    case CPU_op_movi_i64: return CPU_exec_op_movi_i64(cpu, inst)
    case CPU_op_addi_i64: return CPU_exec_op_addi_i64(cpu, inst)
    case CPU_op_srli_i64: return CPU_exec_op_srli_i64(cpu, inst)
    case CPU_op_srai_i64: return CPU_exec_op_srai_i64(cpu, inst)
    case CPU_op_slli_i64: return CPU_exec_op_slli_i64(cpu, inst)
    case CPU_op_addib_i64: return CPU_exec_op_addib_i64(cpu, inst)
    case CPU_op_leapc_i64: return CPU_exec_op_leapc_i64(cpu, inst)
    case CPU_op_loadpc_i64: return CPU_exec_op_loadpc_i64(cpu, inst)
    case CPU_op_storepc_i64: return CPU_exec_op_storepc_i64(cpu, inst)
    case CPU_op_load_i64: return CPU_exec_op_load_i64(cpu, inst)
    case CPU_op_store_i64: return CPU_exec_op_store_i64(cpu, inst)
    case CPU_op_compare_i64: return CPU_exec_op_compare_i64(cpu, inst)
    case CPU_op_logic_i64: return CPU_exec_op_logic_i64(cpu, inst)
    case CPU_op_pin_i64: return CPU_exec_op_pin_i64(cpu, inst)
    case CPU_op_and_i64: return CPU_exec_op_and_i64(cpu, inst)
    case CPU_op_or_i64: return CPU_exec_op_or_i64(cpu, inst)
    case CPU_op_xor_i64: return CPU_exec_op_xor_i64(cpu, inst)
    case CPU_op_add_i64: return CPU_exec_op_add_i64(cpu, inst)
    case CPU_op_srl_i64: return CPU_exec_op_srl_i64(cpu, inst)
    case CPU_op_sra_i64: return CPU_exec_op_sra_i64(cpu, inst)
    case CPU_op_sll_i64: return CPU_exec_op_sll_i64(cpu, inst)
    case CPU_op_sub_i64: return CPU_exec_op_sub_i64(cpu, inst)
    case CPU_op_mul_i64: return CPU_exec_op_mul_i64(cpu, inst)
    case CPU_op_div_i64: return CPU_exec_op_div_i64(cpu, inst)
    case CPU_op_illegal: return CPU_exec_op_illegal(cpu, inst)
    }
    return -1
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
