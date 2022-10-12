#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define MAX_RETREIES 10


/*
struct semid_ds {
    struct ipc_perm sem_perm;   /* ownership and permissions 
    time_t          sem_otime;  /* Last semop time 
    time_t          sem_ctime;  /* Last change time
    unsigned short  sem_nsems;  /* No. of semaphores in set
}*/ // this structure exists on system

/*struct sembuf{
    ushort sem_num;
    short sem_op;
    short sem_flg;
};*/ // this structure exists on system

union semun{
    int val;                /* used for SETVAL only*/
    struct semid_ds *buf;   /* used for IPC_STAT and IPC_SET*/
    ushort *array;          /* used for GETALL and SETALL */
};

/*
** initsem() --mor-than-inspired by W. Richard Stevens 'UNIX Network
** Programming 2nd Edition, volume 2, lockvsem.c, page 295
*/
int initsem(key_t key, int nsems) /* key from ftok() */
{
    int i;
    union semun arg;
    struct semid_ds buf;
    struct sembuf sb;
    int semid;

    semid = semget(key, nsems, IPC_CREAT | IPC_EXCL | 0666);

    if(semid >= 0){ /* we got it first */
        sb.sem_op = 1; sb.sem_flg = 0;
        arg.val = 1;

        printf("press return\n"); getchar();

        for(sb.sem_num = 0; sb.sem_num < nsems; sb.sem_num ++){
            /* do a semop() to "free" the semaphores */
            /* this sets the sem_otime field, as needed below */
            if(semop(semid, &sb, 1) == -1){
                int e = errno;
                semctl(semid, 0, IPC_RMID); /* clean up */
                errno = e;
                return -1; /* error, check errno */
            }

        }
    } else if (errno == EEXIST) { /* someone else got it first */
        int ready = 0;

        semid = semget(key, nsems, 0); /* get the id */
        if(semid < 0) return semid; /* error, check errno */

        /* wait for other process to initialize the semaphore */
        arg.buf = &buf;
        for(i = 0; i < MAX_RETREIES && !ready; i++ ){
            semctl(semid, nsems-1, IPC_STAT, arg);
            if(arg.buf->sem_otime != 0){
                ready = 1;
            }else{
                sleep(1);
            }
        }
        if(!ready){
            errno = ETIME;
            return -1;
        }
    }else{
        return semid; /* error, check errno */
    }
    return semid;
}

int main(void)
{
    key_t key;
    int semid;
    struct sembuf sb;

    sb.sem_num = 0; /* cero es el  primer semaforo del conjunto */
    sb.sem_op = -1; /* set to allocate resource */
    sb.sem_flg = SEM_UNDO; /* if process dies, the kernel undoes the actions taken by sem */


    if((key = ftok("my_sem.c", 'J')) == -1){
        perror("ftok");
        exit(1);
    }

    /* grab the semaphore set created by seminit.c */
    if((semid = initsem(key, 1)) == -1){
        perror("initsem");
        exit(1);
    }

    printf("Press return to lock: ");
    getchar();
    printf("Trying to lock..\n");

    if(semop(semid, &sb, 1) == -1){
        perror("semop");
        exit(1);
    }

    printf("Locked,\n");
    printf("Press return to unlock");
    getchar();

    sb.sem_op = 1; /* free resource */
    if(semop(semid, &sb, 1) == -1){
        perror("semop");
        exit(1);
    }

    printf("Unlocked\n");
        
    return 0;
}