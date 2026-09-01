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
    int vetorA[TAM];
    int vetorB[TAM];
    int i, j, shmid, pid, status;
    int segmento = TAM / NPROC;
    int *resultado;

    for (i = 0; i < TAM; i++) {
        vetorA[i] = i + 1;
        vetorB[i] = TAM - i;
    }

    shmid = shmget(IPC_PRIVATE, TAM * sizeof(int), IPC_CREAT | 0666);
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
            int k;

            resultado = shmat(shmid, NULL, 0);
            if (resultado == (void *) -1) {
                perror("shmat");
                exit(1);
            }

            printf("Processo filho pid %d soma indices %d a %d\n",
                   getpid(), inicio, fim - 1);

            for (k = inicio; k < fim; k++)
                resultado[k] = vetorA[k] + vetorB[k];

            shmdt(resultado);
            exit(0);
        }
    }

    for (j = 0; j < NPROC; j++)
        wait(&status);

    resultado = shmat(shmid, NULL, 0);
    if (resultado == (void *) -1) {
        perror("shmat");
        return 1;
    }

    printf("Processo pai pid %d imprime o vetor resultado\n", getpid());
    for (i = 0; i < TAM; i++)
        printf("%d ", resultado[i]);
    printf("\n");

    shmdt(resultado);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
