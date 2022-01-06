#include "config.h"


void insertBlock(Transaction** transactionBlock)
{

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

void createBlock(Transaction** transactionPool, pid_t nodePid, int* msgBlockSendId, int* msgBlockReplyId)
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
            //TODO: eliminare transactionpool[i]
        }
        else
        {
            blockIdx--; //per non lasciare transactionBlock[i] vuoto
        }
    }

    rewardTransaction(transactionBlock, nodePid);
    usleep((rand() % SO_MAX_TRANS_PROC_NSEC + SO_MIN_TRANS_PROC_NSEC) / 1000);
    insertBlock(transactionBlock);
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

void nodeStart(int* msgTransactionSendId, int* msgTransactionReplyId, int* msgBlockSendId, int* msgBlockReplyId)
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
        createBlock(transactionPool, nodePid, msgBlockSendId, msgBlockReplyId);
    }

    exit(EXIT_SUCCESS);
}
