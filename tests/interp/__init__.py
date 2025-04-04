#!/usr/bin/env python3
#
# glyph is a super regular RISC that encodes constants in immediate blocks.
#
# Copyright (c) 2024-2025 Michael Clark <michaeljclark@mac.com>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

from enum import Enum

#
# opcodes
#

class Opcode(Enum):
    op_break        = 0b00000 # op0r_imm9
    op_j            = 0b00001 # op0r_imm9 pcrel9*2
    op_b            = 0b00010 # op0r_imm9 pcrel9*2
    op_ibj          = 0b00011 # op0r_imm9 pcrel9*64
    op_jalib        = 0b00100 # op1r_imm6 ibrel(imm6*8,i32x2)
    op_jtlib        = 0b00101 # op1r_imm6 ibrel(imm6*8,i32x2)
    op_lib_i64      = 0b00110 # op1r_imm6 ibrel(imm6*8,i64)
    op_li_i64       = 0b00111 # op1r_imm6
    op_addi_i64     = 0b01000 # op1r_imm6
    op_srli_i64     = 0b01001 # op2r_imm3
    op_srai_i64     = 0b01010 # op2r_imm3
    op_slli_i64     = 0b01011 # op2r_imm3
    op_addib_i64    = 0b01100 # op2r_imm3
    op_load_i64     = 0b01101 # op2r_imm3
    op_loadib_i64   = 0b01110 # op2r_imm3
    op_cmp_i64      = 0b01111 # op2r_fun3
    op_subib_i64    = 0b10000 # op2r_imm3
    op_store_i64    = 0b10001 # op2r_imm3
    op_storeib_i64  = 0b10010 # op2r_imm3
    op_logic_i64    = 0b10011 # op2r_fun3
    op_pin_i64      = 0b10100 # op3r
    op_and_i64      = 0b10101 # op3r
    op_or_i64       = 0b10110 # op3r
    op_xor_i64      = 0b10111 # op3r
    op_sub_i64      = 0b11000 # op3r
    op_srl_i64      = 0b11001 # op3r
    op_sra_i64      = 0b11010 # op3r
    op_sll_i64      = 0b11011 # op3r
    op_add_i64      = 0b11100 # op3r
    op_mul_i64      = 0b11101 # op3r
    op_div_i64      = 0b11110 # op3r
    op_illegal      = 0b11111 # op0r_imm9

#
# compare op fun3
#

class Fun3Compare(Enum):
    compare_lt      = 0b000
    compare_ge      = 0b001
    compare_eq      = 0b010
    compare_ne      = 0b011
    compare_ltu     = 0b100
    compare_geu     = 0b101

#
# logic op fun3
#

class Fun3Logic(Enum):
    logic_mov       = 0b000
    logic_not       = 0b001
    logic_neg       = 0b010
    logic_bswap     = 0b011
    logic_ctz       = 0b100
    logic_clz       = 0b101
    logic_ctpop     = 0b110

#
# cpu state
#

reg_count = 8

class CpuState():
    def __init__(self, mem_size):
        self.flag = 0
        self.r = [ 0 ] * reg_count
        self.pc = 0x800
        self.ib = 0x400
        self.mem = bytearray(mem_size)
        self.is_trace = True
        self.is_dump = False

def cpu_debug(*args):
    print(*args)

#
# bitmanip functions
#

def bswap(w,v):
    return int.from_bytes(v.to_bytes(w>>3, byteorder='little'), byteorder='big')

def clz(v):
    count = 0
    for i in reversed(range(128)):
        if (v & (1 << i)) == 0:
            count += 1
        else:
            break
    return count

def ctz(v):
    count = 0
    for i in range(v.bit_length()):
        if (v & (1 << i)) == 0:
            count += 1
        else:
            break
    return count

def ctpop(v):
    count = 0
    for i in range(v.bit_length()):
        if (v & (1 << i)) != 0:
            count += 1
    return count

#
# integer utilities
#

def suw(w,n):
    x = (2 ** w)
    return n & (x-1)

def usw(w,n):
    x = (2 ** w)
    t = n & (x-1)
    return t - x if n & (x >> 1) else t

