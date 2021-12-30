#include "config.h"

pid_t** usersProcesses;
pid_t** nodeProcesses;
pid_t* masterBookProcId;
Transaction*** masterBookRegistry;

void *masterStart()
{
    printf("Creato processo master: %d\n", getpid());
    int i = 0;
    int status;
    pid_t pid;
    pid_t masterPid = getpid();
    int wd;

    usersProcesses = (pid_t**)malloc(sizeof(pid_t*) * SO_USERS_NUM);
    nodeProcesses = (pid_t**)malloc(sizeof(pid_t*) * SO_NODES_NUM);
    masterBookProcId = (pid_t*)malloc(sizeof(pid_t));

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
                usersProcesses[i] = (pid_t*)malloc(sizeof(pid_t));
                *usersProcesses[i] = getpid();
                //printf("%d\n", *usersProcesses[i]);
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
                nodeProcesses[i] = (pid_t*)malloc(sizeof(pid_t));
                *nodeProcesses[i] = getpid();
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
            *masterBookProcId = getpid();
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
    int status;
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
