#include <stdio.h>
#include <time.h>
#include "config.h"


void *masterBookStart(void *treadId)
{
  printf("Test creazione libro mastro\n");
  pthread_exit(NULL);
}
