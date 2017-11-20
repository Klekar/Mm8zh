#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h> // wati()

#include <time.h>


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
int wSec = 2; // arg -w
char udpSendPort[6] = "33434";
//char udpRcvPort[6] = "33434";
int udpRcvPort = -1;
int nOfNodes;
char* nodes[30];
int bytesOfData = 56;

/**
 * function used for getting time difference in ms beween base and min
 * @param min
 * @param base
 * @return double base - min [ms]
 */
double subTimeval(struct timeval *min, struct timeval *base) {
    double sub;

    sub =   (double)(base->tv_sec - min->tv_sec) * 1000.0 +
            (double)(base->tv_usec - min->tv_usec) / 1000.0;
    return (sub);
}

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

void udpGetRtt(int nodeI) {
	printf("%s\n", nodes[nodeI]);

	struct sockaddr_in server, from;	// address structures of the server and the client
	struct hostent *servent;			// network host entry required by gethostbyname()
	socklen_t len, fromlen;

	memset(&server,0,sizeof(server));	// erase the server structure
	server.sin_family = AF_INET;  

	if ((servent = gethostbyname(nodes[nodeI])) == NULL) {
		fprintf(stderr, "Nepovedlo se vytvořit socket.\n");
		exit(1);
	}
	memcpy(&server.sin_addr,servent->h_addr,servent->h_length);
	server.sin_port = htons(atoi(udpSendPort));	// set up port number

	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		fprintf(stderr, "Nepovedlo se vytvořit socket.\n");
		exit(1);
		}
	struct timeval timOut; // initialize timeout
	timOut.tv_sec = 2;
	timOut.tv_usec = 0;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timOut,sizeof(struct timeval)) < 0) {
		printf("Nepodarilo se nastavit socket. (udp send)");
		_Exit(1);
	}

	len = sizeof(server);
	fromlen = sizeof(from);

	if (connect(sock, (struct sockaddr *)&server, sizeof(server))  == -1) {
		fprintf(stderr, "Nepovedlo se vytvořit socket.\n");
		exit(1);
	}

	struct timeval t1, t2;
	struct timezone tzone;
	unsigned char buffer[bytesOfData]; // sending buffer
	unsigned char buffer2[bytesOfData]; // receiving buffer

	srand(time(NULL));
	for ( int i = 0; i < bytesOfData; i++) {
		buffer[i] = rand() % 256;
		//int r = (rand() % 256) + '0';
		//printf("%c\n", buffer[i]);
	}

	(void) gettimeofday(&t1, &tzone); //////////// ZAČÁTEK MĚŘENÍ ČASU
	/*char temp;
	temp = &t1;*/

	/*unsigned char buffer[bytesOfData];	  posilani time v packetu
	int timevalSize = sizeof(t1);
	for ( int i = 0; i < timevalSize; i++) {
		buffer[i] = (t1 >> ((timevalSize - 1)*8) ) & 0xFFFFFFFF;
	}
	srand(time(NULL));
	for ( int i = timevalSize; i < bytesOfData; i++) {
		buffer[i] = rand() % 256;
		//int r = (rand() % 256) + '0';
		//printf("%c\n", buffer[i]);
	}*/

	int ok = send(sock, buffer, sizeof(buffer), 0);
	if (ok == -1) {
		fprintf(stderr, "Nepovedlo se odeslat data.\n");
		exit(1);
	} else if (ok != bytesOfData)
		fprintf(stderr, "Data byla odeslana castecne.\n");

	if (getsockname(sock, (struct sockaddr *) &from, &len) == -1) {
		fprintf(stderr, "Nepovedlo se zjistit lokální adresu a port.\n");
		exit(1);
	}

	printf("data sent from %s, port %d (%d) to %s, port %d (%d)\n",inet_ntoa(from.sin_addr), ntohs(from.sin_port), from.sin_port, inet_ntoa(server.sin_addr),ntohs(server.sin_port), server.sin_port);


	if ((ok = recv(sock, buffer2, BUFFER_SIZE,0)) == -1) {
		fprintf(stderr, "recv() se nezdarilo\n");
		exit(1);
	} else if (ok > 0){
		(void) gettimeofday(&t2, &tzone); //////////// KONEC MĚŘENÍ ČASU
		// obtain the remote IP adddress and port from the server (cf. recfrom())
		if (getpeername(sock, (struct sockaddr *) &from, &fromlen) != 0) {
			fprintf(stderr, "getpeername() se nezdarilo\n");
			exit(1);
		}

		time_t t = t2.tv_sec;
		struct tm* lt = localtime(&t);
		char ts[26];

		strftime(ts, 26, "%Y-%m-%d %H:%M:%S", nt);

		printf("%s.%02d %d bytes from %s (ip addr) time=%.2f ms\n", ts, t2.tv_usec % 1000, bytesOfData, nodes[nodeI], subTimeval(&t1, &t2));


		printf("data received from %s, port %d\n",inet_ntoa(from.sin_addr),ntohs(from.sin_port));
		if (strcmp(buffer, buffer2)) {
			printf("data se shoduji\n");
		}
		//printf("%.*s",ok,buffer2);				// print the answer
	}

	/*memset(&hints, 0, sizeof hints); ipv4 vs ipv6
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
	}*/
	/*int sock = socket(nodeInfo->ai_family, nodeInfo->ai_socktype, nodeInfo->ai_protocol);
	if (sock == -1) {
		fprintf(stderr, "Nepovedlo se vytvořit socket.\n");
		exit(1);
	}*/
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
	/*if (connect(sock, nodeInfo->ai_addr, nodeInfo->ai_addrlen) < 0) {
		fprintf(stderr, "Nešlo se spojit s požadovanou adresou.\n");
		exit(1);
	}*/
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
				udpGetRtt(i);
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
	//server.sin_port = htons(atoi(udpRcvPort));		// the server listens on this port
	server.sin_port = htons(udpRcvPort);			// the server listens on this port

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
		while((msgSize = recvfrom(fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client, &len)) >= 0) {
			//printf("buffer = \"%.*s\"\n",msgSize,buffer);
			cliSock = sendto(fd, buffer, msgSize, 0, (struct sockaddr *)&client, len); // send the answer
			if (cliSock == -1)								// check if data was successfully sent out
				fprintf(stderr, "sendto() failed\n");
			else if (cliSock != msgSize)
				fprintf(stderr, "sendto(): buffer written partially\n");
			else
				printf("data received from port %d to %s, port %d\n",/*cliSock,buffer,*/ntohs(server.sin_port), inet_ntoa(client.sin_addr),ntohs(client.sin_port));
		}
	}
}

