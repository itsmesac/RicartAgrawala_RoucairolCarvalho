/*
 *    File             :    Node.c
 *    Author           :    Shachindra Chandrashekar
 *    Date             :    03/06/2014
 *    Course           :    Advanced Operating Systems
 *    Assignment       :    Programming Project 1
 */


#include "Initialize.h"

FILE*       debugFile;
FILE*       dataFile_odd;
FILE*       dataFile_even;

int         using;
int         waiting;

int         permissions[MAX_NUM_NODES];
int         completed[MAX_NUM_NODES];
int         reply_deferred[MAX_NUM_NODES];

int         Our_Sequence_Number;
int         Hgst_Sequence_Number;

pthread_mutex_t lock;

/*  Data collection variables*/
int         total_num_of_request_messages;
int         total_num_of_response_messages;

int         total_request_count_of_all;
int         total_response_count_of_all;

int         number_of_msgs_per_cs_access;
double      time_reqd_per_cs_access;
int         max_num_of_msgs_per_cs_access;
int         min_num_of_msgs_per_cs_access;

double      avg_waiting_time_for_first_half_cs_requests;
double      avg_waiting_time_for_second_half_cs_requests;

void sendTerminateToAll(connection_info* pConn_info)
{
    int i = 0;

    message msg;
    msg.msg_type = MSG_TERMINATE;

    for(i = 1; i < num_of_nodes; i++)
    {
        if(-1 == send(pConn_info->nodeSock[i],
                (void*)&msg,sizeof(message),0))
        {
            fprintf(stdout,"\nleader: send of terminate message to %d failed\n",pConn_info->numConnected);
            fprintf(stdout,"\nleader : error : %s",strerror(errno));
            fflush(stdout);
            while(1);
        }
    }

    fprintf(stdout,"\nleader: send of terminate message to all done\n");
    fflush(stdout);


}

void sendCompletionNotificationToLeader(connection_info* pConn_info)
{
    message msg;
    msg.msg_type = MSG_END_PROCESS;

    end_notification endNotif;
    endNotif.total_number_of_requests = total_num_of_request_messages;
    endNotif.total_number_of_responses = total_num_of_response_messages;

    memcpy(msg.buffer,&endNotif,sizeof(end_notification));

    if(-1 == send(pConn_info->nodeSock[0],
                    (void*)&msg,sizeof(message),0))
    {
        fprintf(stdout,"\nnode(%d): send to %d failed\n",pConn_info->myID,pConn_info->numConnected);
        fprintf(stdout,"\nnode(%d) : error : %s",pConn_info->myID,strerror(errno));
        fflush(stdout);
        while(1);
    }
    
    fprintf(stdout,"\nnode(%d): sent completion notif to leader\n",pConn_info->myID);
    fflush(stdout);
}

void sendRequest(connection_info* pConn_info,int id)
{
    //fprintf(stdout,"\nnode(%d) : sendRequest(%d) invoked",pConn_info->myID,id);
    //fflush(stdout);

    message msg;
    request req;

    msg.msg_type = MSG_REQUEST;
    req.timestamp = Our_Sequence_Number;
    memcpy(msg.buffer,&req,sizeof(request));

    if(permissions[id] == 0)
    {
        //fprintf(stdout,"\nnode(%d) : sendRequest(%d) : request sent with seq no : %d",pConn_info->myID,id,req.timestamp);
        //fflush(stdout);

        number_of_msgs_per_cs_access++;
        total_num_of_request_messages++;
        if(-1 == send(pConn_info->nodeSock[id],
                (void*)&msg,sizeof(message),0))
        {
            fprintf(stdout,"\nnode(%d): send of resp to %d failed\n",pConn_info->myID,id);
            fprintf(stdout,"\nnode(%d) : error : %s",pConn_info->myID,strerror(errno));
            fflush(stdout);
            while(1);
        }
    }
    //fprintf(stdout,"\nnode(%d) : sendRequest(%d) finished",pConn_info->myID,id);
    //fflush(stdout);
}

void requestCritSecAccess(connection_info* pConn_info)
{
    //fprintf(stdout,"\nnode(%d) : requestCritSecAccess() invoked",pConn_info->myID);
    //fflush(stdout);

    int i = 0;
    waiting = 1;

    Our_Sequence_Number = Hgst_Sequence_Number + 1;

    for(i = 0; i < num_of_nodes; i++)
    {
        if(i == pConn_info->myID)
            continue;
        sendRequest(pConn_info,i);
    }

    //fprintf(stdout,"\nnode(%d) : requestCritSecAccess() finished",pConn_info->myID);
    //fflush(stdout);

}

