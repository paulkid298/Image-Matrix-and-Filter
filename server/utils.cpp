#include "global.hpp"
#include "utils.hpp"
#include <unistd.h>

#include "image_filter_sequential.hpp"
#include "image_filter_parallel.cpp"

using namespace cv;

extern image_t *images[NUM_IMAGES];
float maximums[NUM_IMAGES];
extern float *theta[NUM_IMAGES];
extern cv::Mat out_images[3];

extern int ind_p_in, ind_conv, ind_sob, ind_supp, ind_thresh,
    ind_hyster, ind_resize, ind_p_out;
extern int parallel, height, width;

void *run_convolve_thread(void *var)
{
    float *filter = gaussian_kernel(GAUSSIAN_SIZE, SIGMA);
    fprintf(stderr, "starting thread\n");
    while (true)
    {
        if (!ready(ind_conv, ind_p_in))
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            if (parallel)
            {
                ParallelConvolve parallelConvolve(images[(ind_conv + 1) %
                                                         NUM_IMAGES],
                                                  images[ind_conv], height,
                                                  width, filter, GAUSSIAN_SIZE);
                parallel_for_(Range(0, width * height), parallelConvolve);
            }
            else
            {
                convolve(images[(ind_conv + 1) % NUM_IMAGES], images[ind_conv],
                         height, width, filter, GAUSSIAN_SIZE);
            }
            ind_conv = (ind_conv + 1) % NUM_IMAGES;
        }
    }
    fprintf(stderr, "exiting thread\n");
}

void *run_sobel_thread(void *var)
{
    while (true)
    {
        if (!ready(ind_sob, ind_conv))
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            float max = 0;
            if (parallel)
            {
                ParallelSobel parallelSobel(images[(ind_sob + 1) %
                                                   NUM_IMAGES],
                                            images[ind_sob],
                                            theta[(ind_sob) % NUM_THETA],
                                            height,
                                            width, &max);
                parallel_for_(Range(0, width * height), parallelSobel);
            }
            else
            {
                sobel_filter(images[(ind_sob + 1) % NUM_IMAGES],
                             images[ind_sob], theta[(ind_sob) % NUM_THETA],
                             &max, height, width);
            }
            maximums[ind_sob] = max;
            ind_sob = (ind_sob + 1) % NUM_IMAGES;
        }
    }
}

void *run_suppression_thread(void *var)
{
    while (true)
    {
        if (!ready(ind_supp, ind_sob))
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            float max_out;
            float max_in = maximums[ind_supp];
            if (parallel)
            {
                ParallelSuppression parallelSuppression(images[(ind_supp + 1) %
                                                               NUM_IMAGES],
                                                        images[ind_supp],
                                                        theta[(ind_supp + 1) % NUM_THETA],
                                                        height, width,
                                                        max_in, &max_out);
                parallel_for_(Range(0, width * height), parallelSuppression);
            }
            else
            {
                non_max_suppression(images[(ind_supp + 1) % NUM_IMAGES],
                                    images[ind_supp], theta[(ind_supp + 1) % NUM_THETA], max_in,
                                    &max_out, height, width);
            }
            maximums[ind_supp] = max_out;
            ind_supp = (ind_supp + 1) % NUM_IMAGES;
        }
    }
}
void *run_threshold_thread(void *var)
{
    while (true)
    {
        if (!ready(ind_thresh, ind_supp))
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            float max_in = maximums[ind_thresh];
            if (parallel)
            {
                ParallelThreshold
                    parallelThreshold(images[(ind_thresh + 1) % NUM_IMAGES],
                                      images[ind_thresh], height, width,
                                      LOW_THRESHOLD, HIGH_THRESHOLD, max_in);
                parallel_for_(Range(0, width * height), parallelThreshold);
            }
            else
            {
                threshold(images[(ind_thresh + 1) % NUM_IMAGES],
                          images[ind_thresh], height, width, LOW_THRESHOLD,
                          HIGH_THRESHOLD, max_in);
            }
            ind_thresh = (ind_thresh + 1) % NUM_IMAGES;
        }
    }
}
void *run_hysteresis_thread(void *var)
{
    while (true)
    {
        if (!ready(ind_hyster, ind_thresh))
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            if (parallel)
            {
                ParallelHysteresis
                    parallelHysteresis(images[(ind_hyster + 1) % NUM_IMAGES],
                                       images[ind_hyster], height, width);
                parallel_for_(Range(0, width * height), parallelHysteresis);
            }
            else
            {
                hysteresis(images[(ind_hyster + 1) % NUM_IMAGES],
                           images[ind_hyster], height, width);
            }
            ind_hyster = (ind_hyster + 1) % NUM_IMAGES;
        }
    }
}
void *run_resize_thread(void *var)
{
    while (true)
    {
        if (!ready(ind_resize, ind_hyster))
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            cv::Mat input(height, width, CV_8UC1,
                          images[(ind_resize + 1) % NUM_IMAGES]);
            cv::Mat output;
            cv::resize(input, output, Size(DOWN_WIDTH, DOWN_HEIGHT),
                       INTER_LINEAR);
            out_images[ind_resize % NUM_OUTPUTS] = output;

            ind_resize = (ind_resize + 1) % NUM_IMAGES;
        }
    }
}