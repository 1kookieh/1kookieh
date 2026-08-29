# Atividade 2: Comunicação entre Processos com Pipe

Arquivos: `questao1.c`, `questao2.c` e `dados.txt` (arquivo de teste).

Compilação e execução:

```
gcc -Wall -Wextra -o q1 questao1.c
gcc -Wall -Wextra -o q2 questao2.c
./q1
./q2 dados.txt
```

## Questão 1

O pai cria o pipe, faz `fork` e lê N inteiros do teclado, escrevendo cada um em
`fd[1]`. O filho lê de `fd[0]` até o EOF, calcula soma, média, maior e menor,
imprime e sai com `exit(0)`. O pai fecha a ponta de escrita e recolhe o filho com
`waitpid`. Cada mensagem mostra o processo e o PID.

Execução com 10, -3, 7, 0 e 42:

```
[PAI    pid=2135] filho criado com pid=2136
[FILHO  pid=2136 ppid=2135] aguardando dados no pipe
[FILHO  pid=2136] recebidos: 5
[FILHO  pid=2136] soma  = 56
[FILHO  pid=2136] media = 11.20
[FILHO  pid=2136] maior = 42
[FILHO  pid=2136] menor = -3
[PAI    pid=2135] escrita encerrada, aguardando o filho
[PAI    pid=2135] filho terminou com codigo 0
```

Com `ps -o pid,ppid,stat,cmd -C q1` durante a digitação, o pai aparece em `S+`
esperando o teclado e o filho em `S` bloqueado no `read`. Depois que o pai fecha
`fd[1]`, o filho passa para `R` e o pai fica em `S` no `waitpid`. O filho aparece
como `Z` entre o `exit` e o retorno do `waitpid`.

### Respostas

**1. Em que momento ocorre o bloqueio do processo durante a leitura ou escrita no pipe.**

O `read` bloqueia com o buffer vazio, desde que ainda exista alguma ponta de escrita
aberta. O `write` bloqueia com o buffer cheio. Aqui o filho fica bloqueado no `read`
entre uma digitação e outra do pai.

**2. O que acontece se o processo pai não fechar a extremidade de leitura do pipe.**

O pipe nunca fica sem leitor. O pai pode acabar lendo de volta os dados destinados
ao filho, deixa de receber `SIGPIPE` se o filho morrer antes da hora e ainda vaza o
descritor até o fim do processo.

**3. O que acontece se o processo filho não fechar a extremidade de escrita.**

O EOF só chega ao `read` quando todas as pontas de escrita fecham, e o próprio filho
está segurando uma. Ele trava esperando dados que não virão, e o pai fica preso no
`waitpid`.

**4. Como o tamanho do buffer do pipe pode influenciar o comportamento do algoritmo.**

O buffer tem 65536 bytes por padrão no Linux. Enquanto o total couber nesse limite,
o pai escreve tudo sem bloquear. Acima disso o `write` bloqueia e a execução passa a
depender do consumidor: um programa que escrevesse tudo antes de ler funcionaria com
N pequeno e travaria com N grande.

**5. Qual a diferença estrutural entre resolver o problema com dois processos e resolver com uma simples chamada de função.**

A função roda no mesmo espaço de endereçamento, acessa os dados por endereço e
retorna direto. Com dois processos existem duas memórias e dois PCBs, e cada valor é
copiado para o kernel e de lá para o outro processo, mais o custo de `fork` e das
trocas de contexto. Ganha isolamento, perde desempenho.

**6. De que forma o PCB participa da coordenação desse fluxo de execução.**

O PCB guarda o estado do processo e a tabela de descritores. O `fork` copia essa
tabela, então pai e filho passam a apontar para o mesmo pipe. Sem dados no `read`, o
kernel marca o PCB como bloqueado e põe o processo na fila de espera do pipe; a
escrita do pai o traz de volta para pronto. No fim o PCB do filho fica como zumbi até
o `waitpid` recolher o código de saída.

