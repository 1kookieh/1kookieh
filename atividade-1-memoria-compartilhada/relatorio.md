# Atividade 1: Memória Compartilhada

Nome:

## Questão 1

**Justificativa da quantidade de processos adotada.**

Quatro processos filhos. 1000 divide exatamente por 4, 250 elementos por processo.

**Estrutura de dados utilizada para armazenar os resultados.**

Um vetor de 1000 inteiros em memória compartilhada, uma posição por elemento da soma.

**Para onde os resultados foram enviados.**

Memória compartilhada, criada com shmget antes do fork e anexada por cada filho com shmat.

**Dificuldades encontradas.**

Cada filho precisa anexar a memória por conta própria, o fork não faz isso sozinho. E o segmento tem que ser removido no fim com shmctl, senão fica ocupando memória do sistema.

## Questão 2

**Justificativa da quantidade de processos adotada.**

Cinco processos filhos. 10000 divide exatamente por 5, 2000 elementos por processo.

**Estrutura de dados utilizada para armazenar os resultados.**

Um vetor de 5 valores long em memória compartilhada, uma posição por soma parcial.

**Para onde os resultados foram enviados.**

Memória compartilhada. O pai soma as 5 posições e divide por 10000 para obter a média.

**Dificuldades encontradas.**

Com esses valores a soma não chega a estourar int, mas usei long para não depender da faixa. E o pai só pode ler a memória depois que todos os filhos terminarem, garantido com wait em loop.
