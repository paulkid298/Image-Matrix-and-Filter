#include "global.hpp"
#include "utils.hpp"
#include <unistd.h>
#include "image_filter_sequential.hpp"
#include "image_filter_parallel.cpp"
#include <iostream>
#include <string.h>

#define PARALLEL 0

using namespace cv;

float maximums[NUM_IMAGES];

/**
void *run_camera_thread(void *var)
{
    cv::VideoCapture video(0);
    cv::Mat frame;
    while (true)
    {
        if (ind_cam == ind_p_out)
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            video.read(frame);
            out_images[ind_cam] = frame;
            ind_cam = (ind_cam + 1) % NUM_IMAGES;
        }
    }
    pthread_exit(NULL);
}*/

void *run_gray_thread(void *var)
{
    int width = WIDTH;
    int height = HEIGHT;
    SharedVariable *sv = (SharedVariable *)var;
    while (true)
    {

        if (!ready(sv->ind_gray, sv->ind_cam))
        {
            //printf("Gray thread check %d %d", sv->ind_gray, sv->ind_cam);
            usleep(SLEEP_TIME);
        }
        else
        {
            printf("Running Gray thread %d\n", sv->ind_gray);

            if (PARALLEL)
            {
                ParallelGrayscale parallelGrayscale(sv->out_images[(sv->ind_gray + 1) % NUM_OUTPUTS],
                                                    sv->images[sv->ind_gray], height, width);
            }
            else
            {
                grayscale(sv->out_images[(sv->ind_gray + 1) % NUM_OUTPUTS],
                          sv->images[sv->ind_gray], height, width);
            }

            sv->ind_gray = (sv->ind_gray + 1) % NUM_IMAGES;
        }
    }
    pthread_exit(NULL);
}

void *run_convolve_thread(void *var)
{
    int width = WIDTH;
    int height = HEIGHT;
    SharedVariable *sv = (SharedVariable *)var;
    float *filter = gaussian_kernel(GAUSSIAN_SIZE, SIGMA);
    fprintf(stderr, "starting thread\n");
    while (true)
    {
        if (!ready(sv->ind_conv, sv->ind_gray))
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            printf("Running Conv thread %d\n", sv->ind_conv);
            if (PARALLEL)
            {
                ParallelConvolve parallelConvolve(sv->images[(sv->ind_conv + 1) %
                                                             NUM_IMAGES],
                                                  sv->images[sv->ind_conv], height,
                                                  width, filter, GAUSSIAN_SIZE);
                parallel_for_(Range(0, width * height), parallelConvolve);
            }
            else
            {
                convolve(sv->images[(sv->ind_conv + 1) % NUM_IMAGES], sv->images[sv->ind_conv],
                         height, width, filter, GAUSSIAN_SIZE);
            }

            sv->ind_conv = (sv->ind_conv + 1) % NUM_IMAGES;
        }
    }
    fprintf(stderr, "exiting thread\n");
    pthread_exit(NULL);
}

void *run_sobel_thread(void *var)
{
    int width = WIDTH;
    int height = HEIGHT;
    SharedVariable *sv = (SharedVariable *)var;
    while (true)
    {
        if (!ready(sv->ind_sob, sv->ind_conv))
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            printf("Running sobel thread %d\n", sv->ind_sob);
            float max = 0;
            if (PARALLEL)
            {
                ParallelSobel parallelSobel(sv->images[(sv->ind_sob + 1) %
                                                       NUM_IMAGES],
                                            sv->images[sv->ind_sob],
                                            sv->theta[(sv->ind_sob) % NUM_THETA],
                                            height,
                                            width, &max);
                parallel_for_(Range(0, width * height), parallelSobel);
            }
            else
            {
                sobel_filter(sv->images[(sv->ind_sob + 1) % NUM_IMAGES],
                             sv->images[sv->ind_sob], sv->theta[(sv->ind_sob) % NUM_THETA],
                             &max, height, width);
            }
            maximums[sv->ind_sob] = max;
            sv->ind_sob = (sv->ind_sob + 1) % NUM_IMAGES;
        }
    }
    pthread_exit(NULL);
}

