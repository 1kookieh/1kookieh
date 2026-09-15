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
    int i, shmid, seg = TAM / NPROC;
    long *parciais, total = 0;

    srand(1);
    for (i = 0; i < TAM; i++)
        vetor[i] = rand() % 100;

    shmid = shmget(IPC_PRIVATE, NPROC * sizeof(long), IPC_CREAT | 0666);

    for (i = 0; i < NPROC; i++) {
        if (fork() == 0) {
            int inicio = i * seg, fim = inicio + seg, k;
            long soma = 0;

            parciais = shmat(shmid, NULL, 0);
            for (k = inicio; k < fim; k++)
                soma = soma + vetor[k];
            parciais[i] = soma;
            exit(0);
        }
    }

    for (i = 0; i < NPROC; i++)
        wait(NULL);

    parciais = shmat(shmid, NULL, 0);
    for (i = 0; i < NPROC; i++)
        total = total + parciais[i];

    printf("%.2f\n", (double) total / TAM);

    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
