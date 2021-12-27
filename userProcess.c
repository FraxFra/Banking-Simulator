#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <Windows.h>
#include <winuser.h>
#include "config.h"

int calcBalance(pthread_t threadId)
{
    int amountTransactions = 0;
    int i, j;

    if(masterBook != NULL)
    {
        for(i = 0; i < SO_REGISTRY_SIZE; i++)
        {
            if(masterBook[i] != NULL)
            {
                for(j = 0; j < SO_BLOCK_SIZE; j++)
                {
                    if(masterBook[i][j] != NULL)
                    {
                        if(pthread_equal(masterBook[i][j]->sender, threadId))
                        {
                                amountTransactions = amountTransactions - masterBook[i][j]->qty - masterBook[i][j]->reward;
                        }
                        else if(pthread_equal(masterBook[i][j]->receiver, threadId))
                        {
                                amountTransactions = amountTransactions + masterBook[i][j]->qty;
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

void* userStart(void *threadId)
{
    printf("Creato processo utente Id: %d\n", pthread_self());
    int actual_retry = 0;
    while(actual_retry <= SO_RETRY)
    {
        int balance = calcBalance(pthread_self());
        if(balance >= 2)
        {
                Transaction* t = createTransaction(threadId, balance);
                pthread_t node = findNode();
                //invia la transazione al node
                usleep((rand() % SO_MAX_TRANS_GEN_NSEC) + SO_MIN_TRANS_GEN_NSEC);
                //incrementare actual_retry in caso negativo
        }
    }
    //se ci si trova qui allora il processo per SO_RETRY volte non è riuscito a portare a termine la transazione -> deve terminare
    pthread_exit(NULL);
}
