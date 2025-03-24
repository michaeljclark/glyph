/*
 * glyph is a super regular RISC that encodes constants in immediate blocks.
 *
 * Copyright (c) 2024-2025 Michael Clark <michaeljclark@mac.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int uint;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef struct cpu_state cpu_state;

/*
 * opcodes
 */

enum
{
    cpu_op_break        = 0b00000 << 2, // op0r_imm9
    cpu_op_j            = 0b00001 << 2, // op0r_imm9 pcrel9*2
    cpu_op_b            = 0b00010 << 2, // op0r_imm9 pcrel9*2
    cpu_op_ibl          = 0b00011 << 2, // op1r_imm6 ibrel(imm6*8,i64)
    cpu_op_jalib        = 0b00100 << 2, // op1r_imm6 ibrel(imm6*8,i32x2)
    cpu_op_jtlib        = 0b00101 << 2, // op1r_imm6 ibrel(imm6*8,i32x2)
    cpu_op_lib_i64      = 0b00110 << 2, // op1r_imm6 ibrel(imm6*8,i64)
    cpu_op_li_i64       = 0b00111 << 2, // op1r_imm6
    cpu_op_addi_i64     = 0b01000 << 2, // op1r_imm6
    cpu_op_srli_i64     = 0b01001 << 2, // op2r_imm3
    cpu_op_srai_i64     = 0b01010 << 2, // op2r_imm3
    cpu_op_slli_i64     = 0b01011 << 2, // op2r_imm3
    cpu_op_addib_i64    = 0b01100 << 2, // op2r_imm3
    cpu_op_load_i64     = 0b01101 << 2, // op2r_imm3
    cpu_op_loadib_i64   = 0b01110 << 2, // op2r_imm3
    cpu_op_cmp_i64      = 0b01111 << 2, // op2r_fun3
    cpu_op_subib_i64    = 0b10000 << 2, // op2r_imm3
    cpu_op_store_i64    = 0b10001 << 2, // op2r_imm3
    cpu_op_storeib_i64  = 0b10010 << 2, // op2r_imm3
    cpu_op_logic_i64    = 0b10011 << 2, // op2r_fun3
    cpu_op_pin_i64      = 0b10100 << 2, // op3r
    cpu_op_and_i64      = 0b10101 << 2, // op3r
    cpu_op_or_i64       = 0b10110 << 2, // op3r
    cpu_op_xor_i64      = 0b10111 << 2, // op3r
    cpu_op_sub_i64      = 0b11000 << 2, // op3r
    cpu_op_srl_i64      = 0b11001 << 2, // op3r
    cpu_op_sra_i64      = 0b11010 << 2, // op3r
    cpu_op_sll_i64      = 0b11011 << 2, // op3r
    cpu_op_add_i64      = 0b11100 << 2, // op3r
    cpu_op_nop          = 0b11101 << 2, // op0r_imm9
    cpu_op_ud1          = 0b11110 << 2, // op0r_imm9
    cpu_op_ud2          = 0b11111 << 2, // op0r_imm9
};

/*
 * compare op fun3
 */

enum
{
    cpu_compare_lt      = 0b000,
    cpu_compare_ge      = 0b001,
    cpu_compare_eq      = 0b010,
    cpu_compare_ne      = 0b011,
    cpu_compare_ltu     = 0b100,
    cpu_compare_geu     = 0b101,
};

/*
 * logic op fun3
 */

enum
{
    cpu_logic_mov       = 0b000,
    cpu_logic_not       = 0b001,
    cpu_logic_neg       = 0b010,
    cpu_logic_bswap     = 0b011,
    cpu_logic_ctz       = 0b100,
    cpu_logic_clz       = 0b101,
    cpu_logic_ctpop     = 0b110,
};

/*
 * decode
 */

static inline uint opc(int64_t insn) { return (insn >> 2) & 0b11111; }
static inline i64 uimm9(int64_t insn) { return (u64)insn << 48 >> 55; }
static inline i64 uimm6(int64_t insn) { return (u64)insn << 51 >> 58; }
static inline i64 uimm3(int64_t insn) { return (u64)insn << 54 >> 61; }
static inline i64 simm9(int64_t insn) { return insn << 48 >> 55; }
static inline i64 simm6(int64_t insn) { return insn << 51 >> 58; }
static inline i64 simm3(int64_t insn) { return insn << 54 >> 61; }
static inline uint ra(int64_t insn) { return (insn >> 7) & 7; }
static inline uint rb(int64_t insn) { return (insn >> 10) & 7; }
static inline uint rc(int64_t insn) { return (insn >> 13) & 7; }

