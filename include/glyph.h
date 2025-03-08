/*
 * glyph is a super regular RISC that encodes constants in immediate blocks.
 *
 * Copyright (c) 2020 Michael Clark <michaeljclark@mac.com>
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
    cpu_op_rsrv1        = 0b00011 << 2,
    cpu_op_ibl          = 0b00100 << 2, // op1r_imm6 ibrel(imm6*8,i64)
    cpu_op_jalib        = 0b00101 << 2, // op1r_imm6 ibrel(imm6*8,i32x2)
    cpu_op_jtlib        = 0b00110 << 2, // op1r_imm6 ibrel(imm6*8,i32x2)
    cpu_op_lib_i64      = 0b00111 << 2, // op1r_imm6 ibrel(imm6*8,i64)
    cpu_op_li_i64       = 0b01000 << 2, // op1r_imm6
    cpu_op_addi_i64     = 0b01001 << 2, // op1r_imm6
    cpu_op_srli_i64     = 0b01010 << 2, // op2r_imm3
    cpu_op_srai_i64     = 0b01011 << 2, // op2r_imm3
    cpu_op_slli_i64     = 0b01100 << 2, // op2r_imm3
    cpu_op_load_i8      = 0b01101 << 2, // op2r_imm3
    cpu_op_load_u8      = 0b01110 << 2, // op2r_imm3
    cpu_op_load_i64     = 0b01111 << 2, // op2r_imm3
    cpu_op_cmp_i64      = 0b10000 << 2, // op2r_fun3
    cpu_op_log_i64      = 0b10001 << 2, // op2r_fun3
    cpu_op_store_i8     = 0b10010 << 2, // op2r_imm3
    cpu_op_store_i64    = 0b10011 << 2, // op2r_imm3
    cpu_op_pin          = 0b10100 << 2, // op3r
    cpu_op_srl_i64      = 0b10101 << 2, // op3r
    cpu_op_sra_i64      = 0b10110 << 2, // op3r
    cpu_op_sll_i64      = 0b10111 << 2, // op3r
    cpu_op_add_i64      = 0b11000 << 2, // op3r
    cpu_op_sub_i64      = 0b11001 << 2, // op3r
    cpu_op_and_i64      = 0b11010 << 2, // op3r
    cpu_op_or_i64       = 0b11011 << 2, // op3r
    cpu_op_xor_i64      = 0b11100 << 2, // op3r
    cpu_op_nop          = 0b11101 << 2, // op0r_imm9
    cpu_op_dump         = 0b11110 << 2, // op0r_imm9
    cpu_op_illegal      = 0b11111 << 2, // op0r_imm9
};

/*
 * cmp op fun3
 */

enum
{
    cpu_cmp_lt  = 0b000,
    cpu_cmp_ge  = 0b001,
    cpu_cmp_eq  = 0b010,
    cpu_cmp_ne  = 0b011,
    cpu_cmp_ltu  = 0b100,
    cpu_cmp_geu  = 0b101,
};

/*
 * log2 op fun3
 */

enum
{
    cpu_log_not = 0b000,
    cpu_log_neg = 0b001,
    cpu_log_ctz = 0b010,
    cpu_log_clz = 0b011,
    cpu_log_pop = 0b100,
};

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

static inline i64 cpu_load_i8(cpu_state *cpu, u64 offset)
{
    return *(i8*)(cpu->mem + offset);
}

static inline u64 cpu_load_u8(cpu_state *cpu, u64 offset)
{
    return *(u8*)(cpu->mem + offset);
}

static inline i64 cpu_load_i64(cpu_state *cpu, u64 offset)
{
    return *(i64*)(cpu->mem + offset);
}

