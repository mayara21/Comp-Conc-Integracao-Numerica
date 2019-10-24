#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "funcoes.h" // definiu-se as funcoes a serem usadas em um outro arquivo


long double (*funcao)(long double);
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
    long double (*funcoes[7])(long double) = {&f1, &f2, &f3, &f4, &f5, &f6, &f7};
    long double integral; // variavel para armazenar valor calculado da integral da funcao
    long double inicio, fim;
    char escolha[2];

    if(argc < 4) {
        printf("<inicio do intervalo> <fim do intervalo> <erro permitido>\n");
        exit(-1);
    }

    inicio = strtold(argv[1], NULL);
    fim = strtold(argv[2], NULL);
    erroPermitido = strtold(argv[3], NULL);

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

        // determinar qual funcao foi escolhida
        // switch (escolha[1]) {
        //     case '1':
        //         funcao = &f1;
        //         break;
        //     case '2':
        //         funcao = &f2;
        //         break;
        //     case '3':
        //         funcao = &f3;
        //         break;
        //     case '4':
        //         funcao = &f4;
        //         break;
        //     case '5':
        //         funcao = &f5;
        //         break;
        //     case '6':
        //         funcao = &f6;
        //         break;
        //     case '7':
        //         funcao = &f7;
        //         break;
        // }
        //
        // break;
     }

    integral = integracaoRetangular(inicio, fim);
    printf("Valor aproximado da Integral da funcao: %Lf\n", integral);

    return 0;
}
