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

/*
 * opcodes
 */

enum
{
    cpu_op_break        = 0b00000, // op0r_imm9
    cpu_op_j            = 0b00001, // op0r_imm9 pcrel9*2
    cpu_op_b            = 0b00010, // op0r_imm9 pcrel9*2
    cpu_op_ibj          = 0b00011, // op0r_imm9 pcrel9*64
    cpu_op_jalib        = 0b00100, // op1r_imm6 ibrel(imm6*8,i32x2)
    cpu_op_jtlib        = 0b00101, // op1r_imm6 ibrel(imm6*8,i32x2)
    cpu_op_lib_i64      = 0b00110, // op1r_imm6 ibrel(imm6*8,i64)
    cpu_op_li_i64       = 0b00111, // op1r_imm6
    cpu_op_addi_i64     = 0b01000, // op1r_imm6
    cpu_op_srli_i64     = 0b01001, // op2r_imm3
    cpu_op_srai_i64     = 0b01010, // op2r_imm3
    cpu_op_slli_i64     = 0b01011, // op2r_imm3
    cpu_op_addib_i64    = 0b01100, // op2r_imm3
    cpu_op_load_i64     = 0b01101, // op2r_imm3
    cpu_op_loadib_i64   = 0b01110, // op2r_imm3
    cpu_op_cmp_i64      = 0b01111, // op2r_fun3
    cpu_op_subib_i64    = 0b10000, // op2r_imm3
    cpu_op_store_i64    = 0b10001, // op2r_imm3
    cpu_op_storeib_i64  = 0b10010, // op2r_imm3
    cpu_op_logic_i64    = 0b10011, // op2r_fun3
    cpu_op_pin_i64      = 0b10100, // op3r
    cpu_op_and_i64      = 0b10101, // op3r
    cpu_op_or_i64       = 0b10110, // op3r
    cpu_op_xor_i64      = 0b10111, // op3r
    cpu_op_sub_i64      = 0b11000, // op3r
    cpu_op_srl_i64      = 0b11001, // op3r
    cpu_op_sra_i64      = 0b11010, // op3r
    cpu_op_sll_i64      = 0b11011, // op3r
    cpu_op_add_i64      = 0b11100, // op3r
    cpu_op_mul_i64      = 0b11101, // op3r
    cpu_op_div_i64      = 0b11110, // op3r
    cpu_op_illegal      = 0b11111, // op0r_imm9
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
    cpu_compare_mov     = 0b110,
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
 * forward decls
 */

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

#ifndef __glyph_inline__
#define __glyph_inline__ static inline
#endif

#ifndef __glyph_func__
#define __glyph_func__  static inline
#endif

#define VA_ARGS(...) , ##__VA_ARGS__
#define cpu_debug(fmt, ...) printf(fmt "\n" VA_ARGS(__VA_ARGS__))
#define cpu_test(name,c,i) cpu_test_impl(name,c,sizeof(c),i,sizeof(i))

/*
 * cpu state
 */

enum
{
    cpu_reg_count = 8,
};

struct cpu_state
{
    u8 flag;
    u64 r[cpu_reg_count];
    u64 pc;
    u64 ib;
    u8 *mem;
    size_t mem_size;
    u8 is_trace;
    u8 is_dump;
};

/*
 * instruction decode helpers
 */

__glyph_inline__ uint opc(u64 insn) { return (insn >> 2) & 0b11111; }
__glyph_inline__ u64 uimm9(u64 insn) { return insn << 48 >> 55; }
__glyph_inline__ u64 uimm6(u64 insn) { return insn << 51 >> 58; }
__glyph_inline__ u64 uimm3(u64 insn) { return insn << 54 >> 61; }
__glyph_inline__ i64 simm9(u64 insn) { return (i64)insn << 48 >> 55; }
__glyph_inline__ i64 simm6(u64 insn) { return (i64)insn << 51 >> 58; }
__glyph_inline__ i64 simm3(u64 insn) { return (i64)insn << 54 >> 61; }
__glyph_inline__ uint ra(u64 insn) { return (insn >> 7) & 7; }
__glyph_inline__ uint rb(u64 insn) { return (insn >> 10) & 7; }
__glyph_inline__ uint rc(u64 insn) { return (insn >> 13) & 7; }

/*
 * cpu load, store and constants
 */

__glyph_inline__ u64 cpu_load_i64(cpu_state *cpu, u64 offset)
{
    return (u64)*(i64*)(cpu->mem + offset);
}
__glyph_inline__ u64 cpu_load_i32(cpu_state *cpu, u64 offset)
{
    return (u64)(i64)*(i32*)(cpu->mem + offset);
}
__glyph_inline__ u64 cpu_load_i16(cpu_state *cpu, u64 offset)
{
    return (u64)(i64)*(i16*)(cpu->mem + offset);
}
__glyph_inline__ u64 cpu_load_i8(cpu_state *cpu, u64 offset)
{
    return (u64)(i64)*(i8*)(cpu->mem + offset);
}
__glyph_inline__ void cpu_store_i64(cpu_state *cpu, u64 offset, u64 val)
{
    *(u64*)(cpu->mem + offset) = val;
}
__glyph_inline__ void cpu_store_i32(cpu_state *cpu, u64 offset, u64 val)
{
    *(u32*)(cpu->mem + offset) = (u32)val;
}
__glyph_inline__ void cpu_store_i16(cpu_state *cpu, u64 offset, u64 val)
{
    *(u16*)(cpu->mem + offset) = (u16)val;
}
__glyph_inline__ void cpu_store_i8(cpu_state *cpu, u64 offset, u64 val)
{
    *(u8*)(cpu->mem + offset) = (u8)val;
}
__glyph_inline__ u64 cpu_const_i64(cpu_state *cpu, u64 offset)
{
    return *(u64*)(cpu->mem + cpu->ib + offset * 8);
}
__glyph_inline__ u64 cpu_fetch(cpu_state *cpu)
{
    return (u64)*(i16*)(cpu->mem + cpu->pc);
}

/*
 * cpu emulation
 */

__glyph_func__ int cpu_exec_op_break(cpu_state *cpu, u64 inst)
{
    return -1;
}
__glyph_func__ int cpu_exec_op_j(cpu_state *cpu, u64 inst)
{
    cpu->pc = cpu->pc + (u64)(simm9(inst) << 1) + 2ull;
    return 0;
}
__glyph_func__ int cpu_exec_op_b(cpu_state *cpu, u64 inst)
{
    if (!cpu->flag) return 2;
    cpu->pc = cpu->pc + (u64)(simm9(inst) << 1) + 2ull;
    return 0;
}
__glyph_func__ int cpu_exec_op_ibj(cpu_state *cpu, u64 inst)
{
    cpu->ib = cpu->ib + (u64)(simm9(inst) << 6);
    return 2;
}
__glyph_func__ int cpu_exec_op_jalib(cpu_state *cpu, u64 inst)
{
    u64 rav = cpu_const_i64(cpu, uimm6(inst));
    i32 rpc = (i32)(rav      );
    i32 rib = (i32)(rav >> 32);
    cpu->pc = cpu->pc + (u64)rpc + 2ull;
    cpu->ib = cpu->ib + (u64)rib;
    cpu->r[rc(inst)] = rav;
    return 0;
}
__glyph_func__ int cpu_exec_op_jtlib(cpu_state *cpu, u64 inst)
{
    u64 rav = cpu->r[rc(inst)];
    i32 rpc = (i32)(rav      );
    i32 rib = (i32)(rav >> 32);
    u64 dav = cpu_const_i64(cpu, uimm6(inst));
    i32 dpc = (i32)(dav      );
    i32 dib = (i32)(dav >> 32);
    cpu->pc = cpu->pc + (u64)(dpc - rpc);
    cpu->ib = cpu->ib + (u64)(dib - rib);
    return 0;
}
__glyph_func__ int cpu_exec_op_lib_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = cpu_const_i64(cpu, uimm6(inst));
    return 2;
}
__glyph_func__ int cpu_exec_op_li_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = (u64)simm6(inst);
    return 2;
}
__glyph_func__ int cpu_exec_op_addi_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rc(inst)] + (u64)simm6(inst);
    return 2;
}
__glyph_func__ int cpu_exec_op_srli_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = (u64)cpu->r[rc(inst)] >> uimm6(inst);
    return 2;
}
__glyph_func__ int cpu_exec_op_srai_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = (u64)((i64)cpu->r[rc(inst)] >> uimm6(inst));
    return 2;
}
__glyph_func__ int cpu_exec_op_slli_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rc(inst)] << uimm6(inst);
    return 2;
}
__glyph_func__ int cpu_exec_op_addib_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rb(inst)] + cpu_const_i64(cpu, uimm3(inst));
    return 2;
}
__glyph_func__ int cpu_exec_op_load_i64(cpu_state *cpu, u64 inst)
{
    u64 tmp;
    tmp = cpu->r[rb(inst)] + (uimm3(inst) << 3);
    cpu->r[rc(inst)] = cpu_load_i64(cpu, tmp);
    return 2;
}
__glyph_func__ int cpu_exec_op_loadib_i64(cpu_state *cpu, u64 inst)
{
    u64 tmp;
    tmp = cpu->r[rb(inst)] + cpu_const_i64(cpu, uimm3(inst));
    cpu->r[rc(inst)] = cpu_load_i64(cpu, tmp);
    return 2;
}
__glyph_func__ int cpu_exec_op_cmp_i64(cpu_state *cpu, u64 inst)
{
    switch(uimm3(inst)) {
    case cpu_compare_lt:
        cpu->flag = (i64)cpu->r[rc(inst)] < (i64)cpu->r[rb(inst)];
        break;
    case cpu_compare_ge:
        cpu->flag = (i64)cpu->r[rc(inst)] >= (i64)cpu->r[rb(inst)];
        break;
    case cpu_compare_eq:
        cpu->flag = cpu->r[rc(inst)] == cpu->r[rb(inst)];
        break;
    case cpu_compare_ne:
        cpu->flag = cpu->r[rc(inst)] != cpu->r[rb(inst)];
        break;
    case cpu_compare_ltu:
        cpu->flag = cpu->r[rc(inst)] < cpu->r[rb(inst)];
        break;
    case cpu_compare_geu:
        cpu->flag = cpu->r[rc(inst)] >= cpu->r[rb(inst)];
        break;
    case cpu_compare_mov:
        if (cpu->flag) {
            cpu->r[rc(inst)] = cpu->r[rb(inst)];
        }
        break;
    default:
        return -1;
    }
    return 2;
}
__glyph_func__ int cpu_exec_op_subib_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rb(inst)] - cpu_const_i64(cpu, uimm3(inst));
    return 2;
}
__glyph_func__ int cpu_exec_op_store_i64(cpu_state *cpu, u64 inst)
{
    u64 tmp;
    tmp = cpu->r[rb(inst)] + (uimm3(inst) << 3);
    cpu_store_i64(cpu, tmp, cpu->r[rc(inst)]);
    return 2;
}
__glyph_func__ int cpu_exec_op_storeib_i64(cpu_state *cpu, u64 inst)
{
    u64 tmp;
    tmp = cpu->r[rb(inst)] + cpu_const_i64(cpu, uimm3(inst));
    cpu_store_i64(cpu, tmp, cpu->r[rc(inst)]);
    return 2;
}
__glyph_func__ int cpu_exec_op_logic_i64(cpu_state *cpu, u64 inst)
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
        cpu->r[rc(inst)] = (u64)__builtin_bswap64(cpu->r[rb(inst)]);
        break;
    case cpu_logic_ctz:
        cpu->r[rc(inst)] = (u64)__builtin_ctzll(cpu->r[rb(inst)]);
        break;
    case cpu_logic_clz:
        cpu->r[rc(inst)] = (u64)__builtin_clzll(cpu->r[rb(inst)]);
        break;
    case cpu_logic_ctpop:
        cpu->r[rc(inst)] = (u64)__builtin_popcountll(cpu->r[rb(inst)]);
        break;
    default:
        return -1;
    }
    return 2;
}
__glyph_func__ int cpu_exec_op_pin_i64(cpu_state *cpu, u64 inst)
{
    i32 rpc = (i32)(cpu->pc - cpu->r[ra(inst)] + 2ull);
    i32 rib = (i32)(cpu->ib - cpu->r[rb(inst)]);
    cpu->r[rc(inst)] = (u64)(u32)rpc | ((u64)rib << 32);
    return 2;
}
__glyph_func__ int cpu_exec_op_and_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rb(inst)] & cpu->r[ra(inst)];
    return 2;
}
__glyph_func__ int cpu_exec_op_or_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rb(inst)] | cpu->r[ra(inst)];
    return 2;
}
__glyph_func__ int cpu_exec_op_xor_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rb(inst)] ^ cpu->r[ra(inst)];
    return 2;
}
__glyph_func__ int cpu_exec_op_sub_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rb(inst)] - cpu->r[ra(inst)];
    return 2;
}
__glyph_func__ int cpu_exec_op_srl_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = (u64)cpu->r[rb(inst)] >> cpu->r[ra(inst)];
    return 2;
}
__glyph_func__ int cpu_exec_op_sra_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = (u64)((i64)cpu->r[rb(inst)] >> cpu->r[ra(inst)]);
    return 2;
}
__glyph_func__ int cpu_exec_op_sll_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rb(inst)] << cpu->r[ra(inst)];
    return 2;
}
__glyph_func__ int cpu_exec_op_add_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = cpu->r[rb(inst)] + cpu->r[ra(inst)];
    return 2;
}
__glyph_func__ int cpu_exec_op_mul_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = (u64)((i64)cpu->r[rb(inst)] * (i64)cpu->r[ra(inst)]);
    return 2;
}
__glyph_func__ int cpu_exec_op_div_i64(cpu_state *cpu, u64 inst)
{
    cpu->r[rc(inst)] = (u64)((i64)cpu->r[rb(inst)] / (i64)cpu->r[ra(inst)]);
    return 2;
}
__glyph_func__ int cpu_exec_op_illegal(cpu_state *cpu, u64 inst)
{
    return -1;
}

