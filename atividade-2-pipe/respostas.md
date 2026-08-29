# Atividade 2: Comunicação entre Processos com Pipe

Nome:

## Questão 1

**1. Em que momento ocorre o bloqueio do processo durante a leitura ou escrita no pipe.**

O read bloqueia com o buffer vazio e alguma ponta de escrita ainda aberta. O write
bloqueia com o buffer cheio.

**2. O que acontece se o processo pai não fechar a extremidade de leitura do pipe.**

O pipe nunca fica sem leitor, então o pai não recebe SIGPIPE se o filho morrer e o
descritor fica aberto até o fim do processo.

**3. O que acontece se o processo filho não fechar a extremidade de escrita.**

O read nunca recebe EOF, porque o próprio filho segura uma ponta de escrita. Ele trava
e o pai fica preso no waitpid.

**4. Como o tamanho do buffer do pipe pode influenciar o comportamento do algoritmo.**

O buffer tem 65536 bytes no Linux. Enquanto os dados couberem o pai escreve sem parar,
acima disso o write bloqueia até o filho consumir.

**5. Qual a diferença estrutural entre resolver o problema com dois processos e resolver com uma simples chamada de função.**

A função usa a mesma memória e retorna direto. Dois processos têm memórias separadas,
então os dados são copiados através do kernel.

**6. De que forma o PCB participa da coordenação desse fluxo de execução.**

O fork copia a tabela de descritores do PCB, o que faz pai e filho usarem o mesmo pipe.
O kernel usa o PCB para bloquear o processo no read e para guardar o código de saída
até o waitpid.

## Questão 2

**1. Por que é necessário fechar descritores que não serão utilizados em cada processo.**

O EOF só chega quando todas as pontas de escrita estão fechadas. Um descritor esquecido
em qualquer processo trava o leitor.

**2. O que ocorre se um processo intermediário terminar antes de consumir todos os dados.**

O pipe fica sem leitor e o processo anterior recebe SIGPIPE na escrita seguinte, ou
EPIPE se o sinal estiver ignorado. O último processo imprime uma contagem parcial.

**3. Como o bloqueio natural do read contribui para sincronização entre processos.**

O consumidor só executa quando há dado e o produtor para quando o buffer enche. Isso
sincroniza os processos sem semáforo.

**4. Como esse modelo reproduz o funcionamento interno de um comando encadeado no shell.**

É o mesmo que cat dados.txt | sort -u | wc -l. O shell cria um pipe por conexão, um fork
por comando, liga as pontas com dup2 e chama exec.

**5. Qual é a diferença estrutural entre resolver esse problema com processos e resolver utilizando threads.**

Threads compartilham memória e dispensam o pipe, mas exigem mutex e uma falha derruba o
processo inteiro. Processos copiam pelo kernel e mantêm as memórias isoladas.

**6. Como os PCBs de cada processo permanecem independentes mesmo estando conectados por pipes.**

O pipe liga descritores, não memória. Cada processo tem PCB próprio e só as entradas da
tabela de arquivos abertos do kernel são comuns.