/*
 * state
 */

enum
{
    cpu_reg_count = 8,
};

struct cpu_state
{
    i8 flag;
    i64 r[cpu_reg_count];
    u64 pc;
    u64 ib;
    i8 *mem;
    size_t mem_size;
};

/*
 * load, store and constant memory
 */

static inline i64 cpu_load_i64(cpu_state *cpu, u64 offset)
{
    return *(i64*)(cpu->mem + offset);
}

static inline void cpu_store_i64(cpu_state *cpu, u64 offset, i64 val)
{
    *(i64*)(cpu->mem + offset) = val;
}

static inline i64 cpu_const_i64(cpu_state *cpu, u64 offset)
{
    return *(i64*)(cpu->mem + cpu->ib + offset * 8);
}

static inline i16 cpu_fetch(cpu_state *cpu)
{
    return *(i16*)(cpu->mem + cpu->pc);
}

/*
 * cpu emulation
 */

static inline int cpu_exec_op_break(cpu_state *cpu, i64 inst)
{
    return -1;
}

static inline int cpu_exec_op_j(cpu_state *cpu, i64 inst)
{
    cpu->pc = cpu->pc + (simm9(inst) << 1) + 2;
    return 0;
}

static inline int cpu_exec_op_b(cpu_state *cpu, i64 inst)
{
    if (cpu->flag) {
        cpu->pc = cpu->pc + (simm9(inst) << 1) + 2;
        return 0;
    }
    return 2;
}

static inline int cpu_exec_op_ibl(cpu_state *cpu, i64 inst)
{
    i64 tmp;
    cpu->r[rc(inst)] = cpu->ib;
    tmp = cpu_const_i64(cpu, uimm6(inst)) & ~7ll;
    cpu->ib = cpu->ib + tmp;
    return 2;
}

static inline int cpu_exec_op_jalib(cpu_state *cpu, i64 inst)
{
    u64 upc, uib;
    i64 tmp;
    tmp = cpu_const_i64(cpu, uimm6(inst));
    upc = ((tmp << 32 >> 32) & ~1ll);
    uib = ((tmp       >> 32) & ~7ll);
    cpu->pc = cpu->pc + upc + 2;
    cpu->ib = cpu->ib + uib;
    cpu->r[rc(inst)] = tmp;
    return 0;
}

static inline int cpu_exec_op_jtlib(cpu_state *cpu, i64 inst)
{
    u64 upc, uib;
    u64 npc, nib;
    i64 tmp;
    tmp = cpu->r[rc(inst)];
    upc = ((tmp << 32 >> 32) & ~1ll);
    uib = ((tmp       >> 32) & ~7ll);
    tmp = cpu_const_i64(cpu, uimm6(inst));
    npc = ((tmp << 32 >> 32) & ~1ll);
    nib = ((tmp       >> 32) & ~7ll);
    cpu->pc = cpu->pc + npc - upc;
    cpu->ib = cpu->ib + nib - uib;
    return 0;
}

static inline int cpu_exec_op_lib_i64(cpu_state *cpu, i64 inst)
{
    i64 tmp;
    tmp = cpu_const_i64(cpu, uimm6(inst));
    cpu->r[rc(inst)] = tmp;
    return 2;
}

static inline int cpu_exec_op_li_i64(cpu_state *cpu, i64 inst)
{
    cpu->r[rc(inst)] = simm6(inst);
    return 2;
}

static inline int cpu_exec_op_addi_i64(cpu_state *cpu, i64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rc(inst)] + simm6(inst);
    return 2;
}

static inline int cpu_exec_op_srli_i64(cpu_state *cpu, i64 inst)
{
    cpu->r[rc(inst)] = (u64)cpu->r[rc(inst)] >> uimm6(inst);
    return 2;
}

