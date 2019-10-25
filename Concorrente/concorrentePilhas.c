// Essa versao nao garante consistencia na ordem de soma dos resultados das areas

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include "../funcoes.h" // definiu-se as funcoes a serem usadas em um outro arquivo
#include "../timer.h" // funcoes para calculo de tempo

#define TAM 100000 // tamanho maximo para a pilha de tarefas usa-se um valor alto para evitar retencao por pilha
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
Intervalo buffer[TAM]; // buffer de uma pilha para armazenar os intervalos a serem processados
int contadorBalanceamento[MAX_THREADS] = {0, 0, 0, 0, 0, 0, 0, 0}; // vetor para contar quantos intervalos cada thread processou
int proximo = 0; // variavel para manter controle da proxima posicao na pilha
int threadsTrabalhando = 0;
int nthreads;

// funcao para remocao de intervalo de um buffer
Intervalo removeBuffer() {
    Intervalo retorno;

    pthread_mutex_lock(&mutex);
    threadsTrabalhando++;
    while(proximo == 0) { // caso pilha esteja vazia
        threadsTrabalhando--;
        if(threadsTrabalhando == 0) { // e nao tenha nenhuma thread trabalhando (ou seja, sem possibilidade de inserir algo na pilha)
            // retorna-se um intervalo invalido, libera as threads presas no wait de remocao e libera o mutex
            retorno.inicio = 0;
            retorno.fim = 0;
            pthread_cond_broadcast(&cond_rem);
            pthread_mutex_unlock(&mutex);
            return retorno;
        }
        pthread_cond_wait(&cond_rem, &mutex);
        threadsTrabalhando++;
    }

    retorno = buffer[proximo - 1];
    proximo--;
    pthread_cond_broadcast(&cond_ins);
    pthread_mutex_unlock(&mutex);

    return retorno;
}

// funcao para insercao de intervalo em um buffer
void insereBuffer(Intervalo intervalo) {
    pthread_mutex_lock(&mutex);
    while(proximo + 1 >= TAM) { // caso pilha esteja cheia, aguarda liberar um espaco
        pthread_cond_wait(&cond_ins, &mutex);
    }

    buffer[proximo] = intervalo;
    proximo++;
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

    if(erro > erroPermitido) { // caso erro seja maior do que o permitido
        Intervalo temp;
        temp.inicio = inicio;
        temp.fim = meio;
        integracaoRetangular(temp, id); // thread pega o intervalo da "esquerda" e ela mesma processa
        temp.inicio = meio;
        temp.fim = fim;
        insereBuffer(temp); // coloca o intervalo da "direita" na pilha
    }

    else resultados[id] += areaTotal; // caso seja um erro aceitavel, soma essa area aos seus resultados
}

// funcao chamada pelas threads para calcular a aproximacao da integral
void *calculaIntegral(void *args) {
    int id = *(int *) args;
    Intervalo intervalo;

    while(1) {
        pthread_mutex_lock(&mutex);
        if(proximo == 0 && threadsTrabalhando == 0) {
            // a condicao de parada das threads consiste em nao ter mais intervalos pendentes na pilha
            // E nao ter mais threads trabalhando (ou seja, nao tem a possibilidade de algo ser colocado na pilha)
            pthread_cond_broadcast(&cond_rem); // liberar threads presas no wait de remocao (do contrario, elas ficariam presas ali)
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        intervalo = removeBuffer(); // pega um intervalo da pilha
        if(intervalo.inicio == 0 && intervalo.fim == 0) { // se for invalido, sai do loop
            break;
        }
        integracaoRetangular(intervalo, id); // processa esse intervalo

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
    double tempoInicio, tempoFim, tempoInicializacao, tempoProcessamento, tempoFinalizacao;
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

    //alocacoes
    threads = (pthread_t *) malloc(sizeof(pthread_t) * nthreads);
    resultados = (long double *) malloc(sizeof(long double) * nthreads);

    // inicializar mutex e condicoes
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_ins, NULL);
    pthread_cond_init(&cond_rem, NULL);

    // colocar intervalo de entrada na pilha
    buffer[proximo] = entrada;
    proximo++;

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

    // fazer main somar resultados
    integral = 0;
    for(i = 0; i < nthreads; i++) {
        integral += resultados[i];
    }
    GET_TIME(tempoFim);

    tempoProcessamento = tempoFim - tempoInicio;

    // destruir mutex e condicoes e liberar espacos nao mais usados
    GET_TIME(tempoInicio);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_ins);
    pthread_cond_destroy(&cond_rem);

    free(threads);
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
