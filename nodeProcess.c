#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include "config.h"

void *nodeStart(void *treadId)
{
  printf("Test creazione processo nodo\n");
  //il nodo esegue l'elaborazione
  pthread_exit(NULL);
}
