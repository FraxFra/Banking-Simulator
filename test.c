#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include "config.h"

void *masterProcess(void *treadId)
{
        int creationError;
        int i = 0;
        int result = 0;
        double time_start = clock();
        double time_finish, time;
        pthread_t userProcess[SO_USERS_NUM];
        pthread_t nodeProcess[SO_NODES_NUM];
        pthread_t masterBook[1];

        for(i = 0; i < SO_USERS_NUM; i++)
        {
                creationError = pthread_create(&userProcess[i], NULL, userStart, 0); //Processi Utente
                if (creationError)
                {
                        printf("Creazione processo utente in errore\n");
                        exit(-1);
                }
        }

        for(i = 0; i < SO_NODES_NUM; i++)
        {
                creationError = pthread_create(&nodeProcess[i], NULL, nodeStart, 0); //Processi Nodi
                if (creationError)
                {
                        printf("Creazione processo nodo in errore\n");
                        exit(-1);
                }
        }

        creationError = pthread_create(&masterBook[0], NULL, masterBookStart, 0); //Processo Libro Mastro
        if (creationError)
        {
                printf("Creazione libro mastro in errore\n");
                exit(-1);
        }

        time_finish = clock();
        time = (double)(time_finish - time_start) / CLOCKS_PER_SEC;
        while(result == 0 && SO_SIM_SEC > time) //controllo del termine del tempo e della terminazione del processo libro mastro
        {
                time_finish = clock();
                time = (double)(time_finish - time_start) / CLOCKS_PER_SEC;
                sleep(1);
        }
        return (void*) 1;
}

int main(int argc, char const *argv[])
{
  /*
fatto  1 preparo l'ambiente alla partenza del processo master (che lancia tutta la simulazione)
fatto  2 lancio i processi appena ho tutti i requisiti
  3 elaboro le transazioni (eseguo), quindi lancio la trans utente, il nodo elabora e invia esito a utente
  4 controllo lo stato delle transazioni (nodi e utente)
  5 salvo nel libro mastro le transazioni eseguite (nodi e utente)
  6 stampo
  */
        pthread_t threads[1]; //Processo Master
        int rc, t;
        void* res;

        printf("Creazione del processo Master\n");
        rc = pthread_create(&threads[0], NULL, masterProcess, &res); //Processo Master
        pthread_join(threads[0], &res);
        if(rc)
        {
                printf("ERROR; return code from pthread_create() is %d\n",rc);
                exit(-1);
        }

        while(res == 0) //il main non termina finche il processo master non termina
        {
                printf("%d\n", res);
                sleep(1);
        }
        printf("Il processo Master e terminato\n");
        //TODO: creazione file di log
        return 0;
}
