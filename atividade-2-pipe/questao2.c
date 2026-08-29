#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int compara(const void *a, const void *b)
{
    int x = *(const int *) a;
    int y = *(const int *) b;

    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

int main(void)
{
    int p1[2], p2[2];
    int n, i, numero, status;
    pid_t pid_filtro, pid_contador;

    printf("Quantidade de numeros: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "Quantidade invalida\n");
        return 1;
    }

    if (pipe(p1) == -1 || pipe(p2) == -1) {
        perror("pipe");
        return 1;
    }

    fflush(stdout);
    pid_filtro = fork();
    if (pid_filtro == -1) {
        perror("fork");
        return 1;
    }

    if (pid_filtro == 0) {
        int *v = NULL;
        int cap = 0;
        int qtd = 0;
        int valor, j;

        close(p1[1]);
        close(p2[0]);
        printf("Processo 2 pid %d ordena e remove duplicados\n", getpid());
        fflush(stdout);

        while (read(p1[0], &valor, sizeof(int)) == sizeof(int)) {
            if (qtd == cap) {
                int *aux;
                cap = (cap == 0) ? 1024 : cap * 2;
                aux = realloc(v, (size_t) cap * sizeof(int));
                if (aux == NULL) {
                    perror("realloc");
                    free(v);
                    exit(1);
                }
                v = aux;
            }
            v[qtd] = valor;
            qtd = qtd + 1;
        }
        close(p1[0]);

        if (qtd > 0)
            qsort(v, qtd, sizeof(int), compara);

        for (j = 0; j < qtd; j++) {
            if (j > 0 && v[j] == v[j - 1])
                continue;
            if (write(p2[1], &v[j], sizeof(int)) != sizeof(int)) {
                perror("write");
                break;
            }
        }

        free(v);
        close(p2[1]);
        printf("Processo 2 pid %d recebeu %d numeros\n", getpid(), qtd);
        exit(0);
    }

    fflush(stdout);
    pid_contador = fork();
    if (pid_contador == -1) {
        perror("fork");
        return 1;
    }

    if (pid_contador == 0) {
        int valor;
        int total = 0;

        close(p1[0]);
        close(p1[1]);
        close(p2[1]);
        printf("Processo 3 pid %d aguardando valores unicos\n", getpid());
        fflush(stdout);

        while (read(p2[0], &valor, sizeof(int)) == sizeof(int))
            total = total + 1;
        close(p2[0]);

        printf("Processo 3 pid %d valores distintos %d\n", getpid(), total);
        exit(0);
    }

    close(p1[0]);
    close(p2[0]);
    close(p2[1]);

    printf("Processo 1 pid %d lendo os numeros\n", getpid());

    for (i = 0; i < n; i++) {
        printf("Numero %d: ", i + 1);
        fflush(stdout);
        if (scanf("%d", &numero) != 1) {
            fprintf(stderr, "Entrada invalida\n");
            break;
        }
        if (write(p1[1], &numero, sizeof(int)) != sizeof(int)) {
            perror("write");
            break;
        }
    }

    close(p1[1]);
    waitpid(pid_filtro, &status, 0);
    waitpid(pid_contador, &status, 0);

    return 0;
}
