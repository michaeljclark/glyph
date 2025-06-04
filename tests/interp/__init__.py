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

#
# opcodes
#

cpu_op_break        = 0b00000 # op0r_imm9
cpu_op_j            = 0b00001 # op0r_imm9 pcrel9*2
cpu_op_b            = 0b00010 # op0r_imm9 pcrel9*2
cpu_op_ibj          = 0b00011 # op0r_imm9 pcrel9*64
cpu_op_link_i64     = 0b00100 # op1r_imm6 ib64(imm6*8)
cpu_op_movh_i64     = 0b00101 # op1r_imm6 ib32(imm6*8)
cpu_op_movw_i64     = 0b00110 # op1r_imm6 ib64(imm6*8)
cpu_op_movi_i64     = 0b00111 # op1r_imm6
cpu_op_addi_i64     = 0b01000 # op1r_imm6
cpu_op_srli_i64     = 0b01001 # op1r_imm6
cpu_op_srai_i64     = 0b01010 # op1r_imm6
cpu_op_slli_i64     = 0b01011 # op1r_imm6
cpu_op_addh_i64     = 0b01100 # op1r_imm6 ib32(imm6*4)
cpu_op_leapc_i64    = 0b01101 # op1r_imm6 ib32(imm6*4)(pc)
cpu_op_loadpc_i64   = 0b01110 # op1r_imm6 ib32(imm6*4)(pc)
cpu_op_storepc_i64  = 0b01111 # op1r_imm6 ib32(imm6*4)(pc)
cpu_op_load_i64     = 0b10000 # op2r_imm3
cpu_op_store_i64    = 0b10001 # op2r_imm3
cpu_op_compare_i64  = 0b10010 # op2r_fun3
cpu_op_logic_i64    = 0b10011 # op2r_fun3
cpu_op_pin_i64      = 0b10100 # op3r
cpu_op_and_i64      = 0b10101 # op3r
cpu_op_or_i64       = 0b10110 # op3r
cpu_op_xor_i64      = 0b10111 # op3r
cpu_op_add_i64      = 0b11000 # op3r
cpu_op_srl_i64      = 0b11001 # op3r
cpu_op_sra_i64      = 0b11010 # op3r
cpu_op_sll_i64      = 0b11011 # op3r
cpu_op_sub_i64      = 0b11100 # op3r
cpu_op_mul_i64      = 0b11101 # op3r
cpu_op_div_i64      = 0b11110 # op3r
cpu_op_illegal      = 0b11111 # op0r_imm9

#
# compare op fun3
#

cpu_compare_lt      = 0b000
cpu_compare_ge      = 0b001
cpu_compare_eq      = 0b010
cpu_compare_ne      = 0b011
cpu_compare_ltu     = 0b100
cpu_compare_geu     = 0b101
cpu_compare_cmov    = 0b110
cpu_compare_ncmov   = 0b111

#
# logic op fun3
#

cpu_logic_mov       = 0b000
cpu_logic_not       = 0b001
cpu_logic_neg       = 0b010
cpu_logic_bswap     = 0b011
cpu_logic_ctz       = 0b100
cpu_logic_clz       = 0b101
cpu_logic_ctpop     = 0b110
cpu_logic_sext      = 0b111

#
# jlrib op fun3
#

cpu_link_jib         = 0b000
cpu_link_rsrv       = 0b001
cpu_link_jalib_r6   = 0b010
cpu_link_jalib_r7   = 0b011
cpu_link_jtlib_r6   = 0b100
cpu_link_jtlib_r7   = 0b101
cpu_link_jalaib_r6  = 0b110
cpu_link_jalaib_r7  = 0b111

#
# op arg
#

op_name               = 0
op_compare            = 1
op_logic              = 2
op_link               = 3
op_ib3                = 4
op_ib6                = 5
op_pcib6              = 6
op_ui3x8              = 7
op_ui6                = 8
op_ui9                = 9
op_si6                = 10
op_si9x2              = 11
op_si9x64             = 12
op_rcj                = 13
op_rc                 = 14
op_rb                 = 15
op_ra                 = 16
op_sp                 = 17
op_sc                 = 18
op_op                 = 19
op_cp                 = 20

