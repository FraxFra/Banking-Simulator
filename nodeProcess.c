#include "config.h"
/*
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
*/

void replyTransaction(BufferTransactionSend* message, int* msgTransactionReplyId, bool res)
{
    int code;
    BufferTransactionReply* messageReply = (BufferTransactionReply*)malloc(sizeof(BufferTransactionReply));
    messageReply->mtype = message->transaction.sender;
    messageReply->result = res;
    code = msgsnd(*msgTransactionReplyId, messageReply, sizeof(BufferTransactionReply), 0);
    if(code == -1)
    {
        printf("Error in replyTransaction; sono il nodo %ld e volevo trasmettere all'utente %ld\n", message->mtype, messageReply->mtype);
        exit(EXIT_FAILURE);
    }
}
void receiveTransactions(pid_t nodePid, Transaction* transactionPool, int* msgTransactionSendId, int* msgTransactionReplyId)
{
    int code;
    int i;
    BufferTransactionSend *message = (BufferTransactionSend*)malloc(sizeof(BufferTransactionSend));;

    for(i = 0; i < SO_TP_SIZE; i++)
    {
        while(code == -1)
        {
            code = msgrcv(*msgTransactionSendId, message, sizeof(BufferTransactionSend), nodePid, IPC_NOWAIT);
        }
        transactionPool[i] = message->transaction;
        replyTransaction(message, msgTransactionReplyId, 0);
    }
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

void *nodeStart(int* msgTransactionSendId, int* msgTransactionReplyId, int* msgBlockSendId, int* msgBlockReplyId)
{
    pid_t nodePid = getpid();
    printf("Creato processo nodo Id: %d\n", nodePid);

    syncNode();
    Transaction* transactionPool = (Transaction*)malloc(sizeof(Transaction) * SO_TP_SIZE);
    //while
    //{
        receiveTransactions(nodePid, transactionPool, msgTransactionSendId, msgTransactionReplyId);
    //}
    usleep((rand() % SO_MAX_TRANS_PROC_NSEC + SO_MIN_TRANS_PROC_NSEC) / 1000);
    //spedisce il blocco al libro mastro

    exit(EXIT_SUCCESS);
}
