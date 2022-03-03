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

extern int parallel;

void run_camera(SharedVariable *sv)
{
    cv::VideoCapture video(0);
    while (true)
    {
        if (sv->ind_cam == sv->ind_p_out)
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            printf("Running Cam Thread %d\n", sv->ind_cam);
            video.read(sv->out_images[sv->ind_cam % NUM_OUTPUTS]);
            sv->ind_cam = (sv->ind_cam + 1) % NUM_IMAGES;
        }
    }
}

int main()
{
    SharedVariable sv;
    sv.ind_p_out = 0;
    sv.ind_resize = sv.ind_p_out + 1;
    sv.ind_hyster = sv.ind_resize + 1;
    sv.ind_thresh = sv.ind_hyster + 1;
    sv.ind_supp = sv.ind_thresh + 1;
    sv.ind_sob = sv.ind_supp + 1;
    sv.ind_conv = sv.ind_sob + 1;
    sv.ind_gray = sv.ind_conv + 1;
    sv.ind_cam = sv.ind_gray + 1;
    parallel = 0;

    int width = WIDTH;
    int height = HEIGHT;

    if (PIPELINE)
    {
        pthread_t t1, t2, t3, t4, t5, t6, t7, t8, t9;
        int wstatus, s;

        for (int i = 0; i < NUM_IMAGES; i++)
        {
            sv.images[i] = (image_t *)malloc(sizeof(image_t) * width * height);
            if (sv.images[i] == NULL)
            {
                printf("Memory allocation failure\n");
                exit(EXIT_FAILURE);
            }
        }
        for (int i = 0; i < NUM_THETA; i++)
        {
            sv.theta[i] = (float *)malloc(sizeof(float) * width * height);
            if (sv.theta[i] == NULL)
            {
                printf("Memory allocation failure\n");
                exit(EXIT_FAILURE);
            }
        }

        s = pthread_create(&t1, NULL, &run_convolve_thread, (void *)(&sv));
        if (s != 0)
            handle_error_en(s, "pthread_create");
        s = pthread_create(&t2, NULL, &run_sobel_thread, (void *)(&sv));
        if (s != 0)
            handle_error_en(s, "pthread_create");
        s = pthread_create(&t3, NULL, &run_suppression_thread, (void *)(&sv));
        if (s != 0)
            handle_error_en(s, "pthread_create");
        s = pthread_create(&t4, NULL, &run_threshold_thread, (void *)(&sv));
        if (s != 0)
            handle_error_en(s, "pthread_create");
        s = pthread_create(&t5, NULL, &run_hysteresis_thread, (void *)(&sv));
        if (s != 0)
            handle_error_en(s, "pthread_create");

        s = pthread_create(&t6, NULL, &run_client_thread, (void *)(&sv));
        if (s != 0)
            handle_error_en(s, "pthread_create");

        s = pthread_create(&t2, NULL, &run_gray_thread, (void *)(&sv));
        if (s != 0)
            handle_error_en(s, "pthread_create");
        s = pthread_create(&t6, NULL, &run_resize_thread, (void *)(&sv));
        if (s != 0)
            handle_error_en(s, "pthread_create");
        /**
        s = pthread_create(&t7, NULL, &run_server_thread, NULL);
        if (s != 0)
            handle_error_en(s, "pthread_create");
        */

        // s = pthread_join(t7, NULL);
        /**
        if (s != 0)
        {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }*/
        run_camera(&sv);

        for (int i = 0; i < NUM_IMAGES; i++)
        {
            free(sv.images[i]);
        }
        for (int i = 0; i < NUM_THETA; i++)
        {
            free(sv.theta[i]);
        }
    }
    else
    {
        run_nonpipeline_server();
    }

    return 1;
}
