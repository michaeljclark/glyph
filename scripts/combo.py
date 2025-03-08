#!/usr/bin/env python3
#
# glyph is a super regular RISC that encodes constants in immediate blocks.
#
# Copyright (c) 2020 Michael Clark <michaeljclark@mac.com>
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

import math
import argparse

fetch_width = 4

class Insn:
    def __init__(self,width,offset):
        self.width = width
        self.offset = offset
    def __str__(self):
        return "({0},{1})".format(self.width, self.offset)

def gen_combos(window,l):
    width = l[-1].offset + l[-1].width
    if window - width >= 1:
        for i in [ Insn(1,width), Insn(2,width), Insn(4,width) ]:
            yield from gen_combos(window,l + [i])
    else:
        yield l

def insn_combos(window):
    s = [
        Insn(2,-1),
        Insn(4,-3),
        Insn(4,-2),
        Insn(4,-1),
    ]
    for o in range(0, window):
        s += [
            Insn(1,o),
            Insn(2,o),
            Insn(4,o),
        ]
    for i in s:
        for l in gen_combos(window,[i]):
            yield l

def insn_format(window,l):
    m = [0] * (8 + window)
    for i in l:
        o = i.offset + 4
        if i.width == 1:
            m[o] = 1
        elif i.width == 2:
            m[o] = 2
        elif i.width == 4:
            m[o] = 4
    insn = ""
    c = 0
    d = 0
    for i,n in enumerate(m):
        if n > 0:
            c = d = n
        if c > 0:
            if i < 4 or i > window + 3:
                insn += "\033[38:5:243m▣▣\033[0m"
            else:
                insn += "\033[38:5:232m▣▣\033[0m"
        else:
            if i >= 4 and i <= window + 3:
                insn += "\033[38:5:243m▣▣\033[0m"
            else:
                insn += "\033[38:5:254m▣▣\033[0m"
        if c <= 1:
            if i >= 4 and i <= window + 3:
                insn += "\033[38:5:248m▫\033[0m"
            else:
                insn += "\033[38:5:254m▫\033[0m"
        else:
            if i < 4 or i > window + 2:
                insn += "\033[38:5:243m▪\033[0m"
            else:
                insn += "\033[38:5:232m▪\033[0m"
        c -= 1
    return insn + " | " + " ".join(str(i) for i in l)

def insn_size_mask(window,l):
    m = [0] * (8 + window)
    n = [3] * (8 + window)
    for i in l:
        o = i.offset + 4
        if i.width == 1:
            m[o] = 3
            n[o] = 0
        elif i.width == 2:
            m[o] = m[o+1] = 3
            n[o] = 1
        elif i.width == 4:
            m[o] = m[o+1] = m[o+2] = m[o+3] = 3
            n[o] = 2
    mask, size = 0, 0
    for o in range(0,window):
        mask |= m[o+4] << (o*2)
        size |= n[o+4] << (o*2)
    return size, mask

def insn_sort_tuple(l):
    return (l[0].offset, int(math.log2(l[0].width)) if l[0].offset < 0 else 0)

def insn_begin_offset_width(window,l):
    width = int(math.log2(l[0].width)) if l[0].offset < 0 else 0
    offset = 4 + l[0].offset
    return offset, width

def insn_end_offset_width(window,l):
    width = int(math.log2(l[-1].width)) if l[-1].offset + l[-1].width != window else 0
    offset = l[-1].offset if l[-1].offset + l[-1].width != window else window
    return offset, width

#
# print decode table
#

