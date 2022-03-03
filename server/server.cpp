#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "global.hpp"
#include "server.hpp"

#define MAXLINE 640 * 480
#define RESPONSE_SIZE 64

extern image_t *images[NUM_IMAGES];
extern int ind_p_out;
/**
void *run_server_thread(void *var)
{
    int sockfd, connfd;
    socklen_t len;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0)
    {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);

    // Accept the data packet from client and verification
    connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
    if (connfd < 0)
    {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accept the client...\n");

    while (true)
    {
        // server side code
        if (ind_p_in > ind_p_out)
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            receivePacket(images[ind_p_in], connfd);

            ind_p_in = (ind_p_in + 1) % NUM_IMAGES;
        }
    }
    close(sockfd);
    pthread_exit(NULL);
}
*/
void receivePacket(unsigned char *packet, int connfd)
{
    int numPackets = ceil((float)MAXLINE / MAXBYTES);
    char *response = "received";
    int index = 0;
    char buf[MAXBYTES];
    int total_read = 0;
    while (total_read < MAXLINE)
    {
        int bytesRead = 0;
        if (bytesRead = read(connfd, buf, MAXBYTES) < 0)
        {
            printf("recv failed\n");
        }
        printf("Received packet rom Client\n");
        memcpy(packet + total_read, buf, bytesRead);
        total_read += bytesRead;
        write(connfd, response, strlen(response));
    }

    for (int i = 0; i < numPackets; i++)
    {
        if (i == numPackets - 1)
        {
            int leftover = numPackets * MAXBYTES - MAXLINE;
            //read(connfd, packet + i * MAXBYTES, leftover);
        }
        else
        {
            //read(connfd, packet + i * MAXBYTES, MAXBYTES);
        }
        int readSize;

        write(connfd, response, sizeof(response));

        // if msg contains "Exit" then server exit and chat ended.
        if (strncmp("exit", (const char *)(packet + i * MAXBYTES), 4) == 0)
        {
            printf("Server Exit...\n");
            break;
        }
    }
    printf("Received image from client\n");
}