#
# op form
#

op0r_uimm9            = 0
op0r_simm9x2          = 1
op0r_simm9x64         = 2
op1r_fun3_ib32x2_link = 3
op1r_ib32_uimm6       = 4
op1r_ib64_uimm6       = 5
op1r_simm6            = 6
op1r_uimm6            = 7
op1r_mib64_uimm6      = 8
op2r_mem64_uimm3x8    = 9
op2r_fun3_compare     = 10
op2r_fun3_logic       = 11
op3r                  = 12

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

def clz(w,v):
    count = 0
    for i in reversed(range(w)):
        if (v & (1 << i)) == 0:
            count += 1
        else:
            break
    return count

def ctz(w,v):
    count = 0
    for i in range(w):
        if (v & (1 << i)) == 0:
            count += 1
        else:
            break
    return count

def ctpop(w,v):
    count = 0
    for i in range(w):
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

def ops(inst):
    return inst & 0b11
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
def cpu_const_i32(cpu, slot):
    o = cpu.ib + slot * 4
    return us32(int.from_bytes(cpu.mem[o:o+4], byteorder='little'))
def cpu_fetch_i16(cpu):
    o = cpu.pc
    return int.from_bytes(cpu.mem[o:o+2], byteorder='little')
def auth_encrypt_i64(cpu, val):
    return val ^ 0x5555555555555555
def auth_decrypt_i64(cpu, val):
    return val ^ 0x5555555555555555

#
# cpu emulation
#

def cpu_exec_op_break(cpu,inst):
    return -1
def cpu_exec_op_j(cpu,inst):
    cpu.pc = sux(cpu.pc + (simm9(inst) << 1))
    return 0
def cpu_exec_op_b(cpu,inst):
    if not cpu.flag:
        return 2
    cpu.pc = sux(cpu.pc + (simm9(inst) << 1))
    return 0
def cpu_exec_op_ibj(cpu,inst):
    cpu.ib = sux(cpu.ib +  (simm9(inst) << 6))
    return 2
def cpu_exec_op_link_i64(cpu,inst):
    fun3 = rc(inst)
    reg = 0b110 + (fun3 & 1)

    c = cpu_const_i64(cpu, uimm6(inst))
    l = 0

    dpc, dib, lpc, lib = 0, 0, 0, 0

    if fun3 == cpu_link_jalaib_r6 or fun3 == cpu_link_jalaib_r7:
        l = auth_decrypt_i64(cpu, cpu.r[reg])
        dpc = us32(c      ) + us32(l      )
        dib = us32(c >> 32) + us32(l >> 32)
    else:
        dpc = us32(c      )
        dib = us32(c >> 32)

    if fun3 == cpu_link_jtlib_r6 or fun3 == cpu_link_jtlib_r7:
        l = auth_decrypt_i64(cpu, cpu.r[reg])
        lpc = us32(l      )
        lib = us32(l >> 32)

    cpu.pc = sux(cpu.pc + dpc - lpc)
    cpu.ib = sux(cpu.ib + dib - lib)

    if fun3 == cpu_link_jalib_r6 or fun3 == cpu_link_jalib_r7 or \
       fun3 == cpu_link_jalaib_r6 or fun3 == cpu_link_jalaib_r7:
        cpu.r[reg] = auth_encrypt_i64(cpu, su32(dpc) | (su32(dib) << 32))

    return 0
def cpu_exec_op_movh_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(cpu_const_i32(cpu, uimm6(inst)))
    return 2
def cpu_exec_op_movw_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(cpu_const_i64(cpu, uimm6(inst)))
    return 2
def cpu_exec_op_movi_i64(cpu,inst):
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
def cpu_exec_op_addh_i64(cpu,inst):
    tmp = cpu.r[rc(inst)] + cpu_const_i32(cpu, uimm6(inst))
    cpu.r[rc(inst)] = sux(tmp)
    return 2
def cpu_exec_op_leapc_i64(cpu,inst):
    tmp = cpu.pc + cpu_const_i32(cpu, uimm6(inst))
    cpu.r[rc(inst)] = sux(tmp)
    return 2
