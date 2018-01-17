/***********************************************************************
    Odsek:          Elektrotehnika i racunarstvo
    Departman:      Racunarstvo i automatika
    Katedra:        Racunarska tehnika i racunarske komunikacije (RT-RK)
    Predmet:        Osnovi Racunarskih Mreza 1
    Godina studija: Treca (III)
    Skolska godina: 2017/2018
    Semestar:       Zimski (V)
    
    Ime fajla:      client.c
    Opis:           TCP/IP klijent
    
    Platforma:      Raspberry Pi 2 - Model B
    OS:             Raspbian
***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define DEFAULT_MSGLEN 45
#define DEFAULT_PORT   27015

int sock;

static int Request();

int main(int argc , char *argv[])
{
    struct sockaddr_in server;

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
		
		return -1;
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(DEFAULT_PORT);

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
		
        return -1;
    }

    puts("Connected\n");

    //Send request to server
    while(1)
    {
        if (Request() == -1)
        {
            return 0;
        }
    }
}


int Request()
{
    char request[DEFAULT_MSGLEN];       // message for server
    char module[100][DEFAULT_MSGLEN];   // array of module names
    char moduleVal[100][10];            // value of modules
    int numOfElements;                  // number of modules to be received
    char numOfEl[3];                    // number of modules to be received 
    char tmpOutput[7]; //type of message that client receives from server
    int i = 0;
    
    puts("\n Please enter your request: \n - [ListAnalog]\n - [ListDigital]\n - [CommandAnalog][Name][Value]\n - [CommandDigital][Name][State]\n - End");

    scanf("%s", request);

    send(sock , request , strlen(request), 0);

    if (strcmp(request,"End") == 0)
    {
        puts("Client disconnected");
        close(sock);
		
        return -1;
    }
    
    recv(sock, tmpOutput, 7, 0); //waiting for info about server message

    if (strncmp(tmpOutput, "Output", 6) == 0)
    {
        //receive list of modules from server
        if ((strncmp(request, "[ListAnalog]", 12) == 0) || (strncmp(request, "[ListDigital]", 13) == 0))
        {
            recv(sock, numOfEl, 3, 0);
            printf("\n ---------------------------------- \n");  

            numOfElements = atoi(numOfEl);

            for (i = 0; i < numOfElements; i++)
            {
                //receiving and printing module name
                recv(sock, module[i], DEFAULT_MSGLEN , 0);
                printf("%s    ", module[i]);
                //receiving and printing module value
                recv(sock, moduleVal[i], 15, 0);
                puts(moduleVal[i]);
            
                if (i == (numOfElements - 1))
                {
                    printf("\n ---------------------------------- \n\n");
                }
            } 
            return 0;
        }
    }
    else if (strncmp(tmpOutput, "Error", 5) == 0)
    {
        //type of request is not supported
        puts("Wrong input! Please enter your request again.. \n");
        return 0;
    }

    return 0;
}

