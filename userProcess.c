#include "config.h"

void sendReportUser(pid_t userPid, int* msgReportUser)
{
    int code;
    BufferReportUser* message = (BufferReportUser*)malloc(sizeof(BufferReportUser));

    message->mtype = userPid;
    message->dead = true;

    code = msgsnd(*msgReportUser, message, sizeof(BufferReportUser), 0);
    if(code == -1)
    {
        printf("Error in msgReportUser; sono il nodo %d e volevo trasmettere al master il report\n", userPid);
        exit(EXIT_FAILURE);
    }
    //printf("%d terminato utente\n", userPid);
    exit(EXIT_SUCCESS);
}

bool checkTerminationUser()
{
    return termination[0] == 0;
}

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
    int res = rand() % SO_NODES_NUM;
    while(nodeProcesses[res] == 0)
    {
            res = rand() % SO_NODES_NUM;
    }
    return nodeProcesses[res];
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

pid_t sendTransaction(pid_t userPid, int balance, int* msgTransactionSendId, int* msgReportUser)
{
    int code = -1;
    BufferTransactionSend* message = (BufferTransactionSend*)malloc(sizeof(BufferTransactionSend));
    Transaction* t = (Transaction*)malloc(sizeof(Transaction));

    if(checkTerminationUser())
    {
        sendReportUser(userPid, msgReportUser);
    }

    createTransaction(t, userPid, balance);
    message->mtype = findNode();
    message->transaction = *t;

    while(code == -1)
    {
        code = msgsnd(*msgTransactionSendId, message, sizeof(BufferTransactionSend), IPC_NOWAIT);

        if(checkTerminationUser())
        {
            sendReportUser(userPid, msgReportUser);
        }
    }
    return message->mtype;
}

bool getTransactionReply(pid_t userPid, int* msgTransactionReplyId, pid_t node, int* msgReportUser)
{
    int code = -1;
    BufferTransactionReply* message = (BufferTransactionReply*)malloc(sizeof(BufferTransactionReply));

    if(checkTerminationUser())
    {
        sendReportUser(userPid, msgReportUser);
    }

    while(code == -1)
    {
        code = msgrcv(*msgTransactionReplyId, message, sizeof(BufferTransactionReply), userPid, IPC_NOWAIT);

        if(checkTerminationUser())
        {
            sendReportUser(userPid, msgReportUser);
        }
    }
    return message->result;
}

void userStart(int* msgTransactionSendId, int* msgTransactionReplyId, int* msgReportUser)
{
    pid_t userPid = getpid();
    //printf("Creato processo utente Id: %d\n", userPid);
    int actual_retry = 0;
    int balance;
    pid_t node;

    while(actual_retry <= SO_RETRY)
    {
        balance = calcBalance(userPid);
        if(balance >= 2)
        {
            node = sendTransaction(userPid, balance, msgTransactionSendId, msgReportUser);
            if(getTransactionReply(userPid, msgTransactionReplyId, node, msgReportUser) == 1)
            {
                actual_retry++;
            }
            else
            {
                actual_retry = 0;
            }
        }

        if(checkTerminationUser())
        {
            sendReportUser(userPid, msgReportUser);
        }
        usleep((rand() % SO_MAX_TRANS_GEN_NSEC + SO_MIN_TRANS_GEN_NSEC) / 1000);
    }
    //se ci si trova qui allora il processo per SO_RETRY volte non è riuscito a portare a termine la transazione -> deve terminare
    //printf("%d terminato utente dead\n", userPid);
    sem_wait(semDeadUsers);
    deadUsers[0] = deadUsers[0] + 1;
    sem_post(semDeadUsers);

    exit(EXIT_SUCCESS);
}
