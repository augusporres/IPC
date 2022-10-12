#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define MAX_RETREIES 10

union semun{
    int val;                /* used for SETVAL only*/
    struct semid_ds *buf;   /* used for IPC_STAT and IPC_SET*/
    ushort *array;          /* used for GETALL and SETALL */
};