def cpu_exec_op_loadpc_i64(cpu,inst):
    tmp = cpu.pc + cpu_const_i32(cpu, uimm6(inst))
    cpu.r[rc(inst)] = sux(cpu_load_i64(cpu, tmp))
    return 2
def cpu_exec_op_storepc_i64(cpu,inst):
    tmp = cpu.pc + cpu_const_i32(cpu, uimm6(inst))
    cpu_store_i64(cpu, sux(tmp), cpu.r[rc(inst)])
    return 2
def cpu_exec_op_load_i64(cpu,inst):
    tmp = cpu.r[rb(inst)] + (uimm3(inst) << 3)
    cpu.r[rc(inst)] = sux(cpu_load_i64(cpu, tmp))
    return 2
def cpu_exec_op_store_i64(cpu,inst):
    tmp = cpu.r[rb(inst)] + (uimm3(inst) << 3)
    cpu_store_i64(cpu, sux(tmp), cpu.r[rc(inst)])
    return 2
def cpu_exec_op_compare_i64(cpu,inst):
    fun = uimm3(inst)
    if fun == cpu_compare_lt:
        cpu.flag = usx(cpu.r[rc(inst)]) < usx(cpu.r[rb(inst)])
    elif fun == cpu_compare_ge:
        cpu.flag = usx(cpu.r[rc(inst)]) >= usx(cpu.r[rb(inst)])
    elif fun == cpu_compare_eq:
        cpu.flag = cpu.r[rc(inst)] == cpu.r[rb(inst)]
    elif fun == cpu_compare_ne:
        cpu.flag = cpu.r[rc(inst)] != cpu.r[rb(inst)]
    elif fun == cpu_compare_ltu:
        cpu.flag = cpu.r[rc(inst)] < cpu.r[rb(inst)]
    elif fun == cpu_compare_geu:
        cpu.flag = cpu.r[rc(inst)] >= cpu.r[rb(inst)]
    elif fun == cpu_compare_cmov:
        if cpu.flag:
            cpu.r[rc(inst)] = cpu.r[rb(inst)]
    elif fun == cpu_compare_ncmov:
        if not cpu.flag:
            cpu.r[rc(inst)] = cpu.r[rb(inst)]
    return 2
def cpu_exec_op_logic_i64(cpu,inst):
    fun = uimm3(inst)
    if fun == cpu_logic_mov:
        # moves whole register irrespective of XLEN
        cpu.r[rc(inst)] = cpu.r[rb(inst)]
    elif fun == cpu_logic_not:
        cpu.r[rc(inst)] = sux(~cpu.r[rb(inst)])
    elif fun == cpu_logic_neg:
        cpu.r[rc(inst)] = sux(-cpu.r[rb(inst)])
    elif fun == cpu_logic_bswap:
        cpu.r[rc(inst)] = sux(bswap(64, cpu.r[rb(inst)]))
    elif fun == cpu_logic_ctz:
        cpu.r[rc(inst)] = ctz(64, cpu.r[rb(inst)])
    elif fun == cpu_logic_clz:
        cpu.r[rc(inst)] = clz(64, cpu.r[rb(inst)])
    elif fun == cpu_logic_ctpop:
        cpu.r[rc(inst)] = ctpop(64, cpu.r[rb(inst)])
    elif fun == cpu_logic_sext:
        # sign-extend is a move unless XLEN > 64
        cpu.r[rc(inst)] = cpu.r[rb(inst)]
    return 2
def cpu_exec_op_pin_i64(cpu,inst):
    rpc = cpu.pc - cpu.r[ra(inst)]
    rib = cpu.ib - cpu.r[rb(inst)]
    cpu.r[rc(inst)] = auth_encrypt_i64(cpu, su32(rpc) | (su32(rib) << 32))
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
def cpu_exec_op_add_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(cpu.r[rb(inst)] + cpu.r[ra(inst)]);
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
def cpu_exec_op_sub_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(cpu.r[rb(inst)] - cpu.r[ra(inst)])
    return 2
def cpu_exec_op_mul_i64(cpu,inst):
    cpu.r[rc(inst)] = sux(usx(cpu.r[rb(inst)]) * usx(cpu.r[ra(inst)]));
    return 2
