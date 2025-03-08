## glyph

glyph is a super regular RISC that encodes constants in immediate blocks.

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
  - 16/32/64/128-bit instruction forms with 2-bit size encoding.

| instruction  | form         | code  | description                                 |
|:-------------|:-------------|:------|:--------------------------------------------|
| break        | op0r_imm9_16 | 00000 | imm9                                        |
| j            | op0r_imm9_16 | 00001 | jump pcrel9*2+2                             |
| b            | op0r_imm9_16 | 00010 | branch pcrel9*2+2                           |
| rsrv1        | op0r_imm9_16 | 00011 | ib-link reg,ibrel(imm6*8,i64)               |
| ibl          | op1r_imm6_16 | 00100 | ib-link reg,ibrel(imm6*8,i64)               |
|              |              |       |   ib += ib(imm6*8,i64)                      |
| jalib        | op1r_imm6_16 | 00101 | jump-and-link-ib reg,ibrel(imm6*8,i32x2)    |
|              |              |       |   (opc,oib) = (pc+2,ib);                    |
|              |              |       |   (pc,ib) += ib(imm6*8,i32x2);              |
|              |              |       |   i32x2(lr) = ib(imm6*8,i32x2);             |
| jtlib        | op1r_imm6_16 | 00110 | jump-to-link-ib reg,ibrel(imm6*8,i32x2)     |
|              |              |       |   (pc,ib) += ib(imm6*8,i32x2) - i32x2(lr);  |
| lib.i64      | op1r_imm6_16 | 00111 | load-imm-ib reg,ib(imm6*8,i64)              |
| li.i64       | op1r_imm6_16 | 01000 | load-imm reg,simm6                          |
| addi.i64     | op1r_imm6_16 | 01001 | add-imm reg,simm6                           |
| srli.i64     | op2r_imm3_16 | 01010 | shift-right-logical-imm reg,reg,uimm3       |
| srai.i64     | op2r_imm3_16 | 01011 | shift-right-arith-imm reg,reg,uimm3         |
| slli.i64     | op2r_imm3_16 | 01100 | shift-left-logical-imm reg,reg,uimm3        |
| load.i8      | op2r_imm3_16 | 01101 | load-i8 reg,uimm3(reg)                      |
| load.u8      | op2r_imm3_16 | 01110 | load-u8 reg,uimm3(reg)                      |
| load.i64     | op2r_imm3_16 | 01111 | load-i64 reg,uimm3(reg)                     |
| cmp.i64      | op2r_fun3_16 | 10000 | cmp reg,reg fun3=lg,ge,eq,ne,ltu,geu        |
| log.i64      | op2r_fun3_16 | 10001 | log reg,uimm3(reg) fun3=not,neg,ctz,clz,pop |
| store.i8     | op2r_imm3_16 | 10010 | store-i8 reg,uimm3(reg)                     |
| store.i64    | op2r_imm3_16 | 10011 | store-i64 reg,uimm3(reg)                    |
| pin          | op3r_16      | 10100 | pack-indirect reg,reg,reg                   |
|              |              |       |   i32x2(lr) = (pc-i32,ib-i32);              |
| srl.i64      | op3r_16      | 10101 | shift-right-logical reg,reg,reg             |
| sra.i64      | op3r_16      | 10110 | shift-right-arith reg,reg,reg               |
| sll.i64      | op3r_16      | 10111 | shift-left-logical reg,reg,reg              |
| add.i64      | op3r_16      | 11000 | add reg,reg,reg                             |
| sub.i64      | op3r_16      | 11001 | sub reg,reg,reg                             |
| and.i64      | op3r_16      | 11010 | and reg,reg,reg                             |
| or.i64       | op3r_16      | 11011 | or reg,reg,reg                              |
| xor.i64      | op3r_16      | 11100 | xor reg,reg,reg                             |
| nop          | op0r_imm9_16 | 11101 | nop imm9                                    |
| dump         | op0r_imm9_16 | 11110 | dump imm9                                   |
| illegal      | op0r_imm9_16 | 11111 | illegal imm9                                |
