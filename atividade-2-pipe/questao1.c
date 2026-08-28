/* Questao 1: pai le N inteiros do usuario e envia por pipe ao filho.
   O filho calcula soma, media, maior e menor, imprime e encerra.  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int fd[2];
    int n, i;
    pid_t pid;

    printf("Quantos numeros serao lidos? ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "Quantidade invalida.\n");
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
        int valor, lidos = 0;
        long soma = 0;
        int maior = 0, menor = 0;

        close(fd[1]);
        printf("[FILHO  pid=%d ppid=%d] aguardando dados no pipe\n",
               getpid(), getppid());

        while (read(fd[0], &valor, sizeof(int)) == sizeof(int)) {
            if (lidos == 0) {
                maior = valor;
                menor = valor;
            } else {
                if (valor > maior) maior = valor;
                if (valor < menor) menor = valor;
            }
            soma += valor;
            lidos++;
        }
        close(fd[0]);

        if (lidos == 0) {
            fprintf(stderr, "[FILHO  pid=%d] nenhum numero recebido\n", getpid());
            exit(2);
        }

        printf("[FILHO  pid=%d] recebidos: %d\n", getpid(), lidos);
        printf("[FILHO  pid=%d] soma  = %ld\n", getpid(), soma);
        printf("[FILHO  pid=%d] media = %.2f\n", getpid(), (double) soma / lidos);
        printf("[FILHO  pid=%d] maior = %d\n", getpid(), maior);
        printf("[FILHO  pid=%d] menor = %d\n", getpid(), menor);
        exit(0);
    }

    close(fd[0]);
    printf("[PAI    pid=%d] filho criado com pid=%d\n", getpid(), pid);

    for (i = 0; i < n; i++) {
        int valor;
        printf("[PAI    pid=%d] numero %d: ", getpid(), i + 1);
        fflush(stdout);
        if (scanf("%d", &valor) != 1) {
            fprintf(stderr, "[PAI    pid=%d] entrada invalida\n", getpid());
            break;
        }
        if (write(fd[1], &valor, sizeof(int)) != sizeof(int)) {
            perror("write");
            break;
        }
    }

    close(fd[1]);
    printf("[PAI    pid=%d] escrita encerrada, aguardando o filho\n", getpid());

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(status))
        printf("[PAI    pid=%d] filho terminou com codigo %d\n",
               getpid(), WEXITSTATUS(status));
    else
        printf("[PAI    pid=%d] filho terminou de forma anormal\n", getpid());

    return 0;
}