def su64(val):
    return suw(64, val)
def su32(val):
    return suw(32, val)
def su16(val):
    return suw(16, val)
def su8(val):
    return suw(8, val)
def us64(val):
    return usw(64, val)
def us32(val):
    return usw(32, val)
def us16(val):
    return usw(16, val)
def us8(val):
    return usw(8, val)

def sux(val):
    return su64(val)
def usx(val):
    return us64(val)

#
# instruction decode helpers
#

def opc(inst):
    return (inst >> 2) & 0b11111
def uimm9(inst):
    return (inst >> 7) & 0b111111111
def uimm6(inst):
    return (inst >> 7) & 0b111111
def uimm3(inst):
    return (inst >> 7) & 0b111
def simm9(inst):
    return usw(9, (inst >> 7))
def simm6(inst):
    return usw(6, (inst >> 7))
def ra(inst):
    return (inst >> 7) & 0b111
def rb(inst):
    return (inst >> 10) & 0b111
def rc(inst):
    return (inst >> 13) & 0b111

#
# cpu load, store and constants
#

def cpu_load_i64(cpu, o):
    return int.from_bytes(cpu.mem[o:o+8], byteorder='little')
def cpu_load_i32(cpu, o):
    return int.from_bytes(cpu.mem[o:o+4], byteorder='little')
def cpu_load_i16(cpu, o):
    return int.from_bytes(cpu.mem[o:o+2], byteorder='little')
def cpu_load_i8(cpu, o):
    return cpu.mem[o]
def cpu_store_i64(cpu, o, val):
    cpu.mem[o:o+8] = su64(val).to_bytes(8, byteorder='little')
def cpu_store_i32(cpu, o, val):
    cpu.mem[o:o+4] = su32(val).to_bytes(4, byteorder='little')
def cpu_store_i16(cpu, o, val):
    cpu.mem[o:o+2] = su16(val).to_bytes(2, byteorder='little')
def cpu_store_i8(cpu, o, val):
    cpu.mem[o] = su8(val)
def cpu_const_i64(cpu, slot):
    o = cpu.ib + slot * 8
    return int.from_bytes(cpu.mem[o:o+8], byteorder='little')
def cpu_fetch_i16(cpu):
    o = cpu.pc
    return int.from_bytes(cpu.mem[o:o+2], byteorder='little')

#
# cpu emulation
#

def cpu_exec_op_break(cpu,inst):
    return -1
def cpu_exec_op_j(cpu,inst):
    cpu.pc = sux(cpu.pc + (simm9(inst) << 1) + 2)
    return 0
def cpu_exec_op_b(cpu,inst):
    if not cpu.flag:
        return 2
    cpu.pc = sux(cpu.pc + (simm9(inst) << 1) + 2)
    return 0
def cpu_exec_op_ibj(cpu,inst):
    cpu.ib = sux(cpu.ib +  (simm9(inst) << 6))
    return 2
def cpu_exec_op_jalib(cpu,inst):
    rav = cpu_const_i64(cpu, uimm6(inst))
    rpc = us32(rav      )
    rib = us32(rav >> 32)
    cpu.pc = sux(cpu.pc + rpc + 2)
    cpu.ib = sux(cpu.ib + rib)
    cpu.r[rc(inst)] = sux(rav)
    return 0
def cpu_exec_op_jtlib(cpu,inst):
    rav = cpu.r[rc(inst)]
    rpc = us32(rav      )
    rib = us32(rav >> 32)
    dav = cpu_const_i64(cpu, uimm6(inst))
    dpc = us32(dav      )
    dib = us32(dav >> 32)
    cpu.pc = sux(cpu.pc + dpc - rpc)
    cpu.ib = sux(cpu.ib + dib - rib)
    return 0
def cpu_exec_op_lib_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(cpu_const_i64(cpu, uimm6(inst)))
    return 2
def cpu_exec_op_li_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(simm6(inst))
    return 2
def cpu_exec_op_addi_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(cpu.r[rc(inst)] + simm6(inst))
    return 2
