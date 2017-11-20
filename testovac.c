#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h> // wati()


#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <linux/errqueue.h>
#include <linux/icmp.h>
#include <netinet/icmp6.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


#define BUFFER_SIZE	(1024)	// length of the receiving buffer


int useUdp = 0;
char udpSendPort[6] = "33434";
//char udpRcvPort[6] = "33434";
int udpRcvPort = -1;
int nOfNodes;
char* nodes[30];

void printHelp() {
	printf("Nápověda.");
}

void printStats() {
	printf("stats\n");
}

void *printClock(void *arg) {
	int t = *((int *) arg);
	while(1)
	{
		sleep(t);
		printStats();
	}
	return 0;
}

void getRtt(int i) {
	printf("%s\n", nodes[i]);

	struct addrinfo hints, *nodeInfo;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if (getaddrinfo(nodes[i], udpSendPort, &hints, &nodeInfo) != 0) {
		fprintf(stderr, "Při zjišťování informací o zadané adrese došlo k chybě.\n");
		exit(1);
	}
	int isV6, sol, ipErr, ttlFlag;
	if (nodeInfo->ai_family == AF_INET) { //IPV4
		isV6 = IPPROTO_IP;
		ttlFlag = IP_TTL;
		sol = SOL_IP;
		ipErr = IP_RECVERR;
	} else if (nodeInfo->ai_family == AF_INET6) { //IPV6
		isV6 = IPPROTO_IPV6;
		ttlFlag = IPV6_UNICAST_HOPS;
		sol = SOL_IPV6;
		ipErr = IPV6_RECVERR;
	} else {
		fprintf(stderr, "Nepodporovaný protokol.\n");
	}
	int sock = socket(nodeInfo->ai_family, nodeInfo->ai_socktype, nodeInfo->ai_protocol);
	if (sock == -1) {
			fprintf(stderr, "Nepovedlo se vytvořit socket.\n");
			exit(1);
	}
	/*ok = setsockopt(sock,
					isV6,
					ttlFlag,
					&ttl,
					sizeof(ttl));
	if (ok < 0) {
		fprintf(stderr, "Nešlo nastavit socket.\n");
		exit(1);
	}*/
	//ok = connect(sock, nodeInfo->ai_addr, nodeInfo->ai_addrlen);
	if (connect(sock, nodeInfo->ai_addr, nodeInfo->ai_addrlen) < 0) {
		fprintf(stderr, "Nešlo se spojit s požadovanou adresou.\n");
		exit(1);
	}
}

void *msgClock(void *arg) {
	printf("msgClock		start\n");
	printStats();
	int i = *((int *) arg);
	while(1)
	{
		usleep(i * 1000);
		pid_t pids[nOfNodes];
		for (int i = 0; i < nOfNodes; i++) {
			pid_t pid;
			//printf("probehne fork\n");
			pid = fork();
			if (pid == 0) { // child proces
				//printf("child process %d\n", i);
				getRtt(i);
				_Exit(0);
			} else { // parent proces
				pids[i] = pid;
				//printf("parent process %d\n", i);
			}
		}
		for (int i = 0; i < nOfNodes; i++) {
			waitpid(pids[i], NULL, 0);
		}
	}
	return 0;
}

void *udpServer() {
	int fd;							// an incoming socket descriptor
	struct sockaddr_in server;		// server's address structure
	struct sockaddr_in client;		// client's address structure

	server.sin_family = AF_INET;					// set IPv4 addressing
	server.sin_addr.s_addr = htonl(INADDR_ANY);		// the server listens to any interface
	server.sin_port = htons(atoi(udpRcvPort));		// the server listens on this port

	printf("opening UDP socket(...)\n");
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)	// create the server UDP socket
		fprintf(stderr, "Couldn't create socket.\n");

	printf("binding to the port %d (%d)\n",htons(server.sin_port),server.sin_port);
	if (bind(fd, (struct sockaddr *)&server, sizeof(server)) == -1)	// binding with the port
		fprintf(stderr, "Couldn't bind socket.\n");

	int len = sizeof(client);
	int cliSock, msgSize;
	char buffer[BUFFER_SIZE];

	while (1) {
		while((msgSize = recvfrom(fd, buffer, BUFFER, 0, (struct sockaddr *)&client, &len)) >= 0) {
			printf("buffer = \"%.*s\"\n",msgSize,buffer);
			i = sendto(fd, buffer, msgSize, 0, (struct sockaddr *)&client, len); // send the answer
			if (i == -1)								// check if data was successfully sent out
				fprintf(stderr, "sendto() failed\n");
			else if (i != msgSize)
				fprintf(stderr, "sendto(): buffer written partially\n");
			else
				printf("data \"%.*s\" sent from port %d to %s, port %d\n",r,buffer,ntohs(server.sin_port), inet_ntoa(client.sin_addr),ntohs(client.sin_port));
		}
	}
}

/**
 * testovac [-h] [-u] [-v] [-t <interval>] [-i <interval>] [-w <timeout>] [-p <port>] [-l <port>] [-s <size>] [-r <value>] <uzel1> <uzel2> <uzel3> ...
 */
int main(int argc, char** argv) {

	char c;
	while ((c = getopt (argc, argv, "huvt:i:w:p:l:s:r:")) != -1) {
		switch(c){
			case 'h':
				printHelp();
				exit(0);
			case 'u':
				useUdp = 1;
				break;
			case 'p':
				strcpy(udpSendPort, optarg);
				break;
			case 'l':
				udpRcvPort = optarg;
				break;
			default:
				fprintf(stderr, "Špatné argumenty.\n");
				break;
		}
	}
	nOfNodes = argc - optind; // number of entered nodes
	//char* nodes[nOfNodes];
	printf("nOfNodes: %d\n", nOfNodes);

	for (int i = 0; i + optind < argc; i++) { //copy entered nodes to "nodes" array
		nodes[i] = argv[i + optind];
	}


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

	if (udpRcvPort != -1) {
		pthread_t updServTID;
		pthread_create(&updServTID, NULL, &udpServer, NULL);
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