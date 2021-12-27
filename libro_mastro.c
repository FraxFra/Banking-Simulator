#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <Windows.h>
#include <winuser.h>
#include "config.h"
void masterBookListener(int i, Transaction* t)
{
        //ascolta le richieste provenienti dai nodi

}

void* masterBookStart(void *treadId)
{
    printf("Creato libro mastro Id: %d\n", pthread_self());

    masterBook = (Transaction***)malloc(sizeof(Transaction**) * SO_REGISTRY_SIZE);
    int i = 0;
    while(SO_REGISTRY_SIZE > i)
    {
        masterBook[i] = (Transaction**)malloc(sizeof(Transaction*) * SO_BLOCK_SIZE);
        //masterBookListener(i);
        i++;
    }
    pthread_exit((void*) 1);
}
