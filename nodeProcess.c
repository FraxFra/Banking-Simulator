#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "config.h"

int getTransQtys(Transaction** transactionBlock)
{
    int i, j, qty;
    for(; i < sizeof(transactionBlock); i++)
    {
        for(; j < sizeof(transactionBlock); j++)
        {
            qty += transactionBlock[i][j].qty;
        }
    }

    return qty;
}


void rewardTransaction(Transaction** transactionBlock)
{
    Transaction* rewardTransaction = (Transaction*)malloc(sizeof(Transaction));
    rewardTransaction -> timestamp = clock();
    rewardTransaction->sender = -1;
    rewardTransaction->reward = 0;
    rewardTransaction->qty = getTransQtys(transactionBlock);
    rewardTransaction->receiver = pthread_self();

    transactionBlock[SO_BLOCK_SIZE] = rewardTransaction;
}


int checkTransaction(Transaction* transaction)
{
    int i, j = 0;

    for(; i < SO_BLOCK_SIZE; i++)
    {
        for(; j < SO_REGISTRY_SIZE; j++)
        {
            if(masterBookTransactions[i][j] != NULL)
            {
                if(masterBookTransactions[i][j]->timestamp == transaction->timestamp
                   && masterBookTransactions[i][j]->sender == transaction->sender
                   && masterBookTransactions[i][j]->receiver == transaction->receiver)
                {
                    return 0;
                }
            }
        }
    }

    return 1;
}


void sendBlock(Transaction** transactionBlock)
{
  //
}

void createBlock(Transaction** transactionPool)
{
    Transaction** transactionBlock = (Transaction**)malloc(sizeof(Transaction*) * SO_BLOCK_SIZE);
    int i, blockIdx;
    for(; i < SO_BLOCK_SIZE - 1; i++)
    {
        //check della transazione nel libro mastro
        if (checkTransaction(transactionPool[i]))
        {
            //creato blocco di transazioni
            transactionBlock[blockIdx] = (Transaction*) malloc(sizeof(Transaction));
            transactionBlock[blockIdx] = transactionPool[i];
            blockIdx++;
        }
    }
    //invio il blocco
    sendBlock(transactionBlock);
}


void discardTransaction()
{

}


void userListener(Transaction** transactionPool)
{
    //leggo il processo utente e inizio la transazione
    int tpSize = sizeof(transactionPool) / sizeof(Transaction*);
    //riempire la transactionPool
    if (tpSize < SO_TP_SIZE)
    {
        createBlock(transactionPool);
    }
    // else
    // {
    //     //informare il sender che la tp è piena
    //     discardTransaction(transactionPool);
    // }
}


void *nodeStart(void *treadId)
{
    printf("Creato processo nodo Id: %ld\n", pthread_self());
    while(1)
    {
        Transaction** transactionPool = (Transaction**)malloc(sizeof(Transaction*) * SO_TP_SIZE);
        userListener(transactionPool);
        usleep((rand() % SO_MAX_TRANS_PROC_NSEC) + SO_MIN_TRANS_PROC_NSEC);
        //spedisce il blocco al libro mastro

        free(transactionPool);
    }
    pthread_exit(NULL);
}
