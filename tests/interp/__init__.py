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

# opcodes

class Opcode(Enum):
    op_break        = 0b00000 << 2 # op0r_imm9
    op_j            = 0b00001 << 2 # op0r_imm9 pcrel9*2
    op_b            = 0b00010 << 2 # op0r_imm9 pcrel9*2
    op_ibl          = 0b00011 << 2 # op1r_imm6 ibrel(imm6*8,i64)
    op_jalib        = 0b00100 << 2 # op1r_imm6 ibrel(imm6*8,i32x2)
    op_jtlib        = 0b00101 << 2 # op1r_imm6 ibrel(imm6*8,i32x2)
    op_lib_i64      = 0b00110 << 2 # op1r_imm6 ibrel(imm6*8,i64)
    op_li_i64       = 0b00111 << 2 # op1r_imm6
    op_addi_i64     = 0b01000 << 2 # op1r_imm6
    op_srli_i64     = 0b01001 << 2 # op2r_imm3
    op_srai_i64     = 0b01010 << 2 # op2r_imm3
    op_slli_i64     = 0b01011 << 2 # op2r_imm3
    op_addib_i64    = 0b01100 << 2 # op2r_imm3
    op_load_i64     = 0b01101 << 2 # op2r_imm3
    op_loadib_i64   = 0b01110 << 2 # op2r_imm3
    op_cmp_i64      = 0b01111 << 2 # op2r_fun3
    op_subib_i64    = 0b10000 << 2 # op2r_imm3
    op_store_i64    = 0b10001 << 2 # op2r_imm3
    op_storeib_i64  = 0b10010 << 2 # op2r_imm3
    op_logic_i64    = 0b10011 << 2 # op2r_fun3
    op_pin_i64      = 0b10100 << 2 # op3r
    op_and_i64      = 0b10101 << 2 # op3r
    op_or_i64       = 0b10110 << 2 # op3r
    op_xor_i64      = 0b10111 << 2 # op3r
    op_sub_i64      = 0b11000 << 2 # op3r
    op_srl_i64      = 0b11001 << 2 # op3r
    op_sra_i64      = 0b11010 << 2 # op3r
    op_sll_i64      = 0b11011 << 2 # op3r
    op_add_i64      = 0b11100 << 2 # op3r
    op_nop          = 0b11101 << 2 # op0r_imm9
    op_ud1          = 0b11110 << 2 # op0r_imm9
    op_ud2          = 0b11111 << 2 # op0r_imm9

# compare op fun3

class Fun3Compare(Enum):
    compare_lt      = 0b000
    compare_ge      = 0b001
    compare_eq      = 0b010
    compare_ne      = 0b011
    compare_ltu     = 0b100
    compare_geu     = 0b101

# logic op fun3

class Fun3Logic(Enum):
    logic_mov       = 0b000
    logic_not       = 0b001
    logic_neg       = 0b010
    logic_bswap     = 0b011
    logic_ctz       = 0b100
    logic_clz       = 0b101
    logic_ctpop     = 0b110

# integer utilities

def IntToUInt(w,n):
    if n < 0:
        n = ((-n ^ (2 ** w - 1)) + 1) % (2 ** w)
        return n
    return n % (2 ** w)

def UIntToInt(w,n):
    if n & ~((2 ** (w-1))-1):
        n = -(((n ^ (2 ** w - 1)) + 1) % (2 ** w))
        return n
    return n % (2 ** w)

def su64(val):
    return IntToUInt(64, val)
def su32(val):
    return IntToUInt(32, val)
def su16(val):
    return IntToUInt(16, val)
def su8(val):
    return IntToUInt(8, val)
def us64(val):
    return UIntToInt(64, val)
def us32(val):
    return UIntToInt(32, val)
def us16(val):
    return UIntToInt(16, val)
def us8(val):
    return UIntToInt(8, val)

def sux(val):
    return su64(val)
def usx(val):
    return us64(val)

# bitmanip functions

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

# decode

def opc(inst):
    return (inst >> 2) & 0b11111
def uimm9(inst):
    return (inst >> 7) & 0b111111111
def uimm6(inst):
    return (inst >> 7) & 0b111111
