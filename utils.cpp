#include "global.h"
#include "utils.hpp"

extern image_t *images[NUM_IMAGES];
unsigned float maximums[NUM_IMAGES];
extern int ind_p_in, ind_conv, ind_sob, ind_supp, ind_thresh,
    ind_hyster, ind_p_out;
extern int parallel, height, width;
#define GAUSSIAN_SIZE 5
#define SIGMA 1
#define LOW_THRESHOLD = 0.05
#define HIGH_THRESHOLD = 0.09

void *run_convolve_thread(void *var)
{
    float *filter = gaussian_kernel(GAUSSIAN_SIZE, SIGMA);
    while (true)
    {
        if (!ready(ind_conv, ind_p_in))
        {
            sleep(10);
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
}

void *run_sobel_thread(void *var)
{
    while (true)
    {
        if (!ready(ind_sob, ind_conv))
        {
            sleep(10);
        }
        else
        {
            float max = 0;
            if (parallel)
            {
                ParallelSobel parallelSobel(images[(ind_sob + 1) %
                                                   NUM_IMAGES],
                                            images[ind_sob], height,
                                            width, &max);
                parallel_for_(Range(0, width * height), parallelSobel);
            }
            else
            {
                sobel_filter(images[(ind_sob + 1) % NUM_IMAGES],
                             images[ind_sob], , &max, height, width);
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
            sleep(10);
        }
        else
        {
            float max_out;
            float max_in = maximums[ind_supp];
            if (parallel)
            {
                ParallelSuppression parallelSuppression(images[(ind_supp + 1) %
                                                               NUM_IMAGES],
                                                        images[ind_supp], , max_in,
                                                        &max_out, height, width);
                parallel_for_(Range(0, width * height), parallelSuppression);
            }
            else
            {
                max = non_max_suppression(images[(ind_supp + 1) % NUM_IMAGES],
                                          images[ind_supp], , max_in,
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
            sleep(10);
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
            sleep(10);
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