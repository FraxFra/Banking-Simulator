#include "config.h"

int calcBalance(pid_t userPid)
{
    int amountTransactions = 0;
    int i, j;

    if(masterBookRegistry != NULL)
    {
        for(i = 0; i < SO_REGISTRY_SIZE; i++)
        {
            if(masterBookRegistry[i] != NULL)
            {
                for(j = 0; j < SO_BLOCK_SIZE; j++)
                {
                    if(masterBookRegistry[i][j] != NULL)
                    {
                        if(masterBookRegistry[i][j]->sender == userPid)
                        {
                                amountTransactions = amountTransactions - masterBookRegistry[i][j]->qty - masterBookRegistry[i][j]->reward;
                        }
                        else if(masterBookRegistry[i][j]->receiver == userPid)
                        {
                                amountTransactions = amountTransactions + masterBookRegistry[i][j]->qty;
                        }
                    }
                }
            }
        }
    }
    return SO_BUDGET_INIT + amountTransactions;
}

pid_t findReceiver(pid_t userPid)
{
    int res = rand() % SO_USERS_NUM;
    printf("%d", *usersProcesses[res]);
    printf("%d", userPid);
    while(*usersProcesses[res] == userPid)
    {
            res = rand() % SO_USERS_NUM;

            printf("aa");
    }
    return *usersProcesses[res];
}

pid_t findNode()
{
    return *nodeProcesses[(rand() % SO_NODES_NUM)];
}

int calcReward(int amount)
{
    return (amount * SO_REWARD) / 100;
}

void createTransaction(Transaction* t, pid_t userPid, int balance)
{
    srand(time(0));
    int amount = rand() % balance + 1;
    t->timestamp = clock();
    t->sender = userPid;
    t->reward = calcReward(amount);
    t->qty = amount - t->reward;
    t->receiver = findReceiver(userPid);
}

void* userStart()
{
    pid_t userPid = getpid();
    printf("Creato processo utente Id: %d\n", userPid);
    int actual_retry = 0;
    int msgid = -1;

    while(actual_retry <= SO_RETRY)
    {
        int balance = calcBalance(userPid);
        if(balance >= 2)
        {
            Transaction* t = (Transaction*)malloc(sizeof(Transaction));
            createTransaction(t, userPid, balance);
            pid_t node = findNode();
            while(msgid == -1)
            {
                msgid = msgget(node, 0);
                printf("%d utente\n", msgid);
                sleep(1);
            }
            printf("aaaaaaaa" );
            int a = msgsnd(msgget(1, IPC_CREAT | S_IRUSR | S_IWUSR), t, sizeof(t), IPC_NOWAIT);
            usleep((rand() % SO_MAX_TRANS_GEN_NSEC) + SO_MIN_TRANS_GEN_NSEC);
            //incrementare actual_retry in caso negativo
        }
    }
    //se ci si trova qui allora il processo per SO_RETRY volte non è riuscito a portare a termine la transazione -> deve terminare
    exit(EXIT_SUCCESS);
}
