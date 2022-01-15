#include "config.h"

bool checkTerminationNode()
{
    return termination[0] == 0;
}

void sendReportNode(Transaction* transactionPool, Transaction* transactionBlock, pid_t nodePid, int* msgReportNode, sem_t* semThread)
{
    int code;
    int j;
    BufferReportNode* message = (BufferReportNode*)malloc(sizeof(BufferReportNode));

    message->mtype = nodePid;
    message->nTransactions = 0;
    for(j = 0; j < SO_TP_SIZE; j++)
    {
        if(transactionPool[j].timestamp != -1)
        {
            message->nTransactions += 1;
        }
    }

    for(j = 0; j < SO_BLOCK_SIZE; j++)
    {
        if(transactionBlock[j].timestamp != -1)
        {
            message->nTransactions += 1;
        }
    }

    code = msgsnd(*msgReportNode, message, sizeof(BufferReportNode), 0);
    if(code == -1)
    {
        printf("Error in msgReportNode; sono il nodo %d e volevo trasmettere al master il report\n", nodePid);
        exit(EXIT_FAILURE);
    }
    //free(message);
    sem_wait(semThread);
    sem_post(semThread);
    printf("LA MOOORTEEE nodo\n");
    exit(EXIT_SUCCESS);
}

void insertBlock(Transaction* transactionBlock, Transaction* transactionPool, pid_t nodePid, int* msgReport, sem_t* semThread, int* msgReportNodeNode)
{
    int i;
    int j = 0;
    int z;

    sem_wait(semRegistry);
    int actualTransactions = nBlocksRegistry[0] * SO_BLOCK_SIZE;
    if(actualTransactions != SO_BLOCK_SIZE * SO_REGISTRY_SIZE)
    {
        for(i = actualTransactions; i < actualTransactions + SO_BLOCK_SIZE; i++)
        {
            //printf("%d\n",i);
            //printf("%d\n", masterBookRegistry[i].qty);
            masterBookRegistry[i] = transactionBlock[j];
            j++;


        }
        nBlocksRegistry[0] = nBlocksRegistry[0] + 1;
        //printf("blocco inserito %d\n %d\n",nBlocksRegistry[0],actualTransactions);
        sem_post(semRegistry);
    }
    else
    {
        sem_post(semRegistry);
        printf("Finito lo spazio");
        sendReportNode(transactionPool, transactionBlock, nodePid, msgReportNodeNode, semThread);
    }

}

int getQuantities(Transaction* transactionBlock)
{
    int i;
    int res = 0;
    for(i = 0; i < SO_BLOCK_SIZE - 1; i++)
    {
        res = res + transactionBlock[i].reward;
    }
    return res;
}

void rewardTransaction(Transaction* transactionBlock, pid_t nodePid)
{
    transactionBlock[SO_BLOCK_SIZE - 1].timestamp = clock();
    transactionBlock[SO_BLOCK_SIZE - 1].sender = -1;
    transactionBlock[SO_BLOCK_SIZE - 1].reward = 0;
    transactionBlock[SO_BLOCK_SIZE - 1].qty = getQuantities(transactionBlock);
    transactionBlock[SO_BLOCK_SIZE - 1].receiver = nodePid;
}

int checkTransactionBlock(Transaction transaction, int x, Transaction* transactionBlock)
{
    int i;
    int res = 1;
    for(i = 0; i < x; i++)
    {
        if(transactionBlock[i].timestamp == transaction.timestamp
        && transactionBlock[i].sender == transaction.sender
        && transactionBlock[i].receiver == transaction.receiver)
        {
            res = 0;
        }
    }
    return res;
}

int checkTransactionRegistry(Transaction transaction)
{
    int i;
    int res = 1;

    for(i = 0; i < nBlocksRegistry[0] * SO_BLOCK_SIZE ; i++)
    {
        if(masterBookRegistry[i].timestamp == transaction.timestamp
        && masterBookRegistry[i].sender == transaction.sender
        && masterBookRegistry[i].receiver == transaction.receiver)
        {
            res = 0;
        }
    }
    return res;
}

int chooseTransaction(Transaction* transactionPool, Transaction* transactionBlock, pid_t nodePid, int* msgReportNode, sem_t* semThread)
{
    int i = rand() % SO_TP_SIZE;
    while(transactionPool[i].timestamp == -1)
    {
        if(checkTerminationNode())
        {
            sendReportNode(transactionPool, transactionBlock, nodePid, msgReportNode, semThread);
        }
        i = rand() % SO_TP_SIZE;
    }
    return i;
}