def cpu_exec_op_srli_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(cpu.r[rc(inst)] >> uimm6(inst))
    return 2
def cpu_exec_op_srai_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(usx(cpu.r[rc(inst)]) >> uimm6(inst))
    return 2
def cpu_exec_op_slli_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(cpu.r[rc(inst)] << uimm6(inst))
    return 2
def cpu_exec_op_addib_i64(cpu,inst):
    tmp = cpu.r[rb(inst)] + cpu_const_i64(cpu, uimm3(inst))
    cpu.r[rc(inst)] = sux(tmp)
    return 2
def cpu_exec_op_load_i64(cpu,inst):
    tmp = cpu.r[rb(inst)] + (uimm3(inst) << 3)
    cpu.r[rc(inst)] = sux(cpu_load_i64(cpu, tmp))
    return 2
def cpu_exec_op_loadib_i64(cpu,inst):
    tmp = cpu.r[rb(inst)] + cpu_const_i64(cpu, uimm3(inst))
    cpu.r[rc(inst)] = sux(cpu_load_i64(cpu, tmp))
    return 2
def cpu_exec_op_cmp_i64(cpu,inst):
    fun = Fun3Compare(uimm3(inst))
    if fun == Fun3Compare.compare_lt:
        cpu.flag = cpu.r[rc(inst)] < cpu.r[rb(inst)]
    elif fun == Fun3Compare.compare_ge:
        cpu.flag = cpu.r[rc(inst)] >= cpu.r[rb(inst)]
    elif fun == Fun3Compare.compare_eq:
        cpu.flag = cpu.r[rc(inst)] == cpu.r[rb(inst)]
    elif fun == Fun3Compare.compare_ne:
        cpu.flag = cpu.r[rc(inst)] != cpu.r[rb(inst)]
    elif fun == Fun3Compare.compare_ltu:
        cpu.flag = usx(cpu.r[rc(inst)]) < usx(cpu.r[rb(inst)])
    elif fun == Fun3Compare.compare_geu:
        cpu.flag = usx(cpu.r[rc(inst)]) >= usx(cpu.r[rb(inst)])
    return 2
def cpu_exec_op_subib_i64(cpu,inst):
    tmp = cpu.r[rb(inst)] - cpu_const_i64(cpu, uimm3(inst))
    cpu.r[rc(inst)] = sux(tmp)
    return 2
def cpu_exec_op_store_i64(cpu,inst):
    tmp = cpu.r[rb(inst)] + (uimm3(inst) << 3)
    cpu_store_i64(cpu, sux(tmp), cpu.r[rc(inst)])
    return 2
def cpu_exec_op_storeib_i64(cpu,inst):
    tmp = cpu.r[rb(inst)] + cpu_const_i64(cpu, uimm3(inst))
    cpu_store_i64(cpu, sux(tmp), cpu.r[rc(inst)])
    return 2
def cpu_exec_op_logic_i64(cpu,inst):
    fun = Fun3Logic(uimm3(inst))
    if fun == Fun3Logic.logic_mov:
        cpu.r[rc(inst)] = cpu.r[rb(inst)]
    elif fun == Fun3Logic.logic_not:
        cpu.r[rc(inst)] = sux(~cpu.r[rb(inst)])
    elif fun == Fun3Logic.logic_neg:
        cpu.r[rc(inst)] = sux(-cpu.r[rb(inst)])
    elif fun == Fun3Logic.logic_bswap:
        cpu.r[rc(inst)] = sux(bswap(64, cpu.r[rb(inst)]))
    elif fun == Fun3Logic.logic_ctz:
        cpu.r[rc(inst)] = ctz(cpu.r[rb(inst)])
    elif fun == Fun3Logic.logic_clz:
        cpu.r[rc(inst)] = clz(cpu.r[rb(inst)])
    elif fun == Fun3Logic.logic_ctpop:
        cpu.r[rc(inst)] = ctpop(cpu.r[rb(inst)])
    return 2
def cpu_exec_op_pin_i64(cpu,inst):
    rpc = cpu.pc - cpu.r[ra(inst)] + 2
    rib = cpu.ib - cpu.r[rb(inst)]
    cpu.r[rc(inst)] = su32(rpc) | (su32(rib) << 32)
    return 2
