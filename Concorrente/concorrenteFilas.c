#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include "../funcoes.h" // definiu-se as funcoes a serem usadas em um outro arquivo
#include "../timer.h" // funcoes para calculo de tempo

#define TAM 1000000 // tamanho maximo para a fila de cada thread, usa-se um valor alto para evitar retencao por fila cheia
#define MAX_THREADS 8

// criacao de uma struct para facilitar a passagem de informacao sobre intervalos (inicio e fim)
typedef struct _Intervalo {
    long double inicio;
    long double fim;
} Intervalo;

// variaveis globais
pthread_mutex_t mutex;
pthread_cond_t cond_ins, cond_rem;
long double (*funcao)(long double); // funcao global para facilitar a passagem da funcao a ser integrada
long double *resultados; // vetor para armazenamento dos resultados obtidos por cada thread
long double erroPermitido;
Intervalo buffer[MAX_THREADS][TAM]; // buffer contendo as filas para cada thread em linhas correspondentes ao id delas (optou-se por uma matriz estatica com linhas = MAX_THREADS para simplificacao)
// usa-se uma fila para garantir consistencia na ordem que as areas dos intervalos sao calculadas
int contadorBalanceamento[MAX_THREADS] = {0, 0, 0, 0, 0, 0, 0, 0}; // vetor para contar quantos intervalos cada thread processou
int *contadoresBuffers; // vetor para cuidar do quao cheio cada fila esta
int *out; // vetor para manter controle das variaveis de saida das filas
int *in; // vetor para manter controle das variaveis de entrada das filas
int threadsTrabalhando = 0;
int nthreads;

// funcao para averiguar se alguma fila ainda tem intervalos a serem processados
int algumIntervaloEmBuffer() {
    for(int i = 0; i < nthreads; i++) {
        if(contadoresBuffers[i] != 0) return 1;
    }
    return 0;
}

// funcao para remocao de intervalo de um buffer
Intervalo removeBuffer(int id) {
    Intervalo retorno;

    pthread_mutex_lock(&mutex);
    threadsTrabalhando++;
    while(contadoresBuffers[id] == 0) { // caso buffer esteja vazio
        threadsTrabalhando--;
        if(threadsTrabalhando == 0 && !algumIntervaloEmBuffer()) {
            // caso nao tenham mais threads trabalhando e nao tenha mais nenhum intervalo em qualquer buffer
            // retorna um intervalo invalido (ponto), libera threads que talvez estejam presas no aguardo de remocao, e libera mutex
            retorno.inicio = 0;
            retorno.fim = 0;
            pthread_cond_broadcast(&cond_rem);
            pthread_mutex_unlock(&mutex);
            return retorno;
        }
        pthread_cond_wait(&cond_rem, &mutex);
        threadsTrabalhando++;
    }

    retorno = buffer[id][out[id]];
    contadoresBuffers[id]--;
    out[id] = (out[id] + 1) % TAM;
    pthread_cond_broadcast(&cond_ins);
    pthread_mutex_unlock(&mutex);

    return retorno;
}

// funcao para insercao de intervalo em um buffer
void insereBuffer(Intervalo intervalo, int id) {
    pthread_mutex_lock(&mutex);
    while(contadoresBuffers[id] + 1 >= TAM) { // caso buffer ja esteja cheio, aguarda liberar um espaco
        pthread_cond_wait(&cond_ins, &mutex);
    }

    buffer[id][in[id]] = intervalo;
    contadoresBuffers[id]++;
    in[id] = (in[id] + 1) % TAM;
    pthread_cond_broadcast(&cond_rem);
    pthread_mutex_unlock(&mutex);
}

// funcao para calculo do ponto medio entre dois pontos a e b
long double pegarPontoMedio(long double inicio, long double fim) {
    return ((inicio + fim)/2);
}

