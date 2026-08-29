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

Programa: `questao1.c`. O pai cria o pipe, faz `fork`, lê N inteiros do teclado e
escreve cada valor em `fd[1]`. O filho lê de `fd[0]` até receber EOF, calcula soma,
média, maior e menor, imprime os resultados e encerra com `exit(0)`. O pai fecha a
ponta de escrita e espera o filho com `waitpid`, exibindo o código de saída. Cada
mensagem impressa identifica o processo e o PID.

Execução com os valores 10, -3, 7, 0 e 42:

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

Observação com `ps`: durante a digitação, `ps -o pid,ppid,stat,cmd -C q1` mostra o
pai em `S+` (bloqueado na entrada padrão) e o filho em `S` (bloqueado no `read` do
pipe). Depois que o pai fecha `fd[1]`, o filho passa para `R` enquanto calcula e o
pai fica em `S` dentro do `waitpid`. Entre o `exit` do filho e o retorno do
`waitpid` o filho aparece como `Z` (zumbi).

### Respostas

**1. Em que momento ocorre o bloqueio do processo durante a leitura ou escrita no pipe.**

O `read` bloqueia quando o buffer do pipe está vazio e ainda existe pelo menos uma
ponta de escrita aberta. O `write` bloqueia quando o buffer está cheio e nenhum
leitor consumiu espaço. No programa, o filho fica bloqueado no `read` desde o
`fork` até o pai enviar o primeiro inteiro, e volta a bloquear entre uma digitação
e outra. O pai só bloquearia no `write` se o volume enviado ultrapassasse a
capacidade do buffer, o que não ocorre com poucos inteiros.

**2. O que acontece se o processo pai não fechar a extremidade de leitura do pipe.**

O pipe continua com um leitor registrado pelo kernel enquanto o pai existir. O
descritor fica vazando até o término do processo e o pai passa a poder ler de volta
o que ele mesmo escreveu, consumindo dados destinados ao filho. O descritor
desperdiçado tem um segundo efeito: o pai deixa de receber `SIGPIPE` ou `EPIPE` caso
o filho morra antes da hora, porque a contagem de leitores nunca chega a zero. A
falha do consumidor deixa de ser sinalizada e o pai pode bloquear no `write` quando
o buffer encher.

**3. O que acontece se o processo filho não fechar a extremidade de escrita.**

O filho mantém um escritor aberto para o próprio pipe. O EOF só é entregue ao
`read` quando todas as pontas de escrita são fechadas, então, mesmo depois de o pai
fechar `fd[1]`, o `read` do filho não retorna 0. O filho fica bloqueado
indefinidamente esperando dados que ninguém enviará, e o pai fica preso no
`waitpid`. É um impasse causado apenas por descritor não fechado.

**4. Como o tamanho do buffer do pipe pode influenciar o comportamento do algoritmo.**

O buffer tem capacidade finita, 65536 bytes por padrão no Linux. Enquanto o total
escrito couber nesse limite, o pai escreve tudo sem bloquear e o filho só precisa
ler depois. Quando o volume excede a capacidade, o `write` bloqueia e a execução
passa a alternar entre escritor e leitor, o que torna o programa dependente de o
consumidor estar ativo. Um algoritmo que escrevesse todos os dados antes de
qualquer leitura funcionaria com N pequeno e travaria com N grande, portanto o
tamanho do buffer define o ponto em que o erro de projeto aparece.

**5. Qual a diferença estrutural entre resolver o problema com dois processos e resolver com uma simples chamada de função.**

A chamada de função executa no mesmo processo, no mesmo espaço de endereçamento e
na mesma pilha. Os dados são acessados por endereço e o retorno é imediato, sem
intervenção do kernel. Com dois processos existem duas imagens de memória
independentes e dois PCBs, e nada é acessível diretamente de um lado para o outro.
A transferência exige cópia da memória do escritor para o buffer do kernel e depois
para a memória do leitor, somada ao custo de `fork`, escalonamento e trocas de
contexto. O ganho é isolamento de falhas e possibilidade de execução concorrente,
o custo é latência e sincronização explícita.

**6. De que forma o PCB participa da coordenação desse fluxo de execução.**

Cada processo tem um PCB com PID, PPID, estado, registradores salvos, mapeamento de
memória e a tabela de descritores de arquivo. O `fork` copia o PCB do pai, e a cópia
da tabela de descritores faz com que as entradas do filho apontem para as mesmas
estruturas de arquivo aberto do kernel, portanto para o mesmo pipe. Quando o `read`
não tem dados, o kernel muda o estado no PCB para bloqueado e coloca o processo na
fila de espera do pipe. Quando o pai escreve, o kernel retira o filho dessa fila e
o marca como pronto. No fim, o PCB do filho permanece como zumbi guardando o código
de saída até o pai recolhê-lo com `waitpid`.

## Questão 2