def cpu_exec_op_and_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(cpu.r[rb(inst)] & cpu.r[ra(inst)])
    return 2
def cpu_exec_op_or_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(cpu.r[rb(inst)] | cpu.r[ra(inst)])
    return 2
def cpu_exec_op_xor_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(cpu.r[rb(inst)] ^ cpu.r[ra(inst)])
    return 2
def cpu_exec_op_sub_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(cpu.r[rb(inst)] - cpu.r[ra(inst)])
    return 2
def cpu_exec_op_srl_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(cpu.r[rb(inst)] >> cpu.r[ra(inst)])
    return 2
def cpu_exec_op_sra_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(usx(cpu.r[rb(inst)]) >> cpu.r[ra(inst)])
    return 2
def cpu_exec_op_sll_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(cpu.r[rb(inst)] << cpu.r[ra(inst)]);
    return 2
def cpu_exec_op_add_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(cpu.r[rb(inst)] + cpu.r[ra(inst)]);
    return 2
def cpu_exec_op_mul_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(usx(cpu.r[rb(inst)]) * usx(cpu.r[ra(inst)]));
    return 2
def cpu_exec_op_div_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(usx(cpu.r[rb(inst)]) // usx(cpu.r[ra(inst)]));
    return 2
def cpu_exec_op_illegal(cpu,inst):
    return -1

#
# cpu disassembly
#

def op_nm(inst):
    return "%s" % cpu_opcode_str[Opcode(opc(inst))]
def op_compare(inst):
    return "%s" % cpu_fun3_compare_str[Fun3Compare(uimm3(inst))]
def op_logic(inst):
    return "%s" % cpu_fun3_logic_str[Fun3Logic(uimm3(inst))]
def op_ib3(inst):
    return "ib(%u)" % uimm3(inst)
def op_ib6(inst):
    return "ib(%u)" % uimm6(inst)
def op_ui3x8(inst):
    return "%u" % (uimm3(inst) << 3)
def op_ui6(inst):
    return "%u" % uimm6(inst)
def op_ui9(inst):
    return "%u" % uimm9(inst)
def op_si6(inst):
    return "%d" % simm6(inst)
def op_si9x2(inst):
    return "%d" % (simm9(inst) << 1)
def op_si9x64(inst):
    return "%d" % (simm9(inst) << 6)
def op_rc(inst):
    return "r%d" % rc(inst)
def op_rb(inst):
    return "r%d" % rb(inst)
def op_ra(inst):
    return "r%d" % ra(inst)
def op_sp(inst):
    return " "
def op_sc(inst):
    return ", "
def op_op(inst):
    return "("
def op_cp(inst):
    return ")"

class OpForm(Enum):
    op0r_uimm9            = 0
    op0r_simm9x2          = 1
    op0r_simm9x64         = 2
    op1r_ib32x2_uimm6_src = 3
    op1r_ib32x2_uimm6_dst = 4
    op1r_ib64_uimm6       = 5
    op1r_simm6            = 6
    op1r_uimm6            = 7
    op2r_ib64_uimm3       = 8
    op2r_mem64_uimm3x8    = 9
    op2r_mib64_uimm3      = 10
    op2r_fun3_compare     = 11
    op2r_fun3_logic       = 12
    op3r                  = 13

cpu_opcode_str = {
    Opcode.op_break:          "break",
    Opcode.op_j:              "j",
    Opcode.op_b:              "b",
    Opcode.op_ibj:            "ibj",
    Opcode.op_jalib:          "jalib",
    Opcode.op_jtlib:          "jtlib",
    Opcode.op_lib_i64:        "lib.i64",
    Opcode.op_li_i64:         "li.i64",
    Opcode.op_addi_i64:       "addi.i64",
    Opcode.op_srli_i64:       "srli.i64",
    Opcode.op_srai_i64:       "srai.i64",
    Opcode.op_slli_i64:       "slli.i64",
    Opcode.op_addib_i64:      "addib.i64",
    Opcode.op_load_i64:       "load.i64",
    Opcode.op_loadib_i64:     "loadib.i64",
    Opcode.op_cmp_i64:        "cmp.i64",
    Opcode.op_subib_i64:      "subib.i64",
    Opcode.op_store_i64:      "store.i64",
    Opcode.op_storeib_i64:    "storeib.i64",
    Opcode.op_logic_i64:      "logic.i64",
    Opcode.op_pin_i64:        "pin.i64",
    Opcode.op_and_i64:        "and.i64",
    Opcode.op_or_i64:         "or.i64",
    Opcode.op_xor_i64:        "xor.i64",
    Opcode.op_sub_i64:        "sub.i64",
    Opcode.op_srl_i64:        "srl.i64",
    Opcode.op_sra_i64:        "sra.i64",
    Opcode.op_sll_i64:        "sll.i64",
    Opcode.op_add_i64:        "add.i64",
    Opcode.op_mul_i64:        "mul.i64",
    Opcode.op_div_i64:        "div.i64",
    Opcode.op_illegal:        "illegal",
}

cpu_fun3_compare_str = {
    Fun3Compare.compare_lt:   "cmp.lt.i64",
    Fun3Compare.compare_ge:   "cmp.ge.i64",
    Fun3Compare.compare_eq:   "cmp.eq.i64",
    Fun3Compare.compare_ne:   "cmp.ne.i64",
    Fun3Compare.compare_ltu:  "cmp.ltu.i64",
    Fun3Compare.compare_geu:  "cmp.geu.i64",
}

cpu_fun3_logic_str = {
    Fun3Logic.logic_mov:      "mov.i64",
    Fun3Logic.logic_not:      "not.i64",
    Fun3Logic.logic_neg:      "neg.i64",
    Fun3Logic.logic_bswap:    "bswap.i64",
    Fun3Logic.logic_ctz:      "ctz.i64",
    Fun3Logic.logic_clz:      "clz.i64",
    Fun3Logic.logic_ctpop:    "ctpop.i64",
}

cpu_op_format_args = {
    OpForm.op0r_uimm9:            [ op_nm, op_sp, op_ui9 ],
    OpForm.op0r_simm9x2:          [ op_nm, op_sp, op_si9x2 ],
    OpForm.op0r_simm9x64:         [ op_nm, op_sp, op_si9x64 ],
    OpForm.op1r_ib32x2_uimm6_src: [ op_nm, op_sp, op_rc, op_sc, op_ib6 ],
    OpForm.op1r_ib32x2_uimm6_dst: [ op_nm, op_sp, op_ib6, op_sc, op_rc ],
    OpForm.op1r_ib64_uimm6:       [ op_nm, op_sp, op_rc, op_sc, op_ib6 ],
    OpForm.op1r_simm6:            [ op_nm, op_sp, op_rc, op_sc, op_si6 ],
    OpForm.op1r_uimm6:            [ op_nm, op_sp, op_rc, op_sc, op_ui6 ],
    OpForm.op2r_ib64_uimm3:       [ op_nm, op_sp, op_rc, op_sc, op_rb,
                                    op_sc, op_ib3 ],
    OpForm.op2r_mem64_uimm3x8:    [ op_nm, op_sp, op_rc, op_sc, op_ui3x8,
                                    op_op, op_rb, op_cp ],
    OpForm.op2r_mib64_uimm3:      [ op_nm, op_sp, op_rc, op_sc, op_ib3,
                                    op_op, op_rb, op_cp ],
    OpForm.op2r_fun3_compare:     [ op_compare, op_sp, op_rc, op_sc, op_rb ],
    OpForm.op2r_fun3_logic:       [ op_logic, op_sp, op_rc, op_sc, op_rb ],
    OpForm.op3r:                  [ op_nm, op_sp, op_rc, op_sc, op_rb,
                                    op_sc, op_ra ],
}

cpu_op_format_type = {
    Opcode.op_break:          OpForm.op0r_uimm9,
    Opcode.op_j:              OpForm.op0r_simm9x2,
    Opcode.op_b:              OpForm.op0r_simm9x2,
    Opcode.op_ibj:            OpForm.op0r_simm9x64,
    Opcode.op_jalib:          OpForm.op1r_ib32x2_uimm6_src,
    Opcode.op_jtlib:          OpForm.op1r_ib32x2_uimm6_dst,
    Opcode.op_lib_i64:        OpForm.op1r_ib64_uimm6,
    Opcode.op_li_i64:         OpForm.op1r_simm6,
    Opcode.op_addi_i64:       OpForm.op1r_simm6,
    Opcode.op_srli_i64:       OpForm.op1r_uimm6,
    Opcode.op_srai_i64:       OpForm.op1r_uimm6,
    Opcode.op_slli_i64:       OpForm.op1r_uimm6,
    Opcode.op_addib_i64:      OpForm.op2r_ib64_uimm3,
    Opcode.op_load_i64:       OpForm.op2r_mem64_uimm3x8,
    Opcode.op_loadib_i64:     OpForm.op2r_mib64_uimm3,
    Opcode.op_cmp_i64:        OpForm.op2r_fun3_compare,
    Opcode.op_subib_i64:      OpForm.op2r_ib64_uimm3,
    Opcode.op_store_i64:      OpForm.op2r_mem64_uimm3x8,
    Opcode.op_storeib_i64:    OpForm.op2r_mib64_uimm3,
    Opcode.op_logic_i64:      OpForm.op2r_fun3_logic,
    Opcode.op_pin_i64:        OpForm.op3r,
    Opcode.op_and_i64:        OpForm.op3r,
    Opcode.op_or_i64:         OpForm.op3r,
    Opcode.op_xor_i64:        OpForm.op3r,
    Opcode.op_sub_i64:        OpForm.op3r,
    Opcode.op_srl_i64:        OpForm.op3r,
    Opcode.op_sra_i64:        OpForm.op3r,
    Opcode.op_sll_i64:        OpForm.op3r,
    Opcode.op_add_i64:        OpForm.op3r,
    Opcode.op_mul_i64:        OpForm.op3r,
    Opcode.op_div_i64:        OpForm.op3r,
    Opcode.op_illegal:        OpForm.op0r_uimm9,
}

def cpu_disasm(cpu,inst):
    tab = cpu_op_format_args[cpu_op_format_type[Opcode(opc(inst))]]
    return '%04x %s' % (su16(inst), ''.join(fn(inst) for fn in tab))

#
# cpu instruction encoding
#

def op0ri9_enc(opcode,imm9):
    return (opcode.value << 2) | ((imm9 & 511)<<7)
def op1ri6_enc(opcode,rc,imm6):
    return (opcode.value << 2) | ((imm6 & 63)<<7) | ((rc & 7)<<13)
def op2ri3_enc(opcode,rc,rb,imm3):
    return (opcode.value << 2) | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def op3ri0_enc(opcode,rc,rb,ra):
    return (opcode.value << 2) | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)

