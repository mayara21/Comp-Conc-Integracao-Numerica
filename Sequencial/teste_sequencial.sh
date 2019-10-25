#!/bin/bash
#Script para automatizar testes da versão sequencial do programa de integração numérica

array_min_intervalo=(-0.9 -10 -100)
array_max_intervalo=(0.9 10 100)
array_erro=(0.0001 0.000000001 0.0000000000000001)
#erro=0.0001

echo "função,intervalo,erro,integral,tempo" > teste.csv

for i in $(seq 1 ${#array_min_intervalo[@]}); do

    min_intervalo=${array_min_intervalo[i-1]}
    max_intervalo=${array_max_intervalo[i-1]}
    echo "Intervalo atual: [$min_intervalo, $max_intervalo]"

    for erro in "${array_erro[@]}"; do
        echo "Erro atual: $erro"

        for j in $(seq 1 6); do
            echo "Função atual: f$j"
            output=$(./sequencial $min_intervalo $max_intervalo $erro f$j)
            echo "f$j,[$min_intervalo, $max_intervalo],$erro,$output" >> teste.csv
        done 
    done
done
