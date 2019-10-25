# Computação Concorrente: Integração Numérica usando Métodos dos Retângulos

## Pastas e Arquivos:
**timer.h:** funcoes referente à medição de tempo\
**funcoes.h:** headers das funcoes\
**funcoes.c:** declaracoes e conteudo das funcoes
### Sequencial
**sequencial.c:** main e funcoes para calculo da integral da versão sequencial

### Concorrente
**concorrente.c** main e funcoes para calculo da integral da versao concorrente

## Como compilar:
**sequencial:** gcc sequencial.c ../funcoes.c -o sequencial -lm -lpthread -Wall\
**concorrente:** gcc concorrente.c ../funcoes.c -o concorrente -lm -lpthread -Wall

## Como rodar:
**sequencial:** ./sequencial \<inicio do intervalo> \<fim do intervalo> \<erro permitido> \<funcao a ser integrada>\
**concorrente:** ./concorrente \<inicio do intervalo> \<fim do intervalo> \<erro permitido> \<numero de threads> \<funcao a ser integrada>
