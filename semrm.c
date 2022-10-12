#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

union semun{
    int val;                /* used for SETVAL only*/
    struct semid_ds *buf;   /* used for IPC_STAT and IPC_SET*/
    ushort *array;          /* used for GETALL and SETALL */
};

int main(void)
{
    key_t key;
    int semid;
    union semun arg;

    if((key = ftok("my_sem.c", 'J')) == -1){
        perror("ftok");
        exit(1);
    }
    /* grab the semaphore set created by seminit.c */
    if((semid = semget(key, 1, 0)) == -1){
        perror("semget");
        exit(1);
    }
    /* remove it */
    if(semctl(semid, 0, IPC_RMID, arg) == -1){
        perror("semctl");
        exit(1);
    }

    return 0;
}