def cpu_encode_op_break(imm9):
    return op0ri9_enc(Opcode.op_break, imm9)
def cpu_encode_op_j(pcrel9):
    return op0ri9_enc(Opcode.op_j, pcrel9 >> 1)
def cpu_encode_op_b(pcrel9):
    return op0ri9_enc(Opcode.op_b, pcrel9 >> 1)
def cpu_encode_op_ibj(pcrel9):
    return op0ri9_enc(Opcode.op_ibj, pcrel9 >> 6)
def cpu_encode_op_jalib(rc, ibrel6):
    return op1ri6_enc(Opcode.op_jalib, rc, ibrel6)
def cpu_encode_op_jtlib(rc, ibrel6):
    return op1ri6_enc(Opcode.op_jtlib, rc, ibrel6)
def cpu_encode_op_lib_i64(rc, ibrel6):
    return op1ri6_enc(Opcode.op_lib_i64, rc, ibrel6)
def cpu_encode_op_li_i64(rc, imm6):
    return op1ri6_enc(Opcode.op_li_i64, rc, imm6)
def cpu_encode_op_addi_i64(rc, imm6):
    return op1ri6_enc(Opcode.op_addi_i64, rc, imm6)
def cpu_encode_op_srli_i64(rc, imm6):
    return op1ri6_enc(Opcode.op_srli_i64, rc, imm6)
