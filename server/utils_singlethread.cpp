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

#define RESPONSE_SIZE 64

#define PORT 8080
#define MAXLINE 640 * 480

extern int height, width;

void receivePacket(int sockfd, unsigned char *image_in, struct sockaddr_in cliaddr);
void sendPacket(int sockfd, unsigned char *packet, struct sockaddr_in servaddr);

void run_nonpipeline_server()
{
    /** prepare server */
    int sockfd;
    unsigned char *images[2];
    images[0] = (unsigned char *)malloc(sizeof(unsigned char) * width * height);
    images[1] = (unsigned char *)malloc(sizeof(unsigned char) * width * height);
    struct sockaddr_in servaddr, cliaddr;

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // bind socket with server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    // create socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    float *filter = gaussian_kernel(GAUSSIAN_SIZE, SIGMA);
    float theta[height][width];
    unsigned char condensedPacket[SIZE_OUT];

    while (true)
    {
        float max_out = 0;

        receivePacket(sockfd, images[0], cliaddr);
        convolve(images[0], images[1], height, width, filter, GAUSSIAN_SIZE);
        sobel_filter(images[1], images[0], theta[0], &max_out, height, width);
        non_max_suppression(images[0], images[1], theta[0], max_out, &max_out,
                            height, width);
        threshold(images[1], images[0], height, width, LOW_THRESHOLD,
                  HIGH_THRESHOLD, max_out);
        hysteresis(images[0], images[1], height, width);
        sendPacket(sockfd, condensedPacket, servaddr);
    }
}

void receivePacket(int sockfd, unsigned char *image_in, struct sockaddr_in cliaddr)
{

    char buffer[MAXLINE];
    char *response = "received";

    unsigned len, n, response_length;
    response_length = strlen(response);
    len = sizeof(cliaddr);

    n = recvfrom(sockfd, (char *)image_in, MAXLINE, MSG_WAITALL,
                 (struct sockaddr *)&cliaddr, &len);
    image_in[n] = '\0';

    sendto(sockfd, (const char *)response, response_length, MSG_CONFIRM,
           (const struct sockaddr *)&cliaddr, len);
    close(sockfd);
    return;
}

void sendPacket(int sockfd, unsigned char *packet, struct sockaddr_in servaddr)
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