static inline int cpu_exec_op_srai_i64(cpu_state *cpu, i64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rc(inst)] >> uimm6(inst);
    return 2;
}

static inline int cpu_exec_op_slli_i64(cpu_state *cpu, i64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rc(inst)] << uimm6(inst);
    return 2;
}

static inline int cpu_exec_op_addib_i64(cpu_state *cpu, i64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rb(inst)] + cpu_const_i64(cpu, uimm3(inst));
    return 2;
}

static inline int cpu_exec_op_load_i64(cpu_state *cpu, i64 inst)
{
    i64 tmp;
    tmp = cpu->r[rb(inst)] + (uimm3(inst) << 3);
    cpu->r[rc(inst)] = cpu_load_i64(cpu, tmp);
    return 2;
}

static inline int cpu_exec_op_loadib_i64(cpu_state *cpu, i64 inst)
{
    i64 tmp;
    tmp = cpu->r[rb(inst)] + cpu_const_i64(cpu, uimm3(inst));
    cpu->r[rc(inst)] = cpu_load_i64(cpu, tmp);
    return 2;
}

static inline int cpu_exec_op_cmp_i64(cpu_state *cpu, i64 inst)
{
    switch(uimm3(inst)) {
    case cpu_compare_lt:
        cpu->flag = cpu->r[rc(inst)] < cpu->r[rb(inst)];
        break;
    case cpu_compare_ge:
        cpu->flag = cpu->r[rc(inst)] >= cpu->r[rb(inst)];
        break;
    case cpu_compare_eq:
        cpu->flag = cpu->r[rc(inst)] == cpu->r[rb(inst)];
        break;
    case cpu_compare_ne:
        cpu->flag = cpu->r[rc(inst)] != cpu->r[rb(inst)];
        break;
    case cpu_compare_ltu:
        cpu->flag = (u64)cpu->r[rc(inst)] < (u64)cpu->r[rb(inst)];
        break;
    case cpu_compare_geu:
        cpu->flag = (u64)cpu->r[rc(inst)] >= (u64)cpu->r[rb(inst)];
        break;
    default:
        return -1;
    }
    return 2;
}

static inline int cpu_exec_op_subib_i64(cpu_state *cpu, i64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rb(inst)] - cpu_const_i64(cpu, uimm3(inst));
    return 2;
}

static inline int cpu_exec_op_store_i64(cpu_state *cpu, i64 inst)
{
    i64 tmp;
    tmp = cpu->r[rb(inst)] + (uimm3(inst) << 3);
    cpu_store_i64(cpu, tmp, cpu->r[rc(inst)]);
    return 2;
}

static inline int cpu_exec_op_storeib_i64(cpu_state *cpu, i64 inst)
{
    i64 tmp;
    tmp = cpu->r[rb(inst)] + cpu_const_i64(cpu, uimm3(inst));
    cpu_store_i64(cpu, tmp, cpu->r[rc(inst)]);
    return 2;
}

static inline int cpu_exec_op_logic_i64(cpu_state *cpu, i64 inst)
{
    switch(uimm3(inst)) {
    case cpu_logic_mov:
        cpu->r[rc(inst)] = cpu->r[rb(inst)];
        break;
    case cpu_logic_not:
        cpu->r[rc(inst)] = ~cpu->r[rb(inst)];
        break;
    case cpu_logic_neg:
        cpu->r[rc(inst)] = -cpu->r[rb(inst)];
        break;
    case cpu_logic_bswap:
        cpu->r[rc(inst)] = __builtin_bswap64(cpu->r[rb(inst)]);
        break;
    case cpu_logic_ctz:
        cpu->r[rc(inst)] = __builtin_ctzll(cpu->r[rb(inst)]);
        break;
    case cpu_logic_clz:
        cpu->r[rc(inst)] = __builtin_clzll(cpu->r[rb(inst)]);
        break;
    case cpu_logic_ctpop:
        cpu->r[rc(inst)] = __builtin_popcountll(cpu->r[rb(inst)]);
        break;
    default:
        return -1;
    }
    return 2;
}

