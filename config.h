#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/ipc.h>

#ifndef __CONFIG_H
#define __CONFIG_H

#ifndef CONFIG_FILE
#define CONFIG_FILE ".config"
#endif

#define SO_USERS_NUM 10
#define SO_NODES_NUM 5
#define SO_REWARD 10
#define SO_MIN_TRANS_GEN_NSEC 100000000
#define SO_MAX_TRANS_GEN_NSEC 200000000
#define SO_RETRY 5
#define SO_TP_SIZE 10
#define SO_BLOCK_SIZE 5
#define SO_MIN_TRANS_PROC_NSEC 10000000
#define SO_MAX_TRANS_PROC_NSEC 20000000
#define SO_REGISTRY_SIZE 50
#define SO_BUDGET_INIT 1000
#define SO_SIM_SEC 400
#define SO_FRIENDS_NUM 10

typedef struct _Transaction
{
    clock_t timestamp;
    pid_t sender;
    pid_t receiver;
    int qty;
    int reward;
} Transaction;

typedef struct _BufferTransactionSend
{
    long mtype;
    Transaction transaction;
}BufferTransactionSend;

typedef struct _BufferTransactionReply
{
    long mtype;
    int result;
}BufferTransactionReply;

typedef struct _BufferReportNode
{
    long mtype;
    int nTransactions;
}BufferReportNode;

typedef struct _PthreadArguments
{
    pid_t nodePid;
    Transaction* transactionPool;
    int* msgTransactionSendId;
    int* msgTransactionReplyId;
    sem_t* semThread;
}PthreadArguments;

extern Transaction* masterBookRegistry;
extern pid_t* userProcesses;
extern pid_t* nodeProcesses;
extern sem_t* semRegistry;
extern sem_t* semDeadUsers;
extern int* nBlocksRegistry;
extern int* nDeadUsers;
extern int* nTerminatedUsers;
extern int* termination;

extern void masterStart();
extern void userStart();
extern void nodeStart();
extern void masterBookStart();


#endif