// funcao com o metodo do retangulo
void integracaoRetangular(Intervalo intervalo, int id) {
    long double inicio, meio, fim;
    long double alturaTotal, alturaEsq, alturaDir; // alturas calculadas a partir dos pontos medios dos intervalos
    long double areaTotal, areaEsq, areaDir; // areas dos retangulos
    long double erro;
    int tid;

    contadorBalanceamento[id]++;

    inicio = intervalo.inicio;
    fim = intervalo.fim;

    meio = pegarPontoMedio(inicio, fim);
    alturaTotal = funcao(meio); // altura referente ao retangulo maior
    alturaEsq = funcao(pegarPontoMedio(inicio, meio));
    alturaDir = funcao(pegarPontoMedio(meio, fim));

    areaTotal = (fim - inicio) * alturaTotal;
    areaEsq = (meio - inicio) * alturaEsq;
    areaDir = (fim - meio) * alturaDir;

    // calculo do modulo da diferenca entre a area do retangulo maior e a soma das areas dos retangulos menores
    erro = fabsl(areaTotal - (areaEsq + areaDir));

    if(erro > erroPermitido) {    // caso erro seja maior do que o permitido
        Intervalo temp;
        temp.inicio = inicio;
        temp.fim = meio;
        integracaoRetangular(temp, id); // thread pega o intervalo da "esquerda" e ela mesma processa
        temp.inicio = meio;
        temp.fim = fim;
        tid = (id + 1) % nthreads;
        insereBuffer(temp, tid); // coloca o intervalo da "direita" na fila da thread seguinte
    }
    else resultados[id] += areaTotal; // caso seja um erro aceitavel, soma essa area aos seus resultados;
}