/*
 * cpu disassembly
 */

typedef int (*op_fn)(char *buf, size_t len, u64 inst);

static const char* cpu_opcode_str[32];
static const char* cpu_fun3_compare_str[8];
static const char* cpu_fun3_logic_str[8];

__glyph_func__ int op_nm(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, "%s", cpu_opcode_str[opc(inst)]);
}
__glyph_func__ int op_compare(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, "%s", cpu_fun3_compare_str[uimm3(inst)]);
}
__glyph_func__ int op_logic(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, "%s", cpu_fun3_logic_str[uimm3(inst)]);
}
__glyph_func__ int op_ib3(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, "ib(%llu)", uimm3(inst));
}
__glyph_func__ int op_ib6(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, "ib(%llu)", uimm6(inst));
}
__glyph_func__ int op_ui3x8(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, "%llu", uimm3(inst) << 3);
}
__glyph_func__ int op_ui6(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, "%llu", uimm6(inst));
}
__glyph_func__ int op_ui9(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, "%llu", uimm9(inst));
}
__glyph_func__ int op_si6(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, "%lld", simm6(inst));
}
__glyph_func__ int op_si9x2(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, "%lld", simm9(inst) << 1);
}
__glyph_func__ int op_si9x64(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, "%lld", simm9(inst) << 6);
}
__glyph_func__ int op_rc(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, "r%d", rc(inst));
}
__glyph_func__ int op_rb(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, "r%d", rb(inst));
}
__glyph_func__ int op_ra(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, "r%d", ra(inst));
}
__glyph_func__ int op_sp(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, " ");
}
__glyph_func__ int op_sc(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, ", ");
}
__glyph_func__ int op_op(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, "(");
}
__glyph_func__ int op_cp(char *buf, size_t len, u64 inst) {
    return snprintf(buf, len, ")");
}