void createBlock(Transaction* transactionPool, pid_t nodePid, int* msgReportNode, sem_t* semThread)
{
    Transaction* transactionBlock = (Transaction*)malloc(sizeof(Transaction) * SO_BLOCK_SIZE);
    int i;
    int blockIdx = 0;

    while(blockIdx < SO_BLOCK_SIZE - 1)
    {
        i = chooseTransaction(transactionPool, transactionBlock, nodePid, msgReportNode, semThread);
          //printf("MI SON ROTTO IL CAZZO ALTRO CHE BUFFERRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR \n");
        if (checkTransactionRegistry(transactionPool[i]) == 1 && checkTransactionBlock(transactionPool[i],blockIdx,transactionBlock) == 1 && transactionPool[i].timestamp != -1)
        {
            transactionBlock[blockIdx] = transactionPool[i];
            //printf("%d block\n", transactionBlock[blockIdx].qty);
            transactionPool[i].timestamp = -1;
            blockIdx++;
        }

        if(checkTerminationNode())
        {

            sendReportNode(transactionPool, transactionBlock, nodePid, msgReportNode, semThread);
        }
    }
    //printf("%d\n", transactionBlock[0].qty);
    rewardTransaction(transactionBlock, nodePid);

    usleep((rand() % SO_MAX_TRANS_PROC_NSEC + SO_MIN_TRANS_PROC_NSEC) / 1000);
    insertBlock(transactionBlock, transactionPool, nodePid, msgReportNode, semThread, msgReportNode);

    //free(transactionBlock);
}

void replyTransaction(BufferTransactionSend* message, int* msgTransactionReplyId, int res)
{
    int code;
    BufferTransactionReply* messageReply = (BufferTransactionReply*)malloc(sizeof(BufferTransactionReply));

    messageReply->mtype = message->transaction.sender;
    messageReply->result = res;
    code = msgsnd(*msgTransactionReplyId, messageReply, sizeof(BufferTransactionReply), 0);
    //free(messageReply);
}

void* manageTransactions(void* args)
{
    PthreadArguments* arguments = (PthreadArguments*) args;
    int code;
    int i;
    int findPlace = 1;
    BufferTransactionSend* message = (BufferTransactionSend*)malloc(sizeof(BufferTransactionSend));
    sem_wait(arguments->semThread);

    while(1)
    {
        code = msgrcv(*arguments->msgTransactionSendId, message, sizeof(BufferTransactionSend), arguments->nodePid, IPC_NOWAIT);
        if(checkTerminationNode() && nTerminatedUsers[0] == SO_USERS_NUM)
        {
            sem_post(arguments->semThread);
            //free(message);
            //free(args);
            pthread_exit(NULL);
        }
        else if(checkTerminationNode() && nTerminatedUsers[0] != SO_USERS_NUM)
        {
            usleep((rand() % SO_MAX_TRANS_PROC_NSEC + SO_MIN_TRANS_PROC_NSEC) / 1000);
        }
        else if (code != -1)
        {
            findPlace = 1;
            for(i = 0; i < SO_TP_SIZE; i++)
            {
                if(findPlace == 1)
                {
                    if(arguments->transactionPool[i].timestamp == -1)
                    {
                        arguments->transactionPool[i] = message->transaction;
                        replyTransaction(message, arguments->msgTransactionReplyId, 0);
                        findPlace = 0;
                    }
                }
            }

            if(findPlace == 1)
            {
                replyTransaction(message, arguments->msgTransactionReplyId, 1);
            }
        }
    }
}

void initArgs(PthreadArguments* args, pid_t nodePid, Transaction* transactionPool, int* msgTransactionSendId, int* msgTransactionReplyId, sem_t* semThread)
{
    args->nodePid = nodePid;
    args->transactionPool = transactionPool;
    args->msgTransactionSendId = msgTransactionSendId;
    args->msgTransactionReplyId = msgTransactionReplyId;
    args->semThread = semThread;
}

void initTransactionPool(Transaction* transactionPool)
{
    int i;
    for(i = 0; i < SO_TP_SIZE; i++)
    {
        transactionPool[i].timestamp = -1;
    }
}

void nodeStart(int* msgTransactionSendId, int* msgTransactionReplyId, int* msgReportNode)
{
    pid_t nodePid = getpid();
    int creationError;

    sem_t* semThread = (sem_t*)malloc(sizeof(sem_t));
    sem_init(semThread, 0, 1);

    Transaction* transactionPool = (Transaction*)malloc(sizeof(Transaction) * SO_TP_SIZE);
    initTransactionPool(transactionPool);

    PthreadArguments* args = (PthreadArguments*)malloc(sizeof(PthreadArguments));
    initArgs(args, nodePid, transactionPool, msgTransactionSendId, msgTransactionReplyId, semThread);

    pthread_t transactionPoolManager[1];
    creationError = pthread_create(&transactionPoolManager[0], NULL, manageTransactions, (void*)args);
    if(creationError)
    {
        printf("Errore nella creazione del thread gestore della transactionPool\n");
        //free(semThread);
        //free(transactionPool);
        //free(args);
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        createBlock(transactionPool, nodePid, msgReportNode, semThread);
    }
}
