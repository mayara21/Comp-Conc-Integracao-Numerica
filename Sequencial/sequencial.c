#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h> // include necessario para poder utilizar o timer.h
#include "../funcoes.h" // definiu-se as funcoes a serem usadas em um outro arquivo
#include "../timer.h" // funcoes para calculo de tempo

long double (*funcao)(long double); // funcao global para facilitar a passagem da funcao a ser integrada
long double erroPermitido;

// funcao para calculo do ponto medio entre dois pontos a e b
long double pegarPontoMedio(long double inicio, long double fim) {
    return ((inicio + fim)/2);
}

// funcao recursiva para integracao numerica usando regra do ponto medio/metodo do retangulo
long double integracaoRetangular(long double inicio, long double fim) {
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
        areaTotal = integracaoRetangular(inicio, meio) + integracaoRetangular(meio, fim); // sendo a nova area "maior" a soma dessas duas areas
    }
    // do contrario, retorna o valor atual da area total
    return areaTotal;
}

int main (int argc, char *argv[]) {
    long double (*funcoes[7])(long double) = {&f1, &f2, &f3, &f4, &f5, &f6, &f7}; // vetor com todas as possibilidades de funcoes
    long double integral; // variavel para armazenar valor calculado da integral da funcao
    long double inicio, fim;
    double tempoInicio, tempoFim, tempoProcessamento;
    char *escolha;

    // forcar usuario a escolher um intervalo, erro e a funcao
    if(argc < 5) {
        printf("<inicio do intervalo> <fim do intervalo> <erro permitido> <funcao a ser integrada>\n");
        printf("(f1) f(x) = 1 + x\n(f2) f(x) = √(1 − xˆ2), −1 < x < 1\n(f3) f(x) = √(1 + xˆ4)\n(f4) f(x) = sen(xˆ2)\n(f5) f(x) = cos(eˆ(-x))\n(f6) f(x) = cos(eˆ(-x)) * x\n(f7) f(x) = cos(eˆ(-x)) * ((0.005 * xˆ3) + 1)\n");
        exit(-1);
    }

    inicio = strtold(argv[1], NULL);
    fim = strtold(argv[2], NULL);
    erroPermitido = strtold(argv[3], NULL);
    escolha = argv[4];

    // condicional para forcar o usuario a escolher uma funcao valida
    if(escolha[0] != 'f' || escolha[1] > '7' || escolha[1] < '1') {
        printf("Insira uma funcao valida\n");
        exit(-1);
    }

    // operacao para atribuir a funcao global a funcao escolhida
    funcao = funcoes[(escolha[1] - '0') - 1];

    GET_TIME(tempoInicio);
    integral = integracaoRetangular(inicio, fim);
    GET_TIME(tempoFim);

    tempoProcessamento = tempoFim - tempoInicio;
   //printf("Valor aproximado da Integral da funcao: %.15Lf\n", integral);
   // printf("Tempo de processamento: %lf\n", tempoProcessamento);

    printf("%.15Lf,", integral);
    printf("%lf\n", tempoProcessamento);
    return 0;
}