def cpu_exec_op_div_i64(cpu,inst):
    cpu.flag = cpu.r[ra(inst)] == 0
    if cpu.flag:
        cpu.r[rc(inst)] = 0
    else:
        cpu.r[rc(inst)] = sux(usx(cpu.r[rb(inst)]) // usx(cpu.r[ra(inst)]));
    return 2
def cpu_exec_op_illegal(cpu,inst):
    return -1

#
# cpu disassembly
#

def op_out_name(inst):
    return "%s" % cpu_opcode_str[opc(inst)]
def op_out_compare(inst):
    return "%s" % cpu_fun3_compare_str[uimm3(inst)]
def op_out_logic(inst):
    return "%s" % cpu_fun3_logic_str[uimm3(inst)]
def op_out_link(inst):
    return "%s" % cpu_fun3_link_str[rc(inst)]
def op_out_ib3(inst):
    return "ib(%u)" % uimm3(inst)
def op_out_ib6(inst):
    return "ib(%u)" % uimm6(inst)
def op_out_pcib6(inst):
    return "ib(%u)(pc)" % uimm6(inst)
def op_out_ui3x8(inst):
    return "%u" % (uimm3(inst) << 3)
def op_out_ui6(inst):
    return "%u" % uimm6(inst)
def op_out_ui9(inst):
    return "%u" % uimm9(inst)
def op_out_si6(inst):
    return "%d" % simm6(inst)
def op_out_si9x2(inst):
    return "%d" % (simm9(inst) << 1)
def op_out_si9x64(inst):
    return "%d" % (simm9(inst) << 6)
def op_out_rcj(inst):
    return "r%d" % (0b110 + (rc(inst) & 1))
def op_out_rc(inst):
    return "r%d" % rc(inst)
def op_out_rb(inst):
    return "r%d" % rb(inst)
def op_out_ra(inst):
    return "r%d" % ra(inst)
def op_out_sp(inst):
    return " "
def op_out_sc(inst):
    return ", "
def op_out_op(inst):
    return "("
def op_out_cp(inst):
    return ")"

cpu_opcode_str = {
    cpu_op_break:          "break",
    cpu_op_j:              "j",
    cpu_op_b:              "b",
    cpu_op_ibj:            "ibj",
    cpu_op_link_i64:       "link.i64",
    cpu_op_movh_i64:       "movh.i64",
    cpu_op_movw_i64:       "movw.i64",
    cpu_op_movi_i64:       "movi.i64",
    cpu_op_addi_i64:       "addi.i64",
    cpu_op_srli_i64:       "srli.i64",
    cpu_op_srai_i64:       "srai.i64",
    cpu_op_slli_i64:       "slli.i64",
    cpu_op_addh_i64:       "addh.i64",
    cpu_op_leapc_i64:      "leapc.i64",
    cpu_op_loadpc_i64:     "loadpc.i64",
    cpu_op_storepc_i64:    "storepc.i64",
    cpu_op_load_i64:       "load.i64",
    cpu_op_store_i64:      "store.i64",
    cpu_op_compare_i64:    "compare.i64",
    cpu_op_logic_i64:      "logic.i64",
    cpu_op_pin_i64:        "pin.i64",
    cpu_op_and_i64:        "and.i64",
    cpu_op_or_i64:         "or.i64",
    cpu_op_xor_i64:        "xor.i64",
    cpu_op_add_i64:        "add.i64",
    cpu_op_srl_i64:        "srl.i64",
    cpu_op_sra_i64:        "sra.i64",
    cpu_op_sll_i64:        "sll.i64",
    cpu_op_sub_i64:        "sub.i64",
    cpu_op_mul_i64:        "mul.i64",
    cpu_op_div_i64:        "div.i64",
    cpu_op_illegal:        "illegal",
}

cpu_fun3_compare_str = {
    cpu_compare_lt:        "cmp.lt.i64",
    cpu_compare_ge:        "cmp.ge.i64",
    cpu_compare_eq:        "cmp.eq.i64",
    cpu_compare_ne:        "cmp.ne.i64",
    cpu_compare_ltu:       "cmp.ltu.i64",
    cpu_compare_geu:       "cmp.geu.i64",
    cpu_compare_cmov:      "cmov.i64",
    cpu_compare_ncmov:     "ncmov.i64",
}

cpu_fun3_logic_str = {
    cpu_logic_mov:         "mov.i64",
    cpu_logic_not:         "not.i64",
    cpu_logic_neg:         "neg.i64",
    cpu_logic_bswap:       "bswap.i64",
    cpu_logic_ctz:         "ctz.i64",
    cpu_logic_clz:         "clz.i64",
    cpu_logic_ctpop:       "ctpop.i64",
    cpu_logic_sext:        "sext.i64",
}


cpu_fun3_link_str = {
    cpu_link_jib:          "jib.i64",
    cpu_link_rsrv:         "link.rsrv",
    cpu_link_jalib_r6:     "jalib.i64",
    cpu_link_jalib_r7:     "jalib.i64",
    cpu_link_jtlib_r6:     "jtlib.i64",
    cpu_link_jtlib_r7:     "jtlib.i64",
    cpu_link_jalaib_r6:    "jalaib.i64",
    cpu_link_jalaib_r7:    "jalaib.i64",
}

cpu_op_out = {
    op_name:               op_out_name,
    op_compare:            op_out_compare,
    op_logic:              op_out_logic,
    op_link:               op_out_link,
    op_ib3:                op_out_ib3,
    op_ib6:                op_out_ib6,
    op_pcib6:              op_out_pcib6,
    op_ui3x8:              op_out_ui3x8,
    op_ui6:                op_out_ui6,
    op_ui9:                op_out_ui9,
    op_si6:                op_out_si6,
    op_si9x2:              op_out_si9x2,
    op_si9x64:             op_out_si9x64,
    op_rcj:                op_out_rcj,
    op_rc:                 op_out_rc,
    op_rb:                 op_out_rb,
    op_ra:                 op_out_ra,
    op_sp:                 op_out_sp,
    op_sc:                 op_out_sc,
    op_op:                 op_out_op,
    op_cp:                 op_out_cp,
}

cpu_op_args = {
    op0r_uimm9:            [ op_name, op_sp, op_ui9 ],
    op0r_simm9x2:          [ op_name, op_sp, op_si9x2 ],
    op0r_simm9x64:         [ op_name, op_sp, op_si9x64 ],
    op1r_fun3_ib32x2_link: [ op_link, op_sp, op_rcj, op_sc, op_ib6 ],
    op1r_ib32_uimm6:       [ op_name, op_sp, op_rc, op_sc, op_ib6 ],
    op1r_ib64_uimm6:       [ op_name, op_sp, op_rc, op_sc, op_ib6 ],
    op1r_simm6:            [ op_name, op_sp, op_rc, op_sc, op_si6 ],
    op1r_uimm6:            [ op_name, op_sp, op_rc, op_sc, op_ui6 ],
    op1r_mib64_uimm6:      [ op_name, op_sp, op_rc, op_sc, op_pcib6 ],
    op2r_mem64_uimm3x8:    [ op_name, op_sp, op_rc, op_sc, op_ui3x8,
                             op_op, op_rb, op_cp ],
    op2r_fun3_compare:     [ op_compare, op_sp, op_rc, op_sc, op_rb ],
    op2r_fun3_logic:       [ op_logic, op_sp, op_rc, op_sc, op_rb ],
    op3r:                  [ op_name, op_sp, op_rc, op_sc, op_rb,
                             op_sc, op_ra ],
}

cpu_op_type = {
    cpu_op_break:          op0r_uimm9,
    cpu_op_j:              op0r_simm9x2,
    cpu_op_b:              op0r_simm9x2,
    cpu_op_ibj:            op0r_simm9x64,
    cpu_op_link_i64:       op1r_fun3_ib32x2_link,
    cpu_op_movh_i64:       op1r_ib32_uimm6,
    cpu_op_movw_i64:       op1r_ib64_uimm6,
    cpu_op_movi_i64:       op1r_simm6,
    cpu_op_addi_i64:       op1r_simm6,
    cpu_op_srli_i64:       op1r_uimm6,
    cpu_op_srai_i64:       op1r_uimm6,
    cpu_op_slli_i64:       op1r_uimm6,
    cpu_op_addh_i64:       op1r_mib64_uimm6,
    cpu_op_leapc_i64:      op1r_mib64_uimm6,
    cpu_op_loadpc_i64:     op1r_mib64_uimm6,
    cpu_op_storepc_i64:    op1r_mib64_uimm6,
    cpu_op_load_i64:       op2r_mem64_uimm3x8,
    cpu_op_store_i64:      op2r_mem64_uimm3x8,
    cpu_op_compare_i64:    op2r_fun3_compare,
    cpu_op_logic_i64:      op2r_fun3_logic,
    cpu_op_pin_i64:        op3r,
    cpu_op_and_i64:        op3r,
    cpu_op_or_i64:         op3r,
    cpu_op_xor_i64:        op3r,
    cpu_op_add_i64:        op3r,
    cpu_op_srl_i64:        op3r,
    cpu_op_sra_i64:        op3r,
    cpu_op_sll_i64:        op3r,
    cpu_op_sub_i64:        op3r,
    cpu_op_mul_i64:        op3r,
    cpu_op_div_i64:        op3r,
    cpu_op_illegal:        op0r_uimm9,
}

def cpu_disasm(cpu,inst):
    if ops(inst) != 0:
        return "unknown"
    tab = cpu_op_args[cpu_op_type[opc(inst)]]
    return ''.join(cpu_op_out[arg](inst) for arg in tab)

#
# cpu instruction encoding
#

def op0ri9_enc(opc,imm9):
    return (opc << 2) | ((imm9 & 511)<<7)
def op1ri6_enc(opc,rc,imm6):
    return (opc << 2) | ((imm6 & 63)<<7) | ((rc & 7)<<13)
def op2ri3_enc(opc,rc,rb,imm3):
    return (opc << 2) | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def op3ri0_enc(opc,rc,rb,ra):
    return (opc << 2) | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)