def uimm3(inst):
    return (inst >> 7) & 0b111
def simm9(inst):
    return UIntToInt(9, (inst >> 7) & 0b111111111)
def simm6(inst):
    return UIntToInt(6, (inst >> 7) & 0b111111)
def simm3(inst):
    return UIntToInt(3, (inst >> 7) & 0b111)
def ra(inst):
    return (inst >> 7) & 0b111
def rb(inst):
    return (inst >> 10) & 0b111
def rc(inst):
    return (inst >> 13) & 0b111

# state

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

    def dump(self):
        print("pc:%016x ib:%016x flag:%d" % (
            self.pc, self.ib, self.flag))
        for i in range(0,reg_count-1,4):
            print("r%d:%016x r%d:%016x r%d:%016x r%d:%016x" % (
                i+0, self.r[i+0], i+1, self.r[i+1],
                i+2, self.r[i+2], i+3, self.r[i+3]))

    def load_i64(self, o):
        return int.from_bytes(self.mem[o:o+8], byteorder='little')
    def load_i32(self, o):
        return int.from_bytes(self.mem[o:o+4], byteorder='little')
    def load_i16(self, o):
        return int.from_bytes(self.mem[o:o+2], byteorder='little')
    def load_i8(self, o):
        return self.mem[o]
    def store_i64(self, o, val):
        self.mem[o:o+8] = su64(val).to_bytes(8, byteorder='little')
    def store_i32(self, o, val):
        self.mem[o:o+4] = su32(val).to_bytes(4, byteorder='little')
    def store_i16(self, o, val):
        self.mem[o:o+2] = su16(val).to_bytes(2, byteorder='little')
    def store_i8(self, o, val):
        self.mem[o] = su8(val)
    def const_i64(self, slot):
        o = self.ib + slot * 8
        return int.from_bytes(self.mem[o:o+8], byteorder='little')
    def fetch_i16(self):
        o = self.pc
        return int.from_bytes(self.mem[o:o+2], byteorder='little')

    def exec_op_break(self,inst):
        if simm9(inst) == -1:
            self.dump()
            return 0
        return None
    def exec_op_j(self,inst):
        self.pc = sux(self.pc +  (simm9(inst) << 1) + 2)
        return 1
    def exec_op_b(self,inst):
        if self.flag:
            self.pc = sux(self.pc +  (simm9(inst) << 1) + 2)
            return 1
        else:
            return 0
    def exec_op_ibl(self,inst):
        self.r[rc(inst)] = self.ib
        tmp = self.const_i64(uimm6(inst)) & ~7
        self.ib = sux(self.ib + tmp)
        return 0
    def exec_op_jalib(self,inst):
        tmp = self.const_i64(uimm6(inst))
        upc = us32((tmp      ) & 0xffffffff & ~1)
        uib = us32((tmp >> 32) & 0xffffffff & ~7)
        self.pc = sux(self.pc + upc + 2)
        self.ib = sux(self.ib + uib)
        self.r[rc(inst)] = sux(tmp)
        return 1
    def exec_op_jtlib(self,inst):
        tmp = self.r[rc(inst)]
        upc = us32((tmp      ) & 0xffffffff & ~1)
        uib = us32((tmp >> 32) & 0xffffffff & ~7)
        tmp = self.const_i64(uimm6(inst))
        npc = us32((tmp      ) & 0xffffffff & ~1)
        nib = us32((tmp >> 32) & 0xffffffff & ~7)
        self.pc = sux(self.pc + npc - upc)
        self.ib = sux(self.ib + nib - uib)
        return 1
    def exec_op_lib_i64(self,inst):
        tmp = self.const_i64(uimm6(inst))
        self.r[rc(inst)] = sux(tmp)
        return 0
    def exec_op_li_i64(self,inst):
        self.r[rc(inst)] = sux(simm6(inst))
        return 0
    def exec_op_addi_i64(self,inst):
        self.r[rc(inst)] = sux(self.r[rc(inst)] + simm6(inst))
        return 0
    def exec_op_srli_i64(self,inst):
        self.r[rc(inst)] = sux(self.r[rc(inst)] >> uimm6(inst))
        return 0
    def exec_op_srai_i64(self,inst):
        self.r[rc(inst)] = sux(usx(self.r[rc(inst)]) >> uimm6(inst))
        return 0
    def exec_op_slli_i64(self,inst):
        self.r[rc(inst)] = sux(self.r[rc(inst)] << uimm6(inst))
        return 0
    def exec_op_addib_i64(self,inst):
        tmp = self.r[rb(inst)] + self.const_i64(uimm3(inst))
        self.r[rc(inst)] = sux(tmp)
        return 0
    def exec_op_load_i64(self,inst):
        tmp = self.r[rb(inst)] + (uimm3(inst) << 3)
        self.r[rc(inst)] = sux(self.load_i64(tmp))
        return 0
    def exec_op_loadib_i64(self,inst):
        tmp = self.r[rb(inst)] + self.const_i64(uimm3(inst))
        self.r[rc(inst)] = sux(self.load_i64(tmp))
        return 0
    def exec_op_cmp_i64(self,inst):
        fun = Fun3Compare(uimm3(inst))
        if fun == Fun3Compare.compare_lt:
            self.flag = self.r[rc(inst)] < self.r[rb(inst)]
        elif fun == Fun3Compare.compare_ge:
            self.flag = self.r[rc(inst)] >= self.r[rb(inst)]
        elif fun == Fun3Compare.compare_eq:
            self.flag = self.r[rc(inst)] == self.r[rb(inst)]
        elif fun == Fun3Compare.compare_ne:
            self.flag = self.r[rc(inst)] != self.r[rb(inst)]
        elif fun == Fun3Compare.compare_ltu:
            self.flag = usx(self.r[rc(inst)]) < usx(self.r[rb(inst)])
        elif fun == Fun3Compare.compare_geu:
            self.flag = usx(self.r[rc(inst)]) >= usx(self.r[rb(inst)])
        return 0
    def exec_op_subib_i64(self,inst):
        tmp = self.r[rb(inst)] - self.const_i64(uimm3(inst))
        self.r[rc(inst)] = sux(tmp)
        return 0
    def exec_op_store_i64(self,inst):
        tmp = self.r[rb(inst)] + (uimm3(inst) << 3)
        self.store_i64(sux(tmp), self.r[rc(inst)])
        return 0
    def exec_op_storeib_i64(self,inst):
        tmp = self.r[rb(inst)] + self.const_i64(uimm3(inst))
        self.store_i64(sux(tmp), self.r[rc(inst)])
        return 0
    def exec_op_logic_i64(self,inst):
        fun = Fun3Logic(uimm3(inst))
        if fun == Fun3Logic.logic_mov:
            self.r[rc(inst)] = self.r[rb(inst)]
        elif fun == Fun3Logic.logic_not:
            self.r[rc(inst)] = sux(~self.r[rb(inst)])
        elif fun == Fun3Logic.logic_neg:
            self.r[rc(inst)] = sux(-self.r[rb(inst)])
        elif fun == Fun3Logic.logic_bswap:
            self.r[rc(inst)] = sux(bswap(64, self.r[rb(inst)]))
        elif fun == Fun3Logic.logic_ctz:
            self.r[rc(inst)] = ctz(self.r[rb(inst)])
        elif fun == Fun3Logic.logic_clz:
            self.r[rc(inst)] = clz(self.r[rb(inst)])
        elif fun == Fun3Logic.logic_ctpop:
            self.r[rc(inst)] = ctpop(self.r[rb(inst)])
        return 0
    def exec_op_pin_i64(self,inst):
        upc = self.pc - self.r[ra(inst)] + 2
        uib = self.ib - self.r[rb(inst)]
        tmp = su32(upc) | (su32(uib) << 32)
        self.r[rc(inst)] = tmp
        return 0
    def exec_op_and_i64(self,inst):
        self.r[rc(inst)] = self.r[rb(inst)] & self.r[ra(inst)]
        return 0
    def exec_op_or_i64(self,inst):
        self.r[rc(inst)] = self.r[rb(inst)] | self.r[ra(inst)]
        return 0
    def exec_op_xor_i64(self,inst):
        self.r[rc(inst)] = self.r[rb(inst)] ^ self.r[ra(inst)]
        return 0
    def exec_op_sub_i64(self,inst):
        self.r[rc(inst)] = sux(self.r[rb(inst)] - self.r[ra(inst)])
        return 0
    def exec_op_srl_i64(self,inst):
        self.r[rc(inst)] = self.r[rb(inst)] >> self.r[ra(inst)]
        return 0
    def exec_op_sra_i64(self,inst):
        self.r[rc(inst)] = sux(usx(self.r[rb(inst)]) >> self.r[ra(inst)])
        return 0
    def exec_op_sll_i64(self,inst):
        self.r[rc(inst)] = sux(self.r[rb(inst)] << self.r[ra(inst)]);
        return 0
    def exec_op_add_i64(self,inst):
        self.r[rc(inst)] = sux(self.r[rb(inst)] + self.r[ra(inst)]);
        return 0
    def exec_op_nop(self,inst):
        return 0
    def exec_op_ud1(self,inst):
        return None
    def exec_op_ud2(self,inst):
        return None

    def disasm_op_break(self,inst):
        return "break %u" % uimm9(inst)
    def disasm_op_j(self,inst):
        return "j %d" % simm9(inst)
    def disasm_op_b(self,inst):
        return "b %d" % simm9(inst)
    def disasm_op_ibl(self,inst):
        return "ibl ib(%u)" % uimm6(inst)
    def disasm_op_jalib(self,inst):
        return "jalib r%d, ib(%u)" % (rc(inst), uimm6(inst))
    def disasm_op_jtlib(self,inst):
        return "jtlib ib(%u), r%d" % (uimm6(inst), rc(inst))
    def disasm_op_lib_i64(self,inst):
        return "lib.i64 r%d, ib(%u)" % (rc(inst), uimm6(inst))
    def disasm_op_li_i64(self,inst):
        return "li.i64 r%d, %d" % (rc(inst), simm6(inst))
    def disasm_op_addi_i64(self,inst):
        return "addi.i64 r%d, %d" % (rc(inst), simm6(inst))
    def disasm_op_srli_i64(self,inst):
        return "srli.i64 r%d, %u" % (rc(inst), uimm6(inst))
    def disasm_op_srai_i64(self,inst):
        return "srai.i64 r%d, %u" % (rc(inst), uimm6(inst))
    def disasm_op_slli_i64(self,inst):
        return "slli.i64 r%d, %u" % (rc(inst), uimm6(inst))
    def disasm_op_addib_i64(self,inst):
        return "addib.i64 r%d, r%d, ib(%u)" % (rc(inst), rb(inst), uimm3(inst))
    def disasm_op_load_i64(self,inst):
        return "load.i64 r%d, %u(r%d)" % (rc(inst), uimm3(inst) << 3, rb(inst))
    def disasm_op_loadib_i64(self,inst):
        return "loadib.i64 r%d, ib(%u)(r%d)" % (rc(inst), rb(inst), uimm3(inst))
    def disasm_op_cmp_i64(self,inst):
        fun = Fun3Compare(uimm3(inst))
        if fun == Fun3Compare.compare_lt:
            return "cmp.lt.i64 r%d, r%d" % (rc(inst), rb(inst))
        elif fun == Fun3Compare.compare_ge:
            return "cmp.ge.i64 r%d, r%d" % (rc(inst), rb(inst))
        elif fun == Fun3Compare.compare_eq:
            return "cmp.eq.i64 r%d, r%d" % (rc(inst), rb(inst))
        elif fun == Fun3Compare.compare_ne:
            return "cmp.ne.i64 r%d, r%d" % (rc(inst), rb(inst))
        elif fun == Fun3Compare.compare_ltu:
            return "cmp.ltu.i64 r%d, r%d" % (rc(inst), rb(inst))
        elif fun == Fun3Compare.compare_geu:
            return "cmp.geu.i64 r%d, r%d" % (rc(inst), rb(inst))
        else:
            return "unknown"
    def disasm_op_subib_i64(self,inst):
        return "subib.i64 r%d, r%d, ib(%u)" % (rc(inst), uimm3(inst), rb(inst))
    def disasm_op_store_i64(self,inst):
        return "store.i64 r%d, %u(r%d)" % (rc(inst), uimm3(inst), rb(inst))
    def disasm_op_storeib_i64(self,inst):
        return "storeib.i64 r%d, ib(%u)(r%d)" % (rc(inst), uimm3(inst), rb(inst))
    def disasm_op_logic_i64(self,inst):
        fun = Fun3Logic(uimm3(inst))
        if fun == Fun3Logic.logic_mov:
            return "mov.i64 r%d, r%d" % (rc(inst), rb(inst))
        elif fun == Fun3Logic.logic_not:
            return "not.i64 r%d, r%d" % (rc(inst), rb(inst))
        elif fun == Fun3Logic.logic_neg:
            return "neg.i64 r%d, r%d" % (rc(inst), rb(inst))
        elif fun == Fun3Logic.logic_bswap:
            return "bswap.i64 r%d, r%d" % (rc(inst), rb(inst))
        elif fun == Fun3Logic.logic_ctz:
            return "ctz.i64 r%d, r%d" % (rc(inst), rb(inst))
        elif fun == Fun3Logic.logic_clz:
            return "clz.i64 r%d, r%d" % (rc(inst), rb(inst))
        elif fun == Fun3Logic.logic_ctpop:
            return "ctpop.i64 r%d, r%d" % (rc(inst), rb(inst))
        else:
            return ""
    def disasm_op_pin_i64(self,inst):
        return "pin.i64 r%d, r%d, r%d" % (rc(inst), rb(inst), ra(inst))
    def disasm_op_and_i64(self,inst):
        return "and.i64 r%d, r%d, r%d" % (rc(inst), rb(inst), ra(inst))
    def disasm_op_or_i64(self,inst):
        return "or.i64 r%d, r%d, r%d" % (rc(inst), rb(inst), ra(inst))
    def disasm_op_xor_i64(self,inst):
        return "xor.i64 r%d, r%d, r%d" % (rc(inst), rb(inst), ra(inst))
    def disasm_op_sub_i64(self,inst):
        return "sub.i64 r%d, r%d, r%d" % (rc(inst), rb(inst), ra(inst))
    def disasm_op_srl_i64(self,inst):
        return "srl.i64 r%d, r%d, r%d" % (rc(inst), rb(inst), ra(inst))
    def disasm_op_sra_i64(self,inst):
        return "sra.i64 r%d, r%d, r%d" % (rc(inst), rb(inst), ra(inst))
    def disasm_op_sll_i64(self,inst):
        return "sll.i64 r%d, r%d, r%d" % (rc(inst), rb(inst), ra(inst))
    def disasm_op_add_i64(self,inst):
        return "add.i64 r%d, r%d, r%d" % (rc(inst), rb(inst), ra(inst))
    def disasm_op_nop(self,inst):
        return "nop %u" % uimm9(inst)
    def disasm_op_ud1(self,inst):
        return "ud1 %u" % uimm9(inst)
    def disasm_op_ud2(self,inst):
        return "ud2 %u" % uimm9(inst)

    def disasm(self,inst):
        op = Opcode(opc(inst) << 2)
        dis = getattr(self, 'disasm_%s' % op.name)
        ret = '%04x %s' % (su16(inst), dis(inst))
        return ret

    def exec(self,inst):
        op = Opcode(opc(inst) << 2)
        fun = getattr(self, 'exec_%s' % op.name)
        ret = fun(inst)
        if ret == 0:
            self.pc += 2
        return ret is not None and ret >= 0

    def run(self):
        while True:
            inst = self.fetch_i16()
            if self.is_trace:
                print('-- %08x %s' % (self.pc, self.disasm(inst)))
            ret = self.exec(inst)
            if self.is_dump:
                self.dump()
            if not ret:
                return

    def test(self,c,i):
        print("\n# constants:")
        for j,k in enumerate(c):
            a = self.ib + j * 8
            print("# %08x ib(%u) <- %016x" % (a, j, su64(k)))
            self.store_i64(a, k)
        print("\n# instructions:")
        for j,k in enumerate(i):
            a = self.pc + j * 2
            t = self.disasm(k)
            print("# %08x %s" % (a, t))
            self.store_i16(a, k)
        print()
        print("++ begin")
        self.run()
        print("++ end")
        print()
        print("# state")
        self.dump()