## Questão 2

O processo inicial cria os dois pipes e faz dois `fork`. P1 lê o arquivo e envia os
inteiros pelo primeiro pipe. P2 recebe tudo, ordena com `qsort`, descarta as
repetições e manda os valores únicos pelo segundo pipe. P3 conta e imprime o total.
Cada processo fecha as pontas que não usa. O vetor de P2 começa com 1024 posições e
dobra com `realloc` quando enche, então nenhum valor é descartado em silêncio.

Execução com `dados.txt` contendo 5, 3, 9, 3, 1, 5, 7, 9, 2 e 7:

```
[P2 filtro   pid=2138] ordena e remove duplicados
[P2 filtro   pid=2138] 10 valores lidos, envio concluido
[P3 contador pid=2139] aguardando valores unicos
[P3 contador pid=2139] valores distintos: 6
[P1 leitor   pid=2137] arquivo: dados.txt | filhos: 2138 e 2139
[P1 leitor   pid=2137] 10 numeros enviados
[P1 leitor   pid=2137] P2 encerrou com codigo 0
[P1 leitor   pid=2137] P3 encerrou com codigo 0
```

Com `ps -o pid,ppid,stat,wchan,cmd -C q2` no início, P2 e P3 aparecem em `S` parados
em `pipe_read` enquanto P1 lê o arquivo. Quando P1 fecha o primeiro pipe, P2 sai do
bloqueio e escreve; P3 só destrava quando P2 fecha a ponta de escrita do segundo
pipe. Os dois filhos passam por `Z` antes de cada `waitpid` retornar.

### Respostas

**1. Por que é necessário fechar descritores que não serão utilizados em cada processo.**

O EOF só é entregue quando o contador de escritores do pipe zera. Como o `fork`
duplica tudo, um descritor esquecido em processo que nem escreve já segura o pipe
aberto. Se P3 não fechasse `p1[1]`, P2 nunca sairia do `read`.

**2. O que ocorre se um processo intermediário terminar antes de consumir todos os dados.**

O primeiro pipe fica sem leitor e P1 leva `SIGPIPE` na escrita seguinte, ou `EPIPE`
se o sinal estiver ignorado. O que estava no buffer se perde e P3 imprime uma
contagem parcial sem sinal de erro nenhum.

**3. Como o bloqueio natural do read contribui para sincronização entre processos.**

É espera por evento feita pelo kernel. O consumidor só executa quando há dado e o
produtor para quando o buffer enche, o que dá produtor e consumidor sem semáforo e
sem espera ocupada. A ordem dos bytes é preservada e o EOF marca o fim do fluxo.

**4. Como esse modelo reproduz o funcionamento interno de um comando encadeado no shell.**

Equivale a `cat dados.txt | sort -u | wc -l`. O shell cria um pipe por conexão e um
`fork` por comando, liga as pontas à entrada e à saída padrão com `dup2` e chama
`exec`. Aqui não há `dup2` nem `exec` porque a lógica está no próprio código, mas a
topologia e o encerramento em cascata por EOF são os mesmos.

**5. Qual é a diferença estrutural entre resolver esse problema com processos e resolver utilizando threads.**

Threads compartilham memória, então o vetor seria acessado direto, sem pipe e sem
cópia, com criação bem mais barata. Em troca a sincronização vira responsabilidade do
programador, com mutex, e uma falha em qualquer thread derruba o processo inteiro.
Com processos há cópia através do kernel, mas as memórias ficam isoladas.

**6. Como os PCBs de cada processo permanecem independentes mesmo estando conectados por pipes.**

O pipe liga descritores, não memória. Cada processo tem PCB próprio, com PID, estado,
tabela de páginas e tabela de descritores. Em comum ficam apenas as entradas da tabela
de arquivos abertos do kernel. Por isso P3 pode estar bloqueado enquanto P2 executa e
P1 já terminou.
