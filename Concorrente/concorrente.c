#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include "funcoes.h" // definiu-se as funcoes a serem usadas em um outro arquivo

#define TAM 10000

typedef struct Intervalo {
    int inicio;
    int fim;
};

pthread_mutex_t mutex;
pthread_cond_t cond_ins, cond_rem;
long double (*funcao)(long double);
long double *resultados;
long double erroPermitido;
Intervalo buffer[TAM];
int topo = 0;
int threadsTrabalhando = 0;
int nthreads;

Intervalo removeBuffer() {
    Intervalo retorno;

    pthread_mutex_lock(&mutex);
    threadsTrabalhando++;
    while(topo == 0) {
        pthread_cond_wait(&cond_rem, &mutex);
    }

    retorno = buffer[topo];
    topo--;
    pthread_cond_broadcast(&cond_ins);
    pthread_mutex_unlock(&mutex);

    return retorno;
}

void insereBuffer(Intervalo intervalo) {
    pthread_mutex_lock(&mutex)
    while(topo + 1 >= TAM) {
        pthread_cond_wait(&cond_ins, &mutex);
    }

    buffer[topo] = intervalo;
    topo++;
    pthread_cond_broadcast(&cond_rem);
    pthread_mutex_unlock(&mutex);
}

void fazMagia(Intervalo intervalo, int id) {
    Intervalo temp;
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
    int *id = (int *) args
    Intervalo intervalo;

    while(1) {
        if(topo == 0 && threadsTrabalhando == 0) break;

        intervalo = removeBuffer();
        fazMagia(intervalo, id);
        threadsTrabalhando--;
    }

    free(args);
    pthread_exit(NULL)
}

// funcao para calculo do ponto medio entre dois pontos a e b
long double pegarPontoMedio(long double inicio, long double fim) {
    return ((inicio + fim)/2);
}

funcao recursiva para integracao numerica usando regra do ponto medio/metodo do retangulo
long double integracaoRetangular(long double inicio, long double fim, long double (*funcao)(long double)) {
    long double meio;
    long double alturaTotal, alturaEsq, alturaDir; // alturas calculadas a partir dos pontos medios dos intervalos
    long double areaTotal, areaEsq, areaDir; // areas dos retangulos
    long double erro;

    meio = pegarPontoMedio(inicio, fim);
    alturaTotal = funcao(meio); // altura referente ao retangulo maior
    alturaEsq = funcao(pegarPontoMedio(inicio, meio));
    alturaDir = funcao(pegarPontoMedio(meio, fim));

    areaTotal = (fim - inicio) * alturaTotal;
    areaEsq = (meio - inicio) * alturaEsq;
    areaDir = (fim - meio) * alturaDir;

    // calculo do modulo da diferenca entre a area do retangulo maior e a soma das areas dos retangulos menores
    erro = fabsl(areaTotal - (areaEsq + areaDir));
    // caso essa diferenca seja superior ao erro permitido, calcula-se a area dos intervalos [inicio, meio] e [meio, fim]
    if(erro > erroPermitido) {
        areaTotal = integracaoRetangular(inicio, meio, funcao) + integracaoRetangular(meio, fim, funcao); // sendo a nova area "maior" a soma dessas duas areas
    }
    // do contrario, retorna o valor atual da area total
    return areaTotal;
}

int main (int argc, char *argv[]) {
    pthread_t *threads;
    Intervalo entrada;
    long double integral; // variavel para armazenar valor calculado da integral da funcao
    //long double inicio, fim;
    char escolha[2];

    if(argc < 5) {
        printf("<inicio do intervalo> <fim do intervalo> <erro permitido> <numero de threads>\n");
        exit(-1);
    }

    entrada.inicio = strtold(argv[1], NULL);
    entrada.fim = strtold(argv[2], NULL);
    erroPermitido = strtold(argv[3], NULL);
    nthreads = atoi(argv[4], NULL);

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

        // determinar qual funcao foi escolhida
        switch (escolha[1]) {
            case '1':
                funcao = &f1;
                break;
            case '2':
                funcao = &f2;
                break;
            case '3':
                funcao = &f3;
                break;
            case '4':
                funcao = &f4;
                break;
            case '5':
                funcao = &f5;
                break;
            case '6':
                funcao = &f6;
                break;
            case '7':
                funcao = &f7;
                break;
        }

        break;
    }

    threads = (pthread_t *) malloc(sizeof(pthread_t) * nThreads);

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_ins, NULL);
    pthread_cond_init(&cond_rem, NULL);

    buffer[topo] = entrada;
    topo++;

    //Criar threads

    //Fazer main esperar

    // fazer main somar resultados
    // rezar pra nao ficar muito ruim e ter que arranjar alguma forma de ordenar a soma dos floats;

    //dar free no necessario

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_ins);
    pthread_cond_destroy(&cond_rem);

    return 0;
}
