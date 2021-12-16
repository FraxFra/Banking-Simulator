#include <time.h>

#ifndef __CONFIG_H
#define __CONFIG_H

#ifndef CONFIG_FILE
#define CONFIG_FILE ".config"
#endif

#define SO_USERS_NUM 10
#define SO_NODES_NUM 8
#define SO_REWARD 10
#define SO_MIN_TRANS_GEN_NSEC 500000000
#define SO_MAX_TRANS_GEN_NSEC 1000000000
#define SO_RETRY 4
#define SO_TP_SIZE 6
#define SO_BLOCK_SIZE 6
#define SO_MIN_TRANS_PROC_NSEC 500000000
#define SO_MAX_TRANS_PROC_NSEC 1000000000
#define SO_REGISTRY_SIZE 5
#define SO_BUDGET_INIT 100
#define SO_SIM_SEC 10
#define SO_FRIENDS_NUM 10

typedef struct _TransactionMaster
{
    clock_t timestamp;
    int sender;
    int receiver;
} TransactionMaster;

typedef struct _Transaction
{
    clock_t timestamp;
    int sender;
    int receiver;
    int qty;
    int reward;
} Transaction;

//Inizio esecuzione e creazione processi
void *masterProcess(void *threadId);

//Routine utenti
void *userStart(void *threadId);

//Routine nodi
void *nodeStart(void *threadId);

//Routine masterBook
void *masterBookStart(void *threadId);


#endif