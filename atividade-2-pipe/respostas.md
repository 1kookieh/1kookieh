# Atividade 2: Comunicação entre Processos com Pipe

Nome:

## Questão 1

**1. Em que momento ocorre o bloqueio do processo durante a leitura ou escrita no pipe.**

O read bloqueia quando o buffer está vazio e ainda existe alguma ponta de escrita
aberta. O write bloqueia quando o buffer está cheio.

**2. O que acontece se o processo pai não fechar a extremidade de leitura do pipe.**

O pipe fica sempre com um leitor. O pai pode consumir os dados destinados ao filho,
deixa de receber SIGPIPE se o filho morrer antes da hora e mantém o descritor aberto
até o fim do processo.

**3. O que acontece se o processo filho não fechar a extremidade de escrita.**

O read do filho nunca recebe EOF, porque ele mesmo mantém uma ponta de escrita
aberta. Ele trava e o pai fica preso no waitpid.

**4. Como o tamanho do buffer do pipe pode influenciar o comportamento do algoritmo.**

O buffer tem 65536 bytes no Linux. Se os dados couberem, o pai escreve tudo sem
parar. Acima disso o write bloqueia até o filho consumir, e um programa que escrevesse
tudo antes de ler entraria em impasse.

**5. Qual a diferença estrutural entre resolver o problema com dois processos e resolver com uma simples chamada de função.**

A função usa a mesma memória e retorna direto. Dois processos têm memórias e PCBs
separados, então cada valor é copiado para o kernel e de lá para o outro processo,
com custo de fork e de troca de contexto.

**6. De que forma o PCB participa da coordenação desse fluxo de execução.**

O PCB guarda o estado do processo e a tabela de descritores, que o fork copia, o que
faz pai e filho usarem o mesmo pipe. Sem dados no read, o kernel marca o PCB como
bloqueado e o devolve para pronto quando o pai escreve. O PCB do filho fica zumbi até
o waitpid recolher o código de saída.

## Questão 2

**1. Por que é necessário fechar descritores que não serão utilizados em cada processo.**

O EOF só é entregue quando todas as pontas de escrita estão fechadas. Um descritor
esquecido em qualquer processo mantém o pipe aberto e trava o leitor.

**2. O que ocorre se um processo intermediário terminar antes de consumir todos os dados.**

O pipe fica sem leitor e o processo anterior recebe SIGPIPE na escrita seguinte, ou
EPIPE se o sinal estiver ignorado. Os dados que estavam no buffer se perdem e o último
processo imprime uma contagem parcial.

**3. Como o bloqueio natural do read contribui para sincronização entre processos.**

O consumidor só executa quando existe dado e o produtor para quando o buffer enche.
Isso sincroniza os processos sem semáforo e sem espera ocupada.

**4. Como esse modelo reproduz o funcionamento interno de um comando encadeado no shell.**

É o mesmo que cat dados.txt | sort -u | wc -l. O shell cria um pipe por conexão e um
fork por comando, liga as pontas à entrada e à saída padrão com dup2 e chama exec.

**5. Qual é a diferença estrutural entre resolver esse problema com processos e resolver utilizando threads.**

Threads compartilham memória e dispensam pipe e cópia, mas exigem mutex e uma falha
derruba o processo inteiro. Processos copiam os dados através do kernel e mantêm as
memórias isoladas.

**6. Como os PCBs de cada processo permanecem independentes mesmo estando conectados por pipes.**

O pipe liga descritores, não memória. Cada processo tem PCB próprio, com PID, estado
e tabela de descritores, e só as entradas da tabela de arquivos abertos do kernel são
comuns aos três.
