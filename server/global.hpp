#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <unistd.h>
#include <time.h>
#include <iostream>
#include <math.h>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>

#include "image_filter_parallel.hpp"
#define PORT 8080

/** values for edge detection algorithm */
#define LOW_THRESHOLD 0.05
#define HIGH_THRESHOLD 0.09
#define GAUSSIAN_SIZE 5
#define SIGMA 1
#define NUM_THETA 5
#define NUM_IMAGES 10
#define NUM_OUTPUTS 5
typedef unsigned char image_t;

struct SharedVariable_t
{
    int ind_cam, ind_gray, ind_conv, ind_sob, ind_supp, ind_thresh,
        ind_hyster, ind_resize, ind_p_out;
    image_t *images[NUM_IMAGES];
    float *theta[NUM_IMAGES];
    cv::Mat out_images[NUM_OUTPUTS];
};

typedef struct SharedVariable_t SharedVariable;
/**
extern int ind_p_in, ind_conv, ind_sob, ind_supp, ind_thresh, ind_hyster,
    ind_resize, ind_p_out;
extern int parallel;

extern int height, width;*/

static int parallel;

#define SLEEP_TIME 100

#define DOWN_WIDTH 128
#define DOWN_HEIGHT 96
#define WIDTH 640
#define HEIGHT 480
#define SIZE_OUT DOWN_WIDTH *DOWN_WIDTH
#define MAXBYTES 1024
#define SA struct sockaddr

int ready(int, int);

#endif