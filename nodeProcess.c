#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "config.h"

void *nodeStart(void *treadId)
{
        printf("Creato processo nodo Id: %d\n", pthread_self());
        Transaction** transactionPool = (Transaction**)malloc(sizeof(Transaction*) * SO_TP_SIZE);

        pthread_exit(NULL);
}
