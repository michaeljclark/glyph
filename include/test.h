#pragma once

static void run_test(const char *name, i64 *c, size_t cl, i16 *i, size_t il)
{
    printf("# test: %s\n", name);
    cpu_state cpu;
    cpu_init(&cpu, 8192);
    cpu_setup(&cpu, c, cl, i, il);
    printf("\n++ begin\n");
    cpu_run(&cpu, 1, 0);
    printf("++ end\n\n");
    printf("# state\n");
    cpu_dump(&cpu);
}