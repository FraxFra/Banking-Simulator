#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include "config.h"

void *userStart(void *treadId)
{
  printf("Test creazione processo utente\n");
  pthread_exit(NULL);
}
