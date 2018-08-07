/*
 * A simple DVR protocol
 * The configuration file and the node name are passed in as arguments
 * Esther Max-Onakpoya
 * CS 571
 * Received help from:
 * http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/textcode.html
 * https://github.com/rachhshruti/distance-vector-routing/blob/master/dist_vector_RIP.cpp
 * https://github.com/elroyalva/dvrp/blob/master/server.c
 */
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() and alarm() */
#include <errno.h>      /* for errno and EINTR */
#include <signal.h>     /* for sigaction() */
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
#include <time.h>


#define MAXTRIES        2       /* Tries before giving up */
#define PERIOD    20       /* Seconds between retransmits */
#define ECHOMAX         1024     /* Longest string to echo */


 int tries=0;   /* Count of times sent - GLOBAL for signal-handler access */
 void DieWithError(char *errorMessage);   /* Error handling function */
 void CatchAlarm(int ignored);            /* Handler for SIGALRM */
	struct element {
		char dest[50];
		int dist;
	}; /* Part of struct for node's distance vector */
	struct distance_vector_ {
        char sender[50];
        int num_of_dests;
        struct element content[7];
    } distanceVector; /* Storage for node's distance vector */
    struct routing_table {
         int dist;
         char dest[30];
         char nextHop[30];
    }; /* Storage for node's routing table */
    struct neighbour_table {
         int dist;
         char dest[30];
         char IP[30];
    }; /* Storage for neighbour table */
	struct recvd_distance_vector_ {
        char sender[50];
        int num_of_dests;
        struct element content[8];
    } recvdDistanceVector; /* Storage for received distance vector */
    struct routing_table routingTable[7];
	struct neighbour_table neighourTable[7];
    char *servIP;                    /* IP address of server */
    unsigned short servPort;     /* Echo server port */
    void printDistVect(struct recvd_distance_vector_ recvdDistanceVector);
    void printRoutingTbl(struct routing_table routingTable[7]);
    void update(char* recv_node,int recv_cost,int index);
    void process_routing_tbl(struct recvd_distance_vector_ recvdDistanceVector, char* recv_node, struct routing_table routingTable[7], struct distance_vector_ distanceVector);
    void sendUDPPacket(char *servIP, int echoServPort,  struct distance_vector_ distanceVector);
    void sendPacketsToNeighbors(struct neighbour_table neighourTable[7], struct distance_vector_ distanceVector, int servPort);

