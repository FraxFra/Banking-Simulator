#include "config.h"

volatile Transaction** masterBookRegistry;
volatile pid_t* userProcesses;
volatile pid_t* nodeProcesses;
volatile pid_t* masterBookProcess;

void allocUserProcesses()
{
    int i = 0;
    userProcesses = mmap(NULL, SO_USERS_NUM * sizeof(pid_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(!userProcesses)
    {
        perror("la mappatura dei pid utente e' fallita");
        exit(1);
    }
}

void allocNodeProcesses()
{
    nodeProcesses = mmap(NULL, SO_NODES_NUM * sizeof(pid_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(!nodeProcesses)
    {
        perror("la mappatura dei pid nodo e' fallita");
        exit(1);
    }
}

void allocMasterBookProcess()
{
    masterBookProcess = mmap(NULL, sizeof(pid_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(!masterBookProcess)
    {
        perror("la mappatura del pid masterBook e' fallita");
        exit(1);
    }
}

void allocMasterBookRegistry()
{
    masterBookRegistry = mmap(NULL, SO_REGISTRY_SIZE * SO_BLOCK_SIZE * sizeof(Transaction), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); //non testato
    if(!masterBookRegistry)
    {
        perror("la mappatura del masterBookRegistry e' fallita");
        exit(1);
    }
}

void *masterStart()
{
    pid_t masterPid = getpid();
    printf("Creato processo master: %d\n", masterPid);

    int i = 0;
    pid_t pid;

    allocUserProcesses();
    allocNodeProcesses();
    allocMasterBookProcess();
    allocMasterBookRegistry();

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

    sleep(SO_SIM_SEC);
    while(wait(NULL) > 0);
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
