# Esquema para desenhar à mão

## Questão 1

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

## Questão 2

Pai cria vetor de 10000 inteiros
Pai cria memoria compartilhada com shmget (5 valores long)
Pai faz fork 5 vezes: Filho 1 a Filho 5
Cada filho anexa a memoria com shmat
Filho 1 soma posicoes 0 a 1999, grava em parciais[0]
Filho 2 soma posicoes 2000 a 3999, grava em parciais[1]
Filho 3 soma posicoes 4000 a 5999, grava em parciais[2]
Filho 4 soma posicoes 6000 a 7999, grava em parciais[3]
Filho 5 soma posicoes 8000 a 9999, grava em parciais[4]
Cada filho termina (exit)
Pai espera os 5 com wait
Pai anexa a memoria, soma as 5 parciais e divide por 10000
Pai imprime a media e remove a memoria com shmctl
