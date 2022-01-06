#include "config.h"

int calcBalance(pid_t userPid)
{
    int amountTransactions = 0;
    int i, j;

    if(masterBookRegistry != NULL && nblocksRegistry != NULL)
    {
        for(i = 0; i < nblocksRegistry[0] * SO_BLOCK_SIZE ; i++)
        {
            if(masterBookRegistry[i].sender == userPid)
            {
                amountTransactions = amountTransactions - masterBookRegistry[i].qty - masterBookRegistry[i].reward;
            }
            else if(masterBookRegistry[i].receiver == userPid)
            {
                amountTransactions = amountTransactions + masterBookRegistry[i].qty;
            }
        }
    }
    return SO_BUDGET_INIT + amountTransactions;
}

pid_t findReceiver(pid_t userPid)
{
    int res = rand() % SO_USERS_NUM;
    while(userProcesses[res] == userPid)
    {
            res = rand() % SO_USERS_NUM;
    }
    return userProcesses[res];
}

pid_t findNode()
{
    return nodeProcesses[(rand() % SO_NODES_NUM)];
}

int calcReward(int amount)
{
    return (amount * SO_REWARD) / 100;
}

void createTransaction(Transaction* t, pid_t userPid, int balance)
{
    srand(userPid * time(NULL));
    int amount = rand() % balance + 1;
    t->timestamp = clock();
    t->sender = userPid;
    t->receiver = findReceiver(userPid);
    t->reward = calcReward(amount);
    t->qty = amount - t->reward;
}

pid_t sendTransaction(pid_t userPid, int balance, int* msgTransactionSendId)
{
    int code;
    BufferTransactionSend* message = (BufferTransactionSend*)malloc(sizeof(BufferTransactionSend));
    Transaction* t = (Transaction*)malloc(sizeof(Transaction));

    createTransaction(t, userPid, balance);
    message->mtype = findNode();
    message->transaction = *t;

    code = msgsnd(*msgTransactionSendId, message, sizeof(BufferTransactionSend), 0); //bloccante
    if(code == -1)
    {
        printf("Error in sendTransaction; sono l'utente %d e volevo trasmettere al nodo %ld\n", userPid, message->mtype);
        exit(EXIT_FAILURE);
    }
    return message->mtype;
}

bool getTransactionReply(pid_t userPid, int* msgTransactionReplyId, pid_t node)
{
    int code;
    BufferTransactionReply* message = (BufferTransactionReply*)malloc(sizeof(BufferTransactionReply));

    code = msgrcv(*msgTransactionReplyId, message, sizeof(BufferTransactionReply), userPid, 0);//bloccante
    if(code == -1)
    {
        printf("Error in getTransactionReply; sono l'utente %d e ricevere una risposta dal nodo %ld\n", userPid, message->mtype);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("%d user %ld node ok!\n", userPid, message->mtype); //da rimuovere
    }
    return message->result;
}

void syncUser()
{
    size_t np;
    size_t mbp;
    while((np + mbp) == (SO_NODES_NUM + 1))
    {
        np = sizeof(nodeProcesses) / sizeof(nodeProcesses[0]);
        mbp = sizeof(masterBookProcess) / sizeof(masterBookProcess[0]);
    }
}

void userStart(int* msgTransactionSendId, int* msgTransactionReplyId)
{
    pid_t userPid = getpid();
    printf("Creato processo utente Id: %d\n", userPid);
    int actual_retry = 0;
    int balance;
    pid_t node;

    syncUser();
    while(actual_retry <= SO_RETRY)
    {
        balance = calcBalance(userPid);
        if(balance >= 2)
        {
            node = sendTransaction(userPid, balance, msgTransactionSendId);
            if(getTransactionReply(userPid, msgTransactionReplyId, node) == 1)
            {
                actual_retry++;
            }
            else
            {
                actual_retry = 0;
            }
        }
        usleep((rand() % SO_MAX_TRANS_GEN_NSEC + SO_MIN_TRANS_GEN_NSEC) / 1000);
    }
    //se ci si trova qui allora il processo per SO_RETRY volte non è riuscito a portare a termine la transazione -> deve terminare
    exit(EXIT_SUCCESS);
}
