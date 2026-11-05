#include <time.h>
#include <stdint.h>

/* Devolve o tempo monotónico em nanossegundos. */
int64_t rt_get_cpu_time_ns(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

/* Espera aproximadamente 1 ms de tempo monotónico. */
static void wait_1ms(void)
{
    int64_t end = rt_get_cpu_time_ns() + 1000000LL;

    while (rt_get_cpu_time_ns() < end) {
        /* Espera ativa. */
    }
}

/* Calcula o número de iterações para cada função. */
static int get_iterations(int turma, int grupo, int extra)
{
    return (turma + 2 * grupo + 4) % 9 + extra;
}

void f1(int turma, int grupo)
{
    int iterations = get_iterations(turma, grupo, 30);

    for (int i = 0; i < iterations; i++) {
        wait_1ms();
    }
}

void f2(int turma, int grupo)
{
    int iterations = get_iterations(turma, grupo, 50);

    for (int i = 0; i < iterations; i++) {
        wait_1ms();
    }
}

void f3(int turma, int grupo)
{
    int iterations = get_iterations(turma, grupo, 80);

    for (int i = 0; i < iterations; i++) {
        wait_1ms();
    }
}