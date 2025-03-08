# glyph

> a super regular RISC that encodes constants in immediate blocks.

## introduction

glyph is a super regular RISC architecture that encodes constants in a
secondary constant stream accessed via an immediate base register. the
immediate base register branches like the program counter and the call
instructions sets _(pc,ib)_ together for procedure calls and returns.

glyph uses relative addresses in its link register which is different
to typical RISC architectures. glyph achieves this by packing two 32-bit
relative _(pc,ib)_ displacements into an _i32x2_ vector.

immediate blocks can be linked together using relative displacements
and switched using a constant branch instruction detailed below.
immediate blocks, unlike typical RISC architectures, mean relocations
are simple word sizes like CISC architectures, are aligned and can use
C-style structure packing rules.

## architecture

- 16-bit compressed instruction packets can access 8x64-bit registers.
- (pc,ib) is a special program counter and immediate base register pair.
- `ibl` adds 64-bit displacement ib(imm6*8) to link immediate blocks.
  - immediate blocks contain constants used by the current block.
  - _ibrel_ is immediate block relative indirect addressing for constants.
- `lib` uses unsigned 6-bit displacement to access 64x64-bit constants (64x8).
- `jalib` _(call)_ branch links address and adds ibrel i32x2 disp to (pc,ib).
  - links i32x2 _ibrel_ displacement of program counter and immediate base.
  - jumps to _ibrel_ (pc,ib) displacement in constant.
- `jtlib` _(ret)_ branch subtracts link and adds ibrel i32x2 disp to (pc,ib).
  - jumps to link register plus i32x2 _ibrel_ (pc,ib) displacement to entry.
  - returns from calls and requires a block-entry delta relocation.
- `pin` packs two absolute addresses as relative displacements from (pc,ib).
  - is used to call virtual functions within +/-2GiB from (pc,ib).
  - composes relative (pc,ib) address vector.
- link register contains packed i32x2 relative displacement to function entry.
- instruction format is detailed here: [packet.pdf](/doc/packet.pdf).
  - 16/32/64/128-bit instruction packets with 2-bit size encoding.
  - presently only the 16-bit compressed packet has an encoding.

## opcodes

### 16-bit opcodes

| instruction  | form         | code  | description                                 |
|:-------------|:-------------|:------|:--------------------------------------------|
| break        | op0r_imm9_16 | 00000 | **break** imm9                              |
| j            | op0r_imm9_16 | 00001 | **jump** pcrel9*2+2                         |
| b            | op0r_imm9_16 | 00010 | **branch** pcrel9*2+2                       |
| rsrv1        | op0r_imm9_16 | 00011 | **rsrv1** imm9                              |
| ibl          | op1r_imm6_16 | 00100 | **ib-link** rc,ibrel(imm6*8,i64)            |
|              |              |       |   ib += ib(imm6*8,i64)                      |
| jalib        | op1r_imm6_16 | 00101 | **jump-and-link-ib** rc,ibrel(imm6*8,i32x2) |
|              |              |       |   (opc,oib) = (pc+2,ib);                    |
|              |              |       |   (pc,ib) += ib(imm6*8,i32x2);              |
|              |              |       |   i32x2(lr) = ib(imm6*8,i32x2);             |
| jtlib        | op1r_imm6_16 | 00110 | **jump-to-link-ib** rc,ibrel(imm6*8,i32x2)  |
|              |              |       |   (pc,ib) += ib(imm6*8,i32x2) - i32x2(lr);  |
| lib.i64      | op1r_imm6_16 | 00111 | **load-imm-ib** rc,ib(imm6*8,i64)           |
| li.i64       | op1r_imm6_16 | 01000 | **load-imm** rc,simm6                       |
| addi.i64     | op1r_imm6_16 | 01001 | **add-imm** rc,simm6                        |
| srli.i64     | op2r_imm3_16 | 01010 | **shift-right-logical-imm** rc,rb,uimm3     |
| srai.i64     | op2r_imm3_16 | 01011 | **shift-right-arith-imm** rc,rb,uimm3       |
| slli.i64     | op2r_imm3_16 | 01100 | **shift-left-logical-imm** rc,rb,uimm3      |
| load.i8      | op2r_imm3_16 | 01101 | **load-i8** rc,uimm3(rb)                    |
| load.u8      | op2r_imm3_16 | 01110 | **load-u8** rc,uimm3(rb)                    |
| load.i64     | op2r_imm3_16 | 01111 | **load-i64** rc,uimm3(rb)                   |
| cmp.i64      | op2r_fun3_16 | 10000 | **cmp** rc,rb fun3=lg,ge,eq,ne,ltu,geu      |
| log.i64      | op2r_fun3_16 | 10001 | **log** rc,rb fun3=not,neg,ctz,clz,pop      |
| store.i8     | op2r_imm3_16 | 10010 | **store-i8** rc,uimm3(rb)                   |
| store.i64    | op2r_imm3_16 | 10011 | **store-i64** rc,uimm3(rb)                  |
| pin          | op3r_16      | 10100 | **pack-indirect** rc,rb,ra                  |
|              |              |       |   i32x2(lr) = (pc-i32,ib-i32);              |
| srl.i64      | op3r_16      | 10101 | **shift-right-logical** rc,rb,ra            |
| sra.i64      | op3r_16      | 10110 | **shift-right-arith** rc,rb,ra              |
| sll.i64      | op3r_16      | 10111 | **shift-left-logical** rc,rb,ra             |
| add.i64      | op3r_16      | 11000 | **add** rc,rb,ra                            |
| sub.i64      | op3r_16      | 11001 | **sub** rc,rb,ra                            |
| and.i64      | op3r_16      | 11010 | **and** rc,rb,ra                            |
| or.i64       | op3r_16      | 11011 | **or** rc,rb,ra                             |
| xor.i64      | op3r_16      | 11100 | **xor** rc,rb,ra                            |
| nop          | op0r_imm9_16 | 11101 | **nop** imm9                                |
| dump         | op0r_imm9_16 | 11110 | **dump** imm9                               |
| illegal      | op0r_imm9_16 | 11111 | **illegal** imm9                            |

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

### 32-bit instruction formats

![32-bit instruction packet](/doc/packet-32.png)

### 64-bit instruction formats

![64-bit instruction packet](/doc/packet-64.png)

## vectorized decoder

this Python script allows one to explore the combinatorial decode window
for various widths using 16-bit alignment for instructions. the following
invocation prints the decode offsets for a 4-wide 64-bit decoder.

```
./scripts/combo.py --print-decode -w 4
```

![combinatorial decode offsets for 4-wide decoder](/doc/combos-4.png)

_Figure 1 - screenshot showing partial output from combos.py_
