#include "config.h"

Transaction* masterBookRegistry;
int* masterBookBlockLength;
pid_t* userProcesses;
pid_t* nodeProcesses;
pid_t* masterBookProcess;

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

    masterBookProcess = mmap(NULL, sizeof(pid_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(!masterBookProcess)
    {
        perror("la mappatura del pid masterBook e' fallita");
        exit(EXIT_FAILURE);
    }

    masterBookRegistry = mmap(NULL, sizeof(Transaction) * SO_REGISTRY_SIZE * SO_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(!masterBookRegistry)
    {
        perror("la mappatura del masterBookRegistry e' fallita");
        exit(EXIT_FAILURE);
    }

    masterBookBlockLength = mmap(NULL, sizeof(int) * SO_REGISTRY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(!masterBookBlockLength)
    {
        perror("la mappatura del masterBookBlockLength e' fallita");
        exit(EXIT_FAILURE);
    }
}

void initMsgId(int* msgTransactionSendId, int* msgTransactionReplyId, int* msgBlockSendId, int* msgBlockReplyId)
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

    *msgBlockSendId = msgget(2, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(*msgBlockSendId == -1)
    {
        printf("Errore nella creazione dell'id msgBlockSendId\n");
        exit(EXIT_FAILURE);
    }

    *msgBlockReplyId = msgget(3, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(*msgBlockReplyId == -1)
    {
        printf("Errore nella creazione dell'id msgBlockReplyId\n");
        exit(EXIT_FAILURE);
    }
}

void deallocBuffers(int* msgTransactionSendId, int* msgTransactionReplyId, int* msgBlockSendId, int* msgBlockReplyId)
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

    code = msgctl(*msgBlockSendId, IPC_RMID, NULL);
    if(code == -1)
    {
        printf("Errore con la rimozione dell'id msgBlockSendId\n");
        exit(EXIT_FAILURE);
    }

    code = msgctl(*msgBlockReplyId, IPC_RMID, NULL);
    if(code == -1)
    {
        printf("Errore con la rimozione dell'id msgBlockReplyId\n");
        exit(EXIT_FAILURE);
    }
}

void createProcesses(pid_t masterPid, int* msgTransactionSendId, int* msgTransactionReplyId, int* msgBlockSendId, int* msgBlockReplyId)
{
    pid_t pid;
    int i;

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
                userStart();
                //TODO: alla exit() di tutti gli utenti deve corrispondere una terminazione della simulazione
            }
        }
    }

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
                nodeStart();
            }
        }
    }

    if(getpid() == masterPid)
    {
        pid = fork();
        if(pid == -1)
        {
            printf("Errore con la creazione di un processo masterBook\n");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0)
        {
            masterBookProcess[0] = getpid();
            masterBookStart();
            //TODO: alla terminazione del libro mastro corrisponde una terminazione della simulazione
        }
    }
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

    code = munmap(nodeProcesses, SO_USERS_NUM * sizeof(pid_t));
    if(code == -1)
    {
        printf("Errore rimuovere la mappa nodeProcesses");
        exit(EXIT_FAILURE);
    }

    code = munmap(masterBookProcess, SO_USERS_NUM * sizeof(pid_t));
    if(code == -1)
    {
        printf("Errore rimuovere la mappa masterBookProcess");
        exit(EXIT_FAILURE);
    }

    code = munmap(masterBookRegistry, SO_USERS_NUM * sizeof(pid_t));
    if(code == -1)
    {
        printf("Errore rimuovere la mappa masterBookRegistry");
        exit(EXIT_FAILURE);
    }

    code = munmap(masterBookBlockLength, SO_USERS_NUM * sizeof(pid_t));
    if(code == -1)
    {
        printf("Errore rimuovere la mappa masterBookBlockLength");
        exit(EXIT_FAILURE);
    }
}

void* masterStart()
{
    pid_t masterPid = getpid();
    printf("Creato processo master: %d\n", masterPid);

    int msgTransactionSendId;
    int msgTransactionReplyId;
    int msgBlockSendId;
    int msgBlockReplyId;

    mapSharedMemory();
    initMsgId(&msgTransactionSendId, &msgTransactionReplyId, &msgBlockSendId, &msgBlockReplyId);
    createProcesses(masterPid, &msgTransactionSendId, &msgTransactionReplyId, &msgBlockSendId, &msgBlockReplyId);
    sleep(SO_SIM_SEC);
    while(wait(NULL) > 0);

    deallocBuffers(&msgTransactionSendId, &msgTransactionReplyId, &msgBlockSendId, &msgBlockReplyId);
    unMapSharedMemory();
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])
{
    printf("Creazione del processo Master\n");
    pid_t pid = fork();

    if(pid == -1)
    {
        printf("Errore nella creazione del processo master\n");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        masterStart();
        printf("Il processo master e' terminato\n");
        exit(EXIT_SUCCESS);
    }
    else
    {
        while(wait(NULL) > 0);
        printf("Il processo Main e' terminato correttamente\n");
        //TODO: deallocare tutte le risorse
        return 0;
    }
    //TODO: creazione file di log
}
