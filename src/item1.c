
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#include "../func.h"

#define TURMA 1
#define GRUPO 0
#define REPETITIONS 1000

uint64_t to_nanoseconds(struct timespec t)
{
    return (uint64_t)t.tv_sec * 1000000000ULL
           + (uint64_t)t.tv_nsec;
}

void measure_function(const char *name, void (*function)(int, int))
{
    struct timespec start, end;

    uint64_t elapsed;
    uint64_t max_time = 0;
    uint64_t min_time = UINT64_MAX;
    uint64_t total_time = 0;

    for (int i = 0; i < REPETITIONS; i++) {

        clock_gettime(CLOCK_MONOTONIC, &start);

        function(TURMA, GRUPO);

        clock_gettime(CLOCK_MONOTONIC, &end);

        elapsed = to_nanoseconds(end) - to_nanoseconds(start);

        if (elapsed > max_time)
            max_time = elapsed;

        if (elapsed < min_time)
            min_time = elapsed;

        total_time += elapsed;
    }

    printf("\nFuncao %s\n", name);
    printf("-------------------------\n");
    printf("Medicoes: %d\n", REPETITIONS);
    printf("Tempo minimo: %.3f us\n", min_time / 1000.0);
    printf("Tempo medio:  %.3f us\n",
           (total_time / (double)REPETITIONS) / 1000.0);
    printf("Tempo maximo: %.3f us\n", max_time / 1000.0);
}

int main(void)
{
    printf("ITEM 1 - MEDICAO DOS TEMPOS DE EXECUCAO\n");
    printf("Turma: %d | Grupo: %d\n", TURMA, GRUPO);

    measure_function("f1", f1);
    measure_function("f2", f2);
    measure_function("f3", f3);

    printf("\nMedicoes concluidas.\n");

    return 0;
}
