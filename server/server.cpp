#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "global.hpp"

#define PORT 8080
#define MAXLINE 640 * 480

extern image_t *images[NUM_IMAGES];
extern int ind_p_in, ind_p_out;

void *run_server_thread(void *var)
{
    int sockfd;
    char buffer[MAXLINE];
    char *response = "received";
    struct sockaddr_in servaddr, cliaddr;

    // create socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

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

    unsigned len, n, response_length;
    response_length = strlen(response);
    len = sizeof(cliaddr);

    while (true)
    {
        // server side code
        if (ind_p_in > ind_p_out)
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            n = recvfrom(sockfd, (char *)images[ind_p_in], MAXLINE, MSG_WAITALL,
                         (struct sockaddr *)&cliaddr, &len);
            images[ind_p_in][n] = '\0';

            sendto(sockfd, (const char *)response, response_length, MSG_CONFIRM,
                   (const struct sockaddr *)&cliaddr, len);

            ind_p_in = (ind_p_in + 1) % NUM_IMAGES;
        }
    }
    close(sockfd);
    return 0;
}