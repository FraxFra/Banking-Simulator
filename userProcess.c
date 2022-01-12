#include "config.h"

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
    if(((amount * SO_REWARD) / 100)<1)
      return 1;
    else return (amount * SO_REWARD) / 100;
}

void createTransaction(Transaction* t, pid_t userPid, int balance)
{
    srand(userPid * time(NULL));
    int amount = rand() % balance + 2;
    t->timestamp = clock();
    t->sender = userPid;
    t->receiver = findReceiver(userPid);
    t->reward = calcReward(amount);
    t->qty = amount - t->reward;
}

BufferTransactionSend* sendTransaction(pid_t userPid, int balance, int* msgTransactionSendId, int* msgReportUser)
{
    int code = -1;
    BufferTransactionSend* message = (BufferTransactionSend*)malloc(sizeof(BufferTransactionSend));
    Transaction* t = (Transaction*)malloc(sizeof(Transaction));

    if(checkTerminationUser())
    {
        exit(EXIT_SUCCESS);
    }

    createTransaction(t, userPid, balance);
    message->mtype = findNode();
    message->transaction = *t;

//    while(code == -1)
  //  {
        code = msgsnd(*msgTransactionSendId, message, sizeof(BufferTransactionSend),0);

        if(checkTerminationUser())
        {
            printf("morto utente\n" );
            exit(EXIT_SUCCESS);
        }
    //}
    return message;
}

bool getTransactionReply(pid_t userPid, int* msgTransactionReplyId, pid_t node, int* msgReportUser)
{
    int code = -1;
    int res;
    if(checkTerminationUser())
    {
        exit(EXIT_SUCCESS);
    }

    BufferTransactionReply* message = (BufferTransactionReply*)malloc(sizeof(BufferTransactionReply));
    while(code == -1)
    {
        code = msgrcv(*msgTransactionReplyId, message, sizeof(BufferTransactionReply), userPid, IPC_NOWAIT);

        if(checkTerminationUser())
        {
            exit(EXIT_SUCCESS);
        }
    }
    res = message->result;
    //deallocare message
    return res;
}

void allocArrTransaction(Transaction** arrTransaction)
{
    int i;
    for(i = 0; i < 100; i++)
    {
        arrTransaction[i] = (Transaction*)malloc(sizeof(Transaction));
    }
}

void insertArrTransaction(Transaction** arrTransaction, BufferTransactionSend* message)
{
    int i;
    int findPlace = 1;

    for(i = 0; i < 100; i++)
    {
        if(arrTransaction[i] != NULL && findPlace == 1)
        {
            arrTransaction[i] = &message->transaction;
            findPlace = 0;
        }
    }
    if(findPlace == 1)
    {
        printf("spazio esaurito\n" );
    }
}

int calcBalance(Transaction** arrTransaction, pid_t userPid)
{
    int res = SO_BUDGET_INIT;
    int i;
    int j;

    sem_wait(semRegistry);
    for(i = 0; i < nblocksRegistry[0] * SO_BLOCK_SIZE; i++)
    {
        if(masterBookRegistry[i].sender == userPid)
        {
            res = res - masterBookRegistry[i].qty - masterBookRegistry[i].reward;
            for(j = 0; j < 100; j++)
            {
              if(arrTransaction[j]!=NULL)
              {
                if(masterBookRegistry[i].sender == arrTransaction[j]->sender
                && masterBookRegistry[i].receiver == arrTransaction[j]->receiver
                && masterBookRegistry[i].timestamp == arrTransaction[j]->timestamp)
                {
                    arrTransaction[j] = NULL;
                }
            }
          }
        }
         if(masterBookRegistry[i].receiver == userPid)
        {
            res = res + masterBookRegistry[i].qty;
            for(j = 0; j < 100; j++)
            {
              if(arrTransaction[j]!=NULL)
              {
                if(masterBookRegistry[i].sender == arrTransaction[j]->sender
                && masterBookRegistry[i].receiver == arrTransaction[j]->receiver
                && masterBookRegistry[i].timestamp == arrTransaction[j]->timestamp)
                {
                    arrTransaction[j] = NULL;
                }
              }
            }
        }
    }

    for(j = 0; j < 100; j++)
    {
        if(arrTransaction[j] != NULL)
        {
            if(arrTransaction[j]->sender == userPid)
            {
                res = res - arrTransaction[j]->qty - arrTransaction[j]->reward;
            }
            if(arrTransaction[j]->receiver == userPid)
            {
                res = res + arrTransaction[j]->qty;
            }
        }
    }
    sem_post(semRegistry);
    return res;
}

void userStart(int* msgTransactionSendId, int* msgTransactionReplyId, int* msgReportUser, int index)
{
    pid_t userPid = getpid();
    int actual_retry = 0;
    int balance = 0;
    Transaction** arrTransaction = (Transaction**)malloc(sizeof(Transaction*) * 100);
    BufferTransactionSend* message = (BufferTransactionSend*)malloc(sizeof(BufferTransactionSend));
    allocArrTransaction(arrTransaction);

    while(actual_retry <= SO_RETRY)
    {
        balance = calcBalance(arrTransaction, userPid);
        //printf("%d %d\n", balance, userPid);
        if(balance >= 2)
        {
            message = sendTransaction(userPid, balance, msgTransactionSendId, msgReportUser);
            //printf("%d userpid %d rec %d qty %d rew ricevuti\n", userPid, message->transaction.receiver, message->transaction.qty, message->transaction.reward);
            if(getTransactionReply(userPid, msgTransactionReplyId, message->mtype, msgReportUser) == 1)
            {
                actual_retry++;
            }
            else
            {
                actual_retry = 0;
                insertArrTransaction(arrTransaction, message);
            }
        }

        if(checkTerminationUser())
        {
            exit(EXIT_SUCCESS);
        }

        usleep((rand() % SO_MAX_TRANS_GEN_NSEC + SO_MIN_TRANS_GEN_NSEC) / 1000);
    }
    sem_wait(semDeadUsers);
    deadUsers[0] = deadUsers[0] + 1;
    sem_post(semDeadUsers);

    exit(EXIT_SUCCESS);
}