Programa: `questao2.c`. O processo inicial cria os dois pipes e executa dois `fork`.
P1 abre o arquivo e envia cada inteiro pelo primeiro pipe. P2 lê tudo, ordena com
`qsort`, descarta repetições e envia os valores únicos pelo segundo pipe. P3 conta
o que recebeu e imprime o total. Cada processo fecha as pontas que não usa. O vetor
de P2 começa com 1024 posições e dobra com `realloc` sempre que enche, de modo que
não existe limite fixo de quantidade e nenhum valor é descartado em silêncio.

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

Observação com `ps`: com `ps -o pid,ppid,stat,wchan,cmd -C q2` no início da
execução, P2 e P3 aparecem em `S` bloqueados em `pipe_read`, enquanto P1 lê o
arquivo. Ao fechar o primeiro pipe, P2 sai do bloqueio, ordena em `R` e escreve.
P3 continua bloqueado até P2 fechar a ponta de escrita do segundo pipe. Os dois
filhos passam por `Z` antes de cada `waitpid` retornar.

### Respostas

**1. Por que é necessário fechar descritores que não serão utilizados em cada processo.**

O kernel entrega EOF ao leitor somente quando o número de descritores de escrita do
pipe chega a zero. Como o `fork` duplica todos os descritores, um processo que não
escreve ainda assim conta como escritor se não fechar a ponta. Bastaria P3 manter
`p1[1]` aberto para que P2 nunca recebesse EOF e o pipeline travasse. Fechar também
libera entradas da tabela de descritores, que é limitada por processo, e permite ao
kernel destruir o pipe quando ele deixa de ser referenciado.

**2. O que ocorre se um processo intermediário terminar antes de consumir todos os dados.**

Ao terminar, o kernel fecha os descritores de P2, e o primeiro pipe fica sem
leitores. A próxima escrita de P1 gera `SIGPIPE`, que por padrão encerra o processo,
ou retorna `-1` com `errno` igual a `EPIPE` se o sinal estiver ignorado. Os bytes
ainda presentes no buffer são descartados junto com o pipe. P3 recebe EOF no segundo
pipe assim que P2 morre e imprime uma contagem parcial, sem qualquer indicação de
erro. O resultado fica incorreto de forma silenciosa, o que reforça a necessidade de
verificar o retorno de `write` e o status devolvido por `waitpid`.

**3. Como o bloqueio natural do read contribui para sincronização entre processos.**

O `read` bloqueado funciona como espera por evento gerenciada pelo kernel. O
consumidor só volta a executar quando existe dado disponível, e o produtor só para
quando o buffer enche. Isso implementa o padrão produtor e consumidor sem semáforo,
variável de condição ou espera ocupada. A ordem dos bytes no pipe é preservada, de
modo que a sequência de leitura acompanha a sequência de escrita, e o EOF sinaliza o
fim do fluxo sem precisar de mensagem de controle no protocolo.

**4. Como esse modelo reproduz o funcionamento interno de um comando encadeado no shell.**

O programa é o equivalente estrutural de `cat dados.txt | sort -u | wc -l`. O shell
cria um pipe por conexão, faz um `fork` por comando, usa `dup2` para ligar a saída
padrão de um processo à ponta de escrita e a entrada padrão do seguinte à ponta de
leitura, fecha os descritores restantes e chama `exec`. A diferença é que aqui não
há `dup2` nem `exec`, pois a lógica de cada estágio está no próprio código e a
comunicação usa os descritores diretamente. A topologia, o número de processos, o
fechamento das pontas e o encerramento em cascata por EOF são os mesmos.

**5. Qual é a diferença estrutural entre resolver esse problema com processos e resolver utilizando threads.**

Processos têm espaços de endereçamento separados, portanto os números precisam ser
copiados da memória de um para o buffer do kernel e daí para a memória do outro.
Threads do mesmo processo compartilham o heap e os descritores, logo o vetor de
inteiros seria acessado diretamente, sem cópia e sem pipe, com custo de criação e de
troca de contexto bem menor. Em compensação, a sincronização passa a ser
responsabilidade do programador, com mutex e variável de condição, e o acesso
concorrente admite condição de corrida. O isolamento também desaparece: uma falha de
segmentação em uma thread derruba o processo inteiro, enquanto a queda de P2 não
afeta a memória de P1 nem de P3.

**6. Como os PCBs de cada processo permanecem independentes mesmo estando conectados por pipes.**

O pipe conecta descritores, não memória. Cada processo mantém PCB próprio com PID,
PPID, estado, contexto de registradores, tabela de páginas, credenciais e tabela de
descritores. O que existe em comum são apenas as entradas da tabela global de
arquivos abertos do kernel referenciadas por essas tabelas, com contadores de
referência. Um processo bloqueado em `read` tem apenas o próprio PCB marcado como
bloqueado, sem alterar o estado dos demais, e o escalonador continua tratando os
três como entidades separadas. Por isso P3 pode estar bloqueado enquanto P2 executa
e P1 já terminou.