static inline int cpu_exec_op_pin_i64(cpu_state *cpu, i64 inst)
{
    u64 upc, uib;
    i64 tmp;
    upc = cpu->pc - cpu->r[ra(inst)] + 2;
    uib = cpu->ib - cpu->r[rb(inst)];
    tmp = (upc << 32 >> 32) | (uib << 32);
    cpu->r[rc(inst)] = tmp;
    return 2;
}

static inline int cpu_exec_op_and_i64(cpu_state *cpu, i64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rb(inst)] & cpu->r[ra(inst)];
    return 2;
}

static inline int cpu_exec_op_or_i64(cpu_state *cpu, i64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rb(inst)] | cpu->r[ra(inst)];
    return 2;
}

static inline int cpu_exec_op_xor_i64(cpu_state *cpu, i64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rb(inst)] ^ cpu->r[ra(inst)];
    return 2;
}

static inline int cpu_exec_op_sub_i64(cpu_state *cpu, i64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rb(inst)] - cpu->r[ra(inst)];
    return 2;
}

static inline int cpu_exec_op_srl_i64(cpu_state *cpu, i64 inst)
{
    cpu->r[rc(inst)] = (u64)cpu->r[rb(inst)] >> cpu->r[ra(inst)];
    return 2;
}

static inline int cpu_exec_op_sra_i64(cpu_state *cpu, i64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rb(inst)] >> cpu->r[ra(inst)];
    return 2;
}

static inline int cpu_exec_op_sll_i64(cpu_state *cpu, i64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rb(inst)] << cpu->r[ra(inst)];
    return 2;
}

static inline int cpu_exec_op_add_i64(cpu_state *cpu, i64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rb(inst)] + cpu->r[ra(inst)];
    return 2;
}

static inline int cpu_exec_op_nop(cpu_state *cpu, i64 inst)
{
    return 2;
}

static inline int cpu_exec_op_ud1(cpu_state *cpu, i64 inst)
{
    return -1;
}

static inline int cpu_exec_op_ud2(cpu_state *cpu, i64 inst)
{
    return -1;
}

/*
 * cpu disassembly
 */

static inline int cpu_disasm_op_break(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "break %llu", uimm9(i));
}

static inline int cpu_disasm_op_j(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "j %lld", simm9(i));
}

static inline int cpu_disasm_op_b(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "b %lld", simm9(i));
}

static inline int cpu_disasm_op_ibl(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "ibl ib(%llu)", uimm6(i));
}

static inline int cpu_disasm_op_jalib(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "jalib r%d, ib(%llu)", rc(i), uimm6(i));
}

static inline int cpu_disasm_op_jtlib(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "jtlib ib(%llu), r%d", uimm6(i), rc(i));
}

static inline int cpu_disasm_op_lib_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "lib.i64 r%d, ib(%llu)", rc(i), uimm6(i));
}

static inline int cpu_disasm_op_li_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "li.i64 r%d, %lld", rc(i), simm6(i));
}

static inline int cpu_disasm_op_addi_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "addi.i64 r%d, %lld", rc(i), simm6(i));
}

static inline int cpu_disasm_op_srli_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "srli.i64 r%d, %llu", rc(i), uimm6(i));
}

static inline int cpu_disasm_op_srai_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "srai.i64 r%d, %llu", rc(i), uimm6(i));
}

static inline int cpu_disasm_op_slli_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "slli.i64 r%d, %llu", rc(i), uimm6(i));
}

static inline int cpu_disasm_op_addib_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "addib.i64 r%d, r%d, ib(%llu)",
        rc(i), rb(i), uimm3(i));
}

static inline int cpu_disasm_op_load_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "load.i64 r%d, %llu(r%d)",
        rc(i), uimm3(i) << 3, rb(i));
}

static inline int cpu_disasm_op_loadib_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "loadib.i64 r%d, ib(%llu)(r%d)",
        rc(i), uimm3(i) << 3, rb(i));
}

