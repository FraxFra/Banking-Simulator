#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "config.h"

void masterBookListener(transactions, i)
{
        //ascolta le richieste provenienti dai nodi
}

void* masterBookStart(void *treadId)
{
        printf("Test creazione libro mastro\n");

        TransactionMaster*** transactions = (TransactionMaster***)malloc(sizeof(TransactionMaster**) * SO_REGISTRY_SIZE);
        int i = 0;
        while(SO_REGISTRY_SIZE > i)
        {
                transactions[i] = (TransactionMaster**)malloc(sizeof(TransactionMaster*) * SO_BLOCK_SIZE);
                masterBookListener(transactions, i);
                i++;
        }
        return (void*) 1;
}
