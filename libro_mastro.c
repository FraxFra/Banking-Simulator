#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include "config.h"


void *masterBookStart(void *treadId)
{
        printf("Test creazione libro mastro\n");

        int i = 0;
        while(SO_REGISTRY_SIZE > i)
        {

                i++;
        }
        return (void*) 1;
}
