#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <math.h>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"

#include "image_filter_sequential.hpp"
#include "global.hpp"
#include "client.hpp"

#define RESPONSE_SIZE 64
#define BUFLEN 512

#define MAXLINE 640 * 480

extern int height, width;

void sendPacket(int sockfd, unsigned char *packet);

void die(char *s)
{
    perror(s);
    exit(1);
}

void run_nonpipeline_pi()
{
    /** for connection */
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
    if (connect(sockfd, (const sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    while (true)
    {

        video.read(frame);
        grayscale(frame, image, height, width);
        sendPacket(sockfd, image);
    }
    close(sockfd);
}