def cpu_encode_op_srai_i64(rc, imm6):
    return op1ri6_enc(Opcode.op_srai_i64, rc, imm6)
def cpu_encode_op_slli_i64(rc, imm6):
    return op1ri6_enc(Opcode.op_slli_i64, rc, imm6)
def cpu_encode_op_addib_i64(rc, rb, ibimm3):
    return op2ri3_enc(Opcode.op_addib_i64, rc, rb, ibimm3)
def cpu_encode_op_load_i64(rc, rb, imm3):
    return op2ri3_enc(Opcode.op_load_i64, rc, rb, imm3 >> 3)
def cpu_encode_op_loadib_i64(rc, rb, ibimm3):
    return op2ri3_enc(Opcode.op_loadib_i64, rc, rb, ibimm3)
def cpu_encode_op_cmp_i64(rc, rb, fun3):
    return op2ri3_enc(Opcode.op_cmp_i64, rc, rb, fun3)
def cpu_encode_op_subib_i64(rc, rb, ibimm3):
    return op2ri3_enc(Opcode.op_subib_i64, rc, rb, ibimm3)
def cpu_encode_op_store_i64(rc, rb, imm3):
    return op2ri3_enc(Opcode.op_store_i64, rc, rb, imm3 >> 3)
def cpu_encode_op_storeib_i64(rc, rb, ibimm3):
    return op2ri3_enc(Opcode.op_storeib_i64, rc, rb, ibimm3)
