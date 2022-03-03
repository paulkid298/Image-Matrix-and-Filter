#include <stdio.h>
#include "global.hpp"

#include "utils.hpp"
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "utils.cpp"
#include "utils_singlethread.cpp"
#include "client.cpp"

#define handle_error_en(en, msg) \
    do                           \
    {                            \
        errno = en;              \
        perror(msg);             \
        exit(EXIT_FAILURE);      \
    } while (0)

#define handle_error(msg)   \
    do                      \
    {                       \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

extern int height, width, ind_cam, ind_client, ind_gray;
extern image_t *gray_images[NUM_IMAGES];

int main()
{
    height = 480;
    width = 640;
    for (int i = 0; i < NUM_IMAGES; i++)
    {
        gray_images[i] = (image_t *)malloc(sizeof(image_t) * width * height);
        if (gray_images[i] == NULL)
        {
            printf("Memory allocation failure\n");
            exit(EXIT_FAILURE);
        }
    }
    if (PIPELINE)
    {
        ind_client = 0;
        ind_gray = ind_client + 1;
        ind_cam = ind_gray + 1;

        pthread_t tid, w;
        int wstatus, s;

        s = pthread_create(&tid, NULL, run_camera_thread, NULL);
        if (s != 0)
            handle_error_en(s, "pthread_create");
        fprintf(stderr, "cam thread created\n");
        s = pthread_create(&tid, NULL, run_gray_thread, NULL);
        if (s != 0)
            handle_error_en(s, "pthread_create");
        fprintf(stderr, "gray thread created\n");
        s = pthread_create(&tid, NULL, run_client_thread, NULL);
        if (s != 0)
            handle_error_en(s, "pthread_create");
        fprintf(stderr, "client thread created\n");
        w = pthread_join(tid, NULL);
    }
    else
    {
        run_nonpipeline_pi();
    }

    for (int i = 0; i < NUM_IMAGES; i++)
    {
        free(gray_images[i]);
    }
    return 1;
}