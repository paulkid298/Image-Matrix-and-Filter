#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "global.hpp"
#include "client.hpp"

#define RESPONSE_SIZE 64
#define BUFLEN 512

#define MAXLINE 640 * 480
#define TEST_STRING "Hello World!"

// Driver code
// Code copied from
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/

extern int ind_client, ind_gray;
extern image_t *gray_images[NUM_IMAGES];

void *run_client_thread(void *var)
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    // data
    unsigned char image[width * height];
    cv::VideoCapture video(0);
    cv::Mat frame;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }

    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("192.168.1.126");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    while (true)
    {
        if (!ready(ind_client, ind_gray))
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            printf("Running Resize thread: %d\n", sv->ind_resize);
            // conversion taken from
            // https://stackoverflow.com/questions/26681713/convert-mat-to-array-vector-in-opencv
            printf("sending packet\n");
            int n;
            sendPacket(sockfd, gray_images[(ind_client + 1) % NUM_IMAGES]);
        }
    }

    close(sockfd);
    pthread_exit(NULL);
}

void sendPacket(int sockfd, unsigned char *packet)
{
    char buf[100];
    const char *mess = "hello this is from the client pi\n";
    char response[RESPONSE_SIZE];
    int numPackets = ceil((float)MAXLINE / MAXBYTES);
    int total_sent = 0;

    while (total_sent < MAXLINE)
    {
        int toSend = MAXBYTES;
        if (total_sent + toSend > MAXLINE)
        {
            toSend = MAXLINE - total_sent;
        }
        if (write(sockfd, (const char *)(packet + total_sent), toSend) < 0)
        {
            printf("send failed\n");
            return;
        }

        if (read(sockfd, response, RESPONSE_SIZE) < 0)
        {
            puts("recv failed");
            break;
        }
        printf("From server: %s\n", response);

        total_sent += toSend;
    }

    /**
    for (int i = 0; i < numPackets; i++)
    {
        if (i == numPackets - 1)
        {
            int leftover = numPackets * MAXBYTES - MAXLINE;
            //memcpy(buf, packet + i * MAXBYTES, leftover);
            //write(sockfd, buf, leftover);
        }
        else
        {
            //memcpy(buf, packet + i * MAXBYTES, MAXBYTES);
            //write(sockfd, buf, MAXBYTES);
        }
        printf("sending string\n");
        write(sockfd, mess, strlen(mess));
        // Receive a reply from the server
        if (read(sockfd, response, RESPONSE_SIZE) < 0)
        {
            puts("recv failed");
            break;
        }
        printf("From server: %s\n", response);
*/
    /**
        if ((strncmp((const char *)buf, "exit", 4)) == 0)
        {
            printf("Client Exit...\n");
            break;
        }*/
}