enum
{
    op0r_uimm9,
    op0r_simm9x2,
    op0r_simm9x64,
    op1r_ib32x2_uimm6_src,
    op1r_ib32x2_uimm6_dst,
    op1r_ib64_uimm6,
    op1r_simm6,
    op1r_uimm6,
    op2r_ib64_uimm3,
    op2r_mem64_uimm3x8,
    op2r_mib64_uimm3,
    op2r_fun3_compare,
    op2r_fun3_logic,
    op3r,
};

static const char* cpu_opcode_str[32] =
{
    [cpu_op_break]          = "break",
    [cpu_op_j]              = "j",
    [cpu_op_b]              = "b",
    [cpu_op_ibj]            = "ibj",
    [cpu_op_jalib]          = "jalib",
    [cpu_op_jtlib]          = "jtlib",
    [cpu_op_lib_i64]        = "lib.i64",
    [cpu_op_li_i64]         = "li.i64",
    [cpu_op_addi_i64]       = "addi.i64",
    [cpu_op_srli_i64]       = "srli.i64",
    [cpu_op_srai_i64]       = "srai.i64",
    [cpu_op_slli_i64]       = "slli.i64",
    [cpu_op_addib_i64]      = "addib.i64",
    [cpu_op_load_i64]       = "load.i64",
    [cpu_op_loadib_i64]     = "loadib.i64",
    [cpu_op_cmp_i64]        = "cmp.i64",
    [cpu_op_subib_i64]      = "subib.i64",
    [cpu_op_store_i64]      = "store.i64",
    [cpu_op_storeib_i64]    = "storeib.i64",
    [cpu_op_logic_i64]      = "logic.i64",
    [cpu_op_pin_i64]        = "pin.i64",
    [cpu_op_and_i64]        = "and.i64",
    [cpu_op_or_i64]         = "or.i64",
    [cpu_op_xor_i64]        = "xor.i64",
    [cpu_op_sub_i64]        = "sub.i64",
    [cpu_op_srl_i64]        = "srl.i64",
    [cpu_op_sra_i64]        = "sra.i64",
    [cpu_op_sll_i64]        = "sll.i64",
    [cpu_op_add_i64]        = "add.i64",
    [cpu_op_mul_i64]        = "mul.i64",
    [cpu_op_div_i64]        = "div.i64",
    [cpu_op_illegal]        = "illegal",
};

