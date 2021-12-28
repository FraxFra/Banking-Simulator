#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include "config.h"

void *masterStart()
{
    int i = 0;
    int status;
    double time_start = clock();
    double time_finish = 0.0;
    double time = 0.0;
    pid_t pid;

    for(i = 0; i < SO_USERS_NUM; i++)
    {
        if(getpid() != 0)
        {
            pid = fork();
            if(pid == -1)
            {
                printf("Errore con la creazione di un processo utente\n");
                exit(1);
            }
            else if(pid == 0)
            {
                userStart();
                //TODO: alla exit() di tutti gli utenti deve corrispondere una terminazione della simulazione
            }
        }
    }

    for(i = 0; i < SO_NODES_NUM; i++)
    {
        if(getpid() != 0)
        {
            pid = fork();
            if(pid == -1)
            {
                printf("Errore con la creazione di un processo nodo\n");
                exit(1);
            }
            else if(pid == 0)
            {
                nodeStart();
            }
        }
    }

    if(getpid() != 0)
    {
        pid = fork();
        if(pid == -1)
        {
            printf("Errore con la creazione di un processo masterBook\n");
            exit(1);
        }
        else if(pid == 0)
        {
            masterBookStart();
            //TODO: alla terminazione del libro mastro corrisponde una terminazione della simulazione
        }
    }

    while(SO_SIM_SEC > time)
    {
        time_finish = clock();
        time = (double)(time_finish - time_start) / CLOCKS_PER_SEC;
        sleep(1);
    }
    exit(1);
}

int main(int argc, char const *argv[])
{
    int status;
    pid_t pid;

    printf("Creazione del processo Master\n");
    pid = fork();

    if(pid == -1)
    {
        exit(1);
    }
    else if(pid != 0)
    {
        masterStart();
    }

    wait(&status);
    if(status == 1)
    {
        printf("Il processo Master e' terminato correttamente\n");
    }
    //TODO: creazione file di log
    return 0;
}
