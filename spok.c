#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>



struct my_msgbuf {
    long mtype;
    char mtext[200];
};
int main(void)
{
    struct my_msgbuf buf;
    int msqid;
    key_t key;

    if((key = ftok("kirk.c", 'B')) == -1){
        perror("ftok");
        exit(1);
    }
    if((msqid = msgget(key, 0644)) == -1){
        perror("msgget");
        exit(1);
    }

    printf("spock: ready to receive messages, captain.\n");
    buf.mtype = 1; /* we dont really care in this case*/

    for(;;){ /* Spock never quits! */
        if(msgrcv(msqid, &buf, sizeof(buf.mtext), 0, 0) == -1){
            perror("msgrcv");
            exit(1);
        }
        printf("Spock: \"%s\"\n", buf.mtext);
    }

    return 0;
}