def print_table(window):
    combos = sorted(list(insn_combos(window)), key=lambda l: insn_sort_tuple(l), reverse=False)

    print("combinations: ", len(combos), "\n")

    print("% 3s %3s | %3s %3s | %s %s | %s" %
        ("beg", "beg", "end", "end",
            "{:<{}}".format("insn", 2 * window),
            "{:<{}}".format("insn", 2 * window), ""))
    print("% 3s %3s | %3s %3s | %s %s | %s" %
        ("off", "len", "off", "len",
            "{:<{}}".format("width", 2 * window),
            "{:<{}}".format("mask", 2 * window), ""))
    print("% 3s %3s | %3s %3s | %s %s | %s" %
        ("---", "---", "---", "---", "--" * window, "--" * window,
         "-----------|" + ("-" * (window*3-1)) +
         "|------------ | " + ("------" * window)))

    for l in combos:
        bformat = "{0:0>%db}" % (window * 2)
        size, mask = insn_size_mask(window,l)
        begin_offset, begin_width = insn_begin_offset_width(window,l)
        end_offset, end_width = insn_end_offset_width(window,l)
        insn = insn_format(window,l)
        print("% 3d % 3d | % 3d % 3d | %s %s | %s" % (begin_offset, begin_width,
            end_offset, end_width, bformat.format(size), bformat.format(mask), insn))

#
# print decode array
#

"""
struct fetch_metadata
{
    uint8_t size;
    uint8_t mask;
    uint8_t count;
    uint8_t end_offset;
    uint8_t end_width;
    uint8_t roverhang;
    uint8_t lshift[4];
    uint8_t rshift[4];
};
"""

def insn_roverhang(t):
    o = t.offset << 4
    return 64+o if t.offset < 0 else 0

def insn_lrshift(t):
    o, w = t.offset << 4, t.width << 4
    return (64-(o+w), 64-w) if 64-(o+w) >= 0 else (0, o)

def print_array(window):
    combos = sorted(list(insn_combos(window)), key=lambda l: insn_sort_tuple(l), reverse=False)
    a = set()
    for l in combos:
        a.add(insn_sort_tuple(l))
    min_off, max_off = min(a)[0], max(a)[0]
    num_off = max_off - min_off + 1
    print("const fetch_metadata meta[%d][%d][256] = {" % (num_off, 3))
    for off in sorted(set(map(lambda t: t[0] - min_off, sorted(a)))):
        print("  [%d] = {" % off)
        for sz in map(lambda t: t[1], filter(lambda t: off == t[0] - min_off, sorted(a))):
            print("    [%d] = {" % sz)
            for l in combos:
                if insn_sort_tuple(l) == (off + min_off, sz):
                    size, mask = insn_size_mask(window,l)
                    end_offset, end_width = insn_end_offset_width(window,l)
                    count = len(l) if end_offset == 4 else len(l) - 1
                    lshift = map(lambda t: insn_lrshift(t)[0], l[0:count])
                    rshift = map(lambda t: insn_lrshift(t)[1], l[0:count])
                    roverhang = insn_roverhang(l[0])
                    for s in filter(lambda x: (x & mask) == (size & mask), range(0,256)):
                        print("      [%d] = { %d, %d, %d, %d, %d, %d, { %s }, { %s } }, // %s" %
                            (s, size, mask, count, end_offset-1, end_width, roverhang,
                             ", ".join(str(n) for n in lshift),
                             ", ".join(str(n) for n in rshift),
                             ", ".join(str(t) for t in l)))
            print("    },")
        print("  },")
    print("};")

