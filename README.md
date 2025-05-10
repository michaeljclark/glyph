# glyph

> a super regular RISC that encodes constants in immediate blocks.

![instruction and constant streams](/doc/concept.png)

_Figure 1 - instruction (pc-relative) and constant (ib-relative) streams_

## introduction

glyph is a super regular RISC architecture that encodes constants in a
secondary constant stream accessed via an immediate base register. the
immediate base register branches like the program counter and the call
instructions sets _(pc,ib)_ together for procedure calls and returns.

glyph uses relative addresses in its link register which is different
to typical RISC architectures. glyph needs to do this for the branch
instruction to fit _(pc,ib)_ into the link register for compatibility.
glyph achieves this by packing together two 32-bit relative _(pc,ib)_
displacements in an _i32x2_ vector.

immediate blocks can be linked together using relative displacements
and switched using the constant branch instruction detailed below.
immediate blocks, unlike typical RISC architectures, mean that most
relocations are word sized like CISC architectures, and can use C-style
structure packing and alignment rules.

## architecture

this list of points outlines the primary differentiating elements of
the super regular RISC architecture:

- variable length instruction format supporting 16, 32, 64, and 128-bit
  instruction packets.
- instruction formats and 2-bit size encoding is detailed here:
  [packet.pdf](/doc/packet.pdf) and here [glyph.pdf](/doc/glyph.pdf).
- 16-bit compressed instruction packets can access 8x64-bit registers.
- (pc,ib) is a special program counter and immediate base register address
  vector.
- link register contains packed i32x2 relative address vector to function
  entry.
- `ibj` _(immediate-block-jump)_ adds a relative address to the immediate
  base register.
- `movib` _(move-immediate-block)_ uses unsigned displacement to access
  constants.
- `jalib` _(jump-and-link-immediate-block)_ or _(call)_ links address
  vector and adds constants to (pc,ib).
- `jtlib` _(jump-to-link-immediate-block)_ or _(ret)_ subtracts link
  vector from and adds constants to (pc,ib).
- `pin` _(pack-indirect)_ packs two absolute addresses as relative address
  vector from (pc,ib).
- `ret` requires a relocation due to the use of relative link address
  vectors.

## opcodes

this list outlines instructions, opcodes, and descriptions used in the
16-bit compressed instruction packet:

### 16-bit opcodes

| nr | instruction  | form         | code  | description                                 |
|:---|:-------------|:-------------|:------|:--------------------------------------------|
| 00 | break        | op0r_imm9_16 | 00000 | **break** uimm9                             |
| 01 | j            | op0r_imm9_16 | 00001 | **jump** simm9*2                            |
| 02 | b            | op0r_imm9_16 | 00010 | **branch** simm9*2                          |
| 03 | ibj          | op0r_imm9_16 | 00011 | **ib-jump** simm9*64                        |
| 04 | jalib.i64    | op1r_imm6_16 | 00100 | **jump-and-link-ib** rc,ib64(uimm6*8)       |
|    |              |              |       |   rc = ib32x2(uimm6*8);                     |
|    |              |              |       |   (pc,ib) += i32x2(rc) + i32x2(2,0);        |
| 05 | jtlib.i64    | op1r_imm6_16 | 00101 | **jump-to-link-ib** rc,ib64(uimm6*8)        |
|    |              |              |       |   tmp = ib32x2(uimm6*8) - i32x2(rc);        |
|    |              |              |       |   (pc,ib) += i32x2(tmp);                    |
| 06 | movib.i64    | op1r_imm6_16 | 00110 | **mov-ib** rc,ib64(uimm6*8)                 |
| 07 | movi.i64     | op1r_imm6_16 | 00111 | **mov-imm6** rc,simm6                       |
| 08 | addi.i64     | op1r_imm6_16 | 01000 | **add-imm6** rc,simm6                       |
| 09 | srli.i64     | op1r_imm6_16 | 01001 | **shift-right-logical-imm** rc,uimm6        |
| 10 | srai.i64     | op1r_imm6_16 | 01010 | **shift-right-arith-imm** rc,uimm6          |
| 11 | slli.i64     | op1r_imm6_16 | 01011 | **shift-left-logical-imm** rc,uimm6         |
| 12 | addib.i64    | op1r_imm6_16 | 01100 | **add-ib** rc,ib32(uimm6*4)                 |
| 13 | leapc.i64    | op1r_imm6_16 | 01101 | **lea-pc** rc,ib32(uimm6*4)(pc)             |
| 14 | loadpc.i64   | op1r_imm6_16 | 01110 | **load-pc** rc,ib32(uimm6*4)(pc)            |
| 15 | storepc.i64  | op1r_imm6_16 | 01111 | **store-pc** rc,ib32(uimm6*4)(pc)           |
| 16 | load.i64     | op2r_imm3_16 | 10000 | **load** rc,(uimm3*8)(rb)                   |
| 17 | store.i64    | op2r_imm3_16 | 10001 | **store** rc,(uimm3*8)(rb)                  |
| 18 | compare.i64  | op2r_fun3_16 | 10010 | **compare** rc,rb                           |
|    |              |              |       |   fun3=lt,ge,eq,ne,ltu,geu,cmov,ncmov       |
| 19 | logic.i64    | op2r_fun3_16 | 10011 | **logic** rc,rb                             |
|    |              |              |       |   fun3=mov,not,neg,bswap,ctz,clz,ctpop,sext |
| 20 | pin.i64      | op3r_16      | 10100 | **pack-indirect** rc,rb,ra                  |
|    |              |              |       |   i32x2(rc) = (pc-ra+2,ib-rb);              |
| 21 | and.i64      | op3r_16      | 10101 | **and** rc,rb,ra                            |
| 22 | or.i64       | op3r_16      | 10110 | **or** rc,rb,ra                             |
| 23 | xor.i64      | op3r_16      | 10111 | **xor** rc,rb,ra                            |
| 24 | add.i64      | op3r_16      | 11000 | **add** rc,rb,ra                            |
| 25 | srl.i64      | op3r_16      | 11001 | **shift-right-logical** rc,rb,ra            |
| 26 | sra.i64      | op3r_16      | 11010 | **shift-right-arith** rc,rb,ra              |
| 27 | sll.i64      | op3r_16      | 11011 | **shift-left-logical** rc,rb,ra             |
| 28 | sub.i64      | op3r_16      | 11100 | **sub** rc,rb,ra                            |
| 29 | mul.i64      | op3r_16      | 11101 | **mul** rc,rb,ra                            |
| 30 | div.i64      | op3r_16      | 11110 | **div** rc,rb,ra                            |
| 31 | illegal      | op0r_imm9_16 | 11111 | **illegal** uimm9                           |

