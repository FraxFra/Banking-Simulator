#include <stdio.h>
#include <pthread.h>
#include "config.h"

// #define NUM_THREADS 5

void *masterProcess(void *treadId)
{
  /*Creazione di :
  nodi,
  utente,
  libro mastro (un solo processo)
  */
  int creationError;
  pthread_t userProcess[SO_USERS_NUM];
  pthread_t nodeProcess[SO_NODES_NUM];
  pthread_t masterBook[1];


  for(int i = 0; i < SO_USERS_NUM; i++)
  {
    creationError = pthread_create(&userProcess[i], NULL, userStart, 0);
    if (creationError)
    {
      printf("Creazione processo utente in errore\n");
      exit(-1);
    }
  }

  for(int i = 0; i < SO_NODES_NUM; i++)
  {
    creationError = pthread_create(&nodeProcess[i], NULL, nodeStart, 0);
    if (creationError)
    {
      printf("Creazione processo nodo in errore\n");
      exit(-1);
    }
  }

  creationError = pthread_create(&masterBook[0], NULL, masterBookStart, 0);
  if (creationError)
  {
    printf("Creazione libro mastro in errore\n");
    exit(-1);
  }

}

int main(int argc, char const *argv[])
{
  /*
  1 preparo l'ambiente alla partenza del processo master (che lancia tutta la simulazione)
  2 lancio i processi appena ho tutti i requisiti
  3 elaboro le transazioni (eseguo), quindi lancio la trans utente, il nodo elabora e invia esito a utente
  4 controllo lo stato delle transazioni (nodi e utente)
  5 salvo nel libro mastro le transazioni eseguite (nodi e utente)
  6 stampo
  */
  pthread_t threads[1];//Master
  int rc, t;
  //lancio la simulazione, cioè inizio ascolto del libro mastro (processo master)

  printf("Creating master thread\n");
  //Creo i processi nodo, appena il primo processo utente va in exec
  //il nodo esegue l'elaborazione
  rc = pthread_create(&threads[t], NULL, masterProcess, 0);
  if (rc)
  {
    printf("ERROR; return code from pthread_create() is %d\n",rc);
    exit(-1);
  }

  sleep(SO_SIM_SEC);
  pthread_exit(NULL);

  return 0;
}
