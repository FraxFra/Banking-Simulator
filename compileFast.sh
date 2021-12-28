gcc -c -g masterBookProcess.c
gcc -c -g userProcess.c
gcc -c -g nodeProcess.c
gcc -c -g test.c
gcc -o main masterBookProcess.o userProcess.o nodeProcess.o test.o
./main