static inline int cpu_disasm_op_cmp_i64(char *b, size_t l, i64 i, i64 c)
{
    switch(uimm3(i)) {
    case cpu_compare_lt:
        return snprintf(b, l, "cmp.lt.i64 r%d, r%d", rc(i), rb(i));
    case cpu_compare_ge:
        return snprintf(b, l, "cmp.ge.i64 r%d, r%d", rc(i), rb(i));
    case cpu_compare_eq:
        return snprintf(b, l, "cmp.eq.i64 r%d, r%d", rc(i), rb(i));
    case cpu_compare_ne:
        return snprintf(b, l, "cmp.ne.i64 r%d, r%d", rc(i), rb(i));
    case cpu_compare_ltu:
        return snprintf(b, l, "cmp.ltu.i64 r%d, r%d", rc(i), rb(i));
    case cpu_compare_geu:
        return snprintf(b, l, "cmp.geu.i64 r%d, r%d", rc(i), rb(i));
    default:
        break;
    }
    return snprintf(b, l, "invalid");
}

static inline int cpu_disasm_op_subib_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "subib.i64 r%d, r%d, ib(%llu)",
        rc(i), rb(i), uimm3(i));
}

static inline int cpu_disasm_op_store_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "store.i64 r%d, %llu(r%d)",
        rc(i), uimm3(i), rb(i));
}

static inline int cpu_disasm_op_storeib_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "storeib.i64 r%d, ib(%llu)(r%d)",
        rc(i), uimm3(i), rb(i));
}

static inline int cpu_disasm_op_logic_i64(char *b, size_t l, i64 i, i64 c)
{
    switch(uimm3(i)) {
    case cpu_logic_mov:
        return snprintf(b, l, "mov.i64 r%d, r%d", rc(i), rb(i));
    case cpu_logic_not:
        return snprintf(b, l, "not.i64 r%d, r%d", rc(i), rb(i));
    case cpu_logic_neg:
        return snprintf(b, l, "neg.i64 r%d, r%d", rc(i), rb(i));
    case cpu_logic_bswap:
        return snprintf(b, l, "bswap.i64 r%d, r%d", rc(i), rb(i));
    case cpu_logic_ctz:
        return snprintf(b, l, "ctz.i64 r%d, r%d", rc(i), rb(i));
    case cpu_logic_clz:
        return snprintf(b, l, "clz.i64 r%d, r%d", rc(i), rb(i));
    case cpu_logic_ctpop:
        return snprintf(b, l, "ctpop.i64 r%d, r%d", rc(i), rb(i));
    default:
        break;
    }
    return snprintf(b, l, "invalid");
}

static inline int cpu_disasm_op_pin_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "pin.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i));
}

static inline int cpu_disasm_op_and_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "and.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i));
}

static inline int cpu_disasm_op_or_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "or.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i));
}

static inline int cpu_disasm_op_xor_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "xor.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i));
}

static inline int cpu_disasm_op_sub_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "sub.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i));
}

static inline int cpu_disasm_op_srl_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "srl.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i));
}

static inline int cpu_disasm_op_sra_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "sra.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i));
}

static inline int cpu_disasm_op_sll_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "sll.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i));
}

static inline int cpu_disasm_op_add_i64(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "add.i64 r%d, r%d, r%d", rc(i), rb(i), ra(i));
}

static inline int cpu_disasm_op_nop(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "nop %llu", uimm9(i));
}

static inline int cpu_disasm_op_ud1(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "ud1 %llu", uimm9(i));
}

static inline int cpu_disasm_op_ud2(char *b, size_t l, i64 i, i64 c)
{
    return snprintf(b, l, "ud2 %llu", uimm9(i));
}

/*
 * cpu instruction encoding
 */

