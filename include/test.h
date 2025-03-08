#pragma once

enum
{
    test_mem = 8192
};

static void init_const(cpu_state *cpu, i64 *d, size_t l)
{
    memcpy(cpu->mem + cpu->ib, d, l);
    printf("\n# constants:\n");
    for(size_t i = 0; i < (l>>3); i++) {
        printf("# %08llx ib(%zu) <- %016llx\n", cpu->ib + (i<<3), i, d[i]);
    }
}

static void init_inst(cpu_state *cpu, i16 *d, size_t l)
{
    char buf[128];
    memcpy(cpu->mem + cpu->pc, d, l);
    printf("\n# instructions:\n");
    for(size_t i = 0; i < (l>>1); i++) {
        cpu_disasm(buf, sizeof(buf), d[i], cpu->pc + (i<<1));
        printf("# %08llx %s\n", cpu->pc + (i<<1), buf);
    }
}

static void run_test(const char *name, i64 *c, size_t cl, i16 *i, size_t il)
{
    printf("# test: %s\n", name);
    cpu_state cpu;
    cpu_init(&cpu, test_mem);
    init_const(&cpu, c, cl);
    init_inst(&cpu, i, il);
    printf("\n++ begin\n");
    cpu_run(&cpu, 1, 0);
    printf("++ end\n\n");
    printf("# state\n");
    cpu_dump(&cpu);
}