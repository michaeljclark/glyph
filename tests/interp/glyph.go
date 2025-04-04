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
    Op_break        Opcode = 0b00000 // op0r_imm9
    Op_j            Opcode = 0b00001 // op0r_imm9 pcrel9*2
    Op_b            Opcode = 0b00010 // op0r_imm9 pcrel9*2
    Op_ibj          Opcode = 0b00011 // op0r_imm9 pcrel9*64
    Op_jalib        Opcode = 0b00100 // op1r_imm6 ibrel(imm6*8,i32x2)
    Op_jtlib        Opcode = 0b00101 // op1r_imm6 ibrel(imm6*8,i32x2)
    Op_lib_i64      Opcode = 0b00110 // op1r_imm6 ibrel(imm6*8,i64)
    Op_li_i64       Opcode = 0b00111 // op1r_imm6
    Op_addi_i64     Opcode = 0b01000 // op1r_imm6
    Op_srli_i64     Opcode = 0b01001 // op2r_imm3
    Op_srai_i64     Opcode = 0b01010 // op2r_imm3
    Op_slli_i64     Opcode = 0b01011 // op2r_imm3
    Op_addib_i64    Opcode = 0b01100 // op2r_imm3
    Op_load_i64     Opcode = 0b01101 // op2r_imm3
    Op_loadib_i64   Opcode = 0b01110 // op2r_imm3
    Op_cmp_i64      Opcode = 0b01111 // op2r_fun3
    Op_subib_i64    Opcode = 0b10000 // op2r_imm3
    Op_store_i64    Opcode = 0b10001 // op2r_imm3
    Op_storeib_i64  Opcode = 0b10010 // op2r_imm3
    Op_logic_i64    Opcode = 0b10011 // op2r_fun3
    Op_pin_i64      Opcode = 0b10100 // op3r
    Op_and_i64      Opcode = 0b10101 // op3r
    Op_or_i64       Opcode = 0b10110 // op3r
    Op_xor_i64      Opcode = 0b10111 // op3r
    Op_sub_i64      Opcode = 0b11000 // op3r
    Op_srl_i64      Opcode = 0b11001 // op3r
    Op_sra_i64      Opcode = 0b11010 // op3r
    Op_sll_i64      Opcode = 0b11011 // op3r
    Op_add_i64      Opcode = 0b11100 // op3r
    Op_mul_i64      Opcode = 0b11101 // op3r
    Op_div_i64      Opcode = 0b11110 // op3r
    Op_illegal      Opcode = 0b11111 // op0r_imm9
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
func CPU_exec_op_lib_i64(cpu *CPUState, inst uint64) int {
    cpu.R[rc(inst)] = CPU_const_i64(cpu, uimm6(inst))
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

type OpaFn func(inst uint64) string
type OpForm int

func op_nm(inst uint64) string {
    return fmt.Sprintf("%s", cpu_opcode_str[opc(inst)])
}
func op_compare(inst uint64) string {
    return fmt.Sprintf("%s", cpu_fun3_compare_str[uimm3(inst)]);
}
func op_logic(inst uint64) string {
    return fmt.Sprintf("%s", cpu_fun3_logic_str[uimm3(inst)]);
}
func op_ib3(inst uint64) string {
    return fmt.Sprintf("ib(%d)", uimm3(inst));
}
func op_ib6(inst uint64) string {
    return fmt.Sprintf("ib(%d)", uimm6(inst));
}
func op_ui3x8(inst uint64) string {
    return fmt.Sprintf("%d", uimm3(inst) << 3);
}
func op_ui6(inst uint64) string {
    return fmt.Sprintf("%d", uimm6(inst));
}
func op_ui9(inst uint64) string {
    return fmt.Sprintf("%d", uimm9(inst));
}
func op_si6(inst uint64) string {
    return fmt.Sprintf("%d", simm6(inst));
}
func op_si9x2(inst uint64) string {
    return fmt.Sprintf("%d", simm9(inst) << 1);
}
func op_si9x64(inst uint64) string {
    return fmt.Sprintf("%d", simm9(inst) << 6);
}
func op_rc(inst uint64) string {
    return fmt.Sprintf("r%d", rc(inst));
}
func op_rb(inst uint64) string {
    return fmt.Sprintf("r%d", rb(inst));
}
func op_ra(inst uint64) string {
    return fmt.Sprintf("r%d", ra(inst));
}
func op_sp(inst uint64) string {
    return fmt.Sprintf(" ");
}
func op_sc(inst uint64) string {
    return fmt.Sprintf(", ");
}
func op_op(inst uint64) string {
    return fmt.Sprintf("(");
}
func op_cp(inst uint64) string {
    return fmt.Sprintf(")");
}

const (
    op0r_uimm9 OpForm = iota
    op0r_simm9x2
    op0r_simm9x64
    op1r_ib32x2_uimm6_src
    op1r_ib32x2_uimm6_dst
    op1r_ib64_uimm6
    op1r_simm6
    op1r_uimm6
    op2r_ib64_uimm3
    op2r_mem64_uimm3x8
    op2r_mib64_uimm3
    op2r_fun3_compare
    op2r_fun3_logic
    op3r
)

var cpu_opcode_str = [32]string{
    Op_break:          "break",
    Op_j:              "j",
    Op_b:              "b",
    Op_ibj:            "ibj",
    Op_jalib:          "jalib",
    Op_jtlib:          "jtlib",
    Op_lib_i64:        "lib.i64",
    Op_li_i64:         "li.i64",
    Op_addi_i64:       "addi.i64",
    Op_srli_i64:       "srli.i64",
    Op_srai_i64:       "srai.i64",
    Op_slli_i64:       "slli.i64",
    Op_addib_i64:      "addib.i64",
    Op_load_i64:       "load.i64",
    Op_loadib_i64:     "loadib.i64",
    Op_cmp_i64:        "cmp.i64",
    Op_subib_i64:      "subib.i64",
    Op_store_i64:      "store.i64",
    Op_storeib_i64:    "storeib.i64",
    Op_logic_i64:      "logic.i64",
    Op_pin_i64:        "pin.i64",
    Op_and_i64:        "and.i64",
    Op_or_i64:         "or.i64",
    Op_xor_i64:        "xor.i64",
    Op_sub_i64:        "sub.i64",
    Op_srl_i64:        "srl.i64",
    Op_sra_i64:        "sra.i64",
    Op_sll_i64:        "sll.i64",
    Op_add_i64:        "add.i64",
    Op_mul_i64:        "mul.i64",
    Op_div_i64:        "div.i64",
    Op_illegal:        "illegal",
}

var cpu_fun3_compare_str = [8]string{
    Compare_lt:        "cmp.lt.i64",
    Compare_ge:        "cmp.ge.i64",
    Compare_eq:        "cmp.eq.i64",
    Compare_ne:        "cmp.ne.i64",
    Compare_ltu:       "cmp.ltu.i64",
    Compare_geu:       "cmp.geu.i64",
}

var cpu_fun3_logic_str = [8]string{
    Logic_mov:         "mov.i64",
    Logic_not:         "not.i64",
    Logic_neg:         "neg.i64",
    Logic_bswap:       "bswap.i64",
    Logic_ctz:         "ctz.i64",
    Logic_clz:         "clz.i64",
    Logic_ctpop:       "ctpop.i64",
}

var cpu_op_format_args = [14][10]OpaFn{
    op0r_uimm9:            { op_nm, op_sp, op_ui9 },
    op0r_simm9x2:          { op_nm, op_sp, op_si9x2 },
    op0r_simm9x64:         { op_nm, op_sp, op_si9x64 },
    op1r_ib32x2_uimm6_src: { op_nm, op_sp, op_rc, op_sc, op_ib6 },
    op1r_ib32x2_uimm6_dst: { op_nm, op_sp, op_ib6, op_sc, op_rc },
    op1r_ib64_uimm6:       { op_nm, op_sp, op_rc, op_sc, op_ib6 },
    op1r_simm6:            { op_nm, op_sp, op_rc, op_sc, op_si6 },
    op1r_uimm6:            { op_nm, op_sp, op_rc, op_sc, op_ui6 },
    op2r_ib64_uimm3:       { op_nm, op_sp, op_rc, op_sc, op_rb,
                             op_sc, op_ib3 },
    op2r_mem64_uimm3x8:    { op_nm, op_sp, op_rc, op_sc, op_ui3x8,
                             op_op, op_rb, op_cp },
    op2r_mib64_uimm3:      { op_nm, op_sp, op_rc, op_sc, op_ib3,
                             op_op, op_rb, op_cp },
    op2r_fun3_compare:     { op_compare, op_sp, op_rc, op_sc, op_rb },
    op2r_fun3_logic:       { op_logic, op_sp, op_rc, op_sc, op_rb },
    op3r:                  { op_nm, op_sp, op_rc, op_sc, op_rb,
                             op_sc, op_ra },
};

var cpu_op_format_type = [32]OpForm{
    Op_break:          op0r_uimm9,
    Op_j:              op0r_simm9x2,
    Op_b:              op0r_simm9x2,
    Op_ibj:            op0r_simm9x64,
    Op_jalib:          op1r_ib32x2_uimm6_src,
    Op_jtlib:          op1r_ib32x2_uimm6_dst,
    Op_lib_i64:        op1r_ib64_uimm6,
    Op_li_i64:         op1r_simm6,
    Op_addi_i64:       op1r_simm6,
    Op_srli_i64:       op1r_uimm6,
    Op_srai_i64:       op1r_uimm6,
    Op_slli_i64:       op1r_uimm6,
    Op_addib_i64:      op2r_ib64_uimm3,
    Op_load_i64:       op2r_mem64_uimm3x8,
    Op_loadib_i64:     op2r_mib64_uimm3,
    Op_cmp_i64:        op2r_fun3_compare,
    Op_subib_i64:      op2r_ib64_uimm3,
    Op_store_i64:      op2r_mem64_uimm3x8,
    Op_storeib_i64:    op2r_mib64_uimm3,
    Op_logic_i64:      op2r_fun3_logic,
    Op_pin_i64:        op3r,
    Op_and_i64:        op3r,
    Op_or_i64:         op3r,
    Op_xor_i64:        op3r,
    Op_sub_i64:        op3r,
    Op_srl_i64:        op3r,
    Op_sra_i64:        op3r,
    Op_sll_i64:        op3r,
    Op_add_i64:        op3r,
    Op_mul_i64:        op3r,
    Op_div_i64:        op3r,
    Op_illegal:        op0r_uimm9,
};

func CPU_disasm(inst, c uint64) string {
    var op Opcode = Opcode(opc(inst))
    var arr [10]OpaFn = cpu_op_format_args[cpu_op_format_type[op]]
    var sb strings.Builder
    for i := 0; i < len(arr) && arr[i] != nil; i++ {
        sb.WriteString(arr[i](inst))
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
    return op0ri9_enc(Op_break, imm9)
}
func CPU_encode_op_j(pcrel9 int) uint16 {
    return op0ri9_enc(Op_j, pcrel9 >> 1)
}
func CPU_encode_op_b(pcrel9 int) uint16 {
    return op0ri9_enc(Op_b, pcrel9 >> 1)
}
func CPU_encode_op_ibj(pcrel9 int) uint16 {
    return op0ri9_enc(Op_ibj, pcrel9 >> 6)
}
func CPU_encode_op_jalib(rc, ibrel6 int) uint16 {
    return op1ri6_enc(Op_jalib, rc, ibrel6)
}
func CPU_encode_op_jtlib(rc, ibrel6 int) uint16 {
    return op1ri6_enc(Op_jtlib, rc, ibrel6)
}
func CPU_encode_op_lib_i64(rc, ibrel6 int) uint16 {
    return op1ri6_enc(Op_lib_i64, rc, ibrel6)
}
func CPU_encode_op_li_i64(rc, imm6 int) uint16 {
    return op1ri6_enc(Op_li_i64, rc, imm6)
}
func CPU_encode_op_addi_i64(rc, imm6 int) uint16 {
    return op1ri6_enc(Op_addi_i64, rc, imm6)
}
func CPU_encode_op_srli_i64(rc, imm6 int) uint16 {
    return op1ri6_enc(Op_srli_i64, rc, imm6)
}
func CPU_encode_op_srai_i64(rc, imm6 int) uint16 {
    return op1ri6_enc(Op_srai_i64, rc, imm6)
}
func CPU_encode_op_slli_i64(rc, imm6 int) uint16 {
    return op1ri6_enc(Op_slli_i64, rc, imm6)
}
func CPU_encode_op_addib_i64(rc, rb, ibimm3 int) uint16 {
    return op2ri3_enc(Op_addib_i64, rc, rb, ibimm3)
}
func CPU_encode_op_load_i64(rc, rb, imm3 int) uint16 {
    return op2ri3_enc(Op_load_i64, rc, rb, imm3 >> 3)
}
func CPU_encode_op_loadib_i64(rc, rb, ibimm3 int) uint16 {
    return op2ri3_enc(Op_loadib_i64, rc, rb, ibimm3)
}
func CPU_encode_op_cmp_i64(rc, rb int, fun3 Fun3Compare) uint16 {
    return op2ri3_enc(Op_cmp_i64, rc, rb, int(fun3))
}
func CPU_encode_op_subib_i64(rc, rb, ibimm3 int) uint16 {
    return op2ri3_enc(Op_subib_i64, rc, rb, ibimm3)
}
func CPU_encode_op_store_i64(rc, rb, imm3 int) uint16 {
    return op2ri3_enc(Op_store_i64, rc, rb, imm3 >> 3)
}
func CPU_encode_op_storeib_i64(rc, rb, ibimm3 int) uint16 {
    return op2ri3_enc(Op_storeib_i64, rc, rb, ibimm3)
}
func CPU_encode_op_logic_i64(rc, rb int, fun3 Fun3Logic) uint16 {
    return op2ri3_enc(Op_logic_i64, rc, rb, int(fun3))
}
func CPU_encode_op_pin_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(Op_pin_i64, rc, rb, ra)
}
func CPU_encode_op_and_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(Op_and_i64, rc, rb, ra)
}
func CPU_encode_op_or_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(Op_or_i64, rc, rb, ra)
}
func CPU_encode_op_xor_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(Op_xor_i64, rc, rb, ra)
}
func CPU_encode_op_sub_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(Op_sub_i64, rc, rb, ra)
}
func CPU_encode_op_srl_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(Op_srl_i64, rc, rb, ra)
}
func CPU_encode_op_sra_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(Op_sra_i64, rc, rb, ra)
}
func CPU_encode_op_sll_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(Op_sll_i64, rc, rb, ra)
}
func CPU_encode_op_add_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(Op_add_i64, rc, rb, ra)
}
func CPU_encode_op_mul_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(Op_mul_i64, rc, rb, ra)
}
func CPU_encode_op_div_i64(rc, rb, ra int) uint16 {
    return op3ri0_enc(Op_div_i64, rc, rb, ra)
}
func CPU_encode_op_illegal(imm9 int) uint16 {
    return op0ri9_enc(Op_illegal, imm9)
}