def cpu_encode_op_logic_i64(rc, rb, fun3):
    return op2ri3_enc(Opcode.op_logic_i64, rc, rb, fun3)
def cpu_encode_op_pin_i64(rc, rb, ra):
    return op3ri0_enc(Opcode.op_pin_i64, rc, rb, ra)
def cpu_encode_op_and_i64(rc, rb, ra):
    return op3ri0_enc(Opcode.op_and_i64, rc, rb, ra)
def cpu_encode_op_or_i64(rc, rb, ra):
    return op3ri0_enc(Opcode.op_or_i64, rc, rb, ra)
def cpu_encode_op_xor_i64(rc, rb, ra):
    return op3ri0_enc(Opcode.op_xor_i64, rc, rb, ra)
def cpu_encode_op_sub_i64(rc, rb, ra):
    return op3ri0_enc(Opcode.op_sub_i64, rc, rb, ra)
def cpu_encode_op_srl_i64(rc, rb, ra):
    return op3ri0_enc(Opcode.op_srl_i64, rc, rb, ra)
def cpu_encode_op_sra_i64(rc, rb, ra):
    return op3ri0_enc(Opcode.op_sra_i64, rc, rb, ra)
def cpu_encode_op_sll_i64(rc, rb, ra):
    return op3ri0_enc(Opcode.op_sll_i64, rc, rb, ra)
def cpu_encode_op_add_i64(rc, rb, ra):
    return op3ri0_enc(Opcode.op_add_i64, rc, rb, ra)
def cpu_encode_op_mul_i64(rc, rb, ra):
    return op3ri0_enc(Opcode.op_mul_i64, rc, rb, ra)
def cpu_encode_op_div_i64(rc, rb, ra):
    return op3ri0_enc(Opcode.op_div_i64, rc, rb, ra)
def cpu_encode_op_illegal(imm9):
    return op0ri9_enc(Opcode.op_illegal, imm9)

#
# cpu dispatch
#

