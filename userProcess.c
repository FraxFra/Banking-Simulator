#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include "config.h"

int calcBalance(void *treadId)
{
        //leggo tutto il libro mastro(ciclo) e prendo tutte le qty delle transazioni
}

pthread_t findReceiver()
{

}

int calcReward(int balance)
{
        return (balance * SO_REWARD) / 100;
}

Transaction* createTransaction(void *treadId, int balance)
{
        int amount = rand() % balance + 2;
        Transaction* t = (Transaction*)malloc(sizeof(Transaction));
        t->timestamp = clock();
        t->sender = pthread_self();
        t->reward = calcReward(balance);
        t->qty = amount - t->reward;
        t->receiver = findReceiver();
        return t;
}

void *userStart(void *treadId)
{
        printf("Test creazione processo utente\n");
        int actual_retry = 0;
        while(actual_retry <= SO_RETRY)
        {
                int balance = calcBalance(treadId);
                if(balance >= 2)
                {
                        Transaction* t = createTransaction(treadId, balance);
                        //sceglie il node
                        //invia la transazione al node
                        //va in wait
                        //se l'esito è negativo actual_retry va incrementato e va ripetuta la stessa transazione
                }
                else
                {
                        //gestione del caso < 2
                        //va in wait e il ciclo riparte da capo
                }
        }
        //se ci si trova qui allora il processo per SO_RETRY volte non è riuscito a portare a termine la transazione -> deve terminare
        pthread_exit(NULL);
}