def cpu_encode_op_break(imm9):
    return op0ri9_enc(cpu_op_break, imm9)
def cpu_encode_op_j(pcrel9):
    return op0ri9_enc(cpu_op_j, pcrel9 >> 1)
def cpu_encode_op_b(pcrel9):
    return op0ri9_enc(cpu_op_b, pcrel9 >> 1)
def cpu_encode_op_ibj(pcrel9):
    return op0ri9_enc(cpu_op_ibj, pcrel9 >> 6)
def cpu_encode_op_jf_i64(ibrel6):
    return op1ri6_enc(cpu_op_link_i64, cpu_link_jib, ibrel6)
def cpu_encode_op_jalib_i64(rc, ibrel6):
    return op1ri6_enc(cpu_op_link_i64, cpu_link_jalib_r6 | (rc & 1), ibrel6)
def cpu_encode_op_jtlib_i64(rc, ibrel6):
    return op1ri6_enc(cpu_op_link_i64, cpu_link_jtlib_r6 | (rc & 1), ibrel6)
def cpu_encode_op_jalaib_i64(rc, ibrel6):
    return op1ri6_enc(cpu_op_link_i64, cpu_link_jalaib_r6 | (rc & 1), ibrel6)
def cpu_encode_op_movh_i64(rc, ibrel6):
    return op1ri6_enc(cpu_op_movh_i64, rc, ibrel6)
