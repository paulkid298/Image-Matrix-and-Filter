#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 8080
#define MAXLINE 640 * 480
#define NUM_IMAGES 4

unsigned char *images[NUM_IMAGES];
int begin_server();

int main()
{
    begin_server();
}

int begin_server()
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
    int index = 0;
    response_length = strlen(response);
    len = sizeof(cliaddr);

    for (int i = 0; i < NUM_IMAGES; i++)
    {
        images[i] = (unsigned char *)malloc(sizeof(unsigned char) * MAXLINE);
        if (images[i] == NULL)
        {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
    }

    while (true)
    {
        n = recvfrom(sockfd, (char *)images[index], MAXLINE, MSG_WAITALL,
                     (struct sockaddr *)&cliaddr, &len);
        images[index][n] = '\0';

        printf("Data: %s\n", images[index]);
        sendto(sockfd, (const char *)response, response_length, MSG_CONFIRM,
               (const struct sockaddr *)&cliaddr, len);
        index = (index + 1) % NUM_IMAGES;
    }
    close(sockfd);
    return 0;
}