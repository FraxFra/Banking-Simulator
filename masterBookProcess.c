#include "config.h"

void masterBookListener(int i, Transaction* t)
{
        //ascolta le richieste provenienti dai nodi

}

void* masterBookStart()
{
    pid_t masterBookPid = getpid();
    printf("Creato libro mastro Id: %d\n", masterBookPid);
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
