/*
 *    File             :    Initialize.c
 *    Author           :    Shachindra Chandrashekar
 *    Date             :    03/06/2014
 *    Course           :    Advanced Operating Systems
 *    Assignment       :    Programming Project 1
 */

#include "Initialize.h"

extern FILE* debugFile;

/*
    packet structure:
    int                         : ID of the node(assigned by leader in the order of connection)
    struct in_addr          : inet address of the hosts connected
*/
void createConnectionDataPacket(connection_info* pConn_info, char* retBuf)
{
    int i = 0;
    char* writePointer = retBuf;
    *((int*)writePointer) = pConn_info->numConnected;
    writePointer = retBuf + sizeof(int);

    for(i = 1; i <= (pConn_info->numConnected - 1); i++)
    {
        memcpy(writePointer,&(pConn_info->nodeAddr[i].sin_addr),sizeof(struct in_addr));
        writePointer += sizeof(struct in_addr);
    }
}


/*
    Called by the leader; to wait for incoming connections
    In addition to accepting connections, this function also
    sends the connection data packets to its clients
*/
void getAllConnections(connection_info* pConn_info)
{
    message msg;

    for (;;) /* Run until all host are connected */
    {
        char connectionDataPacket[MAX_BUF_LENGTH];

        pConn_info->numConnected++;
        int clntLen = sizeof(pConn_info->nodeAddr[pConn_info->numConnected]);

        /* Wait for a client to connect */
        if ((pConn_info->nodeSock[pConn_info->numConnected] = 
            accept(pConn_info->nodeSock[pConn_info->myID], 
            (struct sockaddr *) &pConn_info->nodeAddr[pConn_info->numConnected],
            &clntLen)) < 0)
        {
            fprintf(stdout,"\nleader: accept failed\n");
            fprintf(stdout,"\nleader : error : %s",strerror(errno));
            fflush(stdout);
            while(1);
        }

        pConn_info->nodePort[pConn_info->numConnected] = 
            pConn_info->nodeAddr[pConn_info->numConnected].sin_port;

        /* clntSock is connected to a client! */
        fprintf(stdout,"\nleader : Node %d connected",pConn_info->numConnected);
        fflush(stdout);

        msg.msg_type = MSG_CONN_DATA;
        createConnectionDataPacket(pConn_info,msg.buffer);

        /* send connection information to connected node */
        if(-1 == send(pConn_info->nodeSock[pConn_info->numConnected],
                (void*)&msg,sizeof(message),0))
        {
            fprintf(stdout,"\nleader: send to %d failed\n",pConn_info->numConnected);
            fprintf(stdout,"\nleader : error : %s",strerror(errno));
            fflush(stdout);
            while(1);
        }

        fprintf(stdout,"\nleader : sent conn data to node %d",pConn_info->numConnected);
        fflush(stdout);

        if(pConn_info->numConnected == (num_of_nodes - 1))
        {
            fprintf(stdout,"\nleader : All nodes connected to leader",pConn_info->numConnected);
            fflush(stdout);
            break;
        }
    }
}


/*
    creates the listener socket for the leader
    and waits for connections
*/
void startLeader(connection_info* pConn_info)
{
    int i;
    /* Create socket for incoming connections */
    if ((pConn_info->nodeSock[pConn_info->myID] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stdout,"\nleader: can't create my socket\n");
        fprintf(stdout,"\nleader : error : %s",strerror(errno));
        fflush(stdout);
        while(1);
    }

    /* Construct local address structure */
    memset(&pConn_info->nodeAddr[pConn_info->myID], 0, sizeof(pConn_info->nodeAddr[pConn_info->myID]));   /* Zero out structure */
    pConn_info->nodeAddr[pConn_info->myID].sin_family = AF_INET;                /* Internet address family */
    pConn_info->nodeAddr[pConn_info->myID].sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    pConn_info->nodeAddr[pConn_info->myID].sin_port = htons(pConn_info->nodePort[pConn_info->myID]);      /* Local port */

    /* Bind to the local address */
    if (bind(pConn_info->nodeSock[pConn_info->myID], 
        (struct sockaddr *) &pConn_info->nodeAddr[pConn_info->myID],
        sizeof(pConn_info->nodeAddr[pConn_info->myID])) < 0)
    {
        fprintf(stdout,"\nleader: bind failed\n");
        fprintf(stdout,"\nleader : error : %s",strerror(errno));
        fflush(stdout);
        while(1);
    }

    /* Mark the socket so it will listen for incoming connections */
    if (listen(pConn_info->nodeSock[pConn_info->myID], MAX_PENDING) < 0)
    {
        fprintf(stdout,"\nleader: listen failed\n");
        fprintf(stdout,"\nleader : error : %s",strerror(errno));
        fflush(stdout);
        while(1);
    }

    getAllConnections(pConn_info);
#if 0
    fprintf(stdout,"\n SOCK FDs :\n");
    for(i = 0; i < MAX_NUM_NODES; i++)
    {
        fprintf(stdout,"\t%d",pConn_info->nodeSock[i]);
    }
    fflush(stdout);
#endif
}


