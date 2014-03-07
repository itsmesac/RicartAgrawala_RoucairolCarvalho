/*
 *    File             :    Initialize.h
 *    Author           :    Shachindra Chandrashekar
 *    Date             :    03/06/2014
 *    Course           :    Advanced Operating Systems
 *    Assignment       :    Programming Project 1
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <netinet/in.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <netdb.h>
#include <time.h>

#include <errno.h>
#include "messages.h"
#include "ConfigFileParser.h"

extern int  completed[MAX_NUM_NODES];

/*
    packet structure:
    int                         : ID of the node(assigned by leader in the order of connection)
    struct in_addr          : inet address of the hosts connected
*/
void createConnectionDataPacket(connection_info* pConn_info, char* retBuf);


/*
    Called by the leader; to wait for incoming connections
    In addition to accepting connections, this function also
    sends the connection data packets to its clients
*/
void getAllConnections(connection_info* pConn_info);


/*
    creates the listener socket for the leader
    and waits for connections
*/
void startLeader(connection_info* pConn_info);


/*
    extracts the ID of the node from the packet sent by the leader
*/
int extractID(char* pConnectionDataPacket);


/*
    extracts ith(indexed starts from 1) in_addr 
    struct from the connection data packet
    sent by the leader
*/
struct in_addr extract_in_addr(char* pConnectionDataPacket, int i);

/*
    called by non-leaders to establish connections with other nodes
*/
void establishConnections(const char* configFileName, char* leaderName, unsigned short int leaderPortNum, connection_info* pConn_info);


/*
    called by non-leaders to connect to the leader
*/
void connectToLeader(char* leaderName, unsigned short int leaderPortNum, connection_info* pConn_info);


/*
    Once the node has been connected to the leader,
    this function is called by non-leaders to connect
    to the other *UP* non-leaders
*/
    void connectToNodes(const char* configFileName, connection_info* pConn_info,char* connectionDataPacket, unsigned short int leaderPortNum);


/*
    Called by non-leaders(except the last node to join) 
    after they have connected to the leader
    and other *UP* non-leaders.

    Creates their listen sockets to accept incoming connections from other non-leaders
*/
void waitForIncomingConnections(connection_info* pConn_info);


/*
    test connections between a node and other nodes
*/
void testConnections(connection_info* pConn_info);

void sendStartProcessMessageToAll(connection_info* pConn_info);

int isCompletedSentByAll();

int isInitDoneByAll(int* initDone);










