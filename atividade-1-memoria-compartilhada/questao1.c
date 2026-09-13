#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define TAM 1000
#define NPROC 4

int main(void)
{
    int vetorA[TAM], vetorB[TAM];
    int i, shmid, seg = TAM / NPROC;
    int *resultado;

    for (i = 0; i < TAM; i++) {
        vetorA[i] = i + 1;
        vetorB[i] = TAM - i;
    }

    shmid = shmget(IPC_PRIVATE, TAM * sizeof(int), IPC_CREAT | 0666);

    for (i = 0; i < NPROC; i++) {
        if (fork() == 0) {
            int inicio = i * seg, fim = inicio + seg, k;

            resultado = shmat(shmid, NULL, 0);
            for (k = inicio; k < fim; k++)
                resultado[k] = vetorA[k] + vetorB[k];
            exit(0);
        }
    }

    for (i = 0; i < NPROC; i++)
        wait(NULL);

    resultado = shmat(shmid, NULL, 0);
    for (i = 0; i < TAM; i++)
        printf("%d ", resultado[i]);
    printf("\n");

    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
