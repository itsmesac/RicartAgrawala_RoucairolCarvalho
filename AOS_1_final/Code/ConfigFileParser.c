/*
 *    File             :    ConfigFileParser.c
 *    Author           :    Shachindra Chandrashekar
 *    Date             :    03/06/2014
 *    Course           :    Advanced Operating Systems
 *    Assignment       :    Programming Project 1
 */

#include "Initialize.h"

unsigned short int getPortNum(const char* fileName,int id)
{
    FILE* configFile = fopen(fileName,"r");
    if(configFile == NULL)
    {
        fprintf(stdout,"\nError : File %s doesnt exist.",fileName);
        fflush(stdout);
        while(1);
    }

    char buffer[MAX_BUF_LENGTH];
    char *portNum;

    unsigned short int retVal = -1;

    //leave first line
    if(fgets(buffer , (MAX_BUF_LENGTH - 1) , configFile) == NULL )
    {
        fprintf(stdout,"\nRead error!!!");
        fflush(stdout);
        while(1);
    }

    do
    {
        if(fgets(buffer , (MAX_BUF_LENGTH - 1) , configFile) == NULL )
        {
            fprintf(stdout,"\nRead error!!!");
            fflush(stdout);
            while(1);
        }
    }while(id--);

    portNum = strtok(buffer," #");
    portNum = strtok(NULL," #");
    portNum = strtok(NULL," #");

    fclose(configFile);

    retVal = ((unsigned short int)atoi(portNum));

    //fprintf(stdout,"\nPortNum : %hu",retVal);
    //fflush(stdout);

    return retVal;
}

void getLeaderName(const char* fileName, char* leaderName)
{
    FILE* configFile = fopen(fileName,"r");
    if(configFile == NULL)
    {
        fprintf(stdout,"\nError : File %s doesnt exist.",fileName);
        fflush(stdout);
        while(1);
    }

    char buffer[MAX_BUF_LENGTH];
    char* ptr;

    //leave first line
    if(fgets(buffer , (MAX_BUF_LENGTH - 1) , configFile) == NULL )
    {
        fprintf(stdout,"\nRead error!!!");
        fflush(stdout);
        while(1);
    }

    if(fgets(buffer , (MAX_BUF_LENGTH - 1) , configFile) == NULL )
    {
        fprintf(stdout,"\nRead error!!!");
        fflush(stdout);
        while(1);
    }

    ptr = strtok(buffer," #");
    ptr = strtok(NULL," #");

    strcpy(leaderName,ptr);

    //fprintf(stdout,"\nleaderName : %s",leaderName);
    //fflush(stdout);
}

unsigned short int getLeaderPortNum(const char* fileName)
{
    return getPortNum(fileName,0);
}

int getNumNodes(const char* fileName)
{
    int retVal = -1;
    char buffer[MAX_BUF_LENGTH];
    FILE* configFile = fopen(fileName,"r");
    if(configFile == NULL)
    {
        fprintf(stdout,"\nError : File %s doesnt exist.",fileName);
        fflush(stdout);
        while(1);
    }

    if(fgets(buffer , (MAX_BUF_LENGTH - 1) , configFile) == NULL )
    {
        fprintf(stdout,"\nRead error!!!");
        fflush(stdout);
        while(1);
    }
    retVal = atoi(buffer);

    //fprintf(stdout,"\nNumber of Nodes : %d",retVal);
    //fflush(stdout);

    return retVal;
}

