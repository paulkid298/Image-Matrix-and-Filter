#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <unistd.h>

#include "image_filter_parallel.cpp"
#include "image_filter_sequential.hpp"
#include "global.hpp"

extern cv::Mat images[NUM_IMAGES];
extern image_t *gray_images[NUM_IMAGES];

extern int ind_gray, ind_client, ind_cam;
extern int height, width;

void *run_camera_thread(void *var)
{
    cv::VideoCapture video(0);
    cv::Mat frame;
    while (true)
    {
        if (ind_cam == ind_client)
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            video.read(frame);
            images[ind_cam] = frame;
            ind_cam = (ind_cam + 1) % NUM_IMAGES;
        }
    }
    pthread_exit(NULL);
}

void *run_gray_thread(void *var)
{
    while (true)
    {
        if (!ready(ind_gray, ind_cam))
        {
            usleep(SLEEP_TIME);
        }
        else
        {
            if (PARALLEL)
            {
                ParallelGrayscale parallelGrayscale(images[ind_gray + 1],
                                                    gray_images[ind_gray], height, width);
            }
            else
            {
                grayscale(images[(ind_gray + 1) % NUM_IMAGES], gray_images[ind_gray], height, width);
            }
            ind_gray = (ind_gray + 1) % NUM_IMAGES;
        }
    }
    pthread_exit(NULL);
}