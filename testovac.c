#include <stdio.h>
#include <unistd.h>


void printStats() {
    printf("stats\n");
}


void *statClock(void *arg)
{
    while(1)
    {
        sleep(10);
        printStats();
    }
    return 0;
}


/**
 * testovac [-h] [-u] [-t <interval>] [-i <interval>] [-p <port>] [-l <port>] [-s <size>] [-r <value>] <uzel1> <uzel2> <uzel3> ...
 */
int main(int argc, char** argv) {
	pthread_t tid;
	pthread_create(&tid, NULL, &statClock, NULL);
    /*pid_t pid;
    pid = fork();
    if (pid == 0) {
        printf("child process");
    } else {
        sleep(15);
        printf("parent process");
    }*/
}