void *run_suppression_thread(void *var)
{
    int width = WIDTH;
    int height = HEIGHT;
    SharedVariable *sv = (SharedVariable *)var;
    while (true)
    {
        if (!ready(sv->ind_supp, sv->ind_sob))
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            printf("Running suppression thread %d\n", sv->ind_supp);
            float max_out;
            float max_in = maximums[sv->ind_supp];
            if (PARALLEL)
            {
                ParallelSuppression parallelSuppression(sv->images[(sv->ind_supp + 1) %
                                                                   NUM_IMAGES],
                                                        sv->images[sv->ind_supp],
                                                        sv->theta[(sv->ind_supp + 1) % NUM_THETA],
                                                        height, width,
                                                        max_in, &max_out);
                parallel_for_(Range(0, width * height), parallelSuppression);
            }
            else
            {
                non_max_suppression(sv->images[(sv->ind_supp + 1) % NUM_IMAGES],
                                    sv->images[sv->ind_supp], sv->theta[(sv->ind_supp + 1) % NUM_THETA], max_in,
                                    &max_out, height, width);
            }

            maximums[sv->ind_supp] = max_out;
            sv->ind_supp = (sv->ind_supp + 1) % NUM_IMAGES;
        }
    }
    pthread_exit(NULL);
}
void *run_threshold_thread(void *var)
{
    int width = WIDTH;
    int height = HEIGHT;
    SharedVariable *sv = (SharedVariable *)var;
    while (true)
    {
        if (!ready(sv->ind_thresh, sv->ind_supp))
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            printf("Running threshold thread %d\n", sv->ind_thresh);
            float max_in = maximums[(sv->ind_thresh + 1) % NUM_IMAGES];
            if (PARALLEL)
            {
                ParallelThreshold
                    parallelThreshold(sv->images[(sv->ind_thresh + 1) % NUM_IMAGES],
                                      sv->images[sv->ind_thresh], height, width,
                                      LOW_THRESHOLD, HIGH_THRESHOLD, max_in);
                parallel_for_(Range(0, width * height), parallelThreshold);
            }
            else
            {
                threshold(sv->images[(sv->ind_thresh + 1) % NUM_IMAGES],
                          sv->images[sv->ind_thresh], height, width, LOW_THRESHOLD,
                          HIGH_THRESHOLD, max_in);
            }

            sv->ind_thresh = (sv->ind_thresh + 1) % NUM_IMAGES;
        }
    }
    pthread_exit(NULL);
}
void *run_hysteresis_thread(void *var)
{
    int width = WIDTH;
    int height = HEIGHT;
    SharedVariable *sv = (SharedVariable *)var;
    while (true)
    {
        if (!ready(sv->ind_hyster, sv->ind_thresh))
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            printf("Running Hysteresis thread %d\n", sv->ind_hyster);
            if (PARALLEL)
            {
                ParallelHysteresis
                    parallelHysteresis(sv->images[(sv->ind_hyster + 1) % NUM_IMAGES],
                                       sv->images[sv->ind_hyster], height, width);
                parallel_for_(Range(0, width * height), parallelHysteresis);
            }
            else
            {
                hysteresis(sv->images[(sv->ind_hyster + 1) % NUM_IMAGES],
                           sv->images[sv->ind_hyster], height, width);
            }
            sv->ind_hyster = (sv->ind_hyster + 1) % NUM_IMAGES;
        }
    }
    pthread_exit(NULL);
}
void *run_resize_thread(void *var)
{
    int width = WIDTH;
    int height = HEIGHT;
    SharedVariable *sv = (SharedVariable *)var;

    while (true)
    {
        if (!ready(sv->ind_resize, sv->ind_hyster))
        {
            // printf("Resize thread check %d %d", sv->ind_resize, sv->ind_hyster);
            usleep(SLEEP_TIME);
        }
        else
        {
            printf("Running Resize thread: %d\n", sv->ind_resize);

            cv::Mat input = cv::Mat(height, width, CV_8UC1,
                                    sv->images[(sv->ind_resize + 1) % NUM_IMAGES]);
            cv::imshow("Result", input);
            if (cv::waitKey(25) >= 0)
            {
                int x = 0 + 0;
            }
            cv::Mat output;
            cv::resize(input, output, Size(DOWN_WIDTH, DOWN_HEIGHT),
                       INTER_LINEAR);

            sv->out_images[sv->ind_resize % NUM_OUTPUTS] = output;

            sv->ind_resize = (sv->ind_resize + 1) % NUM_IMAGES;
        }
    }
    pthread_exit(NULL);
}