int main(int argc, char **argv){
	///Initialize distance vector, neighourTable and received distance vector
	int count = 8;
	for (int i = 0; i<count; i++){
		distanceVector.content[i].dist = -50;
		strcpy(distanceVector.content[i].dest, "");
    recvdDistanceVector.content[i].dist = -50;
		strcpy(recvdDistanceVector.content[i].dest, "");
		neighourTable[i].dist = distanceVector.content[i].dist;
		strcpy(neighourTable[i].dest, "");
		strcpy(neighourTable[i].IP, "");
		routingTable[i].dist = -50;
	}
    strcpy(distanceVector.sender, argv[2]);
    distanceVector.num_of_dests = 7;
    ///Finish Initialize distance vector, neighourTable and received distance vector
    
  char myarray[10] = "ABCDEFGH";
    ///Initialize routing table
  strncpy(routingTable[0].dest,&myarray[0], 1);
  strncpy(routingTable[1].dest, &myarray[1], 1);
  strncpy(routingTable[2].dest, &myarray[2], 1);
  strncpy(routingTable[3].dest, &myarray[3], 1);
  strncpy(routingTable[4].dest, &myarray[4], 1);
  strncpy(routingTable[5].dest, &myarray[5], 1);
  strncpy(routingTable[6].dest, &myarray[6], 1);
    ///Finish Initialize routing table
    
    ///Read Configuration file and update Routing table, distance vector and neighour table
	FILE *fptr = NULL; //initate file pointer
	int i=0, N = 0; //counters
	char character;
	char filename[40];
	strcpy(filename, argv[1]);
	if (argc >= 2) {
		fptr = fopen(filename, "r"); //open file
		char configParam[50] = ""; //set to empty
		char configParamDist[50] = ""; //set to empty
		char configParamIP[50] = ""; //set to empty
		int n = 0;
		while (!feof(fptr)) //while it's not EOF
		{
			character = fgetc(fptr); //look at each character
			if (character == '\n'){ //if its newline character
				N++; //count the number of line its on as a new node
				if (N>1){
					char str [20];
					strcpy(str, "");
					sscanf (configParam,"%s ", str);                //get neighbor_1_name eg A and put in str;
					sscanf (configParamDist,"%d ", &i);             //get neighbor_1_cost eg 4 and put in i;
					strcpy(distanceVector.content[N-1].dest, str);  //Populate dist vector with neighbour name
					distanceVector.content[N-1].dist = i;           //populate Distance Vector with cost
					neighourTable[N-1].dist = i;                    //populate Neighbour table with cost
					strcpy(neighourTable[N-1].dest, str);           //populate Neighbour table with destination
					strcpy(neighourTable[N-1].IP, configParamIP);   //populate Neighbour table with IP
				}
				else if (N==1) servPort = atoi(configParam);        //Place content of first line as the server port
				strcpy(configParam, "");                            //set to empty
				strcpy(configParamDist, "");                        //set to empty
				strcpy(configParamIP, "");                          //set to empty
				n = 0;
			}
			else{
				if (n>=2) strncat(configParamDist, &character, 1);  //Read the cost and IP
				if (n>=4) strncat(configParamIP, &character, 1);    //Read the IP
				strncat(configParam, &character, 1);                //Read the whole line
				n++;
			}

		}
		char str [20];
		strcpy(str, "");
		sscanf (configParam,"%s ", str);                            //get neighbor_1_name eg A and put in str;
		sscanf (configParamDist,"%d ", &i);                         //get neighbor_1_cost eg 4 and put in i;
		strcat(distanceVector.content[N].dest, str);                //Populate dist vector with neighbour name
		strcpy(neighourTable[N].dest, str);                         //Populate Neighbour table with neighbour name
		distanceVector.content[N].dist = i;                          //populate DV with cost
		neighourTable[N].dist = i;                                   //Populate neighbour table with cost
		size_t len = strlen(configParamIP);
		memmove(configParamIP+len, configParamIP+len-2, len-2);      //Delete weird character at the end of file
		configParamIP[len-1] = 0;
		strcpy(neighourTable[N].IP, configParamIP);

	} else perror("Error: ");
	fclose(fptr);
    ///Finish reading Configuration file and update Routing table, distance vector and neighour table

    ///Populate routing table with initial values
      for (int z = 0; z < 8; z++) {
        for (i = 0; i < 8; i++) {
          if (strcmp(routingTable[i].dest, neighourTable[z].dest)==0) {
              routingTable[i].dist = neighourTable[z].dist;
              strcpy(routingTable[i].nextHop, argv[2]);
          }

        }
      }
    ///Finish Populating routing table with initial values

    ///Print intial routing table
	printf("Cost-----Destination-----NextHop\n");
	for (i = 0; i < 7; i++)
	{
		if (routingTable[i].dist>0){

		printf("%d          ", routingTable[i].dist);
		printf("%s        ", routingTable[i].dest);
		printf("      %s\n", routingTable[i].nextHop);
				}
	}
    ///Finish printing intial routing table

	sendPacketsToNeighbors(neighourTable, distanceVector, servPort);

	int rsock;                          /* Socket */
    struct sockaddr_in echoServAddr;    /* Local address */
  	unsigned short echoServPort;        /* Server port */
    echoServPort = servPort;            /* First arg:  local port */
    /* Create socket for sending/receiving datagrams */
    if ((rsock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(rsock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");


	struct sockaddr_in address;
	unsigned int len=sizeof(struct sockaddr_in);
	struct sigaction myAction;
	int respStringLen;

    for (;;) /* Run forever */
    {
    	myAction.sa_handler = CatchAlarm;
    	//if (sigfillset(&myAction.sa_mask) < 0) /* block everything in handler */
    	    //DieWithError("sigfillset() failed");
    	myAction.sa_flags = 0;

    	if (sigaction(SIGALRM, &myAction, 0) < 0)
    	    DieWithError("sigaction() failed for SIGALRM");

    	alarm(PERIOD);        /* Set the timeout */
    	while ((respStringLen = recvfrom(rsock, (struct recvd_distance_vector_*)&recvdDistanceVector, sizeof(recvdDistanceVector), 0,(struct sockaddr *) &address, &len)) < 0)
    	    if (errno == EINTR)     /* Alarm went off  */
    	    {
    	        if (tries < MAXTRIES)      /* incremented by signal handler */
    	        {
    	            printf("Resent Packet\n");
    	            sendPacketsToNeighbors(neighourTable, distanceVector, servPort); /* send packets to neighbours */
    	               // DieWithError("sendto() failed");
    	            alarm(PERIOD);
    	        }
    	    }
    	    else
    	        DieWithError("recvfrom() failed");
    	     printf("Distance vector received from: %s\n",recvdDistanceVector.sender); /* print that distance vector was received  */
    	
        char* recv_node;
        recv_node=recvdDistanceVector.sender;
        printDistVect(recvdDistanceVector); /* print that distance vector received  */
        process_routing_tbl(recvdDistanceVector, recv_node, routingTable, distanceVector); /* check if the routing table needs to be updated */
    	
        /* clear sender info from received distance vector storage  */
        strcpy(recvdDistanceVector.sender, "");
    	recvdDistanceVector.num_of_dests = 8;
    	for (int i = 0; i<count; i++){
    		recvdDistanceVector.content[i].dist = -50;
    		strcpy(recvdDistanceVector.content[i].dest, "");
    	}

    	/* recvfrom() got something --  cancel the timeout */
    	alarm(0);
    }

}

void sendUDPPacket(char *servIP, int echoServPort, struct distance_vector_ distanceVector){

	int sock;                        /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */


    /* Create a best-effort datagram socket using UDP */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    //Homie said it helps you to be able to bind your program again to the same port when rerunning it immediately after it has been aborted
    int one = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT,&one, sizeof(one));

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));    /* Zero out structure */
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
    echoServAddr.sin_port = htons(echoServPort);       /* Server port */
    if (sendto(sock, (struct distance_vector_*)&distanceVector, (sizeof(distanceVector)), 0, (struct sockaddr *)
	        &echoServAddr, sizeof(echoServAddr)) != sizeof(distanceVector))
          DieWithError("sendto() sent a different number of bytes than expected");
    close(sock);
}