def run_test(name,c,i):
    print("# test: %s" % name)
    cpu = CpuState(8192)
    cpu.test(c,i)

# instruction encoding

def enc_break(imm9):
    return Opcode.op_break.value | ((imm9 & 511)<<7)
def enc_j(pcrel9):
    return Opcode.op_j.value | ((pcrel9 & 511)<<7)
def enc_b(pcrel9):
    return Opcode.op_b.value | ((pcrel9 & 511)<<7)
def enc_ibl(rc, ibrel6):
    return Opcode.op_ibl.value | ((ibrel6 & 63)<<7) | ((rc & 7)<<13)
def enc_jalib(rc, ibrel6):
    return Opcode.op_jalib.value | ((ibrel6 & 63)<<7) | ((rc & 7)<<13)
def enc_jtlib(rc, ibrel6):
    return Opcode.op_jtlib.value | ((ibrel6 & 63)<<7) | ((rc & 7)<<13)
def enc_lib_i64(rc, ibrel6):
    return Opcode.op_lib_i64.value | ((ibrel6 & 63)<<7) | ((rc & 7)<<13)
def enc_li_i64(rc, imm6):
    return Opcode.op_li_i64.value | ((imm6 & 63)<<7) | ((rc & 7)<<13)
def enc_addi_i64(rc, imm6):
    return Opcode.op_addi_i64.value | ((imm6 & 63)<<7) | ((rc & 7)<<13)
