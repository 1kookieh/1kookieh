#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define TAM 10000
#define NPROC 5

static int vetor[TAM];

int main(void)
{
    int i, j, shmid, pid, status;
    int segmento = TAM / NPROC;
    long *parciais;
    long total;
    double media;

    srand(1);
    for (i = 0; i < TAM; i++)
        vetor[i] = rand() % 100;

    shmid = shmget(IPC_PRIVATE, NPROC * sizeof(long), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    for (j = 0; j < NPROC; j++) {
        pid = fork();
        if (pid == -1) {
            perror("fork");
            return 1;
        }

        if (pid == 0) {
            int inicio = j * segmento;
            int fim = inicio + segmento;
            long soma = 0;
            int k;

            parciais = shmat(shmid, NULL, 0);
            if (parciais == (void *) -1) {
                perror("shmat");
                exit(1);
            }

            for (k = inicio; k < fim; k++)
                soma = soma + vetor[k];

            parciais[j] = soma;

            printf("Processo filho pid %d soma parcial %ld\n", getpid(), soma);

            shmdt(parciais);
            exit(0);
        }
    }

    for (j = 0; j < NPROC; j++)
        wait(&status);

    parciais = shmat(shmid, NULL, 0);
    if (parciais == (void *) -1) {
        perror("shmat");
        return 1;
    }

    total = 0;
    for (j = 0; j < NPROC; j++)
        total = total + parciais[j];

    media = (double) total / TAM;
    printf("Processo pai pid %d media geral %.2f\n", getpid(), media);

    shmdt(parciais);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