void sendResponse(connection_info* pConn_info,int id)
{
    //fprintf(stdout,"\nnode(%d) : sendResponse(%d) invoked",pConn_info->myID,id);
    //fflush(stdout);

    message     msg;
    response    resp;

    msg.msg_type = MSG_RESPONSE;
    resp.timestamp = Our_Sequence_Number;
    memcpy(msg.buffer,&resp,sizeof(response));

    total_num_of_response_messages++;

    if(-1 == send(pConn_info->nodeSock[id],
            (void*)&msg,sizeof(message),0))
    {
        fprintf(stdout,"\nnode(%d): send of resp to %d failed\n",pConn_info->myID,id);
        fprintf(stdout,"\nnode(%d) : error : %s",pConn_info->myID,strerror(errno));
        fflush(stdout);
        while(1);
    }

    //fprintf(stdout,"\nnode(%d) : sendResponse(%d) finished",pConn_info->myID,id);
    //fflush(stdout);

}

void sendDeferredResponses(connection_info* pConn_info)
{
    int i = 0;
    for(i = 0; i < num_of_nodes; i++)
    {
        if(i == pConn_info->myID)
            continue;
            
        if(reply_deferred[i] == 1)
        {
            permissions[i] = 0;
            reply_deferred[i] = 0;
            sendResponse(pConn_info,i);
        }
    }
}

void waitForCS(connection_info* pConn_info)
{
    int j = 0;
    do
    {
        //pthread_mutex_lock(&perm_lock);
        pthread_mutex_lock(&lock);
        for(j = 0; j < num_of_nodes; j++)
        {
            if(j == pConn_info->myID)
                continue;
        
            if(permissions[j] == 0)
            {
                j = 0;
                //pthread_mutex_unlock(&perm_lock);
                pthread_mutex_unlock(&lock);
                break;
            }
        }

        if(j == num_of_nodes)
        {
            break;
        }
    }while(1);
    waiting = 0;
    using = 1;
    //pthread_mutex_unlock(&perm_lock);
    pthread_mutex_unlock(&lock);
}