static inline void cpu_store_i8(cpu_state *cpu, u64 offset, i8 val)
{
    *(i8*)(cpu->mem + offset) = val;
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
 * cpu implementation
 */

static inline i64 imm9(int64_t insn) { return insn << 48 >> 55; }
static inline i64 imm6(int64_t insn) { return insn << 51 >> 58; }
static inline i64 imm3(int64_t insn) { return insn << 54 >> 61; }
static inline uint ra(int64_t insn) { return (insn >> 7) & 7; }
static inline uint rb(int64_t insn) { return (insn >> 10) & 7; }
static inline uint rc(int64_t insn) { return (insn >> 13) & 7; }

static inline int cpu_exec(cpu_state *cpu, i64 inst)
{
    uint op = (inst >> 2) & 0b11111;
    u64 upc, uib;
    i64 tmp;
    switch (op) {
    case cpu_op_break >> 2:
        return -1;
    case cpu_op_j >> 2:
        cpu->pc = cpu->pc + (imm9(inst) << 1) + 2;
        return 0;
    case cpu_op_b >> 2:
        if (cpu->flag) {
            cpu->pc = cpu->pc + (imm9(inst) << 1) + 2;
            return 0;
        }
        break;
    case cpu_op_ibl >> 2:
        cpu->r[rc(inst)] = cpu->ib;
        tmp = cpu_const_i64(cpu, imm6(inst)) & ~7ll;
        cpu->ib = cpu->ib + tmp;
        break;
    case cpu_op_jalib >> 2:
        tmp = cpu_const_i64(cpu, imm6(inst));
        cpu->pc = cpu->pc + ((tmp << 32 >> 32) & ~1ll) + 2;
        cpu->ib = cpu->ib + ((tmp       >> 32) & ~7ll);
        cpu->r[rc(inst)] = tmp;
        return 0;
    case cpu_op_jtlib >> 2:
        tmp = cpu->r[rc(inst)];
        upc = tmp << 32 >> 32;
        uib = tmp       >> 32;
        tmp = cpu_const_i64(cpu, imm6(inst));
        cpu->pc = cpu->pc + ((tmp << 32 >> 32) & ~1ll) - upc;
        cpu->ib = cpu->ib + ((tmp       >> 32) & ~7ll) - uib;
        return 0;
    case cpu_op_lib_i64 >> 2:
        tmp = cpu_const_i64(cpu, imm6(inst));
        cpu->r[rc(inst)] = tmp;
        break;
    case cpu_op_li_i64 >> 2:
        cpu->r[rc(inst)] = imm6(inst);
        break;
    case cpu_op_addi_i64 >> 2:
        cpu->r[rc(inst)] = cpu->r[rc(inst)] + imm6(inst);
        break;
    case cpu_op_srli_i64 >> 2:
        cpu->r[rc(inst)] = (u64)cpu->r[rb(inst)] >> imm3(inst);
        break;
    case cpu_op_srai_i64 >> 2:
        cpu->r[rc(inst)] = cpu->r[rb(inst)] >> imm3(inst);
        break;
    case cpu_op_slli_i64 >> 2:
        cpu->r[rc(inst)] = cpu->r[rb(inst)] << imm3(inst);
        break;
    case cpu_op_load_i8 >> 2:
        cpu->r[rc(inst)] = cpu_load_i8(cpu, cpu->r[rb(inst)] + imm3(inst));
        break;
    case cpu_op_load_u8 >> 2:
        cpu->r[rc(inst)] = cpu_load_u8(cpu, cpu->r[rb(inst)] + imm3(inst));
        break;
    case cpu_op_load_i64 >> 2:
        cpu->r[rc(inst)] = cpu_load_i64(cpu, cpu->r[rb(inst)] + (imm3(inst) << 3));
        break;
    case cpu_op_cmp_i64 >> 2:
        switch(imm3(inst)) {
        case cpu_cmp_lt:
            cpu->flag = cpu->r[rc(inst)] < cpu->r[rb(inst)];
            break;
        case cpu_cmp_ge:
            cpu->flag = cpu->r[rc(inst)] >= cpu->r[rb(inst)];
            break;
        case cpu_cmp_eq:
            cpu->flag = cpu->r[rc(inst)] == cpu->r[rb(inst)];
            break;
        case cpu_cmp_ne:
            cpu->flag = cpu->r[rc(inst)] != cpu->r[rb(inst)];
            break;
        case cpu_cmp_ltu:
            cpu->flag = (u64)cpu->r[rc(inst)] < (u64)cpu->r[rb(inst)];
            break;
        case cpu_cmp_geu:
            cpu->flag = (u64)cpu->r[rc(inst)] >= (u64)cpu->r[rb(inst)];
            break;
        }
        break;
    case cpu_op_log_i64 >> 2:
        switch(imm3(inst)) {
        case cpu_log_not:
            cpu->r[rc(inst)] = ~cpu->r[rb(inst)];
            break;
        case cpu_log_neg:
            cpu->r[rc(inst)] = -cpu->r[rb(inst)];
            break;
        case cpu_log_ctz:
            cpu->r[rc(inst)] = __builtin_ctz(cpu->r[rb(inst)]);
            break;
        case cpu_log_clz:
            cpu->r[rc(inst)] = __builtin_clz(cpu->r[rb(inst)]);
            break;
        case cpu_log_pop:
            cpu->r[rc(inst)] = __builtin_popcount(cpu->r[rb(inst)]);
            break;
        default:
            return -1;
        }
        break;
    case cpu_op_store_i8 >> 2:
        cpu_store_i8(cpu, cpu->r[rb(inst)] + imm3(inst), cpu->r[rc(inst)]);
        break;
    case cpu_op_store_i64 >> 2:
        cpu_store_i64(cpu, cpu->r[rb(inst)] + (imm3(inst) << 3), cpu->r[rc(inst)]);
        break;
    case cpu_op_pin >> 2:
        upc = cpu->r[ra(inst)] - cpu->pc + 2;
        uib = cpu->r[rb(inst)] - cpu->ib;
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
    case cpu_op_dump >> 2:
        cpu_dump(cpu);
        break;
    case cpu_op_illegal >> 2:
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
    uint op = (inst >> 2) & 0b11111;
    switch (op) {
    case cpu_op_break >> 2:
        return snprintf(buf, len, "break %lld",
            imm9(inst));
    case cpu_op_j >> 2:
        return snprintf(buf, len, "j %lld",
            imm9(inst));
    case cpu_op_b >> 2:
        return snprintf(buf, len, "b %lld",
            imm9(inst));
    case cpu_op_ibl >> 2:
        return snprintf(buf, len, "ibl ib(%lld)",
            imm6(inst));
    case cpu_op_jalib >> 2:
        return snprintf(buf, len, "jalib r%d, ib(%lld)",
            rc(inst), imm6(inst));
    case cpu_op_jtlib >> 2:
        return snprintf(buf, len, "jtlib ib(%lld), r%d",
            imm6(inst), rc(inst));
    case cpu_op_lib_i64 >> 2:
        return snprintf(buf, len, "lib.i64 r%d, ib(%lld)",
            rc(inst), imm6(inst));
    case cpu_op_li_i64 >> 2:
        return snprintf(buf, len, "li.i64 r%d, %lld",
            rc(inst), imm6(inst));
    case cpu_op_addi_i64 >> 2:
        return snprintf(buf, len, "addi.i64 r%d, %lld",
            rc(inst), imm6(inst));
    case cpu_op_srli_i64 >> 2:
        return snprintf(buf, len, "srli.i64 r%d, r%d, %llu",
            rc(inst), rb(inst), imm3(inst));
    case cpu_op_srai_i64 >> 2:
        return snprintf(buf, len, "srai.i64 r%d, r%d, %llu",
            rc(inst), rb(inst), imm3(inst));
    case cpu_op_slli_i64 >> 2:
        return snprintf(buf, len, "slli.i64 r%d, r%d, %llu",
            rc(inst), rb(inst), imm3(inst));
    case cpu_op_load_i8 >> 2:
        return snprintf(buf, len, "load.i8 r%d, %lld(r%d)",
            rc(inst), imm3(inst), rb(inst));
    case cpu_op_load_u8 >> 2:
        return snprintf(buf, len, "load.u8 r%d, %lld(r%d)",
            rc(inst), imm3(inst), rb(inst));
    case cpu_op_load_i64 >> 2:
        return snprintf(buf, len, "load.i64 r%d, %lld(r%d)",
            rc(inst), imm3(inst) << 3, rb(inst));
    case cpu_op_cmp_i64 >> 2:
        switch(imm3(inst)) {
        case cpu_cmp_lt:
            return snprintf(buf, len, "cmp.lt.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_cmp_ge:
            return snprintf(buf, len, "cmp.ge.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_cmp_eq:
            return snprintf(buf, len, "cmp.eq.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_cmp_ne:
            return snprintf(buf, len, "cmp.ne.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_cmp_ltu:
            return snprintf(buf, len, "cmp.ltu.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_cmp_geu:
            return snprintf(buf, len, "cmp.geu.i64 r%d, r%d",
                rc(inst), rb(inst));
        }
        break;
    case cpu_op_log_i64 >> 2:
        switch(imm3(inst)) {
        case cpu_log_not:
            return snprintf(buf, len, "not.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_log_neg:
            return snprintf(buf, len, "neg.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_log_ctz:
            return snprintf(buf, len, "ctz.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_log_clz:
            return snprintf(buf, len, "clz.i64 r%d, r%d",
                rc(inst), rb(inst));
        case cpu_log_pop:
            return snprintf(buf, len, "pop.i64 r%d, r%d",
                rc(inst), rb(inst));
        default:
            return snprintf(buf, len, "invalid");
        }
        break;
    case cpu_op_store_i8 >> 2:
        return snprintf(buf, len, "store.i8 r%d, %lld(r%d)",
            rc(inst), imm3(inst), rb(inst));
    case cpu_op_store_i64 >> 2:
        return snprintf(buf, len, "store.i64 r%d, %lld(r%d)",
            rc(inst), imm3(inst), rb(inst));
    case cpu_op_pin >> 2:
        return snprintf(buf, len, "pin r%d, r%d, r%d",
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
        return snprintf(buf, len, "nop %lld",
            imm9(inst));
    case cpu_op_dump >> 2:
        return snprintf(buf, len, "dump %lld",
            imm9(inst));
    case cpu_op_illegal >> 2:
        return snprintf(buf, len, "illegal %lld",
            imm9(inst));
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
            printf("-- %08llx %s\n", cpu->pc, buf);
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
static inline i16 enc_srli_i64(int rc, int rb, int imm3)
{
    return cpu_op_srli_i64 | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_srai_i64(int rc, int rb, int imm3)
{
    return cpu_op_srai_i64 | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_slli_i64(int rc, int rb, int imm3)
{
    return cpu_op_slli_i64 | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_load_i8(int rc, int rb, int imm3)
{
    return cpu_op_load_i8 | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_load_u8(int rc, int rb, int imm3)
{
    return cpu_op_load_u8 | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_load_i64(int rc, int rb, int imm3)
{
    return cpu_op_load_i64 | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_cmp_i64(int rc, int rb, int fun3)
{
    return cpu_op_cmp_i64 | ((fun3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_log_i64(int rc, int rb, int fun3)
{
    return cpu_op_log_i64 | ((fun3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_store_i8(int rc, int rb, int imm3)
{
    return cpu_op_store_i8 | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_store_i64(int rc, int rb, int imm3)
{
    return cpu_op_store_i64 | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
}
static inline i16 enc_pin(int rc, int rb, int ra)
{
    return cpu_op_pin | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13);
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
static inline i16 enc_dump(int imm9)
{
    return cpu_op_dump | ((imm9 & 511)<<7);
}
static inline i16 enc_illegal(int imm9)
{
    return cpu_op_illegal | ((imm9 & 511)<<7);
}
