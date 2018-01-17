/***********************************************************************
    Odsek:          Elektrotehnika i racunarstvo
    Departman:      Racunarstvo i automatika
    Katedra:        Racunarska tehnika i racunarske komunikacije (RT-RK)
    Predmet:        Osnovi Racunarskih Mreza 1
    Godina studija: Treca (III)
    Skolska godina: 2017/2018
    Semestar:       Zimski (V)
    
    Ime fajla:      server.c
    Opis:           TCP/IP server
    
    Platforma:      Raspberry Pi 2 - Model B
    OS:             Raspbian
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>    
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>   

#define DEFAULT_MSGLEN 45
#define DEFAULT_PORT   27015


/* Analog module */
typedef struct Analog
{
    char name[33];
    int value;
}AnalogModule;

/* Digital module */
typedef struct Digital
{
    char name[33];
    bool state;
}DigitalModule;


/* Syntax check of request, if return = 0, request is not supported */
static int CheckInput(char* request, int length, int clientSock);

/* Function handling CommandModules request */
static void CommandModules(char *mod, char *element, char *value, AnalogModule** am, int analogLen, DigitalModule** dm, int digitalLen, int clientSock);

/* Function handling ListModules request */
static void ListModules(char* mod, AnalogModule** am, int analogLen, DigitalModule** dm, int digitalLen, int clientSock);

/* Function for parsing request message and calling appropriate handler */
static int RequestType(char* request, AnalogModule** am, int analogLen, DigitalModule** dm, int digitalLen, int clientSock);


