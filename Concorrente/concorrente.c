#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include "funcoes.h" // definiu-se as funcoes a serem usadas em um outro arquivo

#define TAM 10000

typedef struct _Intervalo {
    long double inicio;
    long double fim;
} Intervalo;

pthread_mutex_t mutex;
pthread_cond_t cond_ins, cond_rem;
long double (*funcao)(long double);
long double *resultados;
long double erroPermitido;
Intervalo buffer[TAM];
int proximo = 0;
int threadsTrabalhando = 0;
int nthreads;

Intervalo removeBuffer() {
    Intervalo retorno;

    pthread_mutex_lock(&mutex);
    threadsTrabalhando++;
    while(proximo == 0) {
        threadsTrabalhando--;
        if(threadsTrabalhando == 0) {
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

void insereBuffer(Intervalo intervalo) {
    pthread_mutex_lock(&mutex);
    while(proximo + 1 >= TAM) {
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

void fazMagia(Intervalo intervalo, int id) {
    long double inicio, meio, fim;
    long double alturaTotal, alturaEsq, alturaDir; // alturas calculadas a partir dos pontos medios dos intervalos
    long double areaTotal, areaEsq, areaDir; // areas dos retangulos
    long double erro;

    inicio = intervalo.inicio;
    fim = intervalo.fim;

    meio = pegarPontoMedio(inicio, fim);
    alturaTotal = funcao(meio); // altura referente ao retangulo maior
    alturaEsq = funcao(pegarPontoMedio(inicio, meio));
    alturaDir = funcao(pegarPontoMedio(meio, fim));

    areaTotal = (fim - inicio) * alturaTotal;
    areaEsq = (meio - inicio) * alturaEsq;
    areaDir = (fim - meio) * alturaDir;

    erro = fabsl(areaTotal - (areaEsq + areaDir));

    if(erro > erroPermitido) {
        Intervalo temp;
        temp.inicio = inicio;
        temp.fim = meio;
        insereBuffer(temp);
        temp.inicio = meio;
        temp.fim = fim;
        insereBuffer(temp);
    }

    else resultados[id] += areaTotal;
}

void *seiLa(void *args) {
    int id = *(int *) args;
    Intervalo intervalo;

    while(1) {
        pthread_mutex_lock(&mutex);
        if(proximo == 0 && threadsTrabalhando == 0) {
            pthread_cond_broadcast(&cond_rem);
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        intervalo = removeBuffer();
        if(intervalo.inicio == 0 && intervalo.fim == 0) {
            break;
        }
        fazMagia(intervalo, id);

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
    long double (*funcoes[7])(long double) = {&f1, &f2, &f3, &f4, &f5, &f6, &f7};
    long double integral; // variavel para armazenar valor calculado da integral da funcao
    //long double inicio, fim;
    int i, *tid;
    char escolha[2];

    if(argc < 5) {
        printf("<inicio do intervalo> <fim do intervalo> <erro permitido> <numero de threads>\n");
        exit(-1);
    }

    entrada.inicio = strtold(argv[1], NULL);
    entrada.fim = strtold(argv[2], NULL);
    erroPermitido = strtold(argv[3], NULL);
    nthreads = atoi(argv[4]);

    printf("Entrada: %Lf %Lf\n", entrada.inicio, entrada.fim);

    // loop para forcar usuario a escolher uma funcao dentre as disponiveis
    while (1) {
        printf("Escolha a funcao a ser integrada:\n");
        printf("(f1) f(x) = 1 + x\n(f2) f(x) = √(1 − xˆ2), −1 < x < 1\n(f3) f(x) = √(1 + xˆ4)\n(f4) f(x) = sen(xˆ2)\n(f5) f(x) = cos(eˆ(-x))\n(f6) f(x) = cos(eˆ(-x)) * x\n(f7) f(x) = f(x) = cos(eˆ(-x)) * ((0.005 * xˆ3) + 1)\n");

        scanf("%s", escolha);

        // checagens de validacao da escolha do usuario
        if (escolha[0] != 'f') {
            continue;
        }
        if (escolha[1] > '7' || escolha[1] < '1') {
            continue;
        }

        funcao = funcoes[(escolha[1] - '0') - 1];
        break;
    }

    threads = (pthread_t *) malloc(sizeof(pthread_t) * nthreads);
    resultados = (long double *) malloc(sizeof(long double) * nthreads);

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_ins, NULL);
    pthread_cond_init(&cond_rem, NULL);

    buffer[proximo] = entrada;
    proximo++;

    printf("Buffer: %Lf %Lf\n", buffer[0].inicio, buffer[0].fim);
    //Criar threads
    for(i = 0; i < nthreads; i++) {
        tid = malloc(sizeof(int)); if(tid == NULL) return -1;
        *tid = i;
        pthread_create(&threads[i], NULL, seiLa, (void *) tid);
        printf("Criei a thread %d\n", *tid);
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

    printf("Valor aproximado da Integral da funcao: %Lf\n", integral);
    // rezar pra nao ficar muito ruim e ter que arranjar alguma forma de ordenar a soma dos floats;

    //dar free no necessario

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_ins);
    pthread_cond_destroy(&cond_rem);

    free(threads);
    free(resultados);

    pthread_exit(NULL);

    return 0;
}
