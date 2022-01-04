#include "config.h"

int calcBalance(pid_t userPid)
{
    int amountTransactions = 0;
    int i, j;

    if(masterBookRegistry != NULL)
    {
        for(i = 0; i < SO_REGISTRY_SIZE * SO_BLOCK_SIZE ; i++)
        {
            for(j = 0; j < masterBookBlockLength[i]; j++)
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

Transaction createTransaction(Transaction* t, pid_t userPid, int balance)
{
    srand(time(0));
    int amount = rand() % balance + 1;
    t->timestamp = clock();
    t->sender = userPid;
    t->receiver = findReceiver(userPid);
    t->reward = calcReward(amount);
    t->qty = amount - t->reward;
}

void *userStart()
{
    pid_t userPid = getpid();
    printf("Creato processo utente Id: %d\n", userPid);
    int actual_retry = 0;
    int msgid = -1;

    //sleep(2);
    //while(actual_retry <= SO_RETRY)
    //{
        int balance = calcBalance(userPid);
        if(balance >= 2)
        {
            Transaction* t = malloc(sizeof(Transaction));
            createTransaction(t, userPid, balance);
            pid_t node = findNode();
            while(msgid == 0)
            {
                //msgid = msgget(node, 0);
                usleep(500000000);
            }
            //int a = msgsnd(msgid, t, sizeof(Transaction), IPC_NOWAIT);
            //incrementare actual_retry in caso negativo
        }
        //usleep((rand() % SO_MAX_TRANS_GEN_NSEC) + SO_MIN_TRANS_GEN_NSEC);
    //}
    //se ci si trova qui allora il processo per SO_RETRY volte non è riuscito a portare a termine la transazione -> deve terminare
    exit(EXIT_SUCCESS);
}
