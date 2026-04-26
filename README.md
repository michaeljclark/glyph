# glyph

> a super regular RISC that encodes constants in immediate blocks.

![instruction and constant streams](/doc/concept.png)

_Figure 1 - instruction (pc-relative) and constant (ib-relative) streams_

## introduction

glyph is a proposed computer architecture that emphasizes simplicity,
efficiency, and flexibility. unlike typical processors, it separates
instructions from constants, which can make programs smaller and faster
to execute. it uses a compact, regular instruction format and a small
'predicate' register for control flow. the design carefully considers
how a processor reads, interprets, and executes instructions.

## highlights

this list outlines architectural highlights of the glyph architecture:

- *simplified decode* - instruction decode is dramatically simpler than CISC architectures.
- *constant blocks* - increase fetch bandwidth and speed up dynamic linking.
- *true carry* - simplifies big integer carry-add arithmetic used for cryptography.
- *divide-by-zero branch* - eliminates conditional branch instructions before divisions.
- *simplified vector SIMD* - 64-bit instruction packets support AVX10-like vector SIMD.
- *arithmetic vector swizzle* - speeds up subgroup shuffle operations for vector arithmetic.
- *capabilities and domains* - provide context switching and page-table color permissions.
- *privilege level free* - uses capabilities and domains to abstract privilege levels.
- *page table colors* - for zoned physical memory and address translation permissions.
- *message-signaled interrupts* - support globally-scoped inter-domain message interrupts.
- *multicast network-on-chip* - enables synchronized messaging for thread groups.
- *deadline timer interrupts* - use the processor high-precision clock for deadline timers.
- *muliple page sizes* - supports 64KiB and 4KiB page sizes to makes emulation easier.
- *software-defined MMU* - accelerates address translation for foreign page-table formats.
- *address-space prefixes* - enable support for OpenCL high-speed local scratchpad memory.
- *virtual machine definition* - provides EFI firmware, device-tree, and VirtIO device support.
- *comprehensible specification* - uses semantic normalization aligned with ARPA conventions.

## architecture

glyph is a super-regular RISC architecture that encodes constants in a
secondary constant stream accessed via an immediate base _(ib)_ register
that points to immediate blocks containing constants. the _(ib)_ register
can branch like the program counter _(pc)_, and call instructions set
_(pc, ib)_ simultaneously for procedure calls.

glyph uses relative addresses in its link register, allowing branch
instructions to pack _(pc, ib)_ displacements for compatibility with a
single link register. this is achieved by packing two 32-bit relative
_(pc, ib)_ displacements into an _i32x2_ vector. a 128-bit version of
the instruction set relaxes this relative address requirement.

glyph immediate blocks can be chained using relative displacements and
switched via the constant branch instruction. unlike typical RISC
architectures, immediate blocks containing constants allow relocations
to be word-sized, following C-style packing and alignment rules.

this list outlines differentiating features of the glyph architecture:

- variable length instruction format supporting 16, 32, and 64-bit instructions.
- 16-bit compressed instruction packets that can access 8 registers.
- 1-bit predicate for compare, branch, divide-by-zero, and add or subtract with carry.
- _(pc,ib)_ is a program counter and immediate base register address vector.
- link register contains a packed relative _(pc,ib)_ address vector to function entry.
- `ibj` _(immediate-block-jump)_ adds a relative address to the immediate base register.
- `mov` _(move-word)_ uses a displacement to access an immediate block constant.
- `jal` _(jump-and-link)_ links address and adds immediate block constants to _(pc,ib)_.
- `jtl` _(jump-to-link)_ subtracts link vector and adds immediate block constants to _(pc,ib)_.
- `pin` _(pack-indirect)_ packs two absolute addresses as relative address vector from _(pc,ib)_.

compiled versions of the architecture specification are available from the
following URLs:

- current: [glyph.pdf](https://metaparadigm.com/~mclark/glyph.pdf)
- latest: [glyph-20260426.pdf](https://metaparadigm.com/~mclark/glyph-20260426.pdf)

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
| 04 | link.i64     | op1r_imm6_16 | 00100 | **link** fun3,ib64(uimm6*8)                 |
| 05 | movd.i64     | op1r_imm6_16 | 00101 | **move-dword** rc,ib32(uimm6*8)             |
| 06 | movq.i64     | op1r_imm6_16 | 00110 | **move-qword** rc,ib64(uimm6*8)             |
| 07 | movi.i64     | op1r_imm6_16 | 00111 | **mov-imm6** rc,simm6                       |
| 08 | addi.i64     | op1r_imm6_16 | 01000 | **add-imm6** rc,simm6; flag                 |
| 09 | srli.i64     | op1r_imm6_16 | 01001 | **shift-right-logical-imm** rc,uimm6        |
| 10 | srai.i64     | op1r_imm6_16 | 01010 | **shift-right-arith-imm** rc,uimm6          |
| 11 | slli.i64     | op1r_imm6_16 | 01011 | **shift-left-logical-imm** rc,uimm6         |
| 12 | addd.i64     | op1r_imm6_16 | 01100 | **add-dword** rc,ib32(uimm6*4); flag        |
| 13 | leapc.i64    | op1r_imm6_16 | 01101 | **lea** rc,ib32(uimm6*4)(pc)                |
| 14 | loadpc.i64   | op1r_imm6_16 | 01110 | **load** rc,ib32(uimm6*4)(pc)               |
| 15 | storepc.i64  | op1r_imm6_16 | 01111 | **store** rc,ib32(uimm6*4)(pc)              |
| 16 | load.i64     | op2r_imm3_16 | 10000 | **load** rc,(uimm3*8)(rb)                   |
| 17 | store.i64    | op2r_imm3_16 | 10001 | **store** rc,(uimm3*8)(rb)                  |
| 18 | cmp.i64      | op2r_fun3_16 | 10010 | **compare** rc,rb,fun3; flag                |
| 19 | logic.i64    | op2r_fun3_16 | 10011 | **logic** rc,rb,fun3                        |
| 20 | pin.i64      | op3r_16      | 10100 | **pack-indirect** rc,rb,ra                  |
| 21 | and.i64      | op3r_16      | 10101 | **and** rc,rb,ra                            |
| 22 | or.i64       | op3r_16      | 10110 | **or** rc,rb,ra                             |
| 23 | xor.i64      | op3r_16      | 10111 | **xor** rc,rb,ra                            |
| 24 | add.i64      | op3r_16      | 11000 | **add** rc,rb,ra; flag                      |
| 25 | srl.i64      | op3r_16      | 11001 | **shift-right-logical** rc,rb,ra            |
| 26 | sra.i64      | op3r_16      | 11010 | **shift-right-arith** rc,rb,ra              |
| 27 | sll.i64      | op3r_16      | 11011 | **shift-left-logical** rc,rb,ra             |
| 28 | sub.i64      | op3r_16      | 11100 | **sub** rc,rb,ra; flag                      |
| 29 | mul.i64      | op3r_16      | 11101 | **mul** rc,rb,ra                            |
| 30 | div.i64      | op3r_16      | 11110 | **div** rc,rb,ra; flag                      |
| 31 | illegal      | op0r_imm9_16 | 11111 | **illegal** uimm9                           |

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