void computeThreadHandler(void* arg)
{
    int cCritSec = 0;
    connection_info* pConn_info = (connection_info*)arg;
    clock_t start, end;
    
    memset(permissions,0,sizeof(int) * MAX_NUM_NODES);

    while(cCritSec < NUM_CS_ACCESSES)
    {
        fprintf(stdout,"\nnode(%d) : CS count %d",pConn_info->myID,cCritSec);
        fflush(stdout);

        int waitTime = WAIT_MIN + (rand() % (WAIT_MAX - WAIT_MIN));
        usleep(waitTime * 1000);

        pthread_mutex_lock(&lock);
        requestCritSecAccess(pConn_info);
        pthread_mutex_unlock(&lock);

        start = clock();
        waitForCS(pConn_info);
        end = clock();

        debugFile = fopen("logs.txt","a");
        fprintf(debugFile,"\n%d\t\tEntering",pConn_info->myID);
        fclose(debugFile);

        usleep(CS_DURATION * 1000);

        debugFile = fopen("logs.txt","a");
        fprintf(debugFile,"\n%d\t\tLeaving",pConn_info->myID);;
        fclose(debugFile);

        pthread_mutex_lock(&lock);

        if(max_num_of_msgs_per_cs_access < number_of_msgs_per_cs_access) 
            max_num_of_msgs_per_cs_access = number_of_msgs_per_cs_access;
        
        if(min_num_of_msgs_per_cs_access > number_of_msgs_per_cs_access) 
            min_num_of_msgs_per_cs_access = number_of_msgs_per_cs_access;

        time_reqd_per_cs_access = ((double)(end - start)/CLOCKS_PER_SEC);

        avg_waiting_time_for_first_half_cs_requests += time_reqd_per_cs_access;

        if(pConn_info->myID % 2)
        {
            dataFile_odd = fopen("DataFile_odd.txt","a");
            fprintf(dataFile_odd,"\n%d\t\t:\t\t%d\t\t:\t\t%d\t\t:\t\t%.2f",
                        pConn_info->myID,(cCritSec + 1),number_of_msgs_per_cs_access,time_reqd_per_cs_access);
            fclose(dataFile_odd);
        }
        else
        {
            dataFile_even = fopen("DataFile_even.txt","a");
            fprintf(dataFile_even,"\n%d\t\t:\t\t%d\t\t:\t\t%d\t\t:\t\t%.2f",
                        pConn_info->myID,(cCritSec + 1),number_of_msgs_per_cs_access,time_reqd_per_cs_access);
            fclose(dataFile_even);
        }

        fprintf(stdout,"\n\n********************************************************************************");
        fprintf(stdout,"\n%d messages sent/received for CS access",number_of_msgs_per_cs_access);
        fprintf(stdout,"\n%.2f seconds reqd for CS access",time_reqd_per_cs_access);
        fprintf(stdout,"\n********************************************************************************\n\n");
        fflush(stdout);

        number_of_msgs_per_cs_access = 0;

        using = 0;
        //fprintf(stdout,"\nnode(%d) : finished executing CS",pConn_info->myID);
        //fflush(stdout);

        //send defereed responses to all requests in the PQ and remove them from PQ
        sendDeferredResponses(pConn_info);

        pthread_mutex_unlock(&lock);
        cCritSec++;
    }

#if 1
    if((pConn_info->myID % 2) == 0)
    {
        while(cCritSec < (NUM_CS_ACCESSES + NUM_CS_ACCESSES))
        {
            fprintf(stdout,"\nnode(%d) : CS count %d",pConn_info->myID,cCritSec);
            fflush(stdout);

            int waitTime = WAIT_MIN_EVEN + (rand() % (WAIT_MAX_EVEN - WAIT_MIN_EVEN));

            pthread_mutex_lock(&lock);
            requestCritSecAccess(pConn_info);
            pthread_mutex_unlock(&lock);

            start = clock();
            waitForCS(pConn_info);
            end = clock();

            debugFile = fopen("logs.txt","a");
            fprintf(debugFile,"\n%d\t\tEntering",pConn_info->myID);
            fclose(debugFile);

            usleep(CS_DURATION * 1000);

            debugFile = fopen("logs.txt","a");
            fprintf(debugFile,"\n%d\t\tLeaving",pConn_info->myID);
            fclose(debugFile);

            pthread_mutex_lock(&lock);

            if(max_num_of_msgs_per_cs_access < number_of_msgs_per_cs_access) 
                max_num_of_msgs_per_cs_access = number_of_msgs_per_cs_access;

            if(min_num_of_msgs_per_cs_access > number_of_msgs_per_cs_access) 
                min_num_of_msgs_per_cs_access = number_of_msgs_per_cs_access;

            time_reqd_per_cs_access = ((double)(end - start)/CLOCKS_PER_SEC);

            avg_waiting_time_for_second_half_cs_requests += time_reqd_per_cs_access;

            if(pConn_info->myID % 2)
            {
                dataFile_odd = fopen("DataFile_odd.txt","a");
                fprintf(dataFile_odd,"\n%d\t\t:\t\t%d\t\t:\t\t%d\t\t:\t\t%.2f",
                            pConn_info->myID,(cCritSec + 1),number_of_msgs_per_cs_access,time_reqd_per_cs_access);
                fclose(dataFile_odd);
            }
            else
            {
                dataFile_even = fopen("DataFile_even.txt","a");
                fprintf(dataFile_even,"\n%d\t\t:\t\t%d\t\t:\t\t%d\t\t:\t\t%.2f",
                            pConn_info->myID,(cCritSec + 1),number_of_msgs_per_cs_access,time_reqd_per_cs_access);
                fclose(dataFile_even);
            }

            fprintf(stdout,"\n\n********************************************************************************");
            fprintf(stdout,"\n%d messages sent/received for CS access",number_of_msgs_per_cs_access);
            fprintf(stdout,"\n%.2f seconds reqd for CS access",time_reqd_per_cs_access);
            fprintf(stdout,"\n********************************************************************************\n\n");
            fflush(stdout);

            number_of_msgs_per_cs_access = 0;

            using = 0;
            //fprintf(stdout,"\nnode(%d) : finished executing CS",pConn_info->myID);
            //fflush(stdout);

            //send defereed responses to all requests in the PQ and remove them from PQ
            sendDeferredResponses(pConn_info);

            pthread_mutex_unlock(&lock);

            usleep(waitTime * 1000);
            cCritSec++;
        }
    }
    else
    {
        while(cCritSec < (NUM_CS_ACCESSES + NUM_CS_ACCESSES))
        {
            fprintf(stdout,"\nnode(%d) : CS count %d",pConn_info->myID,cCritSec);
            fflush(stdout);

            int waitTime = WAIT_MIN + (rand() % (WAIT_MAX - WAIT_MIN));
            usleep(waitTime * 1000);

            pthread_mutex_lock(&lock);
            requestCritSecAccess(pConn_info);
            pthread_mutex_unlock(&lock);

            start = clock();
            waitForCS(pConn_info);
            end = clock();

            debugFile = fopen("logs.txt","a");
            fprintf(debugFile,"\n%d\t\tEntering",pConn_info->myID);
            fclose(debugFile);

            usleep(CS_DURATION * 1000);

            debugFile = fopen("logs.txt","a");
            fprintf(debugFile,"\n%d\t\tLeaving",pConn_info->myID);
            fclose(debugFile);

            pthread_mutex_lock(&lock);

            if(max_num_of_msgs_per_cs_access < number_of_msgs_per_cs_access) 
                max_num_of_msgs_per_cs_access = number_of_msgs_per_cs_access;

            if(min_num_of_msgs_per_cs_access > number_of_msgs_per_cs_access) 
                min_num_of_msgs_per_cs_access = number_of_msgs_per_cs_access;

            time_reqd_per_cs_access = ((double)(end - start)/CLOCKS_PER_SEC);

            avg_waiting_time_for_second_half_cs_requests += time_reqd_per_cs_access;

            if(pConn_info->myID % 2)
            {
                dataFile_odd = fopen("DataFile_odd.txt","a");
                fprintf(dataFile_odd,"\n%d\t\t:\t\t%d\t\t:\t\t%d\t\t:\t\t%.2f",
                            pConn_info->myID,(cCritSec + 1),number_of_msgs_per_cs_access,time_reqd_per_cs_access);
                fclose(dataFile_odd);
            }
            else
            {
                dataFile_even = fopen("DataFile_even.txt","a");
                fprintf(dataFile_even,"\n%d\t\t:\t\t%d\t\t:\t\t%d\t\t:\t\t%.2f",
                            pConn_info->myID,(cCritSec + 1),number_of_msgs_per_cs_access,time_reqd_per_cs_access);
                fclose(dataFile_even);
            }

            fprintf(stdout,"\n\n********************************************************************************");
            fprintf(stdout,"\n%d messages sent/received for CS access",number_of_msgs_per_cs_access);
            fprintf(stdout,"\n%.2f seconds reqd for CS access",time_reqd_per_cs_access);
            fprintf(stdout,"\n********************************************************************************\n\n");
            fflush(stdout);

            number_of_msgs_per_cs_access = 0;

            using = 0;
            //fprintf(stdout,"\nnode(%d) : finished executing CS",pConn_info->myID);
            //fflush(stdout);

            //send defereed responses to all requests in the PQ and remove them from PQ
            sendDeferredResponses(pConn_info);

            pthread_mutex_unlock(&lock);
            cCritSec++;
        }
    }
#endif
    if(pConn_info->myID != 0)
    {
        sendCompletionNotificationToLeader(pConn_info);
    }
    else
    {
        total_request_count_of_all += total_num_of_request_messages;
        total_response_count_of_all += total_num_of_response_messages;
        completed[0] = 1;
    }

    fprintf(stdout,"\nMy request count = %d",total_num_of_request_messages);
    fprintf(stdout,"\nMy response count = %d",total_num_of_response_messages);
    fprintf(stdout,"\nMin count of msgs per CS access = %d",min_num_of_msgs_per_cs_access);
    fprintf(stdout,"\nMax count of msgs per CS access = %d",max_num_of_msgs_per_cs_access);
    fprintf(stdout,"\nAvg waiting time for first %d CS requests : %.2f",
        NUM_CS_ACCESSES,(avg_waiting_time_for_first_half_cs_requests / NUM_CS_ACCESSES));
    fprintf(stdout,"\nAvg waiting time for second %d CS requests : %.2f",
        NUM_CS_ACCESSES,(avg_waiting_time_for_second_half_cs_requests / NUM_CS_ACCESSES));

    fflush(stdout);

    fprintf(stdout,"\n=============================================node(%d) done with %d critical section accesses=============================================\n",pConn_info->myID,cCritSec);
    fflush(stdout);
}