def cpu_encode_op_movw_i64(rc, ibrel6):
    return op1ri6_enc(cpu_op_movw_i64, rc, ibrel6)
def cpu_encode_op_movi_i64(rc, imm6):
    return op1ri6_enc(cpu_op_movi_i64, rc, imm6)
def cpu_encode_op_addi_i64(rc, imm6):
    return op1ri6_enc(cpu_op_addi_i64, rc, imm6)
def cpu_encode_op_srli_i64(rc, imm6):
    return op1ri6_enc(cpu_op_srli_i64, rc, imm6)
def cpu_encode_op_srai_i64(rc, imm6):
    return op1ri6_enc(cpu_op_srai_i64, rc, imm6)
def cpu_encode_op_slli_i64(rc, imm6):
    return op1ri6_enc(cpu_op_slli_i64, rc, imm6)
def cpu_encode_op_addh_i64(rc, ibimm6):
    return op1ri6_enc(cpu_op_addh_i64, rc, ibimm6)
def cpu_encode_op_leapc_i64(rc, ibimm6):
    return op1ri6_enc(cpu_op_leapc_i64, rc, ibimm6)
def cpu_encode_op_loadpc_i64(rc, ibimm6):
    return op1ri6_enc(cpu_op_loadpc_i64, rc, ibimm6)
