/*
 *    File             :    Messages.h
 *    Author           :    Shachindra Chandrashekar
 *    Date             :    03/06/2014
 *    Course           :    Advanced Operating Systems
 *    Assignment       :    Programming Project 1
 */

#define MSG_INIT_DONE                   1
#define MSG_START_PROCESS               2
#define MSG_END_PROCESS                 3
#define MSG_REQUEST                     4
#define MSG_RESPONSE                    5
#define MSG_CONN_DATA                   6
#define MSG_TEST_CONN                   7
#define MSG_TERMINATE                   8

#define MAX_PENDING                     10
#define MAX_NUM_NODES                   20
#define MAX_BUF_LENGTH                  256

#define NUM_CS_ACCESSES                 20

#define WAIT_MIN                        10
#define WAIT_MAX                        100

#define WAIT_MIN_EVEN                   200
#define WAIT_MAX_EVEN                   500

#define CS_DURATION                     20

#define MIN_MESSAGE_COUNT_PER_CS_ACCESS 999999
#define MAX_MESSAGE_COUNT_PER_CS_ACCESS 0

typedef struct _connection_info_
{
    unsigned short int nodePort[MAX_NUM_NODES];
    int nodeSock[MAX_NUM_NODES];
    struct sockaddr_in nodeAddr[MAX_NUM_NODES];

    int myID;
    int numConnected;
}connection_info;

typedef struct _message_request
{
    int msg_type;
    char buffer[MAX_BUF_LENGTH];
}message;

typedef struct _request
{
    unsigned int timestamp;
}request;

typedef struct _response
{
    unsigned int timestamp;
}response;

typedef struct _end_notification
{
    int total_number_of_requests;
    int total_number_of_responses;
}end_notification;

int num_of_nodes;


