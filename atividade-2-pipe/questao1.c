#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int fd[2];
    int n, i, valor, status;
    pid_t pid;

    printf("Quantidade de numeros: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "Quantidade invalida\n");
        return 1;
    }

    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    fflush(stdout);
    pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        int lidos = 0;
        int maior = 0;
        int menor = 0;
        long soma = 0;

        close(fd[1]);
        printf("Filho pid %d aguardando dados\n", getpid());

        while (read(fd[0], &valor, sizeof(int)) == sizeof(int)) {
            if (lidos == 0) {
                maior = valor;
                menor = valor;
            }
            if (valor > maior)
                maior = valor;
            if (valor < menor)
                menor = valor;
            soma = soma + valor;
            lidos = lidos + 1;
        }
        close(fd[0]);

        if (lidos == 0) {
            fprintf(stderr, "Filho pid %d nao recebeu dados\n", getpid());
            exit(2);
        }

        printf("Filho pid %d recebeu %d numeros\n", getpid(), lidos);
        printf("Filho pid %d soma %ld\n", getpid(), soma);
        printf("Filho pid %d media %.2f\n", getpid(), (double) soma / lidos);
        printf("Filho pid %d maior %d\n", getpid(), maior);
        printf("Filho pid %d menor %d\n", getpid(), menor);
        exit(0);
    }

    close(fd[0]);
    printf("Pai pid %d criou o filho %d\n", getpid(), pid);

    for (i = 0; i < n; i++) {
        printf("Numero %d: ", i + 1);
        fflush(stdout);
        if (scanf("%d", &valor) != 1) {
            fprintf(stderr, "Entrada invalida\n");
            break;
        }
        if (write(fd[1], &valor, sizeof(int)) != sizeof(int)) {
            perror("write");
            break;
        }
    }

    close(fd[1]);
    waitpid(pid, &status, 0);
    printf("Pai pid %d terminou, filho saiu com %d\n",
           getpid(), WEXITSTATUS(status));

    return 0;
}
