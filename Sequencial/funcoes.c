#include <stdio.h>
#include <stdlib.h> 
#include <math.h> 

long double f1(long double x){
    return 1 + x;
}

long double f2(long double x) {
    if ((x < 1) && (x > -1))
        return sqrt(1 - pow(x,2));
    else{
        printf("Argumento inválido para f2: -1 < x < 1\n");
        exit(-1);
    }
}

long double f3(long double x){
    return sqrt(1 + pow(x,4));
}

long double f4(long double x){ 
    return sin(pow(x,2));
}

long double f5(long double x){
    return cos(pow(M_E,-x));
}

long double f6(long double x){
    return cos(pow(M_E, -x)) * x; 
}

long double f7(long double x){
    return cos(pow(M_E, -x)) * (0.005 * (pow(x,3) + 1));    
}
