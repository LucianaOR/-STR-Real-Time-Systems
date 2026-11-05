
#include <stdio.h>
#include <math.h>

#define N 3

/* Tempos de execucao maximos, em ms */
double C[N] = {277.759288, 204.555718, 262.851810};

/* Periodos, em ms */
double T[N] = {100.0, 200.0, 300.0};

const char *names[N] = {"T1", "T2", "T3"};

/* Testa o tempo de resposta de uma tarefa */
int response_test(int task, int higher[], int n_higher,
                  double *response)
{
    double R = C[task];

    for (int iteration = 0; iteration < 100; iteration++) {
        double next = C[task];

        for (int j = 0; j < n_higher; j++) {
            int h = higher[j];
            next += ceil(R / T[h]) * C[h];
        }

        if (next > T[task]) {
            *response = next;
            return 0;
        }

        if (fabs(next - R) < 0.000001) {
            *response = next;
            return 1;
        }

        R = next;
    }

    *response = R;
    return 0;
}

/* Analisa uma ordem de prioridades */
void analyse_order(const char *title, int order[])
{
    printf("\n%s\n", title);
    printf("------------------------------\n");

    for (int i = 0; i < N; i++) {
        int higher[N];
        int count = 0;
        double R;

        for (int j = 0; j < i; j++)
            higher[count++] = order[j];

        int feasible = response_test(order[i], higher, count, &R);

        printf("%s: R = %.3f ms | Deadline = %.3f ms | ",
               names[order[i]], R, T[order[i]]);

        printf("%s\n", feasible ? "ESCALONAVEL" : "NAO ESCALONAVEL");
    }
}

/* Metodo de Audsley */
void audsley(void)
{
    int remaining[N] = {1, 1, 1};
    int priority_order[N];
    int assigned = 0;

    printf("\nMETODO DE AUDSLEY\n");
    printf("------------------------------\n");

    while (assigned < N) {
        int found = 0;

        for (int candidate = 0; candidate < N; candidate++) {
            if (!remaining[candidate])
                continue;

            int higher[N];
            int count = 0;
            double R;

            for (int j = 0; j < N; j++) {
                if (remaining[j] && j != candidate)
                    higher[count++] = j;
            }

            int feasible = response_test(
                candidate, higher, count, &R
            );

            printf("Testar %s como prioridade mais baixa: ",
                   names[candidate]);
            printf("R = %.3f ms -> %s\n",
                   R, feasible ? "PASSA" : "FALHA");

            if (feasible) {
                priority_order[assigned++] = candidate;
                remaining[candidate] = 0;
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("\nNao foi possivel atribuir prioridades ");
            printf("a todas as tarefas.\n");
            return;
        }
    }

    printf("\nOrdem encontrada (prioridade mais baixa primeiro): ");
    for (int i = 0; i < N; i++)
        printf("%s ", names[priority_order[i]]);

    printf("\n");
}

int main(void)
{
    int rmpo[N] = {0, 1, 2};
    int inverse[N] = {2, 1, 0};

    double utilization = 0.0;

    printf("ITEM 2 - ANALISE DE ESCALONABILIDADE\n");

    for (int i = 0; i < N; i++)
        utilization += C[i] / T[i];

    printf("\nUtilizacao total: %.4f (%.2f%%)\n",
           utilization, utilization * 100.0);

    double limite_rm = 3.0 * (pow(2.0, 1.0 / 3.0) - 1.0);

    printf("Limite RM para 3 tarefas: %.4f (%.2f%%)\n", limite_rm, limite_rm * 100.0);

    analyse_order("ORDEM RMPO (T1 > T2 > T3)", rmpo);

    analyse_order("ORDEM INVERSA (T3 > T2 > T1)", inverse);

    audsley();

    return 0;
}
