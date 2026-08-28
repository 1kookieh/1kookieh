/* Questao 2: pipeline com tres processos e dois pipes.
   P1 le o arquivo, P2 ordena e remove duplicados, P3 conta os valores unicos. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX 100000

static int compara(const void *a, const void *b)
{
    int x = *(const int *) a;
    int y = *(const int *) b;
    return (x > y) - (x < y);
}

int main(int argc, char *argv[])
{
    const char *caminho = (argc > 1) ? argv[1] : "dados.txt";
    int p1[2], p2[2];
    pid_t pid_filtro, pid_contador;
    int status;

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
        static int v[MAX];
        int qtd = 0, valor, i;

        close(p1[1]);
        close(p2[0]);
        printf("[P2 filtro   pid=%d] ordena e remove duplicados\n", getpid());
        fflush(stdout);

        while (qtd < MAX && read(p1[0], &valor, sizeof(int)) == sizeof(int))
            v[qtd++] = valor;
        close(p1[0]);

        qsort(v, qtd, sizeof(int), compara);

        for (i = 0; i < qtd; i++) {
            if (i > 0 && v[i] == v[i - 1])
                continue;
            if (write(p2[1], &v[i], sizeof(int)) != sizeof(int)) {
                perror("write p2");
                break;
            }
        }

        close(p2[1]);
        printf("[P2 filtro   pid=%d] %d valores lidos, envio concluido\n",
               getpid(), qtd);
        exit(0);
    }

    fflush(stdout);
    pid_contador = fork();
    if (pid_contador == -1) {
        perror("fork");
        return 1;
    }

    if (pid_contador == 0) {
        int valor, total = 0;

        close(p1[0]);
        close(p1[1]);
        close(p2[1]);
        printf("[P3 contador pid=%d] aguardando valores unicos\n", getpid());
        fflush(stdout);

        while (read(p2[0], &valor, sizeof(int)) == sizeof(int))
            total++;
        close(p2[0]);

        printf("[P3 contador pid=%d] valores distintos: %d\n", getpid(), total);
        exit(0);
    }

    /* P1: le o arquivo e alimenta o primeiro pipe. */
    close(p1[0]);
    close(p2[0]);
    close(p2[1]);

    printf("[P1 leitor   pid=%d] arquivo: %s | filhos: %d e %d\n",
           getpid(), caminho, pid_filtro, pid_contador);

    FILE *arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo");
        close(p1[1]);
        waitpid(pid_filtro, NULL, 0);
        waitpid(pid_contador, NULL, 0);
        return 1;
    }

    int numero, enviados = 0;
    while (fscanf(arquivo, "%d", &numero) == 1) {
        if (write(p1[1], &numero, sizeof(int)) != sizeof(int)) {
            perror("write p1");
            break;
        }
        enviados++;
    }

    fclose(arquivo);
    close(p1[1]);
    printf("[P1 leitor   pid=%d] %d numeros enviados\n", getpid(), enviados);

    waitpid(pid_filtro, &status, 0);
    printf("[P1 leitor   pid=%d] P2 encerrou com codigo %d\n",
           getpid(), WIFEXITED(status) ? WEXITSTATUS(status) : -1);

    waitpid(pid_contador, &status, 0);
    printf("[P1 leitor   pid=%d] P3 encerrou com codigo %d\n",
           getpid(), WIFEXITED(status) ? WEXITSTATUS(status) : -1);

    return 0;
}
