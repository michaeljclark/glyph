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
 * cpu initialization
 */

static inline void cpu_init(cpu_state *cpu, size_t mem_size)
{
    cpu->flag = 0;
    memset(cpu->r, 0, sizeof(cpu->r));
    cpu->pc = 0x800;
    cpu->ib = 0x400;
    cpu->mem = calloc(mem_size, 1);
    cpu->mem_size = mem_size;
}

/*
 * cpu state
 */

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
 * cpu implementation
 */

static inline int cpu_exec(cpu_state *cpu, i64 inst)
{
    u64 upc, uib, npc, nib;
    i64 tmp;
    switch (opc(inst)) {
    case cpu_op_break >> 2:
        switch(uimm9(inst)) {
        case 511: cpu_dump(cpu); break;
        default: return -1;
        }
        break;
    case cpu_op_j >> 2:
        cpu->pc = cpu->pc + (simm9(inst) << 1) + 2;
        return 0;
    case cpu_op_b >> 2:
        if (cpu->flag) {
            cpu->pc = cpu->pc + (simm9(inst) << 1) + 2;
            return 0;
        }
        break;
    case cpu_op_ibl >> 2:
        cpu->r[rc(inst)] = cpu->ib;
        tmp = cpu_const_i64(cpu, uimm6(inst)) & ~7ll;
        cpu->ib = cpu->ib + tmp;
        break;
    case cpu_op_jalib >> 2:
        tmp = cpu_const_i64(cpu, uimm6(inst));
        upc = ((tmp << 32 >> 32) & ~1ll);
        uib = ((tmp       >> 32) & ~7ll);
        cpu->pc = cpu->pc + upc + 2;
        cpu->ib = cpu->ib + uib;
        cpu->r[rc(inst)] = tmp;
        return 0;
    case cpu_op_jtlib >> 2:
        tmp = cpu->r[rc(inst)];
        upc = ((tmp << 32 >> 32) & ~1ll);
        uib = ((tmp       >> 32) & ~7ll);
        tmp = cpu_const_i64(cpu, uimm6(inst));
        npc = ((tmp << 32 >> 32) & ~1ll);
        nib = ((tmp       >> 32) & ~7ll);
        cpu->pc = cpu->pc + npc - upc;
        cpu->ib = cpu->ib + nib - uib;
        return 0;
    case cpu_op_lib_i64 >> 2:
        tmp = cpu_const_i64(cpu, uimm6(inst));
        cpu->r[rc(inst)] = tmp;
        break;
    case cpu_op_li_i64 >> 2:
        cpu->r[rc(inst)] = simm6(inst);
        break;
    case cpu_op_addi_i64 >> 2:
        cpu->r[rc(inst)] = cpu->r[rc(inst)] + simm6(inst);
        break;
    case cpu_op_srli_i64 >> 2:
        cpu->r[rc(inst)] = (u64)cpu->r[rc(inst)] >> uimm6(inst);
        break;
    case cpu_op_srai_i64 >> 2:
        cpu->r[rc(inst)] = cpu->r[rc(inst)] >> uimm6(inst);
        break;
    case cpu_op_slli_i64 >> 2:
        cpu->r[rc(inst)] = cpu->r[rc(inst)] << uimm6(inst);
        break;
    case cpu_op_addib_i64 >> 2:
        cpu->r[rc(inst)] = cpu->r[rb(inst)] + cpu_const_i64(cpu, uimm3(inst));
        break;
    case cpu_op_load_i64 >> 2:
        tmp = cpu->r[rb(inst)] + (uimm3(inst) << 3);
        cpu->r[rc(inst)] = cpu_load_i64(cpu, tmp);
        break;
    case cpu_op_loadib_i64 >> 2:
        tmp = cpu->r[rb(inst)] + cpu_const_i64(cpu, uimm3(inst));
        cpu->r[rc(inst)] = cpu_load_i64(cpu, tmp);
        break;
    case cpu_op_cmp_i64 >> 2:
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
        }
        break;
    case cpu_op_subib_i64 >> 2:
        cpu->r[rc(inst)] = cpu->r[rb(inst)] - cpu_const_i64(cpu, uimm3(inst));
        break;
    case cpu_op_store_i64 >> 2:
        tmp = cpu->r[rb(inst)] + (uimm3(inst) << 3);
        cpu_store_i64(cpu, tmp, cpu->r[rc(inst)]);
        break;
    case cpu_op_storeib_i64 >> 2:
        tmp = cpu->r[rb(inst)] + cpu_const_i64(cpu, uimm3(inst));
        cpu_store_i64(cpu, tmp, cpu->r[rc(inst)]);
        break;
    case cpu_op_logic_i64 >> 2:
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
        break;
    case cpu_op_pin_i64 >> 2:
        upc = cpu->pc - cpu->r[ra(inst)] + 2;
        uib = cpu->ib - cpu->r[rb(inst)];
        tmp = (upc << 32 >> 32) | (uib << 32);
        cpu->r[rc(inst)] = tmp;
        break;
    case cpu_op_srl_i64 >> 2:
        cpu->r[rc(inst)] = (u64)cpu->r[rb(inst)] >> cpu->r[ra(inst)];
        break;
    case cpu_op_sra_i64 >> 2:
        cpu->r[rc(inst)] = cpu->r[rb(inst)] >> cpu->r[ra(inst)];
        break;
    case cpu_op_sll_i64 >> 2:
        cpu->r[rc(inst)] = cpu->r[rb(inst)] << cpu->r[ra(inst)];
        break;
    case cpu_op_add_i64 >> 2:
        cpu->r[rc(inst)] = cpu->r[rb(inst)] + cpu->r[ra(inst)];
        break;
    case cpu_op_sub_i64 >> 2:
        cpu->r[rc(inst)] = cpu->r[rb(inst)] - cpu->r[ra(inst)];
        break;
    case cpu_op_and_i64 >> 2:
        cpu->r[rc(inst)] = cpu->r[rb(inst)] & cpu->r[ra(inst)];
        break;
    case cpu_op_or_i64 >> 2:
        cpu->r[rc(inst)] = cpu->r[rb(inst)] | cpu->r[ra(inst)];
        break;
    case cpu_op_xor_i64 >> 2:
        cpu->r[rc(inst)] = cpu->r[rb(inst)] ^ cpu->r[ra(inst)];
        break;
    case cpu_op_nop >> 2:
        break;
    case cpu_op_ud1 >> 2:
    case cpu_op_ud2 >> 2:
        return -1;
    }
    cpu->pc = cpu->pc + 2;
    return 0;
}

