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
float wSec = 2; // arg -w
float rTime = -1; // arg -r
int verbose = 0;
char udpSendPort[7] = "33434";
//char udpRcvPort[6] = "33434";
int udpRcvPort = -1;
int nOfNodes;
char* nodes[30];
int bytesOfData = 56;
float rttDataHour[6];
float rttData[3];
int aT = 10;	// argument -t print stat interval (300)[s]
int aI = 1000;	// argument -i msg send interval (100)[ms]
int nodeI;


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

void *udpGetRtt(/*int nodeI*/) {
	printf("%s\n", nodes[nodeI]);

	struct sockaddr_in server, from;	// address structures of the server and the client
	struct hostent *servent;			// network host entry required by gethostbyname()
	socklen_t len, fromlen;
	int sock;
	float rtt = wSec/2;


	while (1) {
		memset(&server,0,sizeof(server));	// erase the server structure
		server.sin_family = AF_INET;  

		if ((servent = gethostbyname(nodes[nodeI])) == NULL) {
			fprintf(stderr, "Nepovedlo se vytvořit socket.\n");
			exit(1);
		}
		memcpy(&server.sin_addr,servent->h_addr,servent->h_length);
		server.sin_port = htons(atoi(udpSendPort));	// set up port number

		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if (sock == -1) {
			fprintf(stderr, "Nepovedlo se vytvořit socket.\n");
			exit(1);
			}
		struct timeval timOut; // initialize timeout
		timOut.tv_sec = (int) rtt*2;
		timOut.tv_usec = (int) (rtt - ((int)rtt)) * 100000;
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

		//printf("data sent from %s, port %d (%d) to %s, port %d (%d)\n",inet_ntoa(from.sin_addr), ntohs(from.sin_port), from.sin_port, inet_ntoa(server.sin_addr),ntohs(server.sin_port), server.sin_port);


		if ((ok = recv(sock, buffer2, BUFFER_SIZE,0)) == -1) {
			rttDataHour[0]++;
			rttData[0]++;
			rttDataHour[2]++;
			rttData[2]++;
			//fprintf(stderr, "recv() se nezdarilo\n");
			continue;
		} else if (ok > 0){
			(void) gettimeofday(&t2, &tzone); //////////// KONEC MĚŘENÍ ČASU

			if (getpeername(sock, (struct sockaddr *) &from, &fromlen) != 0) {
				fprintf(stderr, "getpeername() se nezdarilo\n");
				exit(1);
			}

			if (!strcmp(buffer, buffer2)) {
				printf("data se neshoduji\n");
				continue;
			}

			rtt = subTimeval(&t1, &t2);
			rttDataHour[0]++;
			if (rtt == 0 || rtt < rttDataHour[3])
				rttDataHour[3] == rtt;
			if (rtt == 0 || rtt > rttDataHour[4])
				rttDataHour[4] == rtt;
			rttDataHour[5] += rtt;
			rttData[0]++;
			if (rTime > 0 && rtt > rTime) {
				rttDataHour[1]++;
				rttData[1]++;
			}
			if (verbose) {
				time_t t = t2.tv_sec;
				struct tm* lt = localtime(&t);
				char ts[26];

				strftime(ts, 26, "%Y-%m-%d %H:%M:%S", lt);

				printf("%s.%02d %d bytes from %s (ip addr) time=%.2f ms\n", ts, t2.tv_usec % 100, bytesOfData, nodes[nodeI], rtt);
			}
			usleep(aI * 1000);
		}
		//printf("data received from %s, port %d\n",inet_ntoa(from.sin_addr),ntohs(from.sin_port));
		
		//printf("%.*s",ok,buffer2);				// print the answer
	}
}

/*void *msgClock(void *arg) {
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
}*/

void *smallStat() {
	
}

void *bigStat() {
	
}

