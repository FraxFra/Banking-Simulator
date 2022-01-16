#include "config.h"

#define ARR_DIM 100

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

void createTransaction(Transaction* t, pid_t userPid, int balance)
{
    srand(userPid * time(NULL));
    int amount = (rand() % (balance-2+1))+2;
    // amount /= 10;
    // printf("usr:%d balance:%d amount:%d\n", userPid,balance,amount);
    t->timestamp = clock();
    t->sender = userPid;
    t->receiver = findReceiver(userPid);
    t->reward = calcReward(amount);
    t->qty = amount - t->reward;
    // printf("Balance corrente: %d\n", balance);
    // sleep(3);
}

BufferTransactionSend* sendTransaction(pid_t userPid, int balance, int* msgTransactionSendId)
{
    int code = -1;
    BufferTransactionSend* message = (BufferTransactionSend*)malloc(sizeof(BufferTransactionSend));
    Transaction* t = (Transaction*)malloc(sizeof(Transaction));

    createTransaction(t, userPid, balance);
    message->mtype = findNode();
    message->transaction = *t;
    code = msgsnd(*msgTransactionSendId, message, sizeof(BufferTransactionSend), 0);
    //printf("Ho inviato %d balance: %d qty:%d\n",userPid,balance,t->qty);
    free(t);
    free(message);
    return message;
}

bool getTransactionReply(pid_t userPid, int* msgTransactionReplyId, pid_t node)
{
    int code = -1;
    int res;
    BufferTransactionReply* message = (BufferTransactionReply*)malloc(sizeof(BufferTransactionReply));

    code = msgrcv(*msgTransactionReplyId, message, sizeof(BufferTransactionReply), userPid, 0);
    res = message->result;
    //printf("Ho ricevuto %d\n",userPid);
    return res;
}

void insertArrTransaction(Transaction* arrTransaction, BufferTransactionSend* message)
{
    int i = 0;
    int findPlace = 1;

    for(i = 0; i < ARR_DIM; i++)
    {
        if(arrTransaction[i].timestamp == -1 && findPlace == 1)
        {
            arrTransaction[i] = message->transaction;
            findPlace = 0;
        }
    }
    if(findPlace == 1)
    {
        printf("spazio esaurito\n" );
    }
}

void initArrTransaction(Transaction* arrTransaction)
{
    int i;
    for(i = 0; i < ARR_DIM; i++)
    {
        arrTransaction[i].timestamp = -1;
    }
}

// int calcBalance(Transaction* arrTransaction, pid_t userPid)
void calcBalance(Transaction* arrTransaction, pid_t userPid)
{
  // int res = SO_BUDGET_INIT;
    int res = userBalance;
    int i = 0;
    int j = 0;

    sem_wait(semRegistry);
    // for(i = 0; i < nBlocksRegistry[0] * SO_BLOCK_SIZE; i++)
    for(i = masterBookIdx; i < nBlocksRegistry[0] * SO_BLOCK_SIZE; i++)
    {
        if(masterBookRegistry[i].sender == userPid)
        {
            // res = res - masterBookRegistry[i].qty - masterBookRegistry[i].reward;
            for(j = 0; j < ARR_DIM; j++)
            {
                if(masterBookRegistry[i].timestamp == arrTransaction[j].timestamp
                && masterBookRegistry[i].receiver == arrTransaction[j].receiver
                && masterBookRegistry[i].sender == arrTransaction[j].sender)
                {
                    arrTransaction[j].timestamp = -1;
                }
            }
        }

        if(masterBookRegistry[i].receiver == userPid)
        {
            res = res + masterBookRegistry[i].qty;
            for(j = 0; j < ARR_DIM; j++)
            {
                if(masterBookRegistry[i].timestamp == arrTransaction[j].timestamp
                && masterBookRegistry[i].receiver == arrTransaction[j].receiver
                && masterBookRegistry[i].sender == arrTransaction[j].sender)
                {
                    arrTransaction[j].timestamp = -1;
                }
            }
        }
    }

    for(j = 0; j < ARR_DIM; j++)
    {
        if(arrTransaction[j].timestamp != -1)
        {
            if(arrTransaction[j].sender == userPid)
            {
                res = res - arrTransaction[j].qty - arrTransaction[j].reward;
            }
        }
    }

    sem_post(semRegistry);
    masterBookIdx = nBlocksRegistry[0] * SO_BLOCK_SIZE;
    userBalance = res;
    // printf("%d\n",userBalance);
    // sleep(2);
    // return res;
}

void userStart(int* msgTransactionSendId, int* msgTransactionReplyId)
{
    pid_t userPid = getpid();
    int actual_retry = 0;
    int balance = 0;
    Transaction* arrTransaction = (Transaction*)malloc(sizeof(Transaction) * ARR_DIM);
    BufferTransactionSend* message;
    initArrTransaction(arrTransaction);

    while(actual_retry <= SO_RETRY)
    {
        if(checkTerminationUser())
        {
            free(arrTransaction);
            sem_wait(semDeadUsers);
            nTerminatedUsers[0] = nTerminatedUsers[0] + 1;
            sem_post(semDeadUsers);
            exit(EXIT_SUCCESS);
        }

        // balance = calcBalance(arrTransaction, userPid);
        calcBalance(arrTransaction, userPid);
        // if(balance >= 2)
        if(userBalance >= 2)
        {
          // message = sendTransaction(userPid, balance, msgTransactionSendId);
            message = sendTransaction(userPid, userBalance, msgTransactionSendId);
            if(getTransactionReply(userPid, msgTransactionReplyId, message->mtype) == 1)
            {
                actual_retry++;
            }
            else
            {
                // printf("prima di decremento balance %d %d\n",userPid,userBalance);
                userBalance -= message->transaction.qty + message->transaction.reward;
                // printf("balance %d %d\n",userPid,userBalance);
                actual_retry = 0;
                insertArrTransaction(arrTransaction, message);
            }
        }
        else
        {
            // printf("balance %d %d\n",userPid,userBalance);
            sleep(1);
        }
        // printf("balance %d %d\n",userPid,balance);
        usleep((rand() % SO_MAX_TRANS_GEN_NSEC + SO_MIN_TRANS_GEN_NSEC) / 1000);
    }

    sem_wait(semDeadUsers);
    free(arrTransaction);
    nDeadUsers[0] = nDeadUsers[0] + 1;
    nTerminatedUsers[0] = nTerminatedUsers[0] + 1;
    sem_post(semDeadUsers);
    exit(EXIT_SUCCESS);


}
