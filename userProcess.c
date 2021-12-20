#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include "config.h"

int calcBalance(pthread_t threadId)
{
        int amountTransactions = 0;
        int i, j;

        if(transactions != NULL)
        {
                for(i = 0; i < SO_REGISTRY_SIZE; i++)
                {
                        if(transactions[i] != NULL)
                        {
                                for(j = 0; j < SO_BLOCK_SIZE; j++)
                                {
                                        if(transactions[i][j] != NULL)
                                        {
                                                if(pthread_equal(transactions[i][j]->sender, threadId))
                                                {
                                                        amountTransactions = amountTransactions - transactions[i][j]->qty - transactions[i][j]->reward;
                                                }
                                                else if(pthread_equal(transactions[i][j]->receiver, threadId))
                                                {
                                                        amountTransactions = amountTransactions + transactions[i][j]->qty;
                                                }
                                        }
                                }
                        }
                }
        }
        return SO_BUDGET_INIT + amountTransactions;
}

pthread_t findReceiver()
{
        pthread_t res = pthread_self();
        while(res == pthread_self())
        {
                res = (rand() % SO_USERS_NUM) + 3;
        }
        return res;
}

pthread_t findNode()
{
        return (rand() % SO_NODES_NUM) + SO_USERS_NUM + 4;
}

int calcReward(int amount)
{
        return (amount * SO_REWARD) / 100;
}

Transaction* createTransaction(void *threadId, int balance)
{
        int amount = rand() % balance + 1;
        Transaction* t = (Transaction*)malloc(sizeof(Transaction));
        t->timestamp = clock();
        t->sender = pthread_self();
        t->reward = calcReward(amount);
        t->qty = amount - t->reward;
        t->receiver = findReceiver();
        return t;
}

void *userStart(void *threadId)
{
        printf("Creato processo utente Id: %d\n", pthread_self());
        int actual_retry = 0;
        while(actual_retry <= SO_RETRY)
        {
                int balance = calcBalance(pthread_self());
                if(balance >= 2)
                {
                        Transaction* t = createTransaction(threadId, balance);
                        //sceglie il node
                        //invia la transazione al node
                        //va in wait
                        //se l'esito è negativo actual_retry va incrementato e va ripetuta la stessa transazione
                        sleep(20);
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