static inline i16 enc_break(int imm9)
{
    return cpu_op_break | ((imm9 & 511)<<7);
}
static inline i16 enc_j(int pcrel9)
{
    return cpu_op_j | ((pcrel9 & 511)<<7);
}
static inline i16 enc_b(int pcrel9)
{
    return cpu_op_b | ((pcrel9 & 511)<<7);
}
static inline i16 enc_ibl(int rc, int ibrel6)
{
    return cpu_op_ibl | ((ibrel6 & 63)<<7) | ((rc & 7)<<13);
}
static inline i16 enc_jalib(int rc, int ibrel6)
{
    return cpu_op_jalib | ((ibrel6 & 63)<<7) | ((rc & 7)<<13);
}
static inline i16 enc_jtlib(int rc, int ibrel6)
{
    return cpu_op_jtlib | ((ibrel6 & 63)<<7) | ((rc & 7)<<13);
}
static inline i16 enc_lib_i64(int rc, int ibrel6)
{
    return cpu_op_lib_i64 | ((ibrel6 & 63)<<7) | ((rc & 7)<<13);
}
static inline i16 enc_li_i64(int rc, int imm6)
{
    return cpu_op_li_i64 | ((imm6 & 63)<<7) | ((rc & 7)<<13);
}
static inline i16 enc_addi_i64(int rc, int imm6)
{
    return cpu_op_addi_i64 | ((imm6 & 63)<<7) | ((rc & 7)<<13);
}
static inline i16 enc_srli_i64(int rc, int imm6)
{
    return cpu_op_srli_i64 | ((imm6 & 63)<<7) | ((rc & 7)<<13);
}
static inline i16 enc_srai_i64(int rc, int imm6)
{
    return cpu_op_srai_i64 | ((imm6 & 63)<<7) | ((rc & 7)<<13);
}
static inline i16 enc_slli_i64(int rc, int imm6)
{
    return cpu_op_slli_i64 | ((imm6 & 63)<<7) | ((rc & 7)<<13);
}
static inline i16 enc_addib_i64(int rc, int rb, int ibimm3)
{
    return cpu_op_addib_i64 | ((ibimm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_load_i64(int rc, int rb, int imm3)
{
    return cpu_op_load_i64 | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_loadib_i64(int rc, int rb, int ibimm3)
{
    return cpu_op_loadib_i64 | ((ibimm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_cmp_i64(int rc, int rb, int fun3)
{
    return cpu_op_cmp_i64 | ((fun3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_subib_i64(int rc, int rb, int ibimm3)
{
    return cpu_op_subib_i64 | ((ibimm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_store_i64(int rc, int rb, int imm3)
{
    return cpu_op_store_i64 | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_storeib_i64(int rc, int rb, int ibimm3)
{
    return cpu_op_storeib_i64 | ((ibimm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_logic_i64(int rc, int rb, int fun3)
{
    return cpu_op_logic_i64 | ((fun3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_pin_i64(int rc, int rb, int ra)
{
    return cpu_op_pin_i64 | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_and_i64(int rc, int rb, int ra)
{
    return cpu_op_and_i64 | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_or_i64(int rc, int rb, int ra)
{
    return cpu_op_or_i64 | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_xor_i64(int rc, int rb, int ra)
{
    return cpu_op_xor_i64 | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_sub_i64(int rc, int rb, int ra)
{
    return cpu_op_sub_i64 | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_srl_i64(int rc, int rb, int ra)
{
    return cpu_op_srl_i64 | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_sra_i64(int rc, int rb, int ra)
{
    return cpu_op_sra_i64 | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_sll_i64(int rc, int rb, int ra)
{
    return cpu_op_sll_i64 | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_add_i64(int rc, int rb, int ra)
{
    return cpu_op_add_i64 | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_nop(int imm9)
{
    return cpu_op_nop | ((imm9 & 511)<<7);
}
static inline i16 enc_ud1(int imm9)
{
    return cpu_op_ud1 | ((imm9 & 511)<<7);
}
static inline i16 enc_ud2(int imm9)
{
    return cpu_op_ud2 | ((imm9 & 511)<<7);
}

/*
 * cpu implementation
 */

static inline int cpu_exec(cpu_state *cpu, i64 inst)
{
    switch (opc(inst)) {
    case cpu_op_break >> 2: return cpu_exec_op_break(cpu, inst);
    case cpu_op_j >> 2: return cpu_exec_op_j(cpu, inst);
    case cpu_op_b >> 2: return cpu_exec_op_b(cpu, inst);
    case cpu_op_ibl >> 2: return cpu_exec_op_ibl(cpu, inst);
    case cpu_op_jalib >> 2: return cpu_exec_op_jalib(cpu, inst);
    case cpu_op_jtlib >> 2: return cpu_exec_op_jtlib(cpu, inst);
    case cpu_op_lib_i64 >> 2: return cpu_exec_op_lib_i64(cpu, inst);
    case cpu_op_li_i64 >> 2: return cpu_exec_op_li_i64(cpu, inst);
    case cpu_op_addi_i64 >> 2: return cpu_exec_op_addi_i64(cpu, inst);
    case cpu_op_srli_i64 >> 2: return cpu_exec_op_srli_i64(cpu, inst);
    case cpu_op_srai_i64 >> 2: return cpu_exec_op_srai_i64(cpu, inst);
    case cpu_op_slli_i64 >> 2: return cpu_exec_op_slli_i64(cpu, inst);
    case cpu_op_addib_i64 >> 2: return cpu_exec_op_addib_i64(cpu, inst);
    case cpu_op_load_i64 >> 2: return cpu_exec_op_load_i64(cpu, inst);
    case cpu_op_loadib_i64 >> 2: return cpu_exec_op_loadib_i64(cpu, inst);
    case cpu_op_cmp_i64 >> 2: return cpu_exec_op_cmp_i64(cpu, inst);
    case cpu_op_subib_i64 >> 2: return cpu_exec_op_subib_i64(cpu, inst);
    case cpu_op_store_i64 >> 2: return cpu_exec_op_store_i64(cpu, inst);
    case cpu_op_storeib_i64 >> 2: return cpu_exec_op_storeib_i64(cpu, inst);
    case cpu_op_logic_i64 >> 2: return cpu_exec_op_logic_i64(cpu, inst);
    case cpu_op_pin_i64 >> 2: return cpu_exec_op_pin_i64(cpu, inst);
    case cpu_op_and_i64 >> 2: return cpu_exec_op_and_i64(cpu, inst);
    case cpu_op_or_i64 >> 2: return cpu_exec_op_or_i64(cpu, inst);
    case cpu_op_xor_i64 >> 2: return cpu_exec_op_xor_i64(cpu, inst);
    case cpu_op_sub_i64 >> 2: return cpu_exec_op_sub_i64(cpu, inst);
    case cpu_op_srl_i64 >> 2: return cpu_exec_op_srl_i64(cpu, inst);
    case cpu_op_sra_i64 >> 2: return cpu_exec_op_sra_i64(cpu, inst);
    case cpu_op_sll_i64 >> 2: return cpu_exec_op_sll_i64(cpu, inst);
    case cpu_op_add_i64 >> 2: return cpu_exec_op_add_i64(cpu, inst);
    case cpu_op_nop >> 2: return cpu_exec_op_nop(cpu, inst);
    case cpu_op_ud1 >> 2: return cpu_exec_op_ud1(cpu, inst);
    case cpu_op_ud2 >> 2: return cpu_exec_op_ud2(cpu, inst);
    }
    return -1;
}

static inline int cpu_disasm(char *b, size_t l, i64 i, i64 c)
{
    switch (opc(i)) {
    case cpu_op_break >> 2: return cpu_disasm_op_break(b, l, i, c);
    case cpu_op_j >> 2: return cpu_disasm_op_j(b, l, i, c);
    case cpu_op_b >> 2: return cpu_disasm_op_b(b, l, i, c);
    case cpu_op_ibl >> 2: return cpu_disasm_op_ibl(b, l, i, c);
    case cpu_op_jalib >> 2: return cpu_disasm_op_jalib(b, l, i, c);
    case cpu_op_jtlib >> 2: return cpu_disasm_op_jtlib(b, l, i, c);
    case cpu_op_lib_i64 >> 2: return cpu_disasm_op_lib_i64(b, l, i, c);
    case cpu_op_li_i64 >> 2: return cpu_disasm_op_li_i64(b, l, i, c);
    case cpu_op_addi_i64 >> 2: return cpu_disasm_op_addi_i64(b, l, i, c);
    case cpu_op_srli_i64 >> 2: return cpu_disasm_op_srli_i64(b, l, i, c);
    case cpu_op_srai_i64 >> 2: return cpu_disasm_op_srai_i64(b, l, i, c);
    case cpu_op_slli_i64 >> 2: return cpu_disasm_op_slli_i64(b, l, i, c);
    case cpu_op_addib_i64 >> 2: return cpu_disasm_op_addib_i64(b, l, i, c);
    case cpu_op_load_i64 >> 2: return cpu_disasm_op_load_i64(b, l, i, c);
    case cpu_op_loadib_i64 >> 2: return cpu_disasm_op_loadib_i64(b, l, i, c);
    case cpu_op_cmp_i64 >> 2: return cpu_disasm_op_cmp_i64(b, l, i, c);
    case cpu_op_subib_i64 >> 2: return cpu_disasm_op_subib_i64(b, l, i, c);
    case cpu_op_store_i64 >> 2: return cpu_disasm_op_store_i64(b, l, i, c);
    case cpu_op_storeib_i64 >> 2: return cpu_disasm_op_storeib_i64(b, l, i, c);
    case cpu_op_logic_i64 >> 2: return cpu_disasm_op_logic_i64(b, l, i, c);
    case cpu_op_pin_i64 >> 2: return cpu_disasm_op_pin_i64(b, l, i, c);
    case cpu_op_and_i64 >> 2: return cpu_disasm_op_and_i64(b, l, i, c);
    case cpu_op_or_i64 >> 2: return cpu_disasm_op_or_i64(b, l, i, c);
    case cpu_op_xor_i64 >> 2: return cpu_disasm_op_xor_i64(b, l, i, c);
    case cpu_op_sub_i64 >> 2: return cpu_disasm_op_sub_i64(b, l, i, c);
    case cpu_op_srl_i64 >> 2: return cpu_disasm_op_srl_i64(b, l, i, c);
    case cpu_op_sra_i64 >> 2: return cpu_disasm_op_sra_i64(b, l, i, c);
    case cpu_op_sll_i64 >> 2: return cpu_disasm_op_sll_i64(b, l, i, c);
    case cpu_op_add_i64 >> 2: return cpu_disasm_op_add_i64(b, l, i, c);
    case cpu_op_nop >> 2: return cpu_disasm_op_nop(b, l, i, c);
    case cpu_op_ud1 >> 2: return cpu_disasm_op_ud1(b, l, i, c);
    case cpu_op_ud2 >> 2: return cpu_disasm_op_ud2(b, l, i, c);
    }
    return snprintf(b, l, "invalid");
}

static inline void cpu_init(cpu_state *cpu, size_t mem_size)
{
    cpu->flag = 0;
    memset(cpu->r, 0, sizeof(cpu->r));
    cpu->pc = 0x800;
    cpu->ib = 0x400;
    cpu->mem = calloc(mem_size, 1);
    cpu->mem_size = mem_size;
}

static inline int cpu_dump(cpu_state *cpu)
{
    printf("pc:%016llx ib:%016llx flag:%d\n",
        cpu->pc, cpu->ib, cpu->flag);
    for (uint i = 0; i < cpu_reg_count; i += 4) {
        printf("r%d:%016llx r%d:%016llx r%d:%016llx r%d:%016llx\n",
            i+0, cpu->r[i+0], i+1, cpu->r[i+1],
            i+2, cpu->r[i+2], i+3, cpu->r[i+3]);
    }
}

static inline void cpu_run(cpu_state *cpu, int trace, int dump)
{
    i16 inst;
    int ret = 0;
    char buf[128];
    for (;;)
    {
        inst = cpu_fetch(cpu);
        if (trace) {
            cpu_disasm(buf, sizeof(buf), inst, cpu->pc);
            printf("-- %08llx %04hx %s\n", cpu->pc, inst, buf);
        }
        if (dump) {
            cpu_dump(cpu);
        }
        ret = cpu_exec(cpu, inst);
        if (ret < 0) {
            printf("** %08llx cpu exception\n", cpu->pc);
            return;
        }
        cpu->pc += ret;
    }
}

static inline void cpu_setup(cpu_state *cpu,
    i64 *c, size_t cl, i16 *i, size_t il)
{
    char buf[128];
    memcpy(cpu->mem + cpu->ib, c, cl);
    memcpy(cpu->mem + cpu->pc, i, il);
    printf("\n# constants:\n");
    for(size_t x = 0; x < (cl>>3); x++) {
        printf("# %08llx ib(%zu) <- %016llx\n", cpu->ib + (x<<3), x, c[x]);
    }
    printf("\n# instructions:\n");
    for(size_t x = 0; x < (il>>1); x++) {
        cpu_disasm(buf, sizeof(buf), i[x], cpu->pc + (x<<1));
        printf("# %08llx %04hx %s\n", cpu->pc + (x<<1), i[x], buf);
    }
}
