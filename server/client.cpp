#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "global.hpp"
#include <time.h>

#define RESPONSE_SIZE 64
#define SERVER "127.0.0.1"
extern int ind_p_out, ind_hyster;
extern cv::Mat out_images[NUM_OUTPUTS];

#define FINAL_WIDTH 96
#define OUT_PORT 8081

// Driver code
// Code copied from
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/

void *run_client_thread(void *var)
{
    clock_t start, end;
    double cpu_time_used;
    int width = WIDTH;
    int height = HEIGHT;
    SharedVariable *sv = (SharedVariable *)var;
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
    servaddr.sin_port = htons(OUT_PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;
    /**
    if (inet_aton(SERVER, &servaddr.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }*/
    int once = 0;
    //FILE *fp;
    //fp = fopen("../arduino/test.txt", "w+");
    start = clock();

    while (true)
    {
        if (!ready(sv->ind_p_out, sv->ind_resize))
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            end = clock();
            cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
            printf("throughput %f", cpu_time_used);
            //printf("Running Client thread: %d\n", sv->ind_p_out);

            // conversion taken from
            // https://stackoverflow.com/questions/26681713/convert-mat-to-array-vector-in-opencv
            cv::Mat image = sv->out_images[(sv->ind_p_out + 1) % NUM_OUTPUTS];
            image = image(cv::Range(0, FINAL_WIDTH), cv::Range(16, 16 + FINAL_WIDTH));
            /**
            cv::imshow("Result_final", image);
            if (cv::waitKey(25) >= 0)
            {
                int x = 0 + 0;
            }
*/
            char out_packet[FINAL_WIDTH * FINAL_WIDTH / 8];

            int start = (DOWN_WIDTH - FINAL_WIDTH) / 2;
            //fprintf(stdout, "\n{");

            /**
            // convert image to condensed packet
            for (int i = 0; i < FINAL_WIDTH; i++)
            {
                for (int j = 0; j < FINAL_WIDTH / 8; j++)
                {
                    unsigned int val = 0;

                    for (int k = 0; k < 8; k++)
                    {
                        val = val << 1;
                        //printf("row: %d, cold: %d \n", i, j * 8 + k);
                        if (image.at<uchar>(i, j * 8 + k))
                        {
                            val += 1;
                        }
                    }

                    // out_packet[(i)*FINAL_WIDTH + (j - start)] = val;
                    if (!once)
                    {
                        //printf("%u, ", (unsigned int)val);
                    }
                }
            }*/
            //fprintf(stdout, "};\n");
            //once = 1;

            /**
            int n;
            socklen_t len = sizeof(servaddr);
            sendto(sockfd, (const char *)out_packet, SIZE_OUT,
                   MSG_CONFIRM, (const struct sockaddr *)&servaddr,
                   sizeof(servaddr));
            n = recvfrom(sockfd, (char *)buffer, RESPONSE_SIZE,
                         MSG_WAITALL, (struct sockaddr *)&servaddr,
                         &len);
            buffer[n] = '\0';
            printf("Server : %s\n", buffer);
            */

            sv->ind_p_out = (sv->ind_p_out + 1) % NUM_IMAGES;
            start = clock();
        }
    }

    pthread_exit(NULL);
}