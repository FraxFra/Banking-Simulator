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
        printf("Test creazione libro mastro\n");

        transactions = (TransactionMaster***)malloc(sizeof(TransactionMaster**) * SO_REGISTRY_SIZE);
        int i = 0;
        while(SO_REGISTRY_SIZE > i)
        {
                transactions[i] = (TransactionMaster**)malloc(sizeof(TransactionMaster*) * SO_BLOCK_SIZE);
                //masterBookListener(i);
                i++;
        }
        pthread_exit((void*) 1);
}
