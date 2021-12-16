#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include "config.h"

int calcBalance(void *treadId)
{

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
        //creazione transazione
        //sender = threadId
        printf("Test creazione processo utente\n");
        int balance = calcBalance(treadId);
        if(balance >= 2)
        {
                createTransaction(treadId, balance);
        }
        printf("aaa\n");
        //gestione del caso < 2
        //gestione del retry
        pthread_exit(NULL);
}
