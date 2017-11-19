#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h> // wati()




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

void sendMsg() {

}

void *msgClock(void *arg)
{
	int nOfNodes = 1; ///////////////////////////////////////////////////////////// počet uzlů
	int i = *((int *) arg);
	while(1)
	{
		usleep(i * 1000);
		pid_t pids[nOfNodes];
		for (int i = 0; i < nOfNodes; i++) {
			pid_t pid;
			pid = fork();
			if (pid == 0) { // child proces
				sendMsg();
				printf("child process %d\n", i);
				_Exit(0);
			} else { // parent proces
				pids[i] = pid;
				printf("parent process %d\n", i);
			}
		}
		for (int i = 0; i < nOfNodes; i++) {
			waitpid(pids[i], NULL, NULL);
		}
	}
	return 0;
}

/**
 * testovac [-h] [-u] [-t <interval>] [-i <interval>] [-p <port>] [-l <port>] [-s <size>] [-r <value>] <uzel1> <uzel2> <uzel3> ...
 */
int main(int argc, char** argv) {
	int aT = 10; // argument -t print stat interval (300)
	int aI = 1000; // argument -i msg send interval (100)

	int *argT = malloc(sizeof(*argT));
	if ( argT == NULL ) {
		fprintf(stderr, "Couldn't allocate memory for thread arg.\n");
		exit(99);
	}
	*argT = aT;
	pthread_t printTID;
	pthread_create(&printTID, NULL, &printClock, argT);

	int *argI = malloc(sizeof(*argI));
	if ( argI == NULL ) {
		fprintf(stderr, "Couldn't allocate memory for thread arg.\n");
		exit(99);
	}
	*argI = aI;
	pthread_t mClockTID;
	pthread_create(&mClockTID, NULL, &msgClock, argI);

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