def enc_srli_i64(rc, imm6):
    return Opcode.op_srli_i64.value | ((imm6 & 63)<<7) | ((rc & 7)<<13)
def enc_srai_i64(rc, imm6):
    return Opcode.op_srai_i64.value | ((imm6 & 63)<<7) | ((rc & 7)<<13)
def enc_slli_i64(rc, imm6):
    return Opcode.op_slli_i64.value | ((imm6 & 63)<<7) | ((rc & 7)<<13)
def enc_addib_i64(rc, rb, ibimm3):
    return Opcode.op_addib_i64.value | ((ibimm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def enc_load_i64(rc, rb, imm3):
    return Opcode.op_load_i64.value | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def enc_loadib_i64(rc, rb, ibimm3):
    return Opcode.op_loadib_i64.value | ((ibimm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def enc_cmp_i64(rc, rb, fun3):
    return Opcode.op_cmp_i64.value | ((fun3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def enc_subib_i64(rc, rb, ibimm3):
    return Opcode.op_subib_i64.value | ((ibimm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def enc_store_i64(rc, rb, imm3):
    return Opcode.op_store_i64.value | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def enc_storeib_i64(rc, rb, ibimm3):
    return Opcode.op_storeib_i64.value | ((ibimm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def enc_logic_i64(rc, rb, fun3):
    return Opcode.op_logic_i64.value | ((fun3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def enc_pin_i64(rc, rb, ra):
    return Opcode.op_pin_i64.value | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def enc_and_i64(rc, rb, ra):
    return Opcode.op_and_i64.value | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def enc_or_i64(rc, rb, ra):
    return Opcode.op_or_i64.value | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def enc_xor_i64(rc, rb, ra):
    return Opcode.op_xor_i64.value | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def enc_sub_i64(rc, rb, ra):
    return Opcode.op_sub_i64.value | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def enc_srl_i64(rc, rb, ra):
    return Opcode.op_srl_i64.value | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def enc_sra_i64(rc, rb, ra):
    return Opcode.op_sra_i64.value | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def enc_sll_i64(rc, rb, ra):
    return Opcode.op_sll_i64.value | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def enc_add_i64(rc, rb, ra):
    return Opcode.op_add_i64.value | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13)
def enc_nop(imm9):
    return Opcode.op_nop.value | ((imm9 & 511)<<7)
def enc_ud1(imm9):
    return Opcode.op_ud1.value | ((imm9 & 511)<<7)
def enc_ud2(imm9):
    return Opcode.op_ud2.value | ((imm9 & 511)<<7)
