# Esquema para desenhar à mão

Pai cria vetorA e vetorB (1000 posições cada)
Pai cria memoria compartilhada com shmget (1000 inteiros)
Pai faz fork 4 vezes: Filho 1, Filho 2, Filho 3, Filho 4
Cada filho anexa a memoria com shmat
Filho 1 escreve posicoes 0 a 249
Filho 2 escreve posicoes 250 a 499
Filho 3 escreve posicoes 500 a 749
Filho 4 escreve posicoes 750 a 999
Cada filho termina (exit)
Pai espera os 4 com wait
Pai anexa a memoria, le o vetor resultado e imprime
Pai remove a memoria com shmctl
