gcc -c -g userProcess.c
gcc -c -g nodeProcess.c
gcc -c -g test.c
gcc -o main userProcess.o nodeProcess.o test.o
./main
