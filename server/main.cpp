#include "global.hpp"
#include "utils.hpp"
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "utils_singlethread.cpp"
#include "utils.hpp"
#include "client.cpp"
#include "server.cpp"

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

#define PIPELINE 1
#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480

extern int parallel, width, height;
extern image_t *images[NUM_IMAGES];
extern float *theta[NUM_IMAGES];
int main()
{
    ind_p_out = 0;
    ind_hyster = ind_p_out + 1;
    ind_thresh = ind_hyster + 1;
    ind_supp = ind_thresh + 1;
    ind_sob = ind_supp + 1;
    ind_conv = ind_sob + 1;
    ind_resize = ind_conv + 1;
    ind_p_in = ind_resize + 1;
    parallel = 0;
    height = IMAGE_HEIGHT;
    width = IMAGE_WIDTH;

    if (PIPELINE)
    {
        pthread_t tid, w;
        int wstatus, s;

        for (int i = 0; i < NUM_IMAGES; i++)
        {
            images[i] = (image_t *)malloc(sizeof(image_t) * width * height);
            if (images[i] == NULL)
            {
                printf("Memory allocation failure\n");
                exit(EXIT_FAILURE);
            }
        }
        for (int i = 0; i < NUM_THETA; i++)
        {
            theta[i] = (float *)malloc(sizeof(float) * width * height);
            if (theta[i] == NULL)
            {
                printf("Memory allocation failure\n");
                exit(EXIT_FAILURE);
            }
        }

        s = pthread_create(&tid, NULL, run_convolve_thread, NULL);
        if (s != 0)
            handle_error_en(s, "pthread_create");
        s = pthread_create(&tid, NULL, run_sobel_thread, NULL);
        if (s != 0)
            handle_error_en(s, "pthread_create");
        s = pthread_create(&tid, NULL, run_suppression_thread, NULL);
        if (s != 0)
            handle_error_en(s, "pthread_create");
        s = pthread_create(&tid, NULL, run_threshold_thread, NULL);
        if (s != 0)
            handle_error_en(s, "pthread_create");
        s = pthread_create(&tid, NULL, run_hysteresis_thread, NULL);
        if (s != 0)
            handle_error_en(s, "pthread_create");
        s = pthread_create(&tid, NULL, run_client_thread, NULL);
        if (s != 0)
            handle_error_en(s, "pthread_create");
        s = pthread_create(&tid, NULL, run_server_thread, NULL);
        if (s != 0)
            handle_error_en(s, "pthread_create");

        while (true)
        {
            w = pthread_cond_wait(tid, &wstatus, 0);
            if (w == -1)
            {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
            if (WIFEXITED(wstatus))
            {
                printf("exited, status=%d\n", WEXITSTATUS(wstatus));
                return 1;
            }
        }
    }
    else
    {
        run_nonpipeline_server();
    }

    return 1;
}
