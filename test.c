#include "config.h"

Transaction* masterBookRegistry;
int* nblocksRegistry;
pid_t* userProcesses;
pid_t* nodeProcesses;
sem_t* semRegistry;
sem_t* semDeadUsers;
int* nblocksRegistry;
int* termination;
int* deadUsers;

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

    nblocksRegistry = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(!nblocksRegistry)
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

    deadUsers = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(!deadUsers)
    {
        perror("la mappatura del deadUsers e' fallita");
        exit(EXIT_FAILURE);
    }
    deadUsers[0] = 0;

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

    code = munmap(nblocksRegistry, sizeof(int));
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

    code = munmap(deadUsers, sizeof(int));
    if(code == -1)
    {
        printf("Errore rimuovere la mappa deadUsers");
        exit(EXIT_FAILURE);
    }

    code = munmap(semDeadUsers, sizeof(sem_t));
    if(code == -1)
    {
        printf("Errore rimuovere la mappa semDeadUsers");
        exit(EXIT_FAILURE);
    }

}

void initBuffers(int* msgTransactionSendId, int* msgTransactionReplyId, int* msgReportUser, int* msgReportNode)
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

    *msgReportUser = msgget(2, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(*msgReportUser == -1)
    {
        printf("Errore nella creazione dell'id msgReportUser\n");
        exit(EXIT_FAILURE);
    }

    *msgReportNode = msgget(3, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(*msgReportNode == -1)
    {
        printf("Errore nella creazione dell'id msgReportNode\n");
        exit(EXIT_FAILURE);
    }
}

void deallocBuffers(int* msgTransactionSendId, int* msgTransactionReplyId, int* msgReportUser, int* msgReportNode)
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

    code = msgctl(*msgReportUser, IPC_RMID, NULL);
    if(code == -1)
    {
        printf("Errore con la rimozione dell'id msgReportUser\n");
        exit(EXIT_FAILURE);
    }

    code = msgctl(*msgReportNode, IPC_RMID, NULL);
    if(code == -1)
    {
        printf("Errore con la rimozione dell'id msgReportNode\n");
        exit(EXIT_FAILURE);
    }
}

void createProcesses(pid_t masterPid, int* msgTransactionSendId, int* msgTransactionReplyId, int* msgReportUser, int* msgReportNode)
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
                nodeStart(msgTransactionSendId, msgTransactionReplyId, msgReportNode, i);
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
                //deallocare p
                userStart(msgTransactionSendId, msgTransactionReplyId, msgReportUser, i);
            }
        }
    }
}

int printBalanceNode(pid_t pid)
{
    int res = 0;
    int i;

    for(i = 0; i < nblocksRegistry[0] * SO_BLOCK_SIZE; i++)
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

    for(i = 0; i < nblocksRegistry[0] * SO_BLOCK_SIZE; i++)
    {
        if(masterBookRegistry[i].sender == pid)
        {
            res = res - masterBookRegistry[i].qty - masterBookRegistry[i].reward;
        }
        else if(masterBookRegistry[i].receiver == pid)
        {
            res = res + masterBookRegistry[i].qty;
        }
    }
    return res;
}

void* printStatus()
{
    while(termination[0] == 1)
    {
        int i, j;

        printf("----------------------------------\n" );
        printf("numero di utenti attivi: %d\n", SO_USERS_NUM - deadUsers[0]);
        printf("----------------------------------\n" );
        if(masterBookRegistry != NULL && nblocksRegistry != NULL)
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
        //system("clear");
    }
}

int setTermination(int* terminationReason, int* msgReportUser)
{
    clock_t begin = clock();
    clock_t end;
    pthread_t printThread[1];
    pthread_create(&printThread[0], NULL, printStatus, NULL);

    while(termination[0] == 1)
    {
        end = clock();
        if(((double)(end - begin) / CLOCKS_PER_SEC) >= SO_SIM_SEC && *terminationReason == -1)
        {
            termination[0] = 0;
            //printf("time\n");
            *terminationReason = 0; //0 -> termination by time
        }
        else if((nblocksRegistry[0] * SO_BLOCK_SIZE == SO_BLOCK_SIZE * SO_REGISTRY_SIZE) && *terminationReason == -1)
        {
            termination[0] = 0;
            //printf("size\n");
            *terminationReason = 1; //1 -> termination by size
        }
        else if(*terminationReason == -1)
        {
            //deadUsers[0] = deadUsers[0] + checkUsersTermination(msgReportUser);
            if(deadUsers[0] == SO_USERS_NUM)
            {
                termination[0] = 0;
                //printf("users\n");
                *terminationReason = 2; //2 -> termination by users
            }
        }


    }
}

void masterStart()
{
    pid_t masterPid = getpid();
    //printf("Creato processo master: %d\n", masterPid);

    int msgTransactionSendId;
    int msgTransactionReplyId;
    int msgReportUser;
    int msgReportNode;
    int terminationReason = -1;

    mapSharedMemory();
    initBuffers(&msgTransactionSendId, &msgTransactionReplyId, &msgReportUser, &msgReportNode);
    createProcesses(masterPid, &msgTransactionSendId, &msgTransactionReplyId, &msgReportUser, &msgReportNode);
    setTermination(&terminationReason, &msgReportUser);
    while(wait(NULL) > 0);

    deallocBuffers(&msgTransactionSendId, &msgTransactionReplyId, &msgReportUser, &msgReportNode);
    unMapSharedMemory();
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])
{
    //printf("Creazione del processo Master\n");
    pid_t pid = fork();

    if(pid == -1)
    {
        printf("Errore nella creazione del processo master\n");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        masterStart();
        //printf("Il processo master e' terminato\n");
        exit(EXIT_SUCCESS);
    }
    else
    {
        while(wait(NULL) > 0);
        //printf("Il processo Main e' terminato correttamente\n");
        //TODO: deallocare tutte le risorse
        return 0;
    }
    //TODO: creazione file di log
}
