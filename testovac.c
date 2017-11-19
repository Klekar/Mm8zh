#include <stdio.h>
#include <unistd.h>
#include <pthread.h>


void printStats() {
    printf("stats\n");
}


void *statClock(int t /*void *arg*/)
{
    while(1)
    {
        sleep(t);
        printStats();
    }
    return 0;
}


/**
 * testovac [-h] [-u] [-t <interval>] [-i <interval>] [-p <port>] [-l <port>] [-s <size>] [-r <value>] <uzel1> <uzel2> <uzel3> ...
 */
int main(int argc, char** argv) {
	printf("program zacal");
	pthread_t tid;
	printf("program init");
	pthread_create(&tid, NULL, &statClock, 1);
	printf("program vytvoril thread");
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