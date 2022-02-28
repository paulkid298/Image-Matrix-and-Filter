#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "global.h"

#define RESPONSE_SIZE 64
extern int ind_p_out, ind_hyster;
extern image_t *images[NUM_IMAGES];

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
    servaddr.sin_addr.s_addr = INADDR_ANY;
    while (true)
    {
        if (!ready(ind_p_out, ind_hyster))
        {
            sleep(10);
        }
        else
        {
            const char *image = (const char *)images[ind_p_out + 1];
            int n, len;
            sendto(sockfd, image, SIZE_OUT,
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