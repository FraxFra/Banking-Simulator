gcc -c -g libro_mastro.c
gcc -c -g userProcess.c
gcc -c -g nodeProcess.c
gcc -c -g test.c
gcc -o main libro_mastro.o userProcess.o nodeProcess.o test.o
main