void nodeProcess(int nOfNode) {
	//float rttDataHour[6];
	rttDataHour[0] = 0; //pocet clk
	rttDataHour[1] = 0; //pocet exceed
	rttDataHour[2] = 0; //pocet lost
	rttDataHour[3] = 0; //min rtt
	rttDataHour[4] = 0; //max rtt
	rttDataHour[5] = 0; //clk rtt
	rttData[0] = 0; //pocet clk
	rttData[1] = 0; //pocet exceed
	rttData[2] = 0; //pocet lost
	nodeI = nOfNode;

	pthread_t workTID;
	pthread_create(&workTID, NULL, &udpGetRtt, NULL);

	pthread_t smallStatTID;
	pthread_create(&smallStatTID, NULL, &smallStat, NULL);

	pthread_t bigStatTID;
	pthread_create(&bigStatTID, NULL, &bigStat, NULL);

	pthread_join(workTID, NULL);
	pthread_join(smallStatTID, NULL);
	pthread_join(bigStatTID, NULL);
}

void *udpServer() {
	int fd;							// an incoming socket descriptor
	struct sockaddr_in server;		// server's address structure
	struct sockaddr_in client;		// client's address structure

	server.sin_family = AF_INET;					// set IPv4 addressing
	server.sin_addr.s_addr = htonl(INADDR_ANY);		// the server listens to any interface
	server.sin_port = htons(udpRcvPort);			// the server listens on this port

	//printf("opening UDP socket(...)\n");
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)	// create the server UDP socket
		fprintf(stderr, "Couldn't create socket.\n");

	//printf("binding to the port %d (%d)\n",htons(server.sin_port),server.sin_port);
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
			case 'v':
				verbose = 1;
				break;
			case 't':
				aT = atoi(optarg);
				break;
			case 'i':
				aI = atoi(optarg);
				break;
			case 'w':
				wSec = atof(optarg);
				break;
			case 'p':
				strcpy(udpSendPort, optarg);
				break;
			case 'l':
				udpRcvPort = atoi(optarg);
				break;
			case 's':
				/*if (atoi(optarg) < sizeof(struct timeval)) {
					bytesOfData = sizeof(struct timeval);
					printf("Moc maly objem dat pro testovani. Pouzije se %dB.", bytesOfData);
				} else if (atoi(optarg) > BUFFER_SIZE) {
					bytesOfData = sizeof(struct timeval);
					printf("Moc velky objem dat pro testovani. Pouzije se %dB.", BUFFER_SIZE);
				} else
					bytesOfData = atoi(optarg);*/
				bytesOfData = atoi(optarg);
				break;
			case 'r':
				rTime = atof(optarg);
				break;
			default:
				//fprintf(stderr, "Špatné argumenty.\n");
				break;
		}
	}
	nOfNodes = argc - optind; // number of entered nodes
	//char* nodes[nOfNodes];
	printf("nOfNodes: %d\n", nOfNodes);

	for (int i = 0; i + optind < argc; i++) { //copy entered nodes to "nodes" array
		nodes[i] = argv[i + optind];
	}

	/*if (nOfNodes > 0) { // run only if any nodes was inserted
		float aSize = (int) (aT/(aI/1000))*1.5;
		if (aSize < 2)
			aSize = 2;
		rtts = (float **) malloc(sizeof(float)*aSize*nOfNodes);

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
	}*/

	if (udpRcvPort != -1) { // run server if receiving port was specified
		pthread_t updServTID;
		pthread_create(&updServTID, NULL, &udpServer, NULL);
	}
	int pids[nOfNodes];
	for (int i = 0; i < nOfNodes; i++) {
		pid_t pid;
		//printf("probehne fork\n");
		pid = fork();
		if (pid == 0) { // child proces
			//printf("child process %d\n", i);
			nodeProcess(i);
			_Exit(0);
		} else { // parent proces
			pids[i] = pid;
			//printf("parent process %d\n", i);
		}
	}
	for (int i = 0; i < nOfNodes; i++) {
		waitpid(pids[i], NULL, 0);
	}
	pthread_join(updServTID, NULL);

	/*pid_t pid;
	pid = fork();
	if (pid == 0) {
		printf("child process");
	} else {
		sleep(15);
		printf("parent process");
	}*/
}