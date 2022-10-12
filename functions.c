#include "functions.h"


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