void sendPacketsToNeighbors(struct neighbour_table neighourTable[7], struct distance_vector_ distanceVector, int servPort){
	int count = 8;
	for(int i=0; i<=count; i++){ // for all nodes

		if(neighourTable[i].dist > 0){ //check if node i is a neigbour

			sendUDPPacket(neighourTable[i].IP, servPort, distanceVector); //if node i is a neigbour send packet

		}
	}
}

/* check if the routing table needs to be updated */
void process_routing_tbl(struct recvd_distance_vector_ recvdDistanceVector, char* recv_node, struct routing_table routingTable[7], struct distance_vector_ distanceVector){
	int cost_curr_to_recv;

	int cnt =8;
	for(int i=0;i<cnt;i++)
	{
		if(strcmp(routingTable[i].dest,recv_node)==0) //get node in question
		{
			cost_curr_to_recv=routingTable[i].dist; //get current distance to the node received from

		}

	}
    //Distance vector algorithm
	for(int m=0;m<cnt;m++) //for all current routingTable entries
	{
		for(int k=0;k<cnt;k++) //for all recvd RoutingTable entries
		{
			if(strcmp(recvdDistanceVector.content[k].dest,routingTable[m].dest)==0) //if recvd entry destination matches a current entry
			{
				int cost_recv_to_dest=recvdDistanceVector.content[k].dist; // update cost recvd
				int recv_cost=cost_curr_to_recv+cost_recv_to_dest; // then add cost of neigbour to non neighbour node to cost of neighbour node to current node
          if (strcmp(routingTable[m].dest, distanceVector.sender)!= 0){
  						if(routingTable[m].dist>recv_cost && recv_cost>0){ //if node in question has lower cost and is positive
  							update(recv_node,recv_cost,m); //update routing table
  						}
  						else if (routingTable[m].dist<0)
  						{
  							if (recv_cost> 0) update(recv_node,recv_cost,m); //if node in question has positive higher cost but routing table cost is negative, update routing table
  						}
  				break;
          }
			}
		}
	}

}

/* updates routing table if the routing table needs to be updated */
void update(char* recv_node,int recv_cost,int index){
	routingTable[index].dist=recv_cost;
	strcpy(routingTable[index].nextHop,recv_node);
	printf("Updated Routing tbl: \n");
	printRoutingTbl(routingTable); /* Prints routing table if the routing table is updated */
    /* Check if distance vector has that entry populated */
    int rand = 0;
        for (int z = 0; z < 8; z++) {
                if (strcmp(routingTable[index].dest, distanceVector.content[z].dest)==0) {
                    printf("%s, %s", routingTable[index].dest, distanceVector.content[z].dest);
                     distanceVector.content[z].dist = recv_cost;
                    rand = 1;
                    break;
                }
        }
    /* If not add entry */
    char d[] = "";
    int w = 0;
    if (rand == 0){
        //find the empty slot
        while (1) {
            if (strcmp(distanceVector.content[w].dest, d) == 0){
                //Populate the distance vector
                distanceVector.content[w].dist = recv_cost;
                strcpy(distanceVector.content[w].dest, routingTable[index].dest);
                break;
            }
            w++;
        }
    }
    sendPacketsToNeighbors(neighourTable, distanceVector, servPort);
}

/* Prints routing table */
void printRoutingTbl(struct routing_table routingTable[7]){
	printf("Routing Table:\n");
	int cnt = 8;
	printf("Dest-----NextHop-----Cost\n");
	for(int i=0;i<cnt;i++){
		printf("%s        %s          %d\n", routingTable[i].dest, routingTable[i].nextHop, routingTable[i].dist);
	}
}

/* Prints routing table */
void printDistVect(struct recvd_distance_vector_ recvdDistanceVector){
	printf("Distance Vector:\n");
	int cnt = 8;
	printf("%s, %d\n", recvdDistanceVector.sender, recvdDistanceVector.num_of_dests);
    printf("Destination-----Cost\n");
	for(int i=0;i<cnt;i++){
		printf("%s                  %d\n", recvdDistanceVector.content[i].dest, recvdDistanceVector.content[i].dist);
	}
}

void CatchAlarm(int ignored){     /* Handler for SIGALRM */
      if (0) return;
  		else alarm(PERIOD);
}


void DieWithError(char *errorMessage){
    perror(errorMessage);
    exit(1);
}
