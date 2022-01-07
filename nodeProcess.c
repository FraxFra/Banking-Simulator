#include "config.h"

bool checkTerminationNode()
{
    return termination[0];
}

void sendReportNode(Transaction** transactionPool, pid_t nodePid, int* msgReport)
{
    int code;
    BufferReport* message = (BufferReport*)malloc(sizeof(BufferReport));

    message->mtype = nodePid;
    message->alive = 0;
    message->transactionPool = transactionPool;

    code = msgsnd(*msgReport, message, sizeof(BufferReport), 0);
    if(code == -1)
    {
        printf("Error in msgReport; sono il nodo %d e volevo trasmettere al master il report\n", nodePid);
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

void insertBlock(Transaction** transactionBlock, Transaction** transactionPool, pid_t nodePid, int* msgReport)
{
    int i;
    int j = 0;
    int actualTransactions = nblocksRegistry[0] * SO_BLOCK_SIZE;
    sem_wait(semRegistry);
    if(actualTransactions != SO_BLOCK_SIZE * SO_REGISTRY_SIZE)
    {
        for(i = actualTransactions; i < actualTransactions + SO_BLOCK_SIZE; i++)
        {
            masterBookRegistry[i] = *transactionBlock[j];
            j++;
        }
        nblocksRegistry[0] = nblocksRegistry[0] + 1;
        sem_post(semRegistry);
        printf("Blocco in posizione %d inserito\n", actualTransactions);
    }
    else
    {
        sem_post(semRegistry);
        sendReportNode(transactionPool, nodePid, msgReport);
    }
}

int getQuantities(Transaction** transactionBlock)
{
    int i;
    int res = 0;
    for(i = 0; i < SO_BLOCK_SIZE - 1; i++)
    {
        res = res + transactionBlock[i]->reward;
    }
    return res;
}

void rewardTransaction(Transaction** transactionBlock, pid_t nodePid)
{
    Transaction* rewardTransaction = (Transaction*)malloc(sizeof(Transaction));
    rewardTransaction->timestamp = clock();
    rewardTransaction->sender = -1;
    rewardTransaction->reward = 0;
    rewardTransaction->qty = getQuantities(transactionBlock);
    rewardTransaction->receiver = nodePid;
    transactionBlock[SO_BLOCK_SIZE - 1] = rewardTransaction;
}

int checkTransaction(Transaction* transaction)
{
    int i;
    int j;
    int res = 1;
    if(masterBookRegistry != NULL && nblocksRegistry != NULL)
    {
        for(i = 0; i < nblocksRegistry[0] * SO_BLOCK_SIZE ; i++)
        {
            if(masterBookRegistry[i].timestamp == transaction->timestamp
            && masterBookRegistry[i].sender == transaction->sender
            && masterBookRegistry[i].receiver == transaction->receiver)
            {
                res = 0;
            }
        }
    }
    return res;
}

int chooseTransaction(Transaction** transactionPool)
{
    int i = rand() % SO_TP_SIZE;
    while(transactionPool[i] == NULL)
    {
        i = rand() % SO_TP_SIZE;
    }
    return i;
}

void createBlock(Transaction** transactionPool, pid_t nodePid, int* msgReport)
{
    Transaction** transactionBlock = (Transaction**)malloc(sizeof(Transaction*) * SO_BLOCK_SIZE);
    int i;
    int blockIdx;
    for(blockIdx = 0; blockIdx < SO_BLOCK_SIZE - 1; blockIdx++)
    {
        i = chooseTransaction(transactionPool);
        if (checkTransaction(transactionPool[i]))
        {
            transactionBlock[blockIdx] = (Transaction*) malloc(sizeof(Transaction));
            transactionBlock[blockIdx] = transactionPool[i];
            transactionPool[i] = NULL;
        }
        else
        {
            blockIdx--; //per non lasciare transactionBlock[i] vuoto
        }
    }

    rewardTransaction(transactionBlock, nodePid);
    usleep((rand() % SO_MAX_TRANS_PROC_NSEC + SO_MIN_TRANS_PROC_NSEC) / 1000);
    insertBlock(transactionBlock, transactionPool, nodePid, msgReport);
}

void replyTransaction(BufferTransactionSend* message, int* msgTransactionReplyId, bool res)
{
    int code;
    BufferTransactionReply* messageReply = (BufferTransactionReply*)malloc(sizeof(BufferTransactionReply));
    messageReply->mtype = message->transaction.sender;
    messageReply->result = res;
    code = msgsnd(*msgTransactionReplyId, messageReply, sizeof(BufferTransactionReply), 0);//bloccante
    if(code == -1)
    {
        printf("Error in replyTransaction; sono il nodo %ld e volevo trasmettere all'utente %ld\n", message->mtype, messageReply->mtype);
        exit(EXIT_FAILURE);
    }
}

void* manageTransactions(void* args)
{
    PthreadArguments* arguments = (PthreadArguments*) args;
    int code;
    int i;
    int findPlace = 1;
    BufferTransactionSend* message = (BufferTransactionSend*)malloc(sizeof(BufferTransactionSend));;

    while(1)
    {
        code = msgrcv(*arguments->msgTransactionSendId, message, sizeof(BufferTransactionSend), arguments->nodePid, 0); //bloccante
        findPlace = 1;
        for(i = 0; i < SO_TP_SIZE; i++)
        {
            if(findPlace == 1)
            {
                if(arguments->transactionPool[i] == NULL)
                {
                    arguments->transactionPool[i] = (Transaction*)malloc(sizeof(Transaction)); //TODO:semaforo sulla transactionPool
                    arguments->transactionPool[i] = &message->transaction;
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
    pthread_exit(NULL);
}

void initArgs(PthreadArguments* args, pid_t nodePid, Transaction** transactionPool, int* msgTransactionSendId, int* msgTransactionReplyId)
{
    args->nodePid = nodePid;
    args->transactionPool = transactionPool;
    args->msgTransactionSendId = msgTransactionSendId;
    args->msgTransactionReplyId = msgTransactionReplyId;
}

void syncNode()
{
    size_t up;
    size_t mbp;
    while((up + mbp) == (SO_USERS_NUM + 1))
    {
        up = sizeof(userProcesses) / sizeof(userProcesses[0]);
        mbp = sizeof(masterBookProcess) / sizeof(masterBookProcess[0]);
    }
}

void nodeStart(int* msgTransactionSendId, int* msgTransactionReplyId, int* msgReport)
{
    pid_t nodePid = getpid();
    printf("Creato processo nodo Id: %d\n", nodePid);
    int creationError;

    syncNode();
    Transaction** transactionPool = (Transaction**)malloc(sizeof(Transaction*) * SO_TP_SIZE);
    PthreadArguments* args = (PthreadArguments*)malloc(sizeof(PthreadArguments));
    initArgs(args, nodePid, transactionPool, msgTransactionSendId, msgTransactionReplyId);

    pthread_t transactionPoolManager[1];
    creationError = pthread_create(&transactionPoolManager[0], NULL, manageTransactions, (void*)args);
    if(creationError)
    {
        printf("Errore nella creazione del thread gestore della transactionPool\n");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        createBlock(transactionPool, nodePid, msgReport);
    }

    exit(EXIT_SUCCESS);
}
