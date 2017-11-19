#include <stdio.h>
#include <unistd.h>
#include <pthread.h>


void printStats() {
	printf("stats\n");
}


void *statClock(void *arg)
{
	int t = *((int *) i);
	while(1)
	{
		sleep(t++);
		printStats();
	}
	return 0;
}


/**
 * testovac [-h] [-u] [-t <interval>] [-i <interval>] [-p <port>] [-l <port>] [-s <size>] [-r <value>] <uzel1> <uzel2> <uzel3> ...
 */
int main(int argc, char** argv) {

	int *arg = malloc(sizeof(*arg));
	if ( arg == NULL ) {
		fprintf(stderr, "Couldn't allocate memory for thread arg.\n");
		exit(99);
	}
	*arg = 0;
	pthread_t tid;
	pthread_create(&tid, NULL, &statClock, arg);
	for (int i = 0; i < 10; i++) {
		sleep(3);
		printf("%d\n", *arg);
	}
	sleep(1000);

	/*pid_t pid;
    pid = fork();
    if (pid == 0) {
        printf("child process");
    } else {
        sleep(15);
        printf("parent process");
    }*/
}