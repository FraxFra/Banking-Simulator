#include "config.h"

void masterBookListener(int i, Transaction* t)
{
        //ascolta le richieste provenienti dai nodi

}

void syncMasterBook()
{
    size_t up;
    size_t np;
    while((up + np) == (SO_NODES_NUM + SO_USERS_NUM))
    {
        up = sizeof(userProcesses) / sizeof(userProcesses[0]);
        np = sizeof(nodeProcesses) / sizeof(nodeProcesses[0]);
    }
}

void* masterBookStart(int* msgBlockSendId, int* msgBlockReplyId)
{
    pid_t masterBookPid = getpid();
    printf("Creato libro mastro Id: %d\n", masterBookPid);

    syncMasterBook();
/*
    masterBookTransactions = (Transaction***)malloc(sizeof(Transaction**) * SO_REGISTRY_SIZE);
    int i = 0;
    while(SO_REGISTRY_SIZE > i)
    {
        masterBookTransactions[i] = (Transaction**)malloc(sizeof(Transaction*) * SO_BLOCK_SIZE);
        //masterBookListener(i);
        i++;
    }*/
    exit(EXIT_SUCCESS);
}