## calling convention

### 16-bit calling convention

the 16-bit instruction packet, while intended to be used in conjunction
with the 32-bit opcodes, is designed as a complete subset, so there is
an ABI variant that targets a subset using only the 16-bit opcodes.

the register assignment for the 16-bit subset was chosen with this rationale:

- 2 blocks of 4 contiguous non-volatile _callee-save_ and volatile
  _caller-save_ registers.
- 3 special registers, 2 argument registers, 1 temporary register,
  and 3 save registers.
- 3 save registers to avoid excessive spilling around function calls.
- 1 temporary register to avoid spilling arguments to free a temporary.

the calling convention for the 16-bit subset is as follows:

- _immediate base_ `ib` is set by `call` instructions and must point to
a valid immediate block on function entry. function symbols are exported
with two labels; one in the `.text` section, and one in the `.const` section.
_immediate base_ must be restored to the entry value in the function
epilogue before it can be restored by `ret`.
- _argument registers_ `a0` and `a1` are used for the first two arguments,
and the remaining arguments are passed on the stack. _return value_ is
places in `a0` and `a1`, _temporary register_ `t0` is a volatile register,
and _frame pointer_ (if enabled) uses `s0`. there are two more non-volatile
_callee-save_ registers, `s1` and `s2`.

### 16-bit register assignment

the following table outlines the 16-bit register assignment, showing
register name alias, description, and non-volatile _callee-save_ or
volatile _caller-save_ status.

| name  | alias | description                                 | save   |
|:------|:------|:--------------------------------------------|:-------|
| r0    | sp    | stack pointer                               | callee |
| r1    | s0/fp | saved register 0 / frame pointer            | callee |
| r2    | s1    | saved register 1                            | callee |
| r3    | s2    | saved register 2                            | callee |
| r4    | t0    | temporary register 0                        | caller |
| r5    | a0    | argument register 0                         | caller |
| r6    | a1    | argument register 1                         | caller |
| r7    | ra    | return address / _(pc,ib)_ link vector      | caller |

## instructions formats

glyph uses a super regular RISC encoding designed for vectorized decoders.
the variable length instruction encoding supports 16-bit, 32-bit, 64-bit,
and 128-bit instruction packets. each 16-bit packet has 2-bits for size.
in contrast, RISC-V currently has a variable size field which requires
up to 7-bits for 64-bit instruction packets. glyph size decoding logic
is simpler and easily supports 16-wide decoders (256-bits) with 8x32-bit
instructions. for this reason, glyph does not support 48-bit instructions.

### 16-bit instruction formats

![16-bit instruction packet](/doc/packet-16.png)

_Figure 2 - one, two, and three operand 16-bit instruction formats_

### 32-bit instruction formats

![32-bit instruction packet](/doc/packet-32.png)

_Figure 3 - one, two, and three operand 32-bit instruction formats_

### 64-bit instruction formats

![64-bit instruction packet](/doc/packet-64.png)

_Figure 4 - one, two, and three operand 64-bit instruction formats_

## implementations

this repository contains three implementations of the super regular
RISC architecture. there is a simple interpreter written in C, another
one written in Go, and a reference interpreter written in Python.

### simple interpreter

the simple interpreter in C can be built using CMake. presently
the implementation is constrained to inline functions in one header.

```
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```

### differential testing

the simple interpreter in C, the Go interpreter and the reference
interpreter in Python all support the 16-bit compressed opcodes and can
be launched using the test script which runs all three then diffs the
output of the tests to perform differential testing between them.

```
python3 scripts/test.py
```

### vectorized decoder

this Python script allows one to explore the combinatorial decode window
for various widths using 16-bit alignment for instructions. the following
invocation prints the decode offsets for a 4-wide 64-bit decoder.

```
./scripts/combo.py --print-decode -w 4
```

![combinatorial decode offsets for 4-wide decoder](/doc/combos-4.png)

_Figure 5 - screenshot showing partial output from combos.py_