def cpu_encode_op_storepc_i64(rc, ibimm6):
    return op1ri6_enc(cpu_op_storepc_i64, rc, ibimm6)
def cpu_encode_op_load_i64(rc, rb, imm3):
    return op2ri3_enc(cpu_op_load_i64, rc, rb, imm3 >> 3)
def cpu_encode_op_store_i64(rc, rb, imm3):
    return op2ri3_enc(cpu_op_store_i64, rc, rb, imm3 >> 3)
def cpu_encode_op_cmp_lt_i64(rc, rb):
    return op2ri3_enc(cpu_op_compare_i64, rc, rb, cpu_compare_lt)
def cpu_encode_op_cmp_ge_i64(rc, rb):
    return op2ri3_enc(cpu_op_compare_i64, rc, rb, cpu_compare_ge)
def cpu_encode_op_cmp_eq_i64(rc, rb):
    return op2ri3_enc(cpu_op_compare_i64, rc, rb, cpu_compare_eq)
def cpu_encode_op_cmp_ne_i64(rc, rb):
    return op2ri3_enc(cpu_op_compare_i64, rc, rb, cpu_compare_ne)
def cpu_encode_op_cmp_ltu_i64(rc, rb):
    return op2ri3_enc(cpu_op_compare_i64, rc, rb, cpu_compare_ltu)
def cpu_encode_op_cmp_geu_i64(rc, rb):
    return op2ri3_enc(cpu_op_compare_i64, rc, rb, cpu_compare_geu)
def cpu_encode_op_cmov_i64(rc, rb):
    return op2ri3_enc(cpu_op_compare_i64, rc, rb, cpu_compare_cmov)
def cpu_encode_op_ncmov_i64(rc, rb):
    return op2ri3_enc(cpu_op_compare_i64, rc, rb, cpu_compare_ncmov)
def cpu_encode_op_mov_i64(rc, rb):
    return op2ri3_enc(cpu_op_logic_i64, rc, rb, cpu_logic_mov)
def cpu_encode_op_not_i64(rc, rb):
    return op2ri3_enc(cpu_op_logic_i64, rc, rb, cpu_logic_not)
def cpu_encode_op_neg_i64(rc, rb):
    return op2ri3_enc(cpu_op_logic_i64, rc, rb, cpu_logic_neg)
def cpu_encode_op_bswap_i64(rc, rb):
    return op2ri3_enc(cpu_op_logic_i64, rc, rb, cpu_logic_bswap)
def cpu_encode_op_ctz_i64(rc, rb):
    return op2ri3_enc(cpu_op_logic_i64, rc, rb, cpu_logic_ctz)
def cpu_encode_op_clz_i64(rc, rb):
    return op2ri3_enc(cpu_op_logic_i64, rc, rb, cpu_logic_clz)
def cpu_encode_op_ctpop_i64(rc, rb):
    return op2ri3_enc(cpu_op_logic_i64, rc, rb, cpu_logic_ctpop)
def cpu_encode_op_sext_i64(rc, rb):
    return op2ri3_enc(cpu_op_logic_i64, rc, rb, cpu_logic_sext)
def cpu_encode_op_pin_i64(rc, rb, ra):
    return op3ri0_enc(cpu_op_pin_i64, rc, rb, ra)
def cpu_encode_op_and_i64(rc, rb, ra):
    return op3ri0_enc(cpu_op_and_i64, rc, rb, ra)
def cpu_encode_op_or_i64(rc, rb, ra):
    return op3ri0_enc(cpu_op_or_i64, rc, rb, ra)
def cpu_encode_op_xor_i64(rc, rb, ra):
    return op3ri0_enc(cpu_op_xor_i64, rc, rb, ra)
def cpu_encode_op_sub_i64(rc, rb, ra):
    return op3ri0_enc(cpu_op_sub_i64, rc, rb, ra)
def cpu_encode_op_srl_i64(rc, rb, ra):
    return op3ri0_enc(cpu_op_srl_i64, rc, rb, ra)
