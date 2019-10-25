# Computação Concorrente: Integração Numérica usando Métodos dos Retângulos

## Pastas e Arquivos:
**timer.h:** funcoes referente à medição de tempo\
**funcoes.h:** headers das funcoes\
**funcoes.c:** declaracoes e conteudo das funcoes
### Sequencial
**sequencial.c:** main e funcoes para calculo da integral da versão sequencial

### Concorrente
**concorrenteFilas.c:** main e funcoes para calculo da integral da versao concorrente usando filas de intervalos e com consistência na ordem da soma\
**concorrentePilhas.c:** main e funcoes para calculo da integral da versao concorrente usando uma pilha de intervalos e sem garantia na ordem da soma

## Como compilar:
**sequencial.c:** gcc sequencial.c ../funcoes.c -o sequencial -lm -lpthread -Wall\
**concorrenteFilas.c:** gcc concorrenteFilas.c ../funcoes.c -o concorrenteFilas -lm -lpthread -Wall\
**concorrentePilhas.c:** gcc concorrentePilhas.c ../funcoes.c -o concorrentePilhas -lm -lpthread -Wall

## Como rodar:
**sequencial:** ./sequencial \<inicio do intervalo> \<fim do intervalo> \<erro permitido> \<funcao a ser integrada>\
**concorrenteFilas:** ./concorrenteFilas \<inicio do intervalo> \<fim do intervalo> \<erro permitido> \<numero de threads> \<funcao a ser integrada>\
**concorrentePilhas:** ./concorrentePilhas \<inicio do intervalo> \<fim do intervalo> \<erro permitido> \<numero de threads> \<funcao a ser integrada>
