#include "config.h"

Transaction* masterBookRegistry;
pid_t* userProcesses;
pid_t* nodeProcesses;
sem_t* semRegistry;
sem_t* semDeadUsers;
int* nBlocksRegistry;
int* nDeadUsers;
int* nTerminatedUsers;
int* termination;


void mapSharedMemory()
{
    userProcesses = mmap(NULL, SO_USERS_NUM * sizeof(pid_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(!userProcesses)
    {
        perror("la mappatura dei pid utente e' fallita");
        exit(EXIT_FAILURE);
    }

    nodeProcesses = mmap(NULL, SO_NODES_NUM * sizeof(pid_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(!nodeProcesses)
    {
        perror("la mappatura dei pid nodo e' fallita");
        exit(EXIT_FAILURE);
    }

    masterBookRegistry = mmap(NULL, sizeof(Transaction) * SO_REGISTRY_SIZE * SO_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(!masterBookRegistry)
    {
        perror("la mappatura del masterBookRegistry e' fallita");
        exit(EXIT_FAILURE);
    }

    nBlocksRegistry = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(!nBlocksRegistry)
    {
        perror("la mappatura del masterBookBlockLength e' fallita");
        exit(EXIT_FAILURE);
    }

    semRegistry = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(!semRegistry)
    {
        perror("la mappatura del semRegistry e' fallita");
        exit(EXIT_FAILURE);
    }
    sem_init(semRegistry, 1, 1);

    termination = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(!termination)
    {
        perror("la mappatura del termination e' fallita");
        exit(EXIT_FAILURE);
    }
    termination[0] = 1;

    nDeadUsers = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(!nDeadUsers)
    {
        perror("la mappatura del nDeadUsers e' fallita");
        exit(EXIT_FAILURE);
    }
    nDeadUsers[0] = 0;

    nTerminatedUsers = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(!nTerminatedUsers)
    {
        perror("la mappatura del nTerminatedUsers e' fallita");
        exit(EXIT_FAILURE);
    }
    nTerminatedUsers[0] = 0;

    semDeadUsers = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(!semDeadUsers)
    {
        perror("la mappatura del semDeadUsers e' fallita");
        exit(EXIT_FAILURE);
    }
    sem_init(semDeadUsers, 1, 1);
}

void unMapSharedMemory()
{
    int code;

    code = munmap(userProcesses, SO_USERS_NUM * sizeof(pid_t));
    if(code == -1)
    {
        printf("Errore rimuovere la mappa userProcesses");
        exit(EXIT_FAILURE);
    }

    code = munmap(nodeProcesses, SO_NODES_NUM * sizeof(pid_t));
    if(code == -1)
    {
        printf("Errore rimuovere la mappa nodeProcesses");
        exit(EXIT_FAILURE);
    }

    code = munmap(masterBookRegistry, SO_REGISTRY_SIZE * SO_BLOCK_SIZE * sizeof(Transaction));
    if(code == -1)
    {
        printf("Errore rimuovere la mappa masterBookRegistry");
        exit(EXIT_FAILURE);
    }

    code = munmap(nBlocksRegistry, sizeof(int));
    if(code == -1)
    {
        printf("Errore rimuovere la mappa masterBookBlockLength");
        exit(EXIT_FAILURE);
    }

    code = munmap(semRegistry, sizeof(sem_t));
    if(code == -1)
    {
        printf("Errore rimuovere la mappa semRegistry");
        exit(EXIT_FAILURE);
    }

    code = munmap(termination, sizeof(int));
    if(code == -1)
    {
        printf("Errore rimuovere la mappa termination");
        exit(EXIT_FAILURE);
    }

    code = munmap(nDeadUsers, sizeof(int));
    if(code == -1)
    {
        printf("Errore rimuovere la mappa nDeadUsers");
        exit(EXIT_FAILURE);
    }

    code = munmap(nTerminatedUsers, sizeof(int));
    if(code == -1)
    {
        printf("Errore rimuovere la mappa nTerminatedUsers");
        exit(EXIT_FAILURE);
    }

    code = munmap(semDeadUsers, sizeof(sem_t));
    if(code == -1)
    {
        printf("Errore rimuovere la mappa semDeadUsers");
        exit(EXIT_FAILURE);
    }

}

void initBuffers(int* msgTransactionSendId, int* msgTransactionReplyId, int* msgReportNode)
{
    *msgTransactionSendId = msgget(0, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(*msgTransactionSendId == -1)
    {
        printf("Errore nella creazione dell'id msgTransactionSendId\n");
        exit(EXIT_FAILURE);
    }

    *msgTransactionReplyId = msgget(1, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(*msgTransactionReplyId == -1)
    {
        printf("Errore nella creazione dell'id msgTransactionReplyId\n");
        exit(EXIT_FAILURE);
    }

    *msgReportNode = msgget(2, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(*msgReportNode == -1)
    {
        printf("Errore nella creazione dell'id msgReportNode\n");
        exit(EXIT_FAILURE);
    }
}

void deallocBuffers(int* msgTransactionSendId, int* msgTransactionReplyId, int* msgReportNode)
{
    int code;
    code = msgctl(*msgTransactionSendId, IPC_RMID, NULL);
    if(code == -1)
    {
        printf("Errore con la rimozione dell'id msgTransactionSendId\n");
        exit(EXIT_FAILURE);
    }

    code = msgctl(*msgTransactionReplyId, IPC_RMID, NULL);
    if(code == -1)
    {
        printf("Errore con la rimozione dell'id msgTransactionReplyId\n");
        exit(EXIT_FAILURE);
    }

    code = msgctl(*msgReportNode, IPC_RMID, NULL);
    if(code == -1)
    {
        printf("Errore con la rimozione dell'id msgReportNode\n");
        exit(EXIT_FAILURE);
    }
}

void killProcesses()
{

}

void createProcesses(pid_t masterPid, int* msgTransactionSendId, int* msgTransactionReplyId, int* msgReportNode)
{
    pid_t pid;
    int i;

    for(i = 0; i < SO_NODES_NUM; i++)
    {
        if(getpid() == masterPid)
        {
            pid = fork();
            if(pid == -1)
            {
                printf("Errore con la creazione di un processo nodo\n");
                exit(EXIT_FAILURE);
            }
            else if(pid == 0)
            {
                nodeProcesses[i] = getpid();
                nodeStart(msgTransactionSendId, msgTransactionReplyId, msgReportNode);
            }
        }
    }

    for(i = 0; i < SO_USERS_NUM; i++)
    {
        if(getpid() == masterPid)
        {
            pid = fork();
            if(pid == -1)
            {
                printf("Errore con la creazione di un processo utente\n");
                exit(EXIT_FAILURE);
            }
            else if(pid == 0)
            {
                userProcesses[i] = getpid();
                userStart(msgTransactionSendId, msgTransactionReplyId, msgReportNode);
            }
        }
    }
}

int printBalanceNode(pid_t pid)
{

    int res = 0;
    int i;

    for(i = 0; i < nBlocksRegistry[0] * SO_BLOCK_SIZE; i++)
    {
        if(masterBookRegistry[i].sender == -1 && pid == masterBookRegistry[i].receiver)
        {
            res = res + masterBookRegistry[i].qty;
        }
    }
    return res;
}

int printBalanceUser(pid_t pid)
{
    int res = SO_BUDGET_INIT;
    int i;

    for(i = 0; i < nBlocksRegistry[0] * SO_BLOCK_SIZE; i++)
    {
        if(masterBookRegistry[i].sender == pid)
        {
            res = res - masterBookRegistry[i].qty - masterBookRegistry[i].reward;
        }
        else if(masterBookRegistry[i].receiver == pid)
        {
            res = res + masterBookRegistry[i].qty;
        }
        //printf("%d\n",masterBookRegistry[i].qty);
    }
    return res;
}

void printMinMaxBalances()
{
    int i;
    pid_t maxUser;
    pid_t minUser;
    pid_t maxNode;
    pid_t minNode;
    int maxUserBalance = 0;
    int maxNodeBalance = 0;
    int minUserBalance = INT_MAX;
    int minNodeBalance = INT_MAX;
    int curBalance;

    for(i = 0; i < SO_USERS_NUM; i++)
    {
        curBalance = printBalanceUser(userProcesses[i]);
        if(curBalance > maxUserBalance)
        {
            maxUserBalance = curBalance;
            maxUser = userProcesses[i];
        }

        if(curBalance < minUserBalance)
        {
            minUserBalance = curBalance;
            minUser = userProcesses[i];
        }
    }

    for(i = 0; i < SO_NODES_NUM; i++)
    {
        curBalance = printBalanceUser(nodeProcesses[i]);
        if(curBalance > maxNodeBalance)
        {
            maxNodeBalance = curBalance;
            maxNode = nodeProcesses[i];
        }

        if(curBalance < minNodeBalance)
        {
            minNodeBalance = curBalance;
            minNode = nodeProcesses[i];
        }
    }

    printf("L'utente %d ha il bilancio maggiore pari a %d\n", maxUser, maxUserBalance);
    printf("L'utente %d ha il bilancio minore pari a %d\n", minUser, minUserBalance);
    printf("----------------------------------\n" );
    printf("Il nodo %d ha il bilancio maggiore pari a %d\n", maxNode, maxNodeBalance);
    printf("Il nodo %d ha il bilancio minore pari a %d\n", minNode, minNodeBalance);
}

void* printStatus()
{
    while(termination[0] == 1)
    {
        int i, j;

        printf("----------------------------------\n" );
        printf("numero di utenti attivi: %d\n", SO_USERS_NUM - nDeadUsers[0]);
        printf("----------------------------------\n" );
        if(SO_USERS_NUM > 10 || SO_NODES_NUM > 10)
        {
            printMinMaxBalances();
        }
        else
        {
            for(i = 0; i < SO_USERS_NUM ; i++)
            {
                printf("utente %d ha bilancio pari a %d\n", userProcesses[i], printBalanceUser(userProcesses[i]));
            }
            printf("----------------------------------\n" );
            for(i = 0; i < SO_NODES_NUM ; i++)
            {
                printf("nodo %d ha bilancio pari a %d\n", nodeProcesses[i], printBalanceNode(nodeProcesses[i]));
            }
            printf("----------------------------------\n" );
        }
        sleep(1);
        // system("clear");
    }
    pthread_exit(NULL);
}

int setTermination()
{
    clock_t begin = clock();
    clock_t end;
    int res = -1;
    pthread_t printThread[1];
    pthread_create(&printThread[0], NULL, printStatus, NULL);

    while(termination[0] == 1)
    {
      //printf("sono verso la fine %d %d\n",nBlocksRegistry[0],termination[0]);
        end = clock();
        if(((double)(end - begin) / CLOCKS_PER_SEC) >= SO_SIM_SEC && res == -1)
        {
            termination[0] = 0;
            res = 0; //0 -> termination by time

        }
        else if((nBlocksRegistry[0]* SO_BLOCK_SIZE == SO_BLOCK_SIZE * SO_REGISTRY_SIZE) && res == -1)
        {
            termination[0] = 0;
            res = 1; //1 -> termination by size
        }
        else if(res == -1)
        {
            if(nDeadUsers[0] == SO_USERS_NUM)
            {
                termination[0] = 0;
                res = 2; //2 -> termination by users
            }
        }
    }
    return res;
}

int checkDuplicates()
{
    int i;
    int j;
    int res = 1;

    for(i = 0; i < nBlocksRegistry[0] * SO_BLOCK_SIZE; i++)
    {
        for(j = 0; j < nBlocksRegistry[0] * SO_BLOCK_SIZE; j++)
        {
            if(masterBookRegistry[i].timestamp == masterBookRegistry[j].timestamp
            && masterBookRegistry[i].sender == masterBookRegistry[j].sender
            && masterBookRegistry[i].receiver == masterBookRegistry[j].receiver
            && j != i)
            {
                res = 0;
            }
        }
    }
    return res;
}

void reasume(int terminationReason, int* msgReportNode)
{
    BufferReportNode* message = (BufferReportNode*)malloc(sizeof(BufferReportNode));
    int i;
    int res;

    if(SO_USERS_NUM > 10 || SO_NODES_NUM > 10)
    {
        printMinMaxBalances();
    }
    else
    {
        if(masterBookRegistry != NULL && nBlocksRegistry != NULL)
        {
            for(i = 0; i < SO_USERS_NUM ; i++)
            {
                printf("utente %d ha bilancio pari a %d\n", userProcesses[i], printBalanceUser(userProcesses[i]));
            }
            printf("----------------------------------\n" );
            for(i = 0; i < SO_NODES_NUM ; i++)
            {
                printf("nodo %d ha bilancio pari a %d\n", nodeProcesses[i], printBalanceNode(nodeProcesses[i]));
            }
            printf("----------------------------------\n" );
        }
    }

    printf("----------------------------------\n" );
    switch(terminationReason)
    {
        case 0:
        printf("Il programma e' terminato per fine tempo di esecuzione\n" );
        break;

        case 1:
        printf("Il programma e' terminato per esaurimento di spazio nel libro mastro\n" );
        break;

        case 2:
        printf("Il programma e' terminato a causa della terminazione di tutti gli utenti\n" );
        break;
    }
    printf("----------------------------------\n" );
    printf("il numero di processi utente terminati prematuramente e' %d\n", nDeadUsers[0]);
    printf("----------------------------------\n" );
    printf("il numero di blocchi nel libro mastro e' %d\n", nBlocksRegistry[0]);
    printf("----------------------------------\n" );

    for(i = 0; i < SO_NODES_NUM; i++)
    {
        msgrcv(*msgReportNode, message, sizeof(BufferReportNode), 0, 0);
        printf("il nodo %ld ha %d transazioni rimanenti nella transactionPool\n", message->mtype, message->nTransactions);
    }
    printf("----------------------------------\n" );

    /*for(i = 0; i < SO_BLOCK_SIZE * nBlocksRegistry[0]; i++)
    {
        printf("%ld %d %d\n", masterBookRegistry[i].timestamp, masterBookRegistry[i].sender, masterBookRegistry[i].receiver);
    }*/
}

void masterStart()
{
    pid_t masterPid = getpid();
    int msgTransactionSendId;
    int msgTransactionReplyId;
    int msgReportNode;
    int terminationReason;

    mapSharedMemory();
    initBuffers(&msgTransactionSendId, &msgTransactionReplyId, &msgReportNode);
    createProcesses(masterPid, &msgTransactionSendId, &msgTransactionReplyId, &msgReportNode);
    terminationReason = setTermination();
    while(wait(NULL) > 0);
    printf("Fine simulazione\n");
    sleep(2);
    reasume(terminationReason, &msgReportNode);
    deallocBuffers(&msgTransactionSendId, &msgTransactionReplyId, &msgReportNode);
    unMapSharedMemory();
    killProcesses();
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])
{
    pid_t pid = fork();

    if(pid == -1)
    {
        printf("Errore nella creazione del processo master\n");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        masterStart();
        exit(EXIT_SUCCESS);
    }
    else
    {
        while(wait(NULL) > 0);
        return 0;
    }
}
