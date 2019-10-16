#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h> 
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
    return sqrt(1 - pow(x,4));
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

int main (int argc, char *argv[]){
    long double arg; 
    long double y1,y2,y3,y4,y5,y6,y7;
    
    arg = strtold(argv[1],NULL); 


    y1 = f1(arg);
    printf("f1(%Lf): %Lf\n",arg, y1);
    y2 = f2(arg);
    printf("f2(%Lf): %Lf\n",arg, y2);
    y3 = f3(arg);
    printf("f3(%Lf): %Lf\n",arg, y3);
    y4 = f4(arg);
    printf("f4(%Lf): %Lf\n",arg, y4);
    y5 = f5(arg);
    printf("f5(%Lf): %Lf\n",arg, y5);
    y6 = f6(arg);
    printf("f6(%Lf): %Lf\n",arg, y6);
    y7 = f7(arg);
    printf("f7(%Lf): %Lf\n",arg, y7);

}