/**
 * testovac [-h] [-u] [-v] [-t <interval>] [-i <interval>] [-w <timeout>] [-p <port>] [-l <port>] [-s <size>] [-r <value>] <uzel1> <uzel2> <uzel3> ...
 */
int main(int argc, char** argv) {
	printf("%d\n", sizeof(char));
	char c;
	while ((c = getopt (argc, argv, "huvt:i:w:p:l:s:r:")) != -1) {
		switch(c){
			case 'h':
				printHelp();
				exit(0);
			case 'u':
				useUdp = 1;
				bytesOfData = 64;
				break;
			case 'p':
				strcpy(udpSendPort, optarg);
				break;
			case 'l':
				udpRcvPort = atoi(optarg);
				break;
			case 's':
				if (atoi(optarg) < sizeof(struct timeval)) {
					bytesOfData = sizeof(struct timeval);
					printf("Moc maly objem dat pro testovani. Pouzije se %dB.", bytesOfData);
				} else if (atoi(optarg) > BUFFER_SIZE) {
					bytesOfData = sizeof(struct timeval);
					printf("Moc velky objem dat pro testovani. Pouzije se %dB.", BUFFER_SIZE);
				} else
					bytesOfData = atoi(optarg);
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

	if (nOfNodes > 0) { // run only if any nodes was inserted
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
	}

	if (udpRcvPort != -1) { // run server if receiving port was specified
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