static const char* cpu_fun3_compare_str[8] =
{
    [cpu_compare_lt]        = "cmp.lt.i64",
    [cpu_compare_ge]        = "cmp.ge.i64",
    [cpu_compare_eq]        = "cmp.eq.i64",
    [cpu_compare_ne]        = "cmp.ne.i64",
    [cpu_compare_ltu]       = "cmp.ltu.i64",
    [cpu_compare_geu]       = "cmp.geu.i64",
    [cpu_compare_mov]       = "cmov.i64",
};

static const char* cpu_fun3_logic_str[8] =
{
    [cpu_logic_mov]         = "mov.i64",
    [cpu_logic_not]         = "not.i64",
    [cpu_logic_neg]         = "neg.i64",
    [cpu_logic_bswap]       = "bswap.i64",
    [cpu_logic_ctz]         = "ctz.i64",
    [cpu_logic_clz]         = "clz.i64",
    [cpu_logic_ctpop]       = "ctpop.i64",
};

static const op_fn cpu_op_format_args[][10] =
{
    [op0r_uimm9]            = { op_nm, op_sp, op_ui9 },
    [op0r_simm9x2]          = { op_nm, op_sp, op_si9x2 },
    [op0r_simm9x64]         = { op_nm, op_sp, op_si9x64 },
    [op1r_ib32x2_uimm6_src] = { op_nm, op_sp, op_rc, op_sc, op_ib6 },
    [op1r_ib32x2_uimm6_dst] = { op_nm, op_sp, op_ib6, op_sc, op_rc },
    [op1r_ib64_uimm6]       = { op_nm, op_sp, op_rc, op_sc, op_ib6 },
    [op1r_simm6]            = { op_nm, op_sp, op_rc, op_sc, op_si6 },
    [op1r_uimm6]            = { op_nm, op_sp, op_rc, op_sc, op_ui6 },
    [op2r_ib64_uimm3]       = { op_nm, op_sp, op_rc, op_sc, op_rb,
                                op_sc, op_ib3 },
    [op2r_mem64_uimm3x8]    = { op_nm, op_sp, op_rc, op_sc, op_ui3x8,
                                op_op, op_rb, op_cp },
    [op2r_mib64_uimm3]      = { op_nm, op_sp, op_rc, op_sc, op_ib3,
                                op_op, op_rb, op_cp },
    [op2r_fun3_compare]     = { op_compare, op_sp, op_rc, op_sc, op_rb },
    [op2r_fun3_logic]       = { op_logic, op_sp, op_rc, op_sc, op_rb },
    [op3r]                  = { op_nm, op_sp, op_rc, op_sc, op_rb,
                                op_sc, op_ra },
};

static const int cpu_op_format_type[32] =
{
    [cpu_op_break]          = op0r_uimm9,
    [cpu_op_j]              = op0r_simm9x2,
    [cpu_op_b]              = op0r_simm9x2,
    [cpu_op_ibj]            = op0r_simm9x64,
    [cpu_op_jalib]          = op1r_ib32x2_uimm6_src,
    [cpu_op_jtlib]          = op1r_ib32x2_uimm6_dst,
    [cpu_op_lib_i64]        = op1r_ib64_uimm6,
    [cpu_op_li_i64]         = op1r_simm6,
    [cpu_op_addi_i64]       = op1r_simm6,
    [cpu_op_srli_i64]       = op1r_uimm6,
    [cpu_op_srai_i64]       = op1r_uimm6,
    [cpu_op_slli_i64]       = op1r_uimm6,
    [cpu_op_addib_i64]      = op2r_ib64_uimm3,
    [cpu_op_load_i64]       = op2r_mem64_uimm3x8,
    [cpu_op_loadib_i64]     = op2r_mib64_uimm3,
    [cpu_op_cmp_i64]        = op2r_fun3_compare,
    [cpu_op_subib_i64]      = op2r_ib64_uimm3,
    [cpu_op_store_i64]      = op2r_mem64_uimm3x8,
    [cpu_op_storeib_i64]    = op2r_mib64_uimm3,
    [cpu_op_logic_i64]      = op2r_fun3_logic,
    [cpu_op_pin_i64]        = op3r,
    [cpu_op_and_i64]        = op3r,
    [cpu_op_or_i64]         = op3r,
    [cpu_op_xor_i64]        = op3r,
    [cpu_op_sub_i64]        = op3r,
    [cpu_op_srl_i64]        = op3r,
    [cpu_op_sra_i64]        = op3r,
    [cpu_op_sll_i64]        = op3r,
    [cpu_op_add_i64]        = op3r,
    [cpu_op_mul_i64]        = op3r,
    [cpu_op_div_i64]        = op3r,
    [cpu_op_illegal]        = op0r_uimm9,
};

__glyph_func__ int cpu_disasm(char *buf, size_t len, u64 inst, u64 c)
{
    size_t offset = 0;
    const op_fn* fn = cpu_op_format_args[cpu_op_format_type[opc(inst)]];
    while (*fn) {
        offset += (size_t)(*fn++)(buf + offset, len - offset, inst);
    }
    return (int)offset;
}

/*
 * cpu instruction encoding
 */