def cpu_encode_op_sra_i64(rc, rb, ra):
    return op3ri0_enc(cpu_op_sra_i64, rc, rb, ra)
def cpu_encode_op_sll_i64(rc, rb, ra):
    return op3ri0_enc(cpu_op_sll_i64, rc, rb, ra)
def cpu_encode_op_add_i64(rc, rb, ra):
    return op3ri0_enc(cpu_op_add_i64, rc, rb, ra)
def cpu_encode_op_mul_i64(rc, rb, ra):
    return op3ri0_enc(cpu_op_mul_i64, rc, rb, ra)
def cpu_encode_op_div_i64(rc, rb, ra):
    return op3ri0_enc(cpu_op_div_i64, rc, rb, ra)
def cpu_encode_op_illegal(imm9):
    return op0ri9_enc(cpu_op_illegal, imm9)

#
# cpu dispatch
#

cpu_exec_table = {
    cpu_op_break:          cpu_exec_op_break,
    cpu_op_j:              cpu_exec_op_j,
    cpu_op_b:              cpu_exec_op_b,
    cpu_op_ibj:            cpu_exec_op_ibj,
    cpu_op_link_i64:       cpu_exec_op_link_i64,
    cpu_op_movh_i64:       cpu_exec_op_movh_i64,
    cpu_op_movw_i64:       cpu_exec_op_movw_i64,
    cpu_op_movi_i64:       cpu_exec_op_movi_i64,
    cpu_op_addi_i64:       cpu_exec_op_addi_i64,
    cpu_op_srli_i64:       cpu_exec_op_srli_i64,
    cpu_op_srai_i64:       cpu_exec_op_srai_i64,
    cpu_op_slli_i64:       cpu_exec_op_slli_i64,
    cpu_op_addh_i64:       cpu_exec_op_addh_i64,
    cpu_op_leapc_i64:      cpu_exec_op_leapc_i64,
    cpu_op_loadpc_i64:     cpu_exec_op_loadpc_i64,
    cpu_op_storepc_i64:    cpu_exec_op_storepc_i64,
    cpu_op_load_i64:       cpu_exec_op_load_i64,
    cpu_op_store_i64:      cpu_exec_op_store_i64,
    cpu_op_compare_i64:    cpu_exec_op_compare_i64,
    cpu_op_logic_i64:      cpu_exec_op_logic_i64,
    cpu_op_pin_i64:        cpu_exec_op_pin_i64,
    cpu_op_and_i64:        cpu_exec_op_and_i64,
    cpu_op_or_i64:         cpu_exec_op_or_i64,
    cpu_op_xor_i64:        cpu_exec_op_xor_i64,
    cpu_op_add_i64:        cpu_exec_op_add_i64,
    cpu_op_srl_i64:        cpu_exec_op_srl_i64,
    cpu_op_sra_i64:        cpu_exec_op_sra_i64,
    cpu_op_sll_i64:        cpu_exec_op_sll_i64,
    cpu_op_sub_i64:        cpu_exec_op_sub_i64,
    cpu_op_mul_i64:        cpu_exec_op_mul_i64,
    cpu_op_div_i64:        cpu_exec_op_div_i64,
    cpu_op_illegal:        cpu_exec_op_illegal
}

def cpu_exec(cpu,inst):
    return -1 if ops(inst) != 0 else cpu_exec_table[opc(inst)](cpu, inst)

#
# cpu implementation
#

def cpu_init(mem_size):
    return CpuState(mem_size)

def cpu_destroy(cpu):
    # release external resources
    pass

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
            cpu_debug('-- %08x %04x %s' % (cpu.pc, su16(inst), asm))
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
        cpu_debug("# %08x %04x %s" % (a, su16(k), t))
        cpu_store_i16(cpu, a, k)

def cpu_test(name,c,i):
    cpu_debug("# test: %s" % name)
    cpu = cpu_init(8192)
    cpu_setup(cpu,c,i)
    cpu_debug()
    cpu_debug("++ begin")
    cpu_run(cpu)
    cpu_debug("++ end")
    cpu_debug()
    cpu_debug("# state")
    cpu_dump(cpu)