def print_switch(window):
    combos = sorted(list(insn_combos(window)), key=lambda l: insn_sort_tuple(l), reverse=False)
    a = set()
    for l in combos:
        a.add(insn_sort_tuple(l))
    min_off, max_off = min(a)[0], max(a)[0]
    num_off = max_off - min_off + 1
    # begin_offset, begin_size, size_word
    print("switch (begin_offset) {")
    for off in sorted(set(map(lambda t: t[0] - min_off, sorted(a)))):
        print("case %d:" % off)
        print("    switch (begin_size) {")
        for sz in map(lambda t: t[1], filter(lambda t: off == t[0] - min_off, sorted(a))):
            print("    case %d:" % sz)
            print("        switch (size_word) {")
            for l in combos:
                if insn_sort_tuple(l) == (off + min_off, sz):
                    size, mask = insn_size_mask(window,l)
                    end_offset, end_width = insn_end_offset_width(window,l)
                    count = len(l) if end_offset == 4 else len(l) - 1
                    lshift = list(map(lambda t: insn_lrshift(t)[0], l[0:count]))
                    rshift = list(map(lambda t: insn_lrshift(t)[1], l[0:count]))
                    roverhang = insn_roverhang(l[0])
                    sizes = list(filter(lambda x: (x & mask) == (size & mask), range(0,256)))
                    bodies = []
                    for s in sizes:
                        body = ("            next->end_offset = " + str(end_offset-1) + ";\n" +
                                "            next->end_size = " + str(end_width) + ";\n" +
                                "            next->count = " + str(count) + ";\n")
                        if roverhang > 0:
                            body += ("            next->data[0] = (word_aligned << " +
                                     str(lshift[0]) + " >> " + str(rshift[0]) +
                                     ") | (prev->word_aligned >> " + str(roverhang) + ");\n")
                        for i in range(int(roverhang > 0),count):
                            body += ("            next->data[" + str(i) +
                                "] = word_aligned << " + str(lshift[i]) +
                                " >> " + str(rshift[i]) + ";\n")
                        body += "            break;\n";
                        bodies.append(body)
                    for i, s in enumerate(sizes):
                        print("        case %d:" % (s))
                        if i+1 >= len(sizes) or bodies[i+1] != bodies[i]:
                            print(bodies[i], end='')
            print("        }")
            print("        break;")
        print("    }")
        print("    break;")
    print("}")

#
# print decode tests
#

def insn_test(window,l):
    n = [0] * (window+8)
    m = [0] * (window+8)
    for i in l:
        o = i.offset + 4
        if i.width == 1:
            n[o] = 0
            m[o] = 0xf000
        elif i.width == 2:
            n[o] = 1
            n[o+1] = 3
            m[o] = 0x0000
            m[o+1] = 0xf000
        elif i.width == 4:
            n[o] = 2
            n[o+1] = n[o+2] = n[o+3] = 3
            m[o] = 0x0000
            m[o+1] = 0x0000
            m[o+2] = 0x0000
            m[o+3] = 0xf000
    insn = 0
    for o in range(0,window+8):
        insn |= n[o] << (o*16)
        insn |= m[o] << (o*16)
    n = insn.to_bytes(length=(window+8)*2, byteorder='little', signed=False)
    return n

def to_array(b):
    return ",".join(map(lambda x: "0x%s" % x, map(lambda x: format(x, '02x'), b)))

def print_tests(window):
    combos = sorted(list(insn_combos(window)), key=lambda l: insn_sort_tuple(l), reverse=False)
    for l in combos:
        bformat = "{0:0>%dx}" % ((window+8) * 4)
        insn = insn_test(window,l)
        begin_offset = 4 + l[0].offset
        end_offset = 4 + l[-1].offset + l[-1].width
        count = len(l)
        sizes = map(lambda t: t.width, l[0:count])
        print("{ %d, %d, %d, { %s }, { %s } }, // %s" % (
            begin_offset*2, end_offset*2, count, to_array(insn),
            ", ".join(str(n) for n in sizes), " ".join(str(i) for i in l)))

#
# command processing
#

parser = argparse.ArgumentParser(description='EBNF parser')
parser.add_argument('-d', '--print-decode', default=False, action='store_true',
                          help='dump instruction decode table')
parser.add_argument('-a', '--print-array', default=False, action='store_true',
                          help='dump instruction decode metadata')
parser.add_argument('-s', '--print-switch', default=False, action='store_true',
                          help='dump instruction decode switch')
parser.add_argument('-t', '--print-tests', default=False, action='store_true',
                          help='dump instruction decode tests')
parser.add_argument('-w', '--window', action='store', type=int,
                          help='fetch window', default=4)
args = parser.parse_args()

if args.print_decode:
    print_table(args.window)
if args.print_array:
    print_array(args.window)
if args.print_switch:
    print_switch(args.window)
if args.print_tests:
    print_tests(args.window)
