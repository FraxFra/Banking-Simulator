#include "config.h"

int userBalance = SO_BUDGET_INIT;
int masterBookIdx = 0;

bool checkTerminationUser()
{
    return termination[0] == 0;
}

pid_t findReceiver(pid_t userPid)
{
    int res = rand() % SO_USERS_NUM;
    while(userProcesses[res] == userPid || userProcesses[res] == 0)
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
    int res = ((amount * SO_REWARD) / 100);
    if(res < 1)
    {
        return 1;
    }
    else
    {
        return res;
    }
}

void createTransaction(Transaction* t, pid_t userPid)
{
    srand(userPid * time(NULL));
    int amount = (rand() % (userBalance - 2 + 1)) + 2;
    t->timestamp = clock();
    t->sender = userPid;
    t->receiver = findReceiver(userPid);
    t->reward = calcReward(amount);
    t->qty = amount - t->reward;
}

BufferTransactionSend* sendTransaction(pid_t userPid, int* msgTransactionSendId)
{
    int code = -1;
    BufferTransactionSend* message = (BufferTransactionSend*)malloc(sizeof(BufferTransactionSend));
    Transaction* t = (Transaction*)malloc(sizeof(Transaction));

    createTransaction(t, userPid);
    message->mtype = findNode();
    message->transaction = *t;
    code = msgsnd(*msgTransactionSendId, message, sizeof(BufferTransactionSend), 0);

    free(t);
    return message;
}

bool getTransactionReply(pid_t userPid, int* msgTransactionReplyId, pid_t node)
{
    int code = -1;
    int res;
    BufferTransactionReply* message = (BufferTransactionReply*)malloc(sizeof(BufferTransactionReply));

    code = msgrcv(*msgTransactionReplyId, message, sizeof(BufferTransactionReply), userPid, 0);
    res = message->result;
    free(message);
    return res;
}

void calcBalance(pid_t userPid)
{
    sem_wait(semRegistry);
    while(masterBookIdx < nBlocksRegistry[0] * SO_BLOCK_SIZE)
    {
        if(masterBookRegistry[masterBookIdx].receiver == userPid)
        {
            userBalance = userBalance + masterBookRegistry[masterBookIdx].qty;
        }
        masterBookIdx++;
    }
    sem_post(semRegistry);
}

void userStart(int* msgTransactionSendId, int* msgTransactionReplyId)
{
    pid_t userPid = getpid();
    int actual_retry = 0;
    int balance = 0;
    BufferTransactionSend* message;

    while(actual_retry <= SO_RETRY)
    {
        if(checkTerminationUser())
        {
            sem_wait(semDeadUsers);
            nTerminatedUsers[0] = nTerminatedUsers[0] + 1;
            sem_post(semDeadUsers);
            exit(EXIT_SUCCESS);
        }

        calcBalance(userPid);
        if(userBalance >= 2)
        {
            message = sendTransaction(userPid, msgTransactionSendId);
            if(getTransactionReply(userPid, msgTransactionReplyId, message->mtype) == 1)
            {
                actual_retry++;
            }
            else
            {
                userBalance = userBalance - message->transaction.qty - message->transaction.reward;
                actual_retry = 0;
            }
            free(message);
        }
        usleep((rand() % SO_MAX_TRANS_GEN_NSEC + SO_MIN_TRANS_GEN_NSEC) / 1000);
    }

    sem_wait(semDeadUsers);
    nDeadUsers[0] = nDeadUsers[0] + 1;
    nTerminatedUsers[0] = nTerminatedUsers[0] + 1;
    sem_post(semDeadUsers);
    exit(EXIT_SUCCESS);
}
