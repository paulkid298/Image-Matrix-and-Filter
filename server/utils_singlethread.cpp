#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "image_filter_sequential.hpp"
#include "global.hpp"
#include "server.hpp"

#define RESPONSE_SIZE 64
#define RESPONSE "received"

#define OUT_PORT 8081
#define MAXLINE 640 * 480

void receivePacket(int sockfd, unsigned char *image_in, struct sockaddr_in &cliaddr);
void sendPacket(int sockfd, unsigned char *packet, struct sockaddr_in &servaddr);

void run_nonpipeline_server()
{
    int width = WIDTH;
    int height = HEIGHT;
    /** prepare TCP server */
    /**

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
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
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
    connfd = accept(sockfd, (sockaddr *)&cli, &len);
    if (connfd < 0)
    {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accept the client...\n");
*/
    /** end TCP server preparation*/

    cv::VideoCapture video(0);
    cv::Mat frame;
    video.read(frame);

    unsigned char *images[2];
    //height = frame.rows;
    //width = frame.cols;
    images[0] = (unsigned char *)malloc(sizeof(unsigned char) * width * height);
    images[1] = (unsigned char *)malloc(sizeof(unsigned char) * width * height);

    float *filter = gaussian_kernel(GAUSSIAN_SIZE, SIGMA);
    float theta[height][width];
    unsigned char condensedPacket[SIZE_OUT];

    while (true)
    {
        float max_out = 0;

        video.read(frame);
        grayscale(frame, images[0], height, width);

        // receivePacket(images[0], sockfd);
        convolve(images[0], images[1], height, width, filter, GAUSSIAN_SIZE);
        sobel_filter(images[1], images[0], theta[0], &max_out, height, width);

        non_max_suppression(images[0], images[1], theta[0], max_out, &max_out,
                            height, width);

        threshold(images[1], images[0], height, width, LOW_THRESHOLD,
                  HIGH_THRESHOLD, max_out);
        hysteresis(images[0], images[1], height, width);

        cv::Mat input(height, width, CV_8UC1, images[1]);
        cv::imshow("image", input);

        if (cv::waitKey(25) >= 0)
        {
            return;
        }
        //sendPacket(sockfd, condensedPacket, servaddr);
    }
    // close(sockfd);
}

void sendPacket(int sockfd, unsigned char *packet, struct sockaddr_in &servaddr)
{
    char buffer[RESPONSE_SIZE];
    char *hello = "Hello from client";

    const char *image = (const char *)packet;
    int n;
    socklen_t len = sizeof(servaddr);
    sendto(sockfd, image, SIZE_OUT,
           MSG_CONFIRM, (const struct sockaddr *)&servaddr,
           sizeof(servaddr));
    n = recvfrom(sockfd, (char *)buffer, RESPONSE_SIZE,
                 MSG_WAITALL, (struct sockaddr *)&servaddr,
                 &len);
    buffer[n] = '\0';
    printf("Server : %s\n", buffer);
}
