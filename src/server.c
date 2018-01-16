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


int analogLen;    //Length of array - analogModules
int digitalLen;   //Length of array - digitalModules

int sock, cllientSock;

/* Syntax check of request, if return = 0, request is not supported */
static int CheckInput(char* request, int length);

/* Function handling CommandModules request */
static void CommandModules(char *mod, char *element, char *value, AnalogModule** am, DigitalModule** dm);

/* Function handling ListModules request */
static void ListModules(char* mod, AnalogModule** am, DigitalModule** dm);

/* Function for parsing request message and calling appropriate handler */
static int RequestType(char* req, AnalogModule** am, DigitalModule** dm);


int main(int argc , char *argv[])
{
    int socketDesc;
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
    while (1)
    {
        while ((readSize = recv(cllientSock , request , DEFAULT_MSGLEN , 0)) > 0 )
        {
            request[readSize] = '\0';
            printf("Request: %s\n",request);
    
            if (!CheckInput(request, readSize))
            {     
                // If syntax is wrong, server will pass calling RequestType()
                if (!RequestType(request, analogModules, digitalModules))
                {
                    return 0;
                }
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


int CheckInput(char* request, int length)
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


void CommandModules(char *mod, char *element, char *value, AnalogModule** am, DigitalModule** dm)
{    
    int i;

    if (strcmp(mod, "analog") == 0)
    {
        for (i = 0; i < analogLen; i++)
        {
            if (strcmp(am[i]->name,element) == 0)
            {
                am[i]->value = atoi(value);
            }
        }    
    }
    else if (strcmp(mod, "digital") == 0)
    {
        if (strcmp(value, "true") == 0)
        {
            for (i = 0; i < digitalLen; i++)
            {
                if (strcmp(dm[i]->name, element) == 0)
                {
                    dm[i]->state = true;
                }
            }
        }
        else if (strcmp(value, "false") == 0)
        {
            for (i = 0; i < digitalLen; i++)
            {
                if (strcmp(dm[i]->name, element) == 0)
                {
                    dm[i]->state = false;
                }
            }
        }
    }

    send(cllientSock, "comm\n", 7, 0); //notification for client that server is sending nothing for listing because request type is [Command...]
}


void ListModules(char* mod, AnalogModule** am, DigitalModule** dm)
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


int RequestType(char* request, AnalogModule** am, DigitalModule** dm)
{
    char *part1;
    char *part2;
    char *part3;

     if (request[1] == 'L')  //[List...]
     {
        part1 = strtok(request, "[]"); //taking string part between '[' and ']'

        if (strcmp(part1,"ListAnalog") == 0)
        {
            ListModules("analog", am, dm);
        }
        else if (strcmp(part1,"ListDigital") == 0)
        {
            ListModules("digital", am, dm);
        }    
    }
    else if (request[1] == 'C')  //[Command...]
    {
        part1 = strtok(request, "[]");
        part2 = strtok(NULL, "[]");
        part3 = strtok(NULL, "[]");

        if (!strcmp(part1, "CommandAnalog"))
        {
            int n = strlen(part3);
            int i;
            int tmp = 0;

            for (i = 0; i < n; i++)
            { //checking if value is number
                if (part3[i] < '0' || part3[i] > '9')
                {
                    tmp = 1;
                }
            }
            if (tmp == 0)
            {
                CommandModules("analog", part2, part3, am, dm);
            }
            else 
            {
                send(cllientSock, "Error\n", 7, 0);
                puts("Error: wrong input!\n");
            }
        }
        else if (!strcmp(part1, "CommandDigital"))
        {
            if (!strcmp(part3,"true") || !strcmp(part3,"false"))
            {
                CommandModules("digital", part2, part3, am, dm);
            }
            else
            {
                send(cllientSock, "Error\n", 7, 0);
                puts("Error: wrong input!\n");
            }
        }
    }
    else if (strncmp(request, "End", 3) == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
        
        return 0;
    }

    return 1;
}