/*
 * cpu dispatch
 */

func CPU_exec(cpu *CPUState, inst uint64) int {
    switch opc(inst) {
    case Op_break: return CPU_exec_op_break(cpu, inst)
    case Op_j: return CPU_exec_op_j(cpu, inst)
    case Op_b: return CPU_exec_op_b(cpu, inst)
    case Op_ibj: return CPU_exec_op_ibj(cpu, inst)
    case Op_jalib: return CPU_exec_op_jalib(cpu, inst)
    case Op_jtlib: return CPU_exec_op_jtlib(cpu, inst)
    case Op_lib_i64: return CPU_exec_op_lib_i64(cpu, inst)
    case Op_li_i64: return CPU_exec_op_li_i64(cpu, inst)
    case Op_addi_i64: return CPU_exec_op_addi_i64(cpu, inst)
    case Op_srli_i64: return CPU_exec_op_srli_i64(cpu, inst)
    case Op_srai_i64: return CPU_exec_op_srai_i64(cpu, inst)
    case Op_slli_i64: return CPU_exec_op_slli_i64(cpu, inst)
    case Op_addib_i64: return CPU_exec_op_addib_i64(cpu, inst)
    case Op_load_i64: return CPU_exec_op_load_i64(cpu, inst)
    case Op_loadib_i64: return CPU_exec_op_loadib_i64(cpu, inst)
    case Op_cmp_i64: return CPU_exec_op_cmp_i64(cpu, inst)
    case Op_subib_i64: return CPU_exec_op_subib_i64(cpu, inst)
    case Op_store_i64: return CPU_exec_op_store_i64(cpu, inst)
    case Op_storeib_i64: return CPU_exec_op_storeib_i64(cpu, inst)
    case Op_logic_i64: return CPU_exec_op_logic_i64(cpu, inst)
    case Op_pin_i64: return CPU_exec_op_pin_i64(cpu, inst)
    case Op_and_i64: return CPU_exec_op_and_i64(cpu, inst)
    case Op_or_i64: return CPU_exec_op_or_i64(cpu, inst)
    case Op_xor_i64: return CPU_exec_op_xor_i64(cpu, inst)
    case Op_sub_i64: return CPU_exec_op_sub_i64(cpu, inst)
    case Op_srl_i64: return CPU_exec_op_srl_i64(cpu, inst)
    case Op_sra_i64: return CPU_exec_op_sra_i64(cpu, inst)
    case Op_sll_i64: return CPU_exec_op_sll_i64(cpu, inst)
    case Op_add_i64: return CPU_exec_op_add_i64(cpu, inst)
    case Op_mul_i64: return CPU_exec_op_mul_i64(cpu, inst)
    case Op_div_i64: return CPU_exec_op_div_i64(cpu, inst)
    case Op_illegal: return CPU_exec_op_illegal(cpu, inst)
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
