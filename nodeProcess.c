#include <stdio.h>
#include <time.h>
#include "config.h"

void *nodeStart(void *treadId)
{
  printf("Test creazione processo nodo\n");
  pthread_exit(NULL);
}