// funcao chamada pelas threads para calcular a aproximacao da integral
void *calculaIntegral(void *args) {
    int id = *(int *) args;
    Intervalo intervalo;

    while(1) {
        pthread_mutex_lock(&mutex);
        if(!algumIntervaloEmBuffer() && threadsTrabalhando == 0) {
            // a condicao de parada das threads eh nao ter mais intervalos a serem processados em NENHUM buffer (outras threads nao tem o que retirar para processar)
            // E não haver NENHUMA thread trabalhando naquele momento (caso tenha, ela poderia inserir algo no buffer posteriormente)
            pthread_cond_broadcast(&cond_rem); // necessario para nao deixar threads presas no wait infinitamente
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        intervalo = removeBuffer(id); // pega algum intervalo do buffer
        if(intervalo.inicio == 0 && intervalo.fim == 0) { // caso seja um intervalo invalido, sai do loop
            break;
        }
        integracaoRetangular(intervalo, id); // processa o intervalo retirado do buffer

        pthread_mutex_lock(&mutex);
        threadsTrabalhando--;
        pthread_mutex_unlock(&mutex);
    }

    free(args);
    pthread_exit(NULL);
}

int main (int argc, char *argv[]) {
    pthread_t *threads;
    Intervalo entrada;
    long double (*funcoes[7])(long double) = {&f1, &f2, &f3, &f4, &f5, &f6, &f7}; // monta-se o vetor de funcoes com as opcoes
    long double integral; // variavel para armazenar valor calculado da integral da funcao
    double tempoInicio, tempoFim, tempoInicializacao, tempoProcessamento, tempoFinalizacao; //variaveis para medicoes de tempo
    int i, *tid;
    char *escolha;

    // forcar usuario a escolher um intervalo, erro, numero de threads e a funcao
    if(argc < 6) {
        printf("<inicio do intervalo> <fim do intervalo> <erro permitido> <numero de threads> <funcao a ser integrada>\n");
        printf("(f1) f(x) = 1 + x\n(f2) f(x) = √(1 − xˆ2), −1 < x < 1\n(f3) f(x) = √(1 + xˆ4)\n(f4) f(x) = sen(xˆ2)\n(f5) f(x) = cos(eˆ(-x))\n(f6) f(x) = cos(eˆ(-x)) * x\n(f7) f(x) = cos(eˆ(-x)) * ((0.005 * xˆ3) + 1)\n");
        exit(-1);
    }

    entrada.inicio = strtold(argv[1], NULL);
    entrada.fim = strtold(argv[2], NULL);
    erroPermitido = strtold(argv[3], NULL);
    nthreads = atoi(argv[4]);
    escolha = argv[5];

    // condicional para forcar o usuario a escolher uma funcao valida
    if(escolha[0] != 'f' || escolha[1] > '7' || escolha[1] < '1') {
        printf("Insira uma funcao valida\n");
        exit(-1);
    }

    // operacao para atribuir a funcao global a funcao escolhida
    funcao = funcoes[(escolha[1] - '0') - 1];

    GET_TIME(tempoInicio);

    // alocacoes
    threads = (pthread_t *) malloc(sizeof(pthread_t) * nthreads); if(!threads) { printf("Erro ao alocar threads\n"); exit(-1); }
    resultados = (long double *) malloc(sizeof(long double) * nthreads); if(!resultados) { printf("Erro ao alocar resultados\n"); exit(-1); }
    contadoresBuffers = (int *) malloc(sizeof(int) * nthreads); if(!contadoresBuffers) { printf("Erro ao alocar contadoresBuffers\n"); exit(-1); }
    out = (int *) malloc(sizeof(int) * nthreads); if(!out) { printf("Erro ao alocar out\n"); exit(-1); }
    in = (int *) malloc(sizeof(int) * nthreads); if(!in) { printf("Erro ao alocar in\n"); exit(-1); }

    // inicializar vetor de contadores e controle de entrada e saida do buffer com 0
    for(i = 0; i < nthreads; i++) {
        contadoresBuffers[i] = 0;
        out[i] = 0;
        in[i] = 0;
    }

    // inicializar mutex e condicoes
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_ins, NULL);
    pthread_cond_init(&cond_rem, NULL);

    // colocar a entrada do programa (intervalo da integral) no buffer
    // escolheu-se colocar no buffer da matriz de id 0 (ainda nao criada), essa condicao implica em essa matriz sempre ser a primeira a iniciar o processamento
    buffer[0][in[0]] = entrada;
    in[0]++;
    contadoresBuffers[0]++;

    GET_TIME(tempoFim);

    tempoInicializacao = tempoFim - tempoInicio;

    GET_TIME(tempoInicio);

    //Criar threads
    for(i = 0; i < nthreads; i++) {
        tid = malloc(sizeof(int)); if(tid == NULL) return -1;
        *tid = i;
        pthread_create(&threads[i], NULL, calculaIntegral, (void *) tid);
    }

    //Fazer main esperar
    for(i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    // fazer main somar resultados obtidos por cada thread
    integral = 0;
    for(i = 0; i < nthreads; i++) {
        integral += resultados[i];
    }
    GET_TIME(tempoFim);

    tempoProcessamento = tempoFim - tempoInicio;

    GET_TIME(tempoInicio);

    // destruir mutex e condicoes e liberar espacos nao mais usados
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_ins);
    pthread_cond_destroy(&cond_rem);

    free(threads);
    free(contadoresBuffers);
    free(resultados);

    GET_TIME(tempoFim);
    tempoFinalizacao = tempoFim - tempoInicio;

    printf("Valor aproximado da Integral da funcao: %.15Lf\n", integral);
    printf("Tempo de inicializacao: %lf\n", tempoInicializacao);
    printf("Tempo de processamento: %lf\n", tempoProcessamento);
    printf("Tempo de finalizacao: %lf\n", tempoFinalizacao);

    printf("Distribuicao de tarefas entre as threads: ")
    for(i = 0; i < nthreads; i++) {
        printf("%d ", contadorBalanceamento[i]);
    }
    printf("\n");

    pthread_exit(NULL);

    return 0;
}
