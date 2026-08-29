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

int main(int argc, char *argv[])
{
    int p1[2], p2[2];
    int numero, enviados, status;
    pid_t pid_filtro, pid_contador;
    FILE *arquivo;
    char *caminho;

    caminho = (argc > 1) ? argv[1] : "dados.txt";

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
        int valor, i;

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

        for (i = 0; i < qtd; i++) {
            if (i > 0 && v[i] == v[i - 1])
                continue;
            if (write(p2[1], &v[i], sizeof(int)) != sizeof(int)) {
                perror("write");
                break;
            }
        }

        free(v);
        close(p2[1]);
        printf("Processo 2 pid %d leu %d numeros\n", getpid(), qtd);
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

    printf("Processo 1 pid %d lendo o arquivo %s\n", getpid(), caminho);

    arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo");
        close(p1[1]);
        waitpid(pid_filtro, NULL, 0);
        waitpid(pid_contador, NULL, 0);
        return 1;
    }

    enviados = 0;
    while (fscanf(arquivo, "%d", &numero) == 1) {
        if (write(p1[1], &numero, sizeof(int)) != sizeof(int)) {
            perror("write");
            break;
        }
        enviados = enviados + 1;
    }

    fclose(arquivo);
    close(p1[1]);
    printf("Processo 1 pid %d enviou %d numeros\n", getpid(), enviados);

    waitpid(pid_filtro, &status, 0);
    waitpid(pid_contador, &status, 0);

    return 0;
}