cpu_exec_table = {
    Opcode.op_break.value:          cpu_exec_op_break,
    Opcode.op_j.value:              cpu_exec_op_j,
    Opcode.op_b.value:              cpu_exec_op_b,
    Opcode.op_ibj.value:            cpu_exec_op_ibj,
    Opcode.op_jalib.value:          cpu_exec_op_jalib,
    Opcode.op_jtlib.value:          cpu_exec_op_jtlib,
    Opcode.op_lib_i64.value:        cpu_exec_op_lib_i64,
    Opcode.op_li_i64.value:         cpu_exec_op_li_i64,
    Opcode.op_addi_i64.value:       cpu_exec_op_addi_i64,
    Opcode.op_srli_i64.value:       cpu_exec_op_srli_i64,
    Opcode.op_srai_i64.value:       cpu_exec_op_srai_i64,
    Opcode.op_slli_i64.value:       cpu_exec_op_slli_i64,
    Opcode.op_addib_i64.value:      cpu_exec_op_addib_i64,
    Opcode.op_load_i64.value:       cpu_exec_op_load_i64,
    Opcode.op_loadib_i64.value:     cpu_exec_op_loadib_i64,
    Opcode.op_cmp_i64.value:        cpu_exec_op_cmp_i64,
    Opcode.op_subib_i64.value:      cpu_exec_op_subib_i64,
    Opcode.op_store_i64.value:      cpu_exec_op_store_i64,
    Opcode.op_storeib_i64.value:    cpu_exec_op_storeib_i64,
    Opcode.op_logic_i64.value:      cpu_exec_op_logic_i64,
    Opcode.op_pin_i64.value:        cpu_exec_op_pin_i64,
    Opcode.op_and_i64.value:        cpu_exec_op_and_i64,
    Opcode.op_or_i64.value:         cpu_exec_op_or_i64,
    Opcode.op_xor_i64.value:        cpu_exec_op_xor_i64,
    Opcode.op_sub_i64.value:        cpu_exec_op_sub_i64,
    Opcode.op_srl_i64.value:        cpu_exec_op_srl_i64,
    Opcode.op_sra_i64.value:        cpu_exec_op_sra_i64,
    Opcode.op_sll_i64.value:        cpu_exec_op_sll_i64,
    Opcode.op_add_i64.value:        cpu_exec_op_add_i64,
    Opcode.op_mul_i64.value:        cpu_exec_op_mul_i64,
    Opcode.op_div_i64.value:        cpu_exec_op_div_i64,
    Opcode.op_illegal.value:        cpu_exec_op_illegal
}

def cpu_exec(cpu,inst):
    op = Opcode(opc(inst))
    return cpu_exec_table[op.value](cpu, inst)

#
# cpu implementation
#

def cpu_dump(cpu):
    cpu_debug("pc:%016x ib:%016x flag:%d" % (
        cpu.pc, cpu.ib, cpu.flag))
    for i in range(0,reg_count-1,4):
        cpu_debug("r%d:%016x r%d:%016x r%d:%016x r%d:%016x" % (
            i+0, cpu.r[i+0], i+1, cpu.r[i+1],
            i+2, cpu.r[i+2], i+3, cpu.r[i+3]))

def cpu_run(cpu):
    while True:
        inst = cpu_fetch_i16(cpu)
        if cpu.is_trace:
            asm = cpu_disasm(cpu, inst)
            cpu_debug('-- %08x %s' % (cpu.pc, asm))
        ret = cpu_exec(cpu, inst)
        if ret < 0:
            cpu_debug("** %08x cpu exception" % cpu.pc)
            return
        cpu.pc += ret
        if cpu.is_dump:
            cpu_dump(cpu)

def cpu_setup(cpu,c,i):
    cpu_debug("\n# constants:")
    for j,k in enumerate(c):
        a = cpu.ib + j * 8
        cpu_debug("# %08x ib(%u) <- %016x" % (a, j, su64(k)))
        cpu_store_i64(cpu, a, k)
    cpu_debug("\n# instructions:")
    for j,k in enumerate(i):
        a = cpu.pc + j * 2
        t = cpu_disasm(cpu, k)
        cpu_debug("# %08x %s" % (a, t))
        cpu_store_i16(cpu, a, k)

def cpu_test(name,c,i):
    cpu_debug("# test: %s" % name)
    cpu = CpuState(8192)
    cpu_setup(cpu,c,i)
    cpu_debug()
    cpu_debug("++ begin")
    cpu_run(cpu)
    cpu_debug("++ end")
    cpu_debug()
    cpu_debug("# state")
    cpu_dump(cpu)