/*
 * disassembler
 */

static inline int cpu_disasm(char *buf, size_t len, i64 inst, i64 pc_offset)
{
    switch (opc(inst)) {
    case cpu_op_break >> 2:
        return snprintf(buf, len, "break %llu",
            uimm9(inst));
    case cpu_op_j >> 2:
        return snprintf(buf, len, "j %lld",
            simm9(inst));
    case cpu_op_b >> 2:
        return snprintf(buf, len, "b %lld",
            simm9(inst));
    case cpu_op_ibl >> 2:
        return snprintf(buf, len, "ibl ib(%llu)",
            uimm6(inst));
    case cpu_op_jalib >> 2:
        return snprintf(buf, len, "jalib r%d, ib(%llu)",
            rc(inst), uimm6(inst));
    case cpu_op_jtlib >> 2:
        return snprintf(buf, len, "jtlib ib(%llu), r%d",
            uimm6(inst), rc(inst));
    case cpu_op_lib_i64 >> 2:
        return snprintf(buf, len, "lib.i64 r%d, ib(%llu)",
            rc(inst), uimm6(inst));
    case cpu_op_li_i64 >> 2:
        return snprintf(buf, len, "li.i64 r%d, %lld",
            rc(inst), simm6(inst));
    case cpu_op_addi_i64 >> 2:
        return snprintf(buf, len, "addi.i64 r%d, %lld",
            rc(inst), simm6(inst));
    case cpu_op_srli_i64 >> 2:
        return snprintf(buf, len, "srli.i64 r%d, %llu",
            rc(inst), uimm6(inst));
    case cpu_op_srai_i64 >> 2:
        return snprintf(buf, len, "srai.i64 r%d, %llu",
            rc(inst), uimm6(inst));
    case cpu_op_slli_i64 >> 2:
        return snprintf(buf, len, "slli.i64 r%d, %llu",
            rc(inst), uimm6(inst));
    case cpu_op_addib_i64 >> 2:
        return snprintf(buf, len, "addib.i64 r%d, r%d, ib(%llu)",
            rc(inst), rb(inst), uimm3(inst));
    case cpu_op_load_i64 >> 2:
        return snprintf(buf, len, "load.i64 r%d, %llu(r%d)",
            rc(inst), uimm3(inst) << 3, rb(inst));
    case cpu_op_loadib_i64 >> 2:
        return snprintf(buf, len, "loadib.i64 r%d, ib(%llu)(r%d)",
            rc(inst), uimm3(inst) << 3, rb(inst));
    case cpu_op_subib_i64 >> 2:
        return snprintf(buf, len, "subib.i64 r%d, r%d, ib(%llu)",
            rc(inst), rb(inst), uimm3(inst));
    case cpu_op_cmp_i64 >> 2:
        switch(uimm3(inst)) {
        case cpu_compare_lt:
            return snprintf(buf, len, "cmp.lt.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_compare_ge:
            return snprintf(buf, len, "cmp.ge.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_compare_eq:
            return snprintf(buf, len, "cmp.eq.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_compare_ne:
            return snprintf(buf, len, "cmp.ne.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_compare_ltu:
            return snprintf(buf, len, "cmp.ltu.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_compare_geu:
            return snprintf(buf, len, "cmp.geu.i64 r%d, r%d",
                rc(inst), rb(inst));
        }
        break;
    case cpu_op_logic_i64 >> 2:
        switch(uimm3(inst)) {
        case cpu_logic_mov:
            return snprintf(buf, len, "mov.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_logic_not:
            return snprintf(buf, len, "not.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_logic_neg:
            return snprintf(buf, len, "neg.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_logic_bswap:
            return snprintf(buf, len, "bswap.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_logic_ctz:
            return snprintf(buf, len, "ctz.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_logic_clz:
            return snprintf(buf, len, "clz.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_logic_ctpop:
            return snprintf(buf, len, "ctpop.i64 r%d, r%d",
                rc(inst), rb(inst));
        default:
            return snprintf(buf, len, "invalid");
        }
        break;
    case cpu_op_store_i64 >> 2:
        return snprintf(buf, len, "store.i64 r%d, %llu(r%d)",
            rc(inst), uimm3(inst), rb(inst));
    case cpu_op_storeib_i64 >> 2:
        return snprintf(buf, len, "storeib.i64 r%d, ib(%llu)(r%d)",
            rc(inst), uimm3(inst), rb(inst));
    case cpu_op_pin_i64 >> 2:
        return snprintf(buf, len, "pin.i64 r%d, r%d, r%d",
            rc(inst), rb(inst), ra(inst));
    case cpu_op_srl_i64 >> 2:
        return snprintf(buf, len, "srl.i64 r%d, r%d, r%d",
            rc(inst), rb(inst), ra(inst));
    case cpu_op_sra_i64 >> 2:
        return snprintf(buf, len, "sra.i64 r%d, r%d, r%d",
            rc(inst), rb(inst), ra(inst));
    case cpu_op_sll_i64 >> 2:
        return snprintf(buf, len, "sll.i64 r%d, r%d, r%d",
            rc(inst), rb(inst), ra(inst));
    case cpu_op_add_i64 >> 2:
        return snprintf(buf, len, "add.i64 r%d, r%d, r%d",
            rc(inst), rb(inst), ra(inst));
    case cpu_op_sub_i64 >> 2:
        return snprintf(buf, len, "sub.i64 r%d, r%d, r%d",
            rc(inst), rb(inst), ra(inst));
    case cpu_op_and_i64 >> 2:
        return snprintf(buf, len, "and.i64 r%d, r%d, r%d",
            rc(inst), rb(inst), ra(inst));
    case cpu_op_or_i64 >> 2:
        return snprintf(buf, len, "or.i64 r%d, r%d, r%d",
            rc(inst), rb(inst), ra(inst));
    case cpu_op_xor_i64 >> 2:
        return snprintf(buf, len, "xor.i64 r%d, r%d, r%d",
            rc(inst), rb(inst), ra(inst));
    case cpu_op_nop >> 2:
        return snprintf(buf, len, "nop %llu",
            uimm9(inst));
    case cpu_op_ud1 >> 2:
        return snprintf(buf, len, "ud1 %llu",
            uimm9(inst));
    case cpu_op_ud2 >> 2:
        return snprintf(buf, len, "ud2 %llu",
            uimm9(inst));
    }
    return snprintf(buf, len, "invalid");
}

/*
 * runloop
 */

static inline int cpu_run(cpu_state *cpu, int trace, int dump)
{
    i16 inst;
    int ret = 0;
    char buf[128];
    while (!ret)
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
    }
}

/*
 * instruction encoding
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
static inline i16 enc_sub_i64(int rc, int rb, int ra)
{
    return cpu_op_sub_i64 | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
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
