#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


void printStats() {
	printf("stats\n");
}


void *printClock(void *arg)
{
	int t = *((int *) arg);
	while(1)
	{
		sleep(t);
		printStats();
	}
	return 0;
}

void *msgClock(void *arg)
{
	sleep(10);
	int i = *((int *) arg);
	while(1)
	{
		usleep(i * 1000);
		printStats();
	}
	return 0;
}

/**
 * testovac [-h] [-u] [-t <interval>] [-i <interval>] [-p <port>] [-l <port>] [-s <size>] [-r <value>] <uzel1> <uzel2> <uzel3> ...
 */
int main(int argc, char** argv) {
	int aT = 10; // argument -t print stat interval (300)
	int aI = 1000; // argument -i msg send interval (100)

	int *arg = malloc(sizeof(*arg));
	if ( arg == NULL ) {
		fprintf(stderr, "Couldn't allocate memory for thread arg.\n");
		exit(99);
	}
	*arg = aT;
	pthread_t printTID;
	pthread_create(&printTID, NULL, &printClock, arg);

	*arg = aI;
	pthread_t msgTID;
	pthread_create(&msgTID, NULL, &msgClock, arg);
	*arg = 13;

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