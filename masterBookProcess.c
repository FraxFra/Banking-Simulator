#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "config.h"
void masterBookListener(int i, Transaction* t)
{
        //ascolta le richieste provenienti dai nodi

}

void* masterBookStart()
{
    printf("Creato libro mastro Id: %d\n", getpid());
/*
    masterBookTransactions = (Transaction***)malloc(sizeof(Transaction**) * SO_REGISTRY_SIZE);
    int i = 0;
    while(SO_REGISTRY_SIZE > i)
    {
        masterBookTransactions[i] = (Transaction**)malloc(sizeof(Transaction*) * SO_BLOCK_SIZE);
        //masterBookListener(i);
        i++;
    }*/
    exit(EXIT_SUCCESS);
}