int main(int argc , char *argv[])
{
    int analogLen;    //Length of array - analogModules
    int digitalLen;   //Length of array - digitalModules
    int socketDesc;
    int cllientSock;
    int readSize;
    int i;
    int c;
    struct sockaddr_in server;
    struct sockaddr_in client;
    char request[DEFAULT_MSGLEN];

    if (argc != 3)
    {
        printf(" Pogresan broj argumenata! Proverite da li ste\n naveli broj analognih i digitalnih modula!\n");
        return -1;
    }
    
    analogLen = atoi(argv[1]);
    digitalLen = atoi(argv[2]);
    AnalogModule*  analogModules[analogLen];
    DigitalModule* digitalModules[digitalLen];

    // Creating analog modules
    for (i = 0; i < analogLen; i++)
    {
        analogModules[i] = (AnalogModule*)malloc(sizeof(AnalogModule));
        sprintf(analogModules[i]->name, "AnalogModule%d", i+1);
        analogModules[i]->value = 0;
    }

    // Creating digital modules
    for (i = 0; i < digitalLen; i++)
    {
        digitalModules[i] = (DigitalModule*)malloc(sizeof(DigitalModule));
        sprintf(digitalModules[i]->name, "DigitalModule%d", i+1);
        digitalModules[i]->state = false;
    }
   
    //Create socket
    socketDesc = socket(AF_INET , SOCK_STREAM , 0);
    if (socketDesc == -1)
    {
        printf("Could not create socket");

        for (i = 0; i < analogLen; i++)
        {
            free(analogModules[i]);
        }

        for (i = 0; i < digitalLen; i++)
        {
            free(digitalModules[i]);
        }
        return -1;
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT);

    //Bind
    if (bind(socketDesc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("Bind failed. Error");

        for (i = 0; i < analogLen; i++)
        {
            free(analogModules[i]);
        }

        for (i = 0; i < digitalLen; i++)
        {
            free(digitalModules[i]);
        }
        return -1;
    }

    //Listen
    listen(socketDesc , 3);

    //Accept an incoming connection
    puts("Waiting for incoming connections...");

    c = sizeof(struct sockaddr_in);

    //accept connection from an incoming client
    cllientSock = accept(socketDesc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (cllientSock < 0)
    {
        perror("Accept failed");

        for (i = 0; i < analogLen; i++)
        {
            free(analogModules[i]);
        }

        for (i = 0; i < digitalLen; i++)
        {
            free(digitalModules[i]);
        }
        return -1;
    }

    puts("Connection accepted");

    //Receive a request from client

    while ((readSize = recv(cllientSock , request , DEFAULT_MSGLEN , 0)) > 0 )
    {
        request[readSize] = '\0';
        printf("Request: %s\n",request);

        if (CheckInput(request, readSize, cllientSock) == 0)
        {     
            if (RequestType(request, analogModules, analogLen, digitalModules, digitalLen, cllientSock) == -1)
            {
                break;
            }
        }            
    }
    
    for (i = 0; i < analogLen; i++)
    {
        free(analogModules[i]);
    }

    for (i = 0; i < digitalLen; i++)
    {
        free(digitalModules[i]);
    }

    return 0;
}


int CheckInput(char* request, int length, int cllientSock)
{
    int i;
    int openedBrackets = 0;
    int closedBrackets = 0;

    for (i = 0; i < length; i++)
    {
        if (request[i] == '[')
        {
            openedBrackets++;
        }

        if (request[i] == ']')
        {
            closedBrackets++;
        }
    }

    if (openedBrackets == 1 && closedBrackets == 1)
    {
        if ((strcmp(request, "[ListAnalog]") == 0) || (strcmp(request, "[ListDigital]") == 0))
        {
            return 0;
        }
        else
        {
            send(cllientSock, "Error", 7, 0);
            puts("Error: wrong input!");

            return -1;
        }
    }
    else if (openedBrackets == 3 && closedBrackets == 3)
    {
        if ((strncmp(request, "[CommandAnalog]", 15) == 0) || (strncmp(request, "[CommandDigital]", 16) == 0))
        {
            return 0;
        }
        else
        {
            send(cllientSock, "Error", 7, 0);
            puts("Error: wrong input!");

            return -1;
        }
    }
    else if (openedBrackets == 0 && closedBrackets == 0)
    {
        if (strcmp(request, "End") == 0)
        {
            return 0;
        }
        else
        {
            send(cllientSock, "Error", 7, 0);
            puts("Error: wrong input!");

            return -1;
        }
    }
    else
    {
        send(cllientSock, "Error", 7, 0);
        puts("Error: wrong input!");

        return -1;
    }
}


int RequestType(char* request, AnalogModule** am, int analogLen, DigitalModule** dm, int digitalLen, int clientSock)
{
    char *part1;
    char *part2;
    char *part3;

     if (request[1] == 'L')  //[List...]
     {
        part1 = strtok(request, "[]"); //taking string part between '[' and ']'

        if (strcmp(part1,"ListAnalog") == 0)
        {
            ListModules("analog", am, analogLen, dm, digitalLen, clientSock);
        }
        else if (strcmp(part1,"ListDigital") == 0)
        {
            ListModules("digital", am, analogLen, dm, digitalLen, clientSock);
        }    
    }
    else if (request[1] == 'C')  //[Command...]
    {
        part1 = strtok(request, "[]");
        part2 = strtok(NULL, "[]");
        part3 = strtok(NULL, "[]");

        if (strcmp(part1, "CommandAnalog") == 0)
        {
            int n = strlen(part3);
            int i;
            int tmp = 0;

            for (i = 0; i < n; i++)
            { 
                //checking if value is number
                if (part3[i] < '0' || part3[i] > '9')
                {
                    tmp = 1;
                }
            }
            if (tmp == 0)
            {
                CommandModules("analog", part2, part3, am, analogLen, dm, digitalLen, clientSock);
            }
            else 
            {
                send(clientSock, "Error\n", 7, 0);
                puts("Error: wrong input!\n");
            }
        }
        else if (strcmp(part1, "CommandDigital") == 0)
        {
            if ((strcmp(part3, "true") == 0) || (strcmp(part3, "false") == 0))
            {
                CommandModules("digital", part2, part3, am, analogLen, dm, digitalLen, clientSock);
            }
            else
            {
                send(clientSock, "Error\n", 7, 0);
                puts("Error: wrong input!\n");
            }
        }
    }
    else if (strncmp(request, "End", 3) == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
        
        return -1;
    }

    return 0;
}


void CommandModules(char *mod, char *element, char *value, AnalogModule** am, int analogLen, DigitalModule** dm, int digitalLen, int clientSock)
{    
    int i;
    int nameFound = 0;

    if (strcmp(mod, "analog") == 0)
    {
        for (i = 0; i < analogLen; i++)
        {
            if (strcmp(am[i]->name, element) == 0)
            {
                nameFound = 1;
                am[i]->value = atoi(value);
            }
        }    
    }
    else if (strcmp(mod, "digital") == 0)
    {
        for (i = 0; i < digitalLen; i++)
        {
            if (strcmp(dm[i]->name, element) == 0)
            {
                if (strcmp(value, "true") == 0)
                {
                    dm[i]->state = true;
                }
                else
                {
                    dm[i]->state = false;
                }
                nameFound = 1;
            }
        }
        
    }
    
    if (nameFound == 0)
    {
        send(clientSock, "Wrong", 5, 0);    //notification for client that module name is wrong
        puts("Error: wrong module name!\n");
    }
    else
    {
        send(clientSock, "comm", 4, 0);             //notification for client that server is sending nothing for listing because request type is [Command...]
    }
}


void ListModules(char* mod, AnalogModule** am, int analogLen, DigitalModule** dm, int digitalLen, int cllientSock)
{
    int i;
    char tmp[10];
    char length[3];

    send(cllientSock, "Output\n", 7, 0); //notification for client that server is sending list of modules
    
    if (strcmp(mod, "analog") == 0)
    {
        sprintf(length, "%d", analogLen);
        send(cllientSock, length, 3, 0);
        
        for (i = 0; i < analogLen; i++)
        {
            send(cllientSock , am[i]->name , DEFAULT_MSGLEN, 0);
            sprintf(tmp, "%d",am[i]->value);
            send(cllientSock, tmp, 15, 0);
        }
    }
    else if (strcmp(mod, "digital") == 0)
    {
        sprintf(length, "%d", digitalLen);
        send(cllientSock, length, 3, 0);

        for (i = 0; i < digitalLen; i++)
        {
            send(cllientSock , dm[i]->name , DEFAULT_MSGLEN, 0);

            if (dm[i]->state == true)
            {
                strcpy(tmp, "true");
            }
            else if (dm[i]->state == false)
            {
                strcpy(tmp, "false");
            }

            send(cllientSock, tmp, 15, 0);
        }
    }
}