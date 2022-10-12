#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
int main(void)
{
	pid_t new_pid;
	new_pid = fork();
	switch(new_pid)
	{
		case -1: /* Error */
			perror("fork:");
			exit(1);
			break;
		case 0: /* Proceso hijo */
			printf("Hola mundo!: Soy el proceso hijo. Mi pid es %d.\n", getpid());
			break;
		default:
			printf("Hola mundo!: Soy el proceso padre. Mi hijo es %d.\n", new_pid);
			break;
	}
	return 0;
}