void handleMessage(connection_info* pConn_info)
{
    pthread_mutex_init (&lock, NULL);

    pthread_t comp_thread;
    message msg;
    int initDone[MAX_NUM_NODES];
    int greatestSockFD = -1;
    int i = 0;
    fd_set readfds;

    for(i = 0; i < num_of_nodes; i++)
    {
        if(i == pConn_info->myID)
            continue;

        if(pConn_info->nodeSock[i] > greatestSockFD)
            greatestSockFD = pConn_info->nodeSock[i];
    } 

    memset(initDone,0,sizeof(int) * (MAX_NUM_NODES));

    while(1)
    {
        int rv = -1;
        struct timeval tv;

        tv.tv_sec = 1;
        tv.tv_usec =  0;
 
        if(isCompletedSentByAll())
        {
            int j = 0;
            fprintf(stdout,"\nTotal request count = %d",total_request_count_of_all);
            fprintf(stdout,"\nTotal response count = %d",total_response_count_of_all);
            fflush(stdout);

            sendTerminateToAll(pConn_info);

            for(j = 0; j < num_of_nodes; j++)
            {
                close(pConn_info->nodeSock[j]);
            }

            fprintf(stdout,"\nAll sockets closed, terminating...\n");
            fflush(stdout);
            return;
        }

        FD_ZERO(&readfds);
        for(i = 0; i < num_of_nodes; i++)
        {
            if(i == pConn_info->myID)
                continue;
            FD_SET(pConn_info->nodeSock[i],&readfds);
        } 

        if((rv = select(greatestSockFD + 1,&readfds,NULL,NULL,&tv)) < 0)
        {
            fprintf(stdout,"\nnode(%d): select failed\n",pConn_info->myID);
            fprintf(stdout,"\nnode(%d) : error : %s",pConn_info->myID,strerror(errno));
            fflush(stdout);
            while(1);
        }

        //select timeout occured
        if(rv == 0)
            continue;

        for(i = 0; i < num_of_nodes; i++)
        {
            if(i == pConn_info->myID)
                continue;
        
            if (FD_ISSET(pConn_info->nodeSock[i], &readfds))
            {
                if((rv = recv(pConn_info->nodeSock[i], &msg, sizeof(message), 0)) < 0)
                {
                    fprintf(stdout,"\nnode(%d): recv from node %d failed\n",pConn_info->myID,i);
                    fprintf(stdout,"\nnode(%d): error : %s",pConn_info->myID,strerror(errno));
                    fflush(stdout);
                    while(1);
                }

                //fprintf(stdout,"\nnode(%d) :Message recd : %d bytes recd from %d, message type : %d",
                //    pConn_info->myID,rv,i,msg.msg_type);
                fflush(stdout);

                switch(msg.msg_type)
                {
                    case MSG_INIT_DONE:
                        initDone[i] = 1;
                        if(isInitDoneByAll(initDone))
                        {
                            debugFile = fopen("logs.txt","w");
                            if(debugFile == NULL)
                            {
                                fprintf(stdout,"\nError : File logs.txt doesnt exist.");
                                fflush(stdout);
                                while(1);
                            }
                            fclose(debugFile);

                            dataFile_odd = fopen("DataFile_odd.txt","w");
                            if(dataFile_odd == NULL)
                            {
                                fprintf(stdout,"\nError : File DataFile_odd.txt doesnt exist.");
                                fflush(stdout);
                                while(1);
                            }
                            fprintf(dataFile_odd,"Node# \t:\t CS Access#\t:\t #Messages \t:\t Time taken");
                            fclose(dataFile_odd);

                            dataFile_even = fopen("DataFile_even.txt","w");
                            if(dataFile_even == NULL)
                            {
                                fprintf(stdout,"\nError : File DataFile_even.txt doesnt exist.");
                                fflush(stdout);
                                while(1);
                            }
                            fprintf(dataFile_even,"Node# \t:\t CS Access#\t:\t #Messages \t:\t Time taken");
                            fclose(dataFile_even);

                            //send a message to all to start testing connections
                            sendStartProcessMessageToAll(pConn_info);
                            //testConnections(pConn_info);

                            if(pthread_create(&comp_thread,NULL,(void *)&computeThreadHandler,(void *)pConn_info) != 0)
                            {
                                fprintf(stdout,"\nnode(%d): Computation Thread creation failure\n",pConn_info->myID);
                                fflush(stdout);
                                while(1);
                            }
                        }
                        break;

                    case MSG_START_PROCESS:
                        //testConnections(pConn_info);
                        if(pthread_create(&comp_thread,NULL,(void *)&computeThreadHandler,(void *)pConn_info) != 0)
                        {
                            fprintf(stdout,"\nnode(%d): Computation Thread creation failure\n",pConn_info->myID);
                            fflush(stdout);
                            while(1);
                        }
                        break;

                    case MSG_REQUEST:
                    {
                        pthread_mutex_lock(&lock);

                        int Our_priority = 0;
                        request req;
                        memcpy(&req,msg.buffer,sizeof(request));

                        Hgst_Sequence_Number = (Hgst_Sequence_Number > req.timestamp)?
                                                Hgst_Sequence_Number:req.timestamp;

                        Our_priority = ((req.timestamp > Our_Sequence_Number)||
                                        ((req.timestamp == Our_Sequence_Number)&&
                                        (i > pConn_info->myID)))? 1 : 0;

                        //fprintf(stdout,"\nOur Seq Num = %d, req's seq no = %d",Our_Sequence_Number,req.timestamp);
                        //fflush(stdout);

                        if((using) || ((waiting && Our_priority)))
                        {
                            //fprintf(stdout,"\n(If #1) using : %d, waiting : %d, our_priority : %d",using,waiting,Our_priority);
                            //fflush(stdout);
                            reply_deferred[i] = 1;
                        }

                        if((!(using || waiting))||
                            ((waiting && (!permissions[i]))) && (!Our_priority))
                        {
                            //fprintf(stdout,"\n(If #2) using : %d, waiting : %d, permissions[i] : %d, our_priority : %d",using,waiting,permissions[i],Our_priority);
                            //fflush(stdout);

                            permissions[i] = 0;
                            sendResponse(pConn_info,i);
                        }

                        if(waiting && permissions[i] && !Our_priority)
                        {
                            //fprintf(stdout,"\n(If #3) using : %d, waiting : %d, permissions[i] : %d, our_priority : %d",using,waiting,permissions[i],Our_priority);
                            //fflush(stdout);

                            permissions[i] = 0;
                            sendResponse(pConn_info,i);
                            sendRequest(pConn_info,i);
                        }

                        pthread_mutex_unlock(&lock);

                        break;
                    }
                    case MSG_RESPONSE:
                    {
                        pthread_mutex_lock(&lock);
                        number_of_msgs_per_cs_access++;
                        permissions[i] = 1;
                        pthread_mutex_unlock(&lock);
                        break;
                    }
                    case MSG_END_PROCESS:
                    {
                        completed[i] = 1;
                        end_notification endNotif;
                        memcpy(&endNotif,msg.buffer,sizeof(end_notification));
                        total_request_count_of_all += endNotif.total_number_of_requests;
                        total_response_count_of_all += endNotif.total_number_of_responses;
                        break;
                    }
                    case MSG_TERMINATE:
                    {
                        int j = 0;

                        for(j = 0; j < num_of_nodes; j++)
                        {
                            if(close(pConn_info->nodeSock[j]) < 0)
                            {
                                fprintf(stdout,"\nnode(%d): close(%d) failed\n",pConn_info->myID,j);
                                fprintf(stdout,"\nnode(%d) : error : %s",pConn_info->myID,strerror(errno));
                                fflush(stdout);
                                while(1);
                            }
                        }

                        fprintf(stdout,"\nAll sockets closed, terminating...\n");
                        fflush(stdout);

                    }
                    return;
                }
            }
        }

    }
}

int main(int argc, char *argv[])
{
    connection_info conn_info;
    memset(&conn_info,0,sizeof(conn_info));

    if(argc != 3)
    {
        fprintf(stdout,"\nusage : <executable name> <process number> <configFile>\n");
        fflush(stdout);
        return -1;
    }

    num_of_nodes = getNumNodes(argv[2]);

    conn_info.myID = atoi(argv[1]);
    conn_info.nodePort[conn_info.myID] = getPortNum(argv[2],conn_info.myID);

    if(conn_info.myID == 0)
    {
        startLeader(&conn_info);
    }
    else
    {
        char leaderName[MAX_BUF_LENGTH];
        getLeaderName(argv[2],leaderName);
        establishConnections(argv[2],leaderName,getLeaderPortNum(argv[2]),&conn_info);
    }
    handleMessage(&conn_info);
}
