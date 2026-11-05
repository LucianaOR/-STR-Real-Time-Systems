
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include "../func.h"

#define TURMA 1
#define GRUPO 0

#define NUM_TAREFAS 3
#define DURACAO 6.0

typedef struct {
    int id;
    int periodo_ms;
    int prioridade;
    void (*funcao)(int, int);
    const char *nome;
} tarefa_t;

typedef struct {
    int id;
    int numero;
    double ativacao;
    double inicio;
    double fim;
    double resposta;
    int deadline_cumprido;
} registo_t;

static tarefa_t tarefas[NUM_TAREFAS] = {
    {1, 100, 0, f1, "T1"},
    {2, 200, 0, f2, "T2"},
    {3, 300, 0, f3, "T3"}
};

static registo_t registos[NUM_TAREFAS][100];
static int total_registos[NUM_TAREFAS] = {0};

static struct timespec inicio_global;

static pthread_mutex_t mutex_registos = PTHREAD_MUTEX_INITIALIZER;

static double diferenca_ms(struct timespec a, struct timespec b)
{
    return (b.tv_sec - a.tv_sec) * 1000.0 +
           (b.tv_nsec - a.tv_nsec) / 1000000.0;
}

static struct timespec somar_ms(struct timespec t, long ms)
{
    t.tv_sec += ms / 1000;
    t.tv_nsec += (ms % 1000) * 1000000L;

    if (t.tv_nsec >= 1000000000L) {
        t.tv_sec++;
        t.tv_nsec -= 1000000000L;
    }

    return t;
}

static void *executar_tarefa(void *arg)
{
    tarefa_t *tarefa = (tarefa_t *)arg;

    struct timespec proxima_ativacao = inicio_global;
    int numero = 0;

    while (1) {
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,
                        &proxima_ativacao, NULL);

        struct timespec agora;
        clock_gettime(CLOCK_MONOTONIC, &agora);

        if (diferenca_ms(inicio_global, agora) >= DURACAO * 1000.0)
            break;

        numero++;

        struct timespec instante_inicio;
        struct timespec instante_fim;

        clock_gettime(CLOCK_MONOTONIC, &instante_inicio);

        tarefa->funcao(TURMA, GRUPO);

        clock_gettime(CLOCK_MONOTONIC, &instante_fim);

        double ativacao = diferenca_ms(inicio_global, proxima_ativacao);
        double inicio = diferenca_ms(inicio_global, instante_inicio);
        double fim = diferenca_ms(inicio_global, instante_fim);

        double resposta = fim - ativacao;
        double deadline = ativacao + tarefa->periodo_ms;

        pthread_mutex_lock(&mutex_registos);

        if (total_registos[tarefa->id - 1] < 100) {
            registo_t *r =
                &registos[tarefa->id - 1][total_registos[tarefa->id - 1]++];

            r->id = tarefa->id;
            r->numero = numero;
            r->ativacao = ativacao;
            r->inicio = inicio;
            r->fim = fim;
            r->resposta = resposta;
            r->deadline_cumprido = (fim <= deadline);
        }

        pthread_mutex_unlock(&mutex_registos);

        proxima_ativacao = somar_ms(proxima_ativacao,
                                    tarefa->periodo_ms);
    }

    return NULL;
}

int main(void)
{
    pthread_t threads[NUM_TAREFAS];

    printf("ITEM 6 - ROUND ROBIN COM PRIORIDADES IGUAIS\n");
    printf("Turma: %d | Grupo: %d\n\n", TURMA, GRUPO);

    clock_gettime(CLOCK_MONOTONIC, &inicio_global);

    pthread_attr_t attr[NUM_TAREFAS];
    struct sched_param param;

    for (int i = 0; i < NUM_TAREFAS; i++) {
        pthread_attr_init(&attr[i]);
        pthread_attr_setinheritsched(&attr[i], PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&attr[i], SCHED_RR);

        param.sched_priority = 20;
        pthread_attr_setschedparam(&attr[i], &param);

        int erro = pthread_create(&threads[i], &attr[i],
                                  executar_tarefa, &tarefas[i]);

        if (erro != 0) {
            fprintf(stderr,
                    "Erro ao criar thread %s: %s\n",
                    tarefas[i].nome, strerror(erro));
            return EXIT_FAILURE;
        }

        pthread_attr_destroy(&attr[i]);
    }

    for (int i = 0; i < NUM_TAREFAS; i++)
        pthread_join(threads[i], NULL);

    printf("REGISTOS DE EXECUCAO\n");
    printf("Tarefa | Exec. | Ativacao | Inicio | Fim | Resposta | Deadline\n");

    for (int i = 0; i < NUM_TAREFAS; i++) {
        for (int j = 0; j < total_registos[i]; j++) {
            registo_t r = registos[i][j];

            printf("%s | %3d | %8.3f | %8.3f | %8.3f | %8.3f | %s\n",
                   tarefas[i].nome,
                   r.numero,
                   r.ativacao,
                   r.inicio,
                   r.fim,
                   r.resposta,
                   r.deadline_cumprido ? "CUMPRIDO" : "FALHOU");
        }
    }

    printf("\nRESUMO\n");

    for (int i = 0; i < NUM_TAREFAS; i++) {
        double max_resposta = 0.0;
        double min_resposta = 1e9;
        double jitter = 0.0;
        int falhas = 0;

        for (int j = 0; j < total_registos[i]; j++) {
            registo_t r = registos[i][j];

            if (r.resposta > max_resposta)
                max_resposta = r.resposta;

            if (r.resposta < min_resposta)
                min_resposta = r.resposta;

            if (!r.deadline_cumprido)
                falhas++;
        }

        if (total_registos[i] > 0)
            jitter = max_resposta - min_resposta;

        printf("%s: execucoes=%d | resposta maxima=%.3f ms | "
               "jitter=%.3f ms | deadlines falhados=%d\n",
               tarefas[i].nome,
               total_registos[i],
               max_resposta,
               jitter,
               falhas);
    }

    return EXIT_SUCCESS;
}
