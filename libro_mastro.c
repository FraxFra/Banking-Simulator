#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "config.h"
void masterBookListener(int i, Transaction* t)
{
        //ascolta le richieste provenienti dai nodi

}

void* masterBookStart(void *treadId)
{
    printf("Creato libro mastro Id: %ld\n", pthread_self());

    masterBookTransactions = (Transaction***)malloc(sizeof(Transaction**) * SO_REGISTRY_SIZE);
    int i = 0;
    while(SO_REGISTRY_SIZE > i)
    {
        masterBookTransactions[i] = (Transaction**)malloc(sizeof(Transaction*) * SO_BLOCK_SIZE);
        //masterBookListener(i);
        i++;
    }
    pthread_exit((void*) 1);
}
