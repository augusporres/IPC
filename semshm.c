#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_SIZE 1024 /* make it a 1K shared memory segment */
#define MAX_RETREIES 10 /* number tries for locking semaphore*/

union semun{
    int val;                /* used for SETVAL only*/
    struct semid_ds *buf;   /* used for IPC_STAT and IPC_SET*/
    ushort *array;          /* used for GETALL and SETALL */
};

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

int main(int argc, char *argv[])
{
    key_t shmkey, semkey;
    int shmid, semid;
    char *data;
    int mode;
    struct sembuf sb;

    if(argc > 2){
        fprintf(stderr, "usage: shmdemo [data_to_write]\n");
        exit(1);
    }

    /*------- INIT KEYS ----------- */
    if((shmkey = ftok("semshm.c", 'R')) == -1){
        perror("shm ftok");
        exit(1);
    }
    if((semkey = ftok("semshm.c", 'A')) == -1){
        perror("sem ftok");
        exit(1);
    }
    /*--------- GET THE IDs--------- */
    /* init sem and get id*/
    if((semid = initsem(semkey, 1)) == -1){
        perror("initsem");
        exit(1);
    }

    /*------- LOCK ----------- */
    /* This code locks the semaphore */
    sb.sem_num = 0; /* first semaphore */
    sb.sem_op = -1; /* allocate resource */
    sb.sem_flg = SEM_UNDO;
    printf("Trying to lock..\n");

    if(semop(semid, &sb, 1) == -1){
        perror("semop");
        exit(1);
    }
    printf("Semaphore Locked,\n");

    /* This code unlocks the semaphore */
    sb.sem_op = 1; /* free resource */
    if(semop(semid, &sb, 1) == -1){
        perror("semop");
        exit(1);
    }
    printf("Semaphore Unlocked\n");
    
    /* connect to (and possibly create) the segment */
    if((shmid = shmget(shmkey, SHM_SIZE, 0644 | IPC_CREAT)) == -1){
        perror("shmget");
        exit(1);
    }
    /* attach to the segment to get a pointer to it */
    data = shmat(shmid, (void *)0, 0);
    if(data == (char *)(-1)){
        perror("shmat");
        exit(1);
    }

    /* read or modify the segment, based on the command line */
    if(argc == 2){
        printf("writing to segment: \"%s\"\n", argv[1]);
        strncpy(data, argv[1], SHM_SIZE);
    }else{
        printf("Segment contains: \"%s\"\n", data);
    }
    /* detach from the segment */
    if(shmdt(data) == -1){
        perror("shmdt");
        exit(1);
    }
    
    return 0;
}