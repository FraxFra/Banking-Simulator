#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "config.h"

void userListener(Transaction** transactionPool)
{

}

Transaction** createBlock(Transaction** transactionPool)
{

}

void sendBlock()
{

}

void *nodeStart(void *treadId)
{
        printf("Creato processo nodo Id: %d\n", pthread_self());
        while(1)
        {
                Transaction** transactionPool = (Transaction**)malloc(sizeof(Transaction*) * SO_TP_SIZE);
                userListener(transactionPool);
                //spedisce il blocco al libro mastro
                free(transactionPool);
        }
        pthread_exit(NULL);
}