/*
    extracts the ID of the node from the packet sent by the leader
*/
int extractID(char* pConnectionDataPacket)
{
    int* readPointer = (int*)pConnectionDataPacket;
    return *readPointer;
}


/*
    extracts ith(indexed starts from 1) in_addr 
    struct from the connection data packet
    sent by the leader
*/
struct in_addr extract_in_addr(char* pConnectionDataPacket, int i)
{
    char* readPointer = pConnectionDataPacket + sizeof(int);
    struct in_addr retVal;
    while((i - 1) != 0)
    {
        readPointer += sizeof(struct in_addr);
        //readPointer += sizeof(unsigned short int);
        i--;
    }
    memcpy(&retVal,(struct in_addr*)readPointer,sizeof(struct in_addr));
    return retVal;
}


/*
    called by non-leaders to connect to the leader
*/
void connectToLeader(char* leaderName, unsigned short int leaderPortNum, connection_info* pConn_info)
{
    struct hostent *h;
    h=gethostbyname(leaderName);

    /* Create a reliable, stream socket using TCP */
    if ((pConn_info->nodeSock[0] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stdout,"\nnode: can't create my socket\n");
        fprintf(stdout,"\nnode : error : %s",strerror(errno));
        fflush(stdout);
        while(1);
    }

    /* Construct the server address structure */
    memset(&pConn_info->nodeAddr[0], 0, sizeof(pConn_info->nodeAddr[0]));     /* Zero out structure */
    pConn_info->nodeAddr[0].sin_family      = AF_INET;             /* Internet address family */
    pConn_info->nodeAddr[0].sin_addr        = *((struct in_addr *) h->h_addr);   /* Server IP address */
    pConn_info->nodeAddr[0].sin_port        = htons(leaderPortNum); /* Server port */

    /* Establish the connection to the leader */
    if (connect(pConn_info->nodeSock[0], (struct sockaddr *) &pConn_info->nodeAddr[0], sizeof(pConn_info->nodeAddr[0])) < 0)
    {
        fprintf(stdout,"\nnode: connect failed\n");
        fprintf(stdout,"\nnode : error : %s",strerror(errno));
        fflush(stdout);
        while(1);
    }

    fprintf(stdout,"\nnode : connected to leader");
    fflush(stdout);
}


/*
    Once the node has been connected to the leader,
    this function is called by non-leaders to connect
    to the other *UP* non-leaders
*/
void connectToNodes(const char* configFileName, connection_info* pConn_info,char* connectionDataPacket, unsigned short int leaderPortNum)
{
    unsigned short int i = 0;
    //connect to all nodes which had previously connected to the leader
    for(i = 1; i <= (pConn_info->myID - 1); i++)
    {
        memset(&pConn_info->nodeAddr[i], 0, sizeof(pConn_info->nodeAddr[i]));           /* Zero out structure */
        pConn_info->nodeAddr[i].sin_family      = AF_INET;                              /* Internet address family */
        pConn_info->nodeAddr[i].sin_addr        = extract_in_addr(connectionDataPacket,i);  /* Server IP address */
        //pConn_info->nodeAddr[i].sin_port        = htons(pConn_info->nodePort[pConn_info->myID] + i);/* Server port */
        //pConn_info->nodeAddr[i].sin_port        = htons(leaderPortNum + i);/* Server port */
        pConn_info->nodeAddr[i].sin_port        = htons(getPortNum(configFileName,i));

        fprintf(stdout,"\nnode(%d) : port : %hu",pConn_info->myID,pConn_info->nodeAddr[i].sin_port);
        fflush(stdout);

        /* Create a reliable, stream socket using TCP */
        if ((pConn_info->nodeSock[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            fprintf(stdout,"\nnode(%d): can't create my socket\n",pConn_info->myID);
            fprintf(stdout,"\nnode(%d) : error : %s",pConn_info->myID,strerror(errno));
            fflush(stdout);
            while(1);
        }

        /* Establish the connection to the node */
        if (connect(pConn_info->nodeSock[i], (struct sockaddr *) &pConn_info->nodeAddr[i], sizeof(pConn_info->nodeAddr[i])) < 0)
        {
            fprintf(stdout,"\nnode(%d): connect failed\n",pConn_info->myID);
            fprintf(stdout,"\nnode(%d) : error : %s",pConn_info->myID,strerror(errno));
            fflush(stdout);

            while(1);
        }

        fprintf(stdout,"\nnode(%d) : connected to node %d",pConn_info->myID,i);
        fflush(stdout);

        pConn_info->numConnected++;
    }
}


/*
    Called by non-leaders(except the last node to join) 
    after they have connected to the leader
    and other *UP* non-leaders.

    Creates their listen sockets to accept incoming connections from other non-leaders
*/
void waitForIncomingConnections(connection_info* pConn_info)
{
    /* Create socket for incoming connections */
    if ((pConn_info->nodeSock[pConn_info->myID] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stdout,"\nnode(%d): can't create my socket\n",pConn_info->myID);
        fprintf(stdout,"\nnode(%d) : error : %s",strerror(errno),pConn_info->myID);
        fflush(stdout);
        while(1);
    }

    fprintf(stdout,"\nnode(%d) : listen socket created",pConn_info->myID);
    fflush(stdout);

    /* Construct local address structure */
    memset(&pConn_info->nodeAddr[pConn_info->myID], 0, sizeof(pConn_info->nodeAddr[pConn_info->myID]));   /* Zero out structure */
    pConn_info->nodeAddr[pConn_info->myID].sin_family = AF_INET;                /* Internet address family */
    pConn_info->nodeAddr[pConn_info->myID].sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    pConn_info->nodeAddr[pConn_info->myID].sin_port = htons(pConn_info->nodePort[pConn_info->myID]);      /* Local port */

    /* Bind to the local address */
    if (bind(pConn_info->nodeSock[pConn_info->myID], 
        (struct sockaddr *) &pConn_info->nodeAddr[pConn_info->myID],
        sizeof(pConn_info->nodeAddr[pConn_info->myID])) < 0)
    {
        fprintf(stdout,"\nnode(%d): bind failed\n",pConn_info->myID);
        fprintf(stdout,"\nnode(%d) : error : %s",pConn_info->myID,strerror(errno));
        fflush(stdout);
        while(1);
    }

    /* Mark the socket so it will listen for incoming connections */
    if (listen(pConn_info->nodeSock[pConn_info->myID], MAX_PENDING) < 0)
    {
        fprintf(stdout,"\nnode(%d): listen failed\n",pConn_info->myID);
        fprintf(stdout,"\nnode(%d) : error : %s",pConn_info->myID,strerror(errno));
        fflush(stdout);
        while(1);
    }

    for (;;)
    {
        pConn_info->numConnected++;
        int clntLen = sizeof(pConn_info->nodeAddr[pConn_info->numConnected]);

        /* Wait for a client to connect */
        if ((pConn_info->nodeSock[pConn_info->numConnected] = 
            accept(pConn_info->nodeSock[pConn_info->myID], 
            (struct sockaddr *) &pConn_info->nodeAddr[pConn_info->numConnected],
            &clntLen)) < 0)
        {
            fprintf(stdout,"\nnode(%d): accept failed\n",pConn_info->myID);
            fprintf(stdout,"\nnode(%d) : error : %s",pConn_info->myID,strerror(errno));
            fflush(stdout);
            while(1);
        }

        pConn_info->nodePort[pConn_info->numConnected] = 
            pConn_info->nodeAddr[pConn_info->numConnected].sin_port;

        /* clntSock is connected to a client! */
        fprintf(stdout,"\nnode(%d) : Node %d connected",pConn_info->myID,pConn_info->numConnected);
        fflush(stdout);

        if(pConn_info->numConnected == (num_of_nodes - 1))
        {
            fprintf(stdout,"\nnode(%d) : All nodes connected",pConn_info->myID,pConn_info->numConnected);
            fflush(stdout);
            break;
        }
    }
}


/*
    sends an init done message to leader
*/
void sendInitDoneToLeader(connection_info* pConn_info)
{
    message msg;
    msg.msg_type = MSG_INIT_DONE;

    if(-1 == send(pConn_info->nodeSock[0],
                    (void*)&msg,sizeof(message),0))
    {
        fprintf(stdout,"\nnode(%d): send to %d failed\n",pConn_info->myID,pConn_info->numConnected);
        fprintf(stdout,"\nnode(%d) : error : %s",pConn_info->myID,strerror(errno));
        fflush(stdout);
        while(1);
    }
}

/*
    called by non-leaders to establish connections with other nodes
*/
void establishConnections(const char* configFileName, char* leaderName, unsigned short int leaderPortNum, connection_info* pConn_info)
{
    message msg;

    int i = 0;

    connectToLeader(leaderName,leaderPortNum,pConn_info);

    //connected to the leader; get the connection information from the leader
    if(-1 == recv(pConn_info->nodeSock[0],&msg,sizeof(message),0))
    {
        fprintf(stdout,"\nnode(%d): recv from leader failed\n",pConn_info->myID);
        fprintf(stdout,"\nnode(%d) : error : %s",pConn_info->myID,strerror(errno));
        fflush(stdout);
        while(1);
    }

    fprintf(stdout,"\nnode(%d) : received connection data from leader",pConn_info->myID);
    fflush(stdout);

    //get my id from the packet sent by the leader
    //pConn_info->myID = extractID(msg.buffer);
    pConn_info->numConnected = 1;   //Just connected to the leader yet.
    //pConn_info->nodePort[pConn_info->myID] = leaderPortNum + pConn_info->myID;

    connectToNodes(configFileName,pConn_info,msg.buffer,leaderPortNum);

    //if this is the last node joining, then all TCP connections have already been established
    //so just return
    if(pConn_info->numConnected == (num_of_nodes - 1))
    {
        fprintf(stdout,"\nnode(%d) : All connections established",pConn_info->myID,pConn_info->numConnected);
        fflush(stdout);
    }
    else
    {
        waitForIncomingConnections(pConn_info);
    }

    sendInitDoneToLeader(pConn_info);
}


/*
    test connections between a node and other nodes
*/
void testConnections(connection_info* pConn_info)
{
    int greatestSockFD = -1;
    int i = 0;
    message msg;
    fd_set readfds;

    //send tcp segments
    for(i = 0; i < num_of_nodes; i++)
    {
        if(i == pConn_info->myID)
            continue;

        msg.msg_type = MSG_TEST_CONN;
        sprintf(msg.buffer,"Message from %d to %d",pConn_info->myID,i);
        if(send(pConn_info->nodeSock[i],&msg,sizeof(message),0) < 0)
        {
            fprintf(stdout,"\nnode(%d): send to %d failed\n",pConn_info->myID,pConn_info->numConnected);
            fprintf(stdout,"\nnode(%d) : error : %s",pConn_info->myID,strerror(errno));
            fflush(stdout);
            while(1);
        }
        fprintf(stdout,"\nnode(%d): Message sent to %d, message content : %s",pConn_info->myID,i,msg.buffer);
        fflush(stdout);
    }

    for(i = 0; i < num_of_nodes; i++)
    {
        if(i == pConn_info->myID)
            continue;

        if(pConn_info->nodeSock[i] > greatestSockFD)
            greatestSockFD = pConn_info->nodeSock[i];
    } 

    //receive tcp segments
    while(1)
    {
        int rv = -1;

        FD_ZERO(&readfds);
        for(i = 0; i < num_of_nodes; i++)
        {
            if(i == pConn_info->myID)
                continue;
            FD_SET(pConn_info->nodeSock[i],&readfds);
        } 

        fprintf(stdout,"\nnode(%d) : Blocking on select",pConn_info->myID);
        fflush(stdout);

        if((rv = select(greatestSockFD + 1,&readfds,NULL,NULL,NULL)) < 0)
        {
            fprintf(stdout,"\nnode(%d): select failed\n",pConn_info->myID);
            fprintf(stdout,"\nnode(%d) : error : %s",pConn_info->myID,strerror(errno));
            fflush(stdout);
            while(1);
        }

        fprintf(stdout,"\nnode(%d) : Return value of select is %d",pConn_info->myID,rv);
        fflush(stdout);

        for(i = 0; i < num_of_nodes; i++)
        {
            if(i == pConn_info->myID)
                continue;

            if (FD_ISSET(pConn_info->nodeSock[i], &readfds))
            {
                if((rv = recv(pConn_info->nodeSock[i],&msg, sizeof(message), 0)) < 0)
                {
                    fprintf(stdout,"\nnode(%d): recv from node %d failed\n",pConn_info->myID,i);
                    fprintf(stdout,"\nnode(%d): error : %s",pConn_info->myID,strerror(errno));
                    fflush(stdout);
                    while(1);
                }

                fprintf(stdout,"\nnode(%d) : %s, %d bytes recd",pConn_info->myID,msg.buffer,rv);
                fflush(stdout);
            }
        }
    }
}

int isInitDoneByAll(int* initDone)
{
    int i = 1;
    for(i = 1; i < num_of_nodes; i++)
    {
        if(initDone[i] == 0)
            return 0;
    }
    return 1;
}

int isCompletedSentByAll()
{
    int i = 0;
    for(i = 0; i < num_of_nodes; i++)
    {
        if(completed[i] == 0)
            return 0;
    }
    return 1;
}

/*
    sent by leader to all the other nodes to indicate start of all processes.
*/
void sendStartProcessMessageToAll(connection_info* pConn_info)
{
    int i = 0;

    message msg;
    msg.msg_type = MSG_START_PROCESS;

    for(i = 1; i < num_of_nodes; i++)
    {
        if(-1 == send(pConn_info->nodeSock[i],
                (void*)&msg,sizeof(message),0))
        {
            fprintf(stdout,"\nleader: send of start message to %d failed\n",pConn_info->numConnected);
            fprintf(stdout,"\nleader : error : %s",strerror(errno));
            fflush(stdout);
            while(1);
        }
    }
}