__glyph_inline__ u16 op0ri9_enc(int opc, int imm9) {
    return (u16)((opc << 2) | ((imm9 & 511)<<7));
}
__glyph_inline__ u16 op1ri6_enc(int opc, int rc, int imm6) {
    return (u16)((opc << 2) | ((imm6 & 63)<<7) | ((rc & 7)<<13));
}
__glyph_inline__ u16 op2ri3_enc(int opc, int rc, int rb, int imm3) {
    return (u16)((opc << 2) | ((imm3 & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13));
}
__glyph_inline__ u16 op3ri0_enc(int opc, int rc, int rb, int ra) {
    return (u16)((opc << 2) | ((ra & 7)<<7) | ((rb & 7)<<10) | ((rc & 7)<<13));
}

__glyph_func__ u16 cpu_encode_op_break(int imm9) {
    return op0ri9_enc(cpu_op_break, imm9);
}
__glyph_func__ u16 cpu_encode_op_j(int pcrel9) {
    return op0ri9_enc(cpu_op_j, pcrel9 >> 1);
}
__glyph_func__ u16 cpu_encode_op_b(int pcrel9) {
    return op0ri9_enc(cpu_op_b, pcrel9 >> 1);
}
__glyph_func__ u16 cpu_encode_op_ibj(int pcrel9) {
    return op0ri9_enc(cpu_op_ibj, pcrel9 >> 6);
}
__glyph_func__ u16 cpu_encode_op_jalib(int rc, int ibrel6) {
    return op1ri6_enc(cpu_op_jalib, rc, ibrel6);
}
__glyph_func__ u16 cpu_encode_op_jtlib(int rc, int ibrel6) {
    return op1ri6_enc(cpu_op_jtlib, rc, ibrel6);
}
__glyph_func__ u16 cpu_encode_op_lib_i64(int rc, int ibrel6) {
    return op1ri6_enc(cpu_op_lib_i64, rc, ibrel6);
}
__glyph_func__ u16 cpu_encode_op_li_i64(int rc, int imm6) {
    return op1ri6_enc(cpu_op_li_i64, rc, imm6);
}
__glyph_func__ u16 cpu_encode_op_addi_i64(int rc, int imm6) {
    return op1ri6_enc(cpu_op_addi_i64, rc, imm6);
}
__glyph_func__ u16 cpu_encode_op_srli_i64(int rc, int imm6) {
    return op1ri6_enc(cpu_op_srli_i64, rc, imm6);
}
__glyph_func__ u16 cpu_encode_op_srai_i64(int rc, int imm6) {
    return op1ri6_enc(cpu_op_srai_i64, rc, imm6);
}
__glyph_func__ u16 cpu_encode_op_slli_i64(int rc, int imm6) {
    return op1ri6_enc(cpu_op_slli_i64, rc, imm6);
}
__glyph_func__ u16 cpu_encode_op_addib_i64(int rc, int rb, int ibimm3) {
    return op2ri3_enc(cpu_op_addib_i64, rc, rb, ibimm3);
}
__glyph_func__ u16 cpu_encode_op_load_i64(int rc, int rb, int imm3) {
    return op2ri3_enc(cpu_op_load_i64, rc, rb, imm3 >> 3);
}
__glyph_func__ u16 cpu_encode_op_loadib_i64(int rc, int rb, int ibimm3) {
    return op2ri3_enc(cpu_op_loadib_i64, rc, rb, ibimm3);
}
__glyph_func__ u16 cpu_encode_op_cmp_lt_i64(int rc, int rb) {
    return op2ri3_enc(cpu_op_cmp_i64, rc, rb, cpu_compare_lt);
}
__glyph_func__ u16 cpu_encode_op_cmp_ge_i64(int rc, int rb) {
    return op2ri3_enc(cpu_op_cmp_i64, rc, rb, cpu_compare_ge);
}
__glyph_func__ u16 cpu_encode_op_cmp_eq_i64(int rc, int rb) {
    return op2ri3_enc(cpu_op_cmp_i64, rc, rb, cpu_compare_eq);
}
__glyph_func__ u16 cpu_encode_op_cmp_ne_i64(int rc, int rb) {
    return op2ri3_enc(cpu_op_cmp_i64, rc, rb, cpu_compare_ne);
}
__glyph_func__ u16 cpu_encode_op_cmp_ltu_i64(int rc, int rb) {
    return op2ri3_enc(cpu_op_cmp_i64, rc, rb, cpu_compare_ltu);
}
__glyph_func__ u16 cpu_encode_op_cmp_geu_i64(int rc, int rb) {
    return op2ri3_enc(cpu_op_cmp_i64, rc, rb, cpu_compare_geu);
}
__glyph_func__ u16 cpu_encode_op_cmov_i64(int rc, int rb) {
    return op2ri3_enc(cpu_op_cmp_i64, rc, rb, cpu_compare_mov);
}
__glyph_func__ u16 cpu_encode_op_subib_i64(int rc, int rb, int ibimm3) {
    return op2ri3_enc(cpu_op_subib_i64, rc, rb, ibimm3);
}
__glyph_func__ u16 cpu_encode_op_store_i64(int rc, int rb, int imm3) {
    return op2ri3_enc(cpu_op_store_i64, rc, rb, imm3 >> 3);
}
__glyph_func__ u16 cpu_encode_op_storeib_i64(int rc, int rb, int ibimm3) {
    return op2ri3_enc(cpu_op_storeib_i64, rc, rb, ibimm3);
}
__glyph_func__ u16 cpu_encode_op_mov_i64(int rc, int rb) {
    return op2ri3_enc(cpu_op_logic_i64, rc, rb, cpu_logic_mov);
}
__glyph_func__ u16 cpu_encode_op_not_i64(int rc, int rb) {
    return op2ri3_enc(cpu_op_logic_i64, rc, rb, cpu_logic_not);
}
__glyph_func__ u16 cpu_encode_op_neg_i64(int rc, int rb) {
    return op2ri3_enc(cpu_op_logic_i64, rc, rb, cpu_logic_neg);
}
__glyph_func__ u16 cpu_encode_op_bswap_i64(int rc, int rb) {
    return op2ri3_enc(cpu_op_logic_i64, rc, rb, cpu_logic_bswap);
}
__glyph_func__ u16 cpu_encode_op_ctz_i64(int rc, int rb) {
    return op2ri3_enc(cpu_op_logic_i64, rc, rb, cpu_logic_ctz);
}
__glyph_func__ u16 cpu_encode_op_clz_i64(int rc, int rb) {
    return op2ri3_enc(cpu_op_logic_i64, rc, rb, cpu_logic_clz);
}
__glyph_func__ u16 cpu_encode_op_ctpop_i64(int rc, int rb) {
    return op2ri3_enc(cpu_op_logic_i64, rc, rb, cpu_logic_ctpop);
}
__glyph_func__ u16 cpu_encode_op_pin_i64(int rc, int rb, int ra) {
    return op3ri0_enc(cpu_op_pin_i64, rc, rb, ra);
}
__glyph_func__ u16 cpu_encode_op_and_i64(int rc, int rb, int ra) {
    return op3ri0_enc(cpu_op_and_i64, rc, rb, ra);
}
__glyph_func__ u16 cpu_encode_op_or_i64(int rc, int rb, int ra) {
    return op3ri0_enc(cpu_op_or_i64, rc, rb, ra);
}
__glyph_func__ u16 cpu_encode_op_xor_i64(int rc, int rb, int ra) {
    return op3ri0_enc(cpu_op_xor_i64, rc, rb, ra);
}
__glyph_func__ u16 cpu_encode_op_sub_i64(int rc, int rb, int ra) {
    return op3ri0_enc(cpu_op_sub_i64, rc, rb, ra);
}
__glyph_func__ u16 cpu_encode_op_srl_i64(int rc, int rb, int ra) {
    return op3ri0_enc(cpu_op_srl_i64, rc, rb, ra);
}
__glyph_func__ u16 cpu_encode_op_sra_i64(int rc, int rb, int ra) {
    return op3ri0_enc(cpu_op_sra_i64, rc, rb, ra);
}
__glyph_func__ u16 cpu_encode_op_sll_i64(int rc, int rb, int ra) {
    return op3ri0_enc(cpu_op_sll_i64, rc, rb, ra);
}
__glyph_func__ u16 cpu_encode_op_add_i64(int rc, int rb, int ra) {
    return op3ri0_enc(cpu_op_add_i64, rc, rb, ra);
}
__glyph_func__ u16 cpu_encode_op_mul_i64(int rc, int rb, int ra) {
    return op3ri0_enc(cpu_op_mul_i64, rc, rb, ra);
}
__glyph_func__ u16 cpu_encode_op_div_i64(int rc, int rb, int ra) {
    return op3ri0_enc(cpu_op_div_i64, rc, rb, ra);
}
__glyph_func__ u16 cpu_encode_op_illegal(int imm9) {
    return op0ri9_enc(cpu_op_illegal, imm9);
}

/*
 * cpu dispatch
 */

__glyph_func__ int cpu_exec(cpu_state *cpu, u64 inst)
{
    switch (opc(inst)) {
    case cpu_op_break: return cpu_exec_op_break(cpu, inst);
    case cpu_op_j: return cpu_exec_op_j(cpu, inst);
    case cpu_op_b: return cpu_exec_op_b(cpu, inst);
    case cpu_op_ibj: return cpu_exec_op_ibj(cpu, inst);
    case cpu_op_jalib: return cpu_exec_op_jalib(cpu, inst);
    case cpu_op_jtlib: return cpu_exec_op_jtlib(cpu, inst);
    case cpu_op_lib_i64: return cpu_exec_op_lib_i64(cpu, inst);
    case cpu_op_li_i64: return cpu_exec_op_li_i64(cpu, inst);
    case cpu_op_addi_i64: return cpu_exec_op_addi_i64(cpu, inst);
    case cpu_op_srli_i64: return cpu_exec_op_srli_i64(cpu, inst);
    case cpu_op_srai_i64: return cpu_exec_op_srai_i64(cpu, inst);
    case cpu_op_slli_i64: return cpu_exec_op_slli_i64(cpu, inst);
    case cpu_op_addib_i64: return cpu_exec_op_addib_i64(cpu, inst);
    case cpu_op_load_i64: return cpu_exec_op_load_i64(cpu, inst);
    case cpu_op_loadib_i64: return cpu_exec_op_loadib_i64(cpu, inst);
    case cpu_op_cmp_i64: return cpu_exec_op_cmp_i64(cpu, inst);
    case cpu_op_subib_i64: return cpu_exec_op_subib_i64(cpu, inst);
    case cpu_op_store_i64: return cpu_exec_op_store_i64(cpu, inst);
    case cpu_op_storeib_i64: return cpu_exec_op_storeib_i64(cpu, inst);
    case cpu_op_logic_i64: return cpu_exec_op_logic_i64(cpu, inst);
    case cpu_op_pin_i64: return cpu_exec_op_pin_i64(cpu, inst);
    case cpu_op_and_i64: return cpu_exec_op_and_i64(cpu, inst);
    case cpu_op_or_i64: return cpu_exec_op_or_i64(cpu, inst);
    case cpu_op_xor_i64: return cpu_exec_op_xor_i64(cpu, inst);
    case cpu_op_sub_i64: return cpu_exec_op_sub_i64(cpu, inst);
    case cpu_op_srl_i64: return cpu_exec_op_srl_i64(cpu, inst);
    case cpu_op_sra_i64: return cpu_exec_op_sra_i64(cpu, inst);
    case cpu_op_sll_i64: return cpu_exec_op_sll_i64(cpu, inst);
    case cpu_op_add_i64: return cpu_exec_op_add_i64(cpu, inst);
    case cpu_op_mul_i64: return cpu_exec_op_mul_i64(cpu, inst);
    case cpu_op_div_i64: return cpu_exec_op_div_i64(cpu, inst);
    case cpu_op_illegal: return cpu_exec_op_illegal(cpu, inst);
    }
    return -1;
}

/*
 * cpu implementation
 */

__glyph_func__ void cpu_init(cpu_state *cpu, size_t mem_size)
{
    cpu->flag = 0;
    memset(cpu->r, 0, sizeof(cpu->r));
    cpu->pc = 0x800;
    cpu->ib = 0x400;
    cpu->mem = calloc(mem_size, 1);
    cpu->mem_size = mem_size;
    cpu->is_trace = 1;
    cpu->is_dump = 0;
}

__glyph_func__ void cpu_dump(cpu_state *cpu)
{
    cpu_debug("pc:%016llx ib:%016llx flag:%d",
        cpu->pc, cpu->ib, cpu->flag);
    for (uint i = 0; i < cpu_reg_count; i += 4) {
        cpu_debug("r%d:%016llx r%d:%016llx r%d:%016llx r%d:%016llx",
            i+0, cpu->r[i+0], i+1, cpu->r[i+1],
            i+2, cpu->r[i+2], i+3, cpu->r[i+3]);
    }
}

__glyph_func__ void cpu_run(cpu_state *cpu)
{
    u16 inst;
    int ret = 0;
    char buf[128];
    for (;;)
    {
        inst = (u16)cpu_fetch(cpu);
        if (cpu->is_trace) {
            cpu_disasm(buf, sizeof(buf), inst, cpu->pc);
            cpu_debug("-- %08llx %04hx %s", cpu->pc, inst, buf);
        }
        ret = cpu_exec(cpu, (u64)inst);
        if (ret < 0) {
            cpu_debug("** %08llx cpu exception", cpu->pc);
            return;
        }
        cpu->pc += (u64)ret;
        if (cpu->is_dump) {
            cpu_dump(cpu);
        }
    }
}

__glyph_func__ void cpu_setup(cpu_state *cpu,
    u64 *c, size_t cl, u16 *i, size_t il)
{
    char buf[128];
    cpu_debug();
    cpu_debug("# constants:");
    for(size_t x = 0; x < (cl>>3); x++) {
        u64 a = cpu->ib + (x<<3);
        cpu_debug("# %08llx ib(%zu) <- %016llx", a, x, c[x]);
        cpu_store_i64(cpu, a, c[x]);
    }
    cpu_debug();
    cpu_debug("# instructions:");
    for(size_t x = 0; x < (il>>1); x++) {
        u64 a = cpu->pc + (x<<1);
        cpu_disasm(buf, sizeof(buf), i[x], a);
        cpu_debug("# %08llx %04hx %s", a, i[x], buf);
        cpu_store_i16(cpu, a, i[x]);
    }
}


__glyph_func__ void cpu_test_impl(const char *name,
    u64 *c, size_t cl, u16 *i, size_t il)
{
    cpu_debug("# test: %s", name);
    cpu_state cpu;
    cpu_init(&cpu, 8192);
    cpu_setup(&cpu, c, cl, i, il);
    cpu_debug();
    cpu_debug("++ begin");
    cpu_run(&cpu);
    cpu_debug("++ end\n");
    cpu_debug("# state");
    cpu_dump(&cpu);
}
