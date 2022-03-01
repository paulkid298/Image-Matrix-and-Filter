#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "global.hpp"

#define RESPONSE_SIZE 64
#define SERVER "127.0.0.1"
extern int ind_p_out, ind_hyster;
extern cv::Mat out_images[NUM_OUTPUTS];

// Driver code
// Code copied from
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/
void *run_client_thread(void *var)
{
    int sockfd;
    char buffer[RESPONSE_SIZE];
    char *hello = "Hello from client";
    struct sockaddr_in servaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    // servaddr.sin_addr.s_addr = INADDR_ANY;
    if (inet_aton(SERVER, &servaddr.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    while (true)
    {
        if (!ready(ind_p_out, ind_hyster))
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            // conversion taken from
            // https://stackoverflow.com/questions/26681713/convert-mat-to-array-vector-in-opencv
            cv::Mat image = out_images[(ind_p_out + 1) % NUM_OUTPUTS];
            uchar *arr = image.isContinuous() ? image.data : image.clone().data;
            const char *packet = (const char *)arr;
            int n;
            socklen_t len = sizeof(servaddr);
            sendto(sockfd, packet, SIZE_OUT,
                   MSG_CONFIRM, (const struct sockaddr *)&servaddr,
                   sizeof(servaddr));
            n = recvfrom(sockfd, (char *)buffer, RESPONSE_SIZE,
                         MSG_WAITALL, (struct sockaddr *)&servaddr,
                         &len);
            buffer[n] = '\0';
            printf("Server : %s\n", buffer);

            ind_p_out = (ind_p_out + 1) % NUM_IMAGES;
        }
    }

    close(sockfd);